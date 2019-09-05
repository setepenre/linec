#ifndef __NODE_HPP
#define __NODE_HPP

#include <memory>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

class Expr {
public:
    virtual ~Expr() = default;
    virtual llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) = 0;
};

class Number: public Expr {
    int lineno;
    double value;

public:
    Number(int lineno, double value): lineno(lineno), value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class String: public Expr {
    int lineno;
    std::string value;

public:
    String(int lineno, const std::string &value) : lineno(lineno), value(value) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Ident: public Expr {
    int lineno;
    std::string name;

public:
    Ident(int lineno, const std::string &name) : lineno(lineno), name(name) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Plus: public Expr {
    int lineno;
    std::unique_ptr<Expr> left, right;

public:
    Plus(int lineno, Expr* left, Expr* right) : lineno(lineno), left(std::move(left)), right(std::move(right)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Assignment: public Expr {
    int lineno;
    std::string name;
    std::unique_ptr<Expr> expr;

public:
    Assignment(int lineno, std::string name, Expr* expr) : lineno(lineno), name(name), expr(std::move(expr)) {}
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

class Block: public Expr {
    int lineno;
    std::vector<std::unique_ptr<Expr>> exprs;

public:
    Block(int lineno, Expr* head) : lineno(lineno) { exprs.push_back(std::unique_ptr<Expr>(std::move(head))); }
    void push_back(Expr* expr) { exprs.push_back(std::unique_ptr<Expr>(std::move(expr))); }
    llvm::Value* codegen(std::unique_ptr<llvm::Module> const &module) override;
};

llvm::Function* get_printf(std::unique_ptr<llvm::Module> const &module);
llvm::Function* build_main(std::unique_ptr<llvm::Module> const &module, llvm::Value* display);
#endif
