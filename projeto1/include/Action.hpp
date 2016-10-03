#ifndef ACTION_HPP
#define ACTION_HPP

#include <list>
#include <vector>
#include "SymbolMap.hpp"
#include "macros.hpp"

namespace {
    auto& symbols = SymbolMap::instance();
}

class Action {
 public:
    virtual ~Action() {}
    virtual std::string to_string(unsigned = 0) const = 0;
    virtual bool error() const { return false; }
    virtual Type type() const = 0;
};

class Nop : public Action {
 public:
    std::string to_string(unsigned = 0) const override {
        return "";
    }
    Type type() const override { return PrimitiveType::VOID; }
};

class Declaration : public Action {
 public:
    Declaration(Type, const std::string& = "var");
    ~Declaration() {
        while (!declarations.empty()) {
            delete declarations.back();
            declarations.pop_back();
        }
    }
    void add(const std::string&);
    void add(const std::string&, Action*);
    void add(const std::string&, const utils::literal&);
    void set_symbol_type(const std::string& id) { symbol_type = id; }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }

 private:
    Type t;
    std::string symbol_type;
    std::list<Action*> declarations;
};

class VarDecl : public Action {
 public:
    VarDecl(Type, const std::string&, Action* = nullptr);
    ~VarDecl() {
        delete value;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }

 private:
    Type t;
    std::string name;
    Action* value;
};

class ArrayDecl : public Action {
 public:
    ArrayDecl(Type, const std::string&, const std::string&);
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }

 private:
    Type t;
    std::string name;
    std::string size;
};


class Variable : public Action {
 public:
    Variable(const std::string&);
    bool error() const override;
    Type type() const override;
    std::string to_string(unsigned = 0) const override;

 private:
    bool fail;
    Type t;
    std::string name;
};


class Constant : public Action {
 public:
    Constant(Type type, const std::string& value);
    bool error() const override;
    Type type() const override;
    std::string to_string(unsigned = 0) const override;

 private:
    Type t;
    std::string value;
};


class Operation : public Action {
 public:
    template<typename... Args>
    Operation(Operator op, Action* first, Args&&... args) : op(op) {
        t = first->type();
        set_children(first, std::forward<Args>(args)...);
    }

    template<typename... Args>
    Operation(Operator op, Type type, Args&&... args)
     : op(op), t(type) {
        set_children(std::forward<Args>(args)...);
    }

    ~Operation() {
        while (!children.empty()) {
            delete children.back();
            children.pop_back();
        }
    }

    bool error() const override;
    Type type() const override;
    std::string to_string(unsigned = 0) const override;
    virtual std::string op_string() const;
    void set_type(Type);

 private:
    Operator op;
    Type t;
    bool fail = false;
    bool needs_coercion = false;
    std::list<Action*> children;

    void check(Action*);
    void set_children();

    template<typename... Args>
    void set_children(Action* action, Args&&... args) {
        fail = fail || action->error();
        if (!fail) {
            check(action);
        }
        children.push_back(action);
        set_children(std::forward<Args>(args)...);
    }
};


class Comparison : public Operation {
 public:
    template<typename... Args>
    Comparison(Operator op, Args&&... args)
     : Operation(op, std::forward<Args>(args)...) {
        set_type(PrimitiveType::BOOL);
    }
};

class BoolOperation : public Operation {
 public:
    template<typename... Args>
    BoolOperation(Operator op, Args&&... args)
     : Operation(op, PrimitiveType::BOOL, std::forward<Args>(args)...) {}
};

class Parenthesis : public Operation {
 public:
    Parenthesis(Action* operand) : Operation(Operator::PAR, operand) {}

    std::string op_string() const override {
        return "";
    }
};

class UnaryMinus : public Operation {
 public:
    UnaryMinus(Action* operand) : Operation(Operator::UNARY_MINUS, operand) {}

    std::string op_string() const override {
        return "-u";
    }
};

class Cast : public Operation {
 public:
    Cast(Type type, Action* operand) : Operation(Operator::CAST, operand) {
        set_type(type);
    }

    std::string op_string() const override {
        return "[" + utils::to_string(type()) + "]";
    }
};


class Assignment : public Action {
 public:
    Assignment(Action*, Action*);
    ~Assignment() {
        delete var;
        delete rhs;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override;

 private:
    Action* var;
    Action* rhs;
    bool fail;
};

class Block : public Action {
 public:
    void add(Action*);
    ~Block() {
        while (!lines.empty()) {
            delete lines.back();
            lines.pop_back();
        }
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return PrimitiveType::VOID; }
 private:
    std::list<Action*> lines;
};

class Conditional : public Action {
 public:
    Conditional(Action*, Action*, Action* = nullptr);
    ~Conditional() {
        delete condition;
        delete accepted;
        delete rejected;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return PrimitiveType::VOID; }
    bool error() const override { return fail; };
 private:
    Action* condition;
    Action* accepted;
    Action* rejected;
    bool fail;
};

class Loop : public Action {
 public:
    Loop(Action*, Action*, Action*, Action*);
    ~Loop() {
        delete init;
        delete test;
        delete update;
        delete code;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return PrimitiveType::VOID; }
    bool error() const override { return fail; }
 private:
    Action* init;
    Action* test;
    Action* update;
    Action* code;
    bool fail = false;
};

class ParamList : public Action {
 public:
    void add(Type, const std::string&);
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return PrimitiveType::VOID; }

    auto begin() const { return vars.cbegin(); }
    auto begin() { return vars.begin(); }
    auto end() const { return vars.cend(); }
    auto end() { return vars.end(); }
 private:
    std::list<std::pair<Type, std::string>> vars;
};

// Function
class Fun : public Action {
 public:
    Fun(Type, const std::string&);
    ~Fun() {
        delete params;
        delete body;
    }
    void inject(Action*);
    void bind(Action*, Action* = nullptr);
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return ret; }
    bool error() const override { return fail; }
 private:
    std::string name;
    Action* params;
    Action* body;
    Type ret;
    bool fail = false;
};

class ExpressionList : public Action {
 public:
    ~ExpressionList() {
        while (!expressions.empty()) {
            delete expressions.back();
            expressions.pop_back();
        }
    }
    void add(Action*);
    size_t size() const { return expressions.size(); }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return PrimitiveType::VOID; }
    bool error() const override { return fail; }

    auto begin() const { return expressions.cbegin(); }
    auto begin() { return expressions.begin(); }
    auto end() const { return expressions.cend(); }
    auto end() { return expressions.end(); }

 private:
    std::list<Action*> expressions;
    bool fail;
};

class FunCall : public Action {
 public:
    FunCall(const std::string&, Action*);
    ~FunCall() {
        delete args;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }
    bool error() const override { return fail; }

 private:
    std::string name;
    ExpressionList* args;
    bool fail;
    Type t;
};

class Return : public Action {
 public:
    Return(Action*);
    ~Return() {
        delete operand;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override;
    bool error() const override { return fail; }

 private:
    Action* operand;
    bool fail;
};

class ArrayIndex : public Action {
 public:
    ArrayIndex(const std::string&, Action*);
    ~ArrayIndex() {
        delete index;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }
    bool error() const override { return fail; }
 private:
    Type t;
    std::string name;
    Action* index;
    bool fail;
};

class Address : public Action {
 public:
    Address(Action*);
    ~Address() {
        delete lvalue;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }
    bool error() const override { return fail; }
 private:
    Type t;
    Action* lvalue;
    bool fail;
};

class Reference : public Action {
 public:
    Reference(Action*);
    ~Reference() {
        delete lvalue;
    }
    std::string to_string(unsigned = 0) const override;
    Type type() const override { return t; }
    bool error() const override { return fail; }
 private:
    Type t;
    Action* lvalue;
    bool fail;
};

#endif /* ACTION_HPP */
