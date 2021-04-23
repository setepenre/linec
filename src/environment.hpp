#ifndef __PROGRAM_HPP
#define __PROGRAM_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

struct Environment {
    std::vector<std::string> lines;

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    Environment(std::string filename, std::vector<std::string> lines): lines(lines) {
        module = std::make_unique<llvm::Module>(filename, context);
        builder = std::make_unique<llvm::IRBuilder<>>(module->getContext());
    }
};

std::pair<std::vector<std::string>, bool> readlines(std::string filename);

#endif
