#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "node.hpp"

static llvm::LLVMContext context;
static std::unique_ptr<llvm::Module> module;

extern FILE* yyin;
extern int yyparse();
extern Line* line;

std::string usage() {
    return "usage: linec [input] [output]";
}

int main(int argc, char* argv[]) {
    
    if(argc != 3) {
        std::cerr << usage() << std::endl;
        return 1;
    }

    auto input = std::string(argv[1]);
    auto output = std::string(argv[2]);

    FILE* fp = fopen(input.c_str(), "r");
    if(!fp) {
        std::cerr << "could not open " << input << std::endl;
        return 1;
    }
    yyin = fp;
    if(auto parse_code = yyparse()) {
        std::cerr << "failed to parse " << input << std::endl;
        return parse_code;
    }

    module = llvm::make_unique<llvm::Module>(input, context);
    auto main = build_main(module, line->codegen(module));
    if(!main) {
        std::cerr << "failed to compile " << input << " to " << output << std::endl;
        return 1;
    }

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(target_triple);

    std::string err;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, err);

    if(!target) {
        std::cerr << err << std::endl;
        return 1;
    }

    auto cpu = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto rm = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::Model::PIC_);
    auto target_machine = target->createTargetMachine(target_triple, cpu, features, opt, rm);

    module->setDataLayout(target_machine->createDataLayout());

    auto obj = "output.o";
    std::error_code ec;
    llvm::raw_fd_ostream dest(obj, ec, llvm::sys::fs::F_None);

    if(ec) {
        std::cerr << "could not open file: " << ec.message() << std::endl;
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto filetype = llvm::TargetMachine::CGFT_ObjectFile;

    if(target_machine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
        std::cerr << "target_machine can't emit a file of this type" << std::endl;
        return 1;
    }

    pass.run(*module);
    dest.flush();

    std::ostringstream oss;
    oss << "g++ " << obj << " -o " << output << " && rm " << obj;
    auto ret_code = system(oss.str().c_str());
    return ret_code;
}
