#ifndef __NODE_HPP
#define __NODE_HPP

#include <memory>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

class Node {
public:
    virtual ~Node() = default;
    virtual llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) = 0;
};

class Expr: public Node {
};

class Number: public Expr {
    double value;

public:
    Number(double value): value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class String: public Expr {
    std::string value;

public:
    String(const std::string &value): value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Plus: public Expr {
    std::unique_ptr<Expr> left, right;

public:
    Plus(Expr* left, Expr* right) : left(std::move(left)), right(std::move(right)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Line {
    std::unique_ptr<Expr> expr;

public:
    Line(Expr* expr) : expr(std::move(expr)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module);
};

llvm::Function* get_printf(std::unique_ptr<llvm::Module> const &module);
llvm::Function* build_main(std::unique_ptr<llvm::Module> const &module, llvm::Value* display);
#endif
