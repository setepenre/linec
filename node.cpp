#include <iostream>
#include <vector>

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "node.hpp"

static std::map<std::string, llvm::Value*> Defined;

llvm::Value* Number::codegen(const Environment& env) {
    return llvm::ConstantFP::get(env.module->getContext(), llvm::APFloat(value));
}

llvm::Value* String::codegen(const Environment& env) {
    llvm::IRBuilder<> builder(env.module->getContext());
    return builder.CreateGlobalStringPtr(value);
}

llvm::Value* Plus::codegen(const Environment& env) {
    auto l = left->codegen(env);
    auto r = right->codegen(env);
    if(!l || !r) {
        return nullptr;
    }

    return env.builder->CreateFAdd(l, r, "addtmp");
}

llvm::Value* Assignment::codegen(const Environment& env) {
    auto count = Defined.count(name);
    if(count) {
        std::cerr << "\e[1m" << lineno << "\e[0m: " << env.lines.at(lineno - 1) << std::endl;
        std::cerr << "  identifier \e[1m" << name << "\e[0m is already defined here" << std::endl;
        return nullptr;
    }

    Defined[name] = expr->codegen(env);
    return Defined[name];
}

llvm::Value* Ident::codegen(const Environment& env) {
    auto count = Defined.count(name);
    if(!count) {
        std::cerr << "\e[1m" << lineno << "\e[0m: " << env.lines.at(lineno - 1) << std::endl;
        std::cerr << "  identifier \e[1m" << name << "\e[0m is unassigned" << std::endl;
        return nullptr;
    }
    return Defined[name];
}

llvm::Value* Block::codegen(const Environment& env) {
    auto last = std::move(exprs.back());
    exprs.pop_back();
    for(auto const& expr : exprs) {
        auto f = expr->codegen(env);
        if(!f) {
            return nullptr;
        }
    }
    return last->codegen(env);
}

llvm::Function* get_printf(const Environment& env) {
    llvm::Function* f = env.module->getFunction("printf");
    if (!f) {
        std::vector<llvm::Type*> types = {
            llvm::PointerType::get(llvm::Type::getInt8Ty(env.module->getContext()), 0)
        };
        llvm::FunctionType* ft = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(env.module->getContext()), types, true
        );

        f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, "printf", env.module.get());
        f->setCallingConv(llvm::CallingConv::C);
    }

    return f;
}

llvm::Function* build_main(const Environment& env, Expr* display) {
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(env.module->getContext()), false);
    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", env.module.get());

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(env.module->getContext(), "entry", f);
    llvm::IRBuilder<> builder(env.module->getContext());
    builder.SetInsertPoint(bb);

    if(auto value = display->codegen(env)) {
        auto printf = get_printf(env);
        llvm::Value* fmt = builder.CreateGlobalStringPtr("%f", "fmt", 0);
        std::vector<llvm::Value*> args = { fmt, value };
        builder.CreateCall(printf, args, "printfCall");

        auto zero = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(env.module->getContext()));
        builder.CreateRet(zero);
        llvm::verifyFunction(*f);
        return f;
    }
    return nullptr;
}
