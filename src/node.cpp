#include <iostream>
#include <vector>

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "node.hpp"

static std::map<std::string, std::unique_ptr<Expr>> Defined;

llvm::Value* Number::codegen(const Environment& env) {
    return llvm::ConstantFP::get(env.module->getContext(), llvm::APFloat(value));
}

llvm::Value* String::codegen(const Environment& env) {
    return env.builder->CreateGlobalStringPtr(value);
}

llvm::Value* Assignment::codegen(const Environment& env) {
    auto count = Defined.count(name);
    if(count) {
        std::cerr << "\e[1m" << lineno << "\e[0m: " << env.lines.at(lineno - 1) << std::endl;
        std::cerr << "  identifier \e[1m" << name << "\e[0m is already defined here" << std::endl;
        return nullptr;
    }

    Defined[name] = std::move(expr);
    return Defined[name]->codegen(env);
}

llvm::Value* Ident::codegen(const Environment& env) {
    auto count = Defined.count(name);
    if(!count) {
        std::cerr << "\e[1m" << lineno << "\e[0m: " << env.lines.at(lineno - 1) << std::endl;
        std::cerr << "  identifier \e[1m" << name << "\e[0m is unassigned" << std::endl;
        return nullptr;
    }
    type = Defined[name]->type;
    return Defined[name]->codegen(env);
}

llvm::Value* Block::codegen(const Environment& env) {
    llvm::Value* f;
    for(auto const& expr : exprs) {
        f = expr->codegen(env);
        if(!f) {
            return nullptr;
        }
        type = expr->type;
    }
    return f;
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
    env.builder->SetInsertPoint(bb);

    if(auto value = display->codegen(env)) {
        auto printf = get_printf(env);
        auto fmts = display->type == NUMBER ? "%f" : "%s";
        llvm::Value* fmt = env.builder->CreateGlobalStringPtr(fmts, "fmt", 0);
        std::vector<llvm::Value*> args = { fmt, value };
        env.builder->CreateCall(printf, args, "printfCall");

        auto zero = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(env.module->getContext()));
        env.builder->CreateRet(zero);
        llvm::verifyFunction(*f);
        return f;
    }
    return nullptr;
}
