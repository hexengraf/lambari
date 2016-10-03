#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <unordered_map>

enum class Type {
    INT, FLOAT, BOOL, VOID, ANY
};

enum class Operator {
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    LESS_THAN,
    GREATER_EQUAL_THAN,
    LESS_EQUAL_THAN,
    AND,
    OR,
    NOT,
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    UNARY_MINUS,
    ASSIGN,
    PAR,
    CAST,
    TEST,
};

enum class Error {
    MULTIPLE_DEFINITION,
    UNDECLARED_VARIABLE,
    INCOMPATIBLE_OPERANDS,
    INCOMPATIBLE_ASSIGNMENT,
    INCOMPATIBLE_TEST,
    DECLARED_BUT_NEVER_DEFINED,
    MULTIPLE_DEFINITION_FN,
    WRONG_PARAM_COUNT,
    INCOMPATIBLE_PARAM,
    INCOMPATIBLE_INDEX,
    NON_ARRAY_INDEX,
};

namespace std {
    template<>
    struct hash<Type> {
        inline size_t operator()(Type type) const {
            return static_cast<int>(type);
        }
    };

    template<>
    struct hash<Operator> {
        inline size_t operator()(Operator op) const {
            return static_cast<int>(op);
        }
    };
}

namespace utils {
    const std::unordered_map<Type, std::string> type_table = {
        {Type::INT, "int"},
        {Type::FLOAT, "float"},
        {Type::BOOL, "bool"},
    };

    const std::unordered_map<Operator, std::string> operator_table = {
        {Operator::EQUAL, "=="},
        {Operator::NOT_EQUAL, "!="},
        {Operator::GREATER_THAN, ">"},
        {Operator::LESS_THAN, "<"},
        {Operator::GREATER_EQUAL_THAN, ">="},
        {Operator::LESS_EQUAL_THAN, "<="},
        {Operator::AND, "&"},
        {Operator::OR, "|"},
        {Operator::NOT, "!"},
        {Operator::PLUS, "+"},
        {Operator::MINUS, "-"},
        {Operator::TIMES, "*"},
        {Operator::DIVIDE, "/"},
    };

    const std::unordered_map<Type, std::string> printable_type_table = {
        {Type::INT, "integer"},
        {Type::FLOAT, "float"},
        {Type::BOOL, "boolean"},
    };

    const std::unordered_map<Operator, std::string> printable_operator_table = {
        {Operator::EQUAL, "equal"},
        {Operator::NOT_EQUAL, "different"},
        {Operator::GREATER_THAN, "greater than"},
        {Operator::LESS_THAN, "less than"},
        {Operator::GREATER_EQUAL_THAN, "greater or equal than"},
        {Operator::LESS_EQUAL_THAN, "less or equal than"},
        {Operator::AND, "and"},
        {Operator::OR, "or"},
        {Operator::NOT, "negation"},
        {Operator::PLUS, "addition"},
        {Operator::MINUS, "subtraction"},
        {Operator::TIMES, "multiplication"},
        {Operator::DIVIDE, "division"},
        {Operator::UNARY_MINUS, "unary minus"},
        {Operator::ASSIGN, "attribution"},
        {Operator::TEST, "test"},
    };

    struct literal {
        char* value;
        Type type;
    };

    class line_counter {
     public:
        static line_counter& instance() {
            static line_counter inst;
            return inst;
        }

        size_t value() const { return val; }
        line_counter& operator++() {
            ++val;
            return *this;
        }

     private:
        size_t val = 1;
        line_counter() = default;
    };

    inline line_counter& counter() {
        return line_counter::instance();
    }

    inline std::string to_string(Type type) {
        return type_table.at(type);
    }

    inline std::string to_string(Operator op) {
        return operator_table.at(op);
    }

    inline std::string to_printable_string(Type type) {
        return printable_type_table.at(type);
    }

    inline std::string to_printable_string(Operator op) {
        return printable_operator_table.at(op);
    }

    inline std::string to_string(const char* value) {
        return value;
    }

    inline bool can_coerce(Type target, Type source) {
        return target == Type::FLOAT && source == Type::INT;
    }

    inline bool type_matches(Type target, Type source) {
        return target == source;
    }

    template<typename T>
    inline void echo(const T& value) {
        std::cerr << value << std::endl;
    }

    inline std::string error_prefix(const std::string& type) {
        auto line = std::to_string(counter().value());
        return "[Line " + line + "] " + type + " error: ";
    }

    template<Error err>
    inline void semantic_error(const std::string&);
    template<Error err>
    inline void semantic_error(Operator, Type, Type);
    template<Error err>
    inline void semantic_error(Type, Type);
    template<Error err>
    inline void semantic_error(Type);
    template<Error err>
    inline void semantic_error(const std::string&, size_t, size_t);
    template<Error err>
    inline void semantic_error(const std::string&, Type, Type);
    template<Error err>
    inline void semantic_error();

    template<>
    inline void semantic_error<Error::MULTIPLE_DEFINITION>(const std::string& name) {
        echo(error_prefix("semantic") + "re-declaration of variable " + name);
    }

    template<>
    inline void semantic_error<Error::MULTIPLE_DEFINITION_FN>(const std::string& name) {
        echo(error_prefix("semantic") + "re-declaration of function " + name);
    }

    template<>
    inline void semantic_error<Error::UNDECLARED_VARIABLE>(const std::string& name) {
        echo(error_prefix("semantic") + "undeclared variable " + name);
    }

    template<>
    inline void semantic_error<Error::INCOMPATIBLE_OPERANDS>(Operator op, Type expected, Type actual) {
        auto op_name = to_printable_string(op);
        auto expected_str = to_printable_string(expected);
        auto actual_str = to_printable_string(actual);
        echo(error_prefix("semantic") + op_name + " operation expected "
            + expected_str + " but received " + actual_str);
    }

    template<>
    inline void semantic_error<Error::INCOMPATIBLE_ASSIGNMENT>(Type expected, Type actual) {
        semantic_error<Error::INCOMPATIBLE_OPERANDS>(Operator::ASSIGN, expected, actual);
    }

    template<>
    inline void semantic_error<Error::INCOMPATIBLE_TEST>(Type received) {
        semantic_error<Error::INCOMPATIBLE_OPERANDS>(Operator::TEST, Type::BOOL, received);
    }

    template<>
    inline void semantic_error<Error::DECLARED_BUT_NEVER_DEFINED>(const std::string& name) {
        echo(error_prefix("semantic") + "function " + name + " is declared but never defined");
    }

    template<>
    inline void semantic_error<Error::WRONG_PARAM_COUNT>(const std::string& name,
                                                         size_t expected,
                                                         size_t actual) {
        echo(error_prefix("semantic") + "function " + name + " expects "
            + std::to_string(expected) + " parameters but received " + std::to_string(actual));
    }

    template<>
    inline void semantic_error<Error::INCOMPATIBLE_PARAM>(const std::string& name,
                                                          Type expected,
                                                          Type actual) {
        auto expected_str = to_printable_string(expected);
        auto actual_str = to_printable_string(actual);
        echo(error_prefix("semantic") + "parameter " + name + " expected "
            + expected_str + " but received " + actual_str);
    }

    template<>
    inline void semantic_error<Error::INCOMPATIBLE_INDEX>(Type expected,
                                                          Type actual) {
        auto expected_str = to_printable_string(expected);
        auto actual_str = to_printable_string(actual);
        echo(error_prefix("semantic") + "index operator expects "
            + expected_str + " but received " + actual_str);
    }

    template<>
    inline void semantic_error<Error::NON_ARRAY_INDEX>() {
        echo(error_prefix("semantic") + "index operator expects an array");
    }
}

#endif /* UTILS_HPP */
