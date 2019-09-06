#ifndef __NODE_HPP
#define __NODE_HPP

#include <memory>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

#include "environment.hpp"

struct Expr {
    int lineno;

    Expr(int lineno): lineno(lineno) {}
    virtual ~Expr() = default;
    virtual llvm::Value* codegen(const Environment& env) = 0;
};

struct Number: Expr {
    double value;

    Number(int lineno, double value): Expr(lineno), value(value) {}
    llvm::Value* codegen(const Environment& env) override;
};

struct String: Expr {
    std::string value;

    String(int lineno, const std::string &value) : Expr(lineno), value(value) {}
    llvm::Value* codegen(const Environment& env) override;
};

struct Ident: Expr {
    std::string name;

    Ident(int lineno, const std::string &name) : Expr(lineno), name(name) {}
    llvm::Value* codegen(const Environment& env) override;
};

struct Plus: Expr {
    std::unique_ptr<Expr> left, right;

    Plus(int lineno, Expr* left, Expr* right) : Expr(lineno), left(std::move(left)), right(std::move(right)) {}
    llvm::Value* codegen(const Environment& env) override;
};

struct Assignment: Expr {
    std::string name;
    std::unique_ptr<Expr> expr;

    Assignment(int lineno, std::string name, Expr* expr) : Expr(lineno), name(name), expr(std::move(expr)) {}
    llvm::Value* codegen(const Environment& env) override;
};

struct Block: Expr {
    std::vector<std::unique_ptr<Expr>> exprs;

    Block(int lineno, Expr* head) : Expr(lineno) { exprs.push_back(std::unique_ptr<Expr>(std::move(head))); }
    void push_back(Expr* expr) { exprs.push_back(std::unique_ptr<Expr>(std::move(expr))); }
    llvm::Value* codegen(const Environment& env) override;
};

llvm::Function* get_printf(const Environment& env);
llvm::Function* build_main(const Environment& env, Expr* display);
#endif
