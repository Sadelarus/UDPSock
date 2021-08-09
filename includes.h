#pragma once
#include <type_traits>
#include <string>

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class Operation: std::uint8_t {
    Undefined,
    Plus = '+',
    Minus = '-',
    Mult = '*',
    Div = '/'
};

inline Operation operation_from_string(const std::string &str){
    if(str == "+") {
        return Operation::Plus;
    }else if(str == "-") {
         return Operation::Minus;
    }else if(str == "*") {
         return Operation::Mult;
    }else if(str == "/") {
        return Operation::Div;
    }
    return Operation::Undefined;
}

struct CalcData {
    double a;
    double b;
    Operation operation;
};

struct Result {
  double res;
};

union unData{
    struct CalcData args;
    struct Result res;
};
