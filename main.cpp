#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
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
extern Block* entry;

std::string usage() {
    return 
"usage: linec [options] source executable\n"
"  linec compiles \e[1msource\e[0m file to \e[1mexecutable\e[0m file\n"
"\n"
"options:\n"
"  -h, --help  display this message\n"
"  --llvm      emits llvm IR to standard output";
}

std::vector<std::string> vectorize(int argc, char* argv[]) {
    std::vector<std::string> args = {};
    for(auto i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return args;
}

std::tuple<bool, bool, std::string, std::string> parse(int argc, char* argv[]) {
    auto contains = [argc, argv](std::string s) {
        for(auto i = 1; i < argc; ++i) {
            if(s == std::string(argv[i])) {
                return true;
            }
        }
        return false;
    };
    auto extract = [](std::string s, const std::vector<std::string>& a) -> std::tuple<bool, std::vector<std::string>> {
        auto copy = a;
        for(auto i = 0; i < copy.size(); ++i) {
            if(s == copy.at(i)) {
                copy.erase(copy.begin() + i);
                return { true, copy };
            }
        }
        return { false, a };
    };
    auto [help, tmp] = contains("-h") ? 
        extract("-h", vectorize(argc, argv)) : 
        contains("--help") ? 
          extract("--help", vectorize(argc, argv)) : 
          std::tuple<bool, std::vector<std::string>>({ false, vectorize(argc, argv) });

    auto [ir, args] = extract("--llvm", tmp);
    return { help, ir, args.at(0), args.at(1) };
}

int main(int argc, char* argv[]) {
    
    if(argc < 3 || argc > 5) {
        std::cerr << usage() << std::endl;
        return 1;
    }
    
    auto [help, ir, input, output] = parse(argc, argv);
    if(help) {
        std::cerr << usage() << std::endl;
        return 1;
    }

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
    auto main = build_main(module, program->codegen(module));
    if(!main) {
        return 1;
    }
    if(ir) {
        module->print(llvm::outs(), nullptr);
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
