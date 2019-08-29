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

llvm::Value* Number::codegen(std::unique_ptr<llvm::Module> const &module) {
    return llvm::ConstantFP::get(module->getContext(), llvm::APFloat(value));
}

llvm::Value* String::codegen(std::unique_ptr<llvm::Module> const &module) {
    llvm::IRBuilder<> builder(module->getContext());
    return builder.CreateGlobalStringPtr(value);
}

llvm::Value* Plus::codegen(std::unique_ptr<llvm::Module> const &module) {
    auto l = left->codegen(module);
    auto r = right->codegen(module);
    if(!l || !r) {
        return nullptr;
    }

    llvm::IRBuilder<> builder(module->getContext());
    return builder.CreateFAdd(l, r, "addtmp");
}

llvm::Function* get_printf(std::unique_ptr<llvm::Module> const &module) {
    llvm::Function* f = module->getFunction("printf");
    if (!f) {
        std::vector<llvm::Type*> types = {
            llvm::PointerType::get(llvm::Type::getInt8Ty(module->getContext()), 0)
        };
        llvm::FunctionType* ft = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(module->getContext()), types, true
        );

        f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage, "printf", module.get());
        f->setCallingConv(llvm::CallingConv::C);
    }

    return f;
}

llvm::Function* build_main(std::unique_ptr<llvm::Module> const &module, llvm::Value* display) {
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(module->getContext()), false);
    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", module.get());

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(module->getContext(), "entry", f);
    llvm::IRBuilder<> builder(module->getContext());
    builder.SetInsertPoint(bb);

    if(display) {
        auto printf = get_printf(module);
        llvm::Value* fmt = builder.CreateGlobalStringPtr("%f", "fmt", 0);
        std::vector<llvm::Value*> args = { fmt, display };
        builder.CreateCall(printf, args, "printfCall");
    }

    auto zero = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(module->getContext()));
    builder.CreateRet(zero);
    llvm::verifyFunction(*f);
    return f;
}

llvm::Value* Line::codegen(std::unique_ptr<llvm::Module> const &module) {
    return expr->codegen(module);
}
