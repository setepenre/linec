#ifndef __NODE_HPP
#define __NODE_HPP

#include <memory>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

struct Expr {
    int lineno;

    Expr(int lineno): lineno(lineno) {}
    virtual ~Expr() = default;
    virtual llvm::Value* codegen(std::unique_ptr<llvm::Module> const& module) = 0;
};

struct Number: Expr {
    double value;

    Number(int lineno, double value): Expr(lineno), value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

struct String: Expr {
    std::string value;

    String(int lineno, const std::string &value) : Expr(lineno), value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

struct Ident: Expr {
    std::string name;

    Ident(int lineno, const std::string &name) : Expr(lineno), name(name) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

struct Plus: Expr {
    std::unique_ptr<Expr> left, right;

    Plus(int lineno, Expr* left, Expr* right) : Expr(lineno), left(std::move(left)), right(std::move(right)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

struct Assignment: Expr {
    std::string name;
    std::unique_ptr<Expr> expr;

    Assignment(int lineno, std::string name, Expr* expr) : Expr(lineno), name(name), expr(std::move(expr)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

struct Block: Expr {
    std::vector<std::unique_ptr<Expr>> exprs;

    Block(int lineno, Expr* head) : Expr(lineno) { exprs.push_back(std::unique_ptr<Expr>(std::move(head))); }
    void push_back(Expr* expr) { exprs.push_back(std::unique_ptr<Expr>(std::move(expr))); }
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

llvm::Function* get_printf(std::unique_ptr<llvm::Module> const &module);
llvm::Function* build_main(std::unique_ptr<llvm::Module> const &module, llvm::Value* display);
#endif
