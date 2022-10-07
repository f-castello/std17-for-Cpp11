#include "any.hpp"
#include <iostream>
#define __ENDL ;std::cout << std::endl;

int main() {
    std17::any val;
    val.emplace<int>(7);
    std::cout << "Integer: " << std17::any_cast<int>(val) __ENDL
    val.emplace<std::string>("test");
    std::cout << "Direct string: " << std17::any_cast<std::string>(val) __ENDL
    val.emplace<float>(3.14);
    std::cout << "Floating: " << std17::any_cast<float>(val) __ENDL
    std::string str("hello");
    val.emplace<std::string>(str);
    std::cout << "Variable string: " << std17::any_cast<std::string>(val) __ENDL
    val.emplace<float>(3.14);
    std::cout << "Floating point: " << std17::any_cast<float>(val) __ENDL
    bool b = true;
    val = b;
    std::cout << "Boolean: ";
    std17::any_cast<bool>(val) ? std::cout << "true" : std::cout << "false" __ENDL
    std17::any sec(val);
    std::cout << "Bool again: " << std17::any_cast<bool>(sec) __ENDL
    std17::any bar(std::move(val));
    std::cout << "Bool again: " << std17::any_cast<bool>(bar) __ENDL
    std::string a("world");
    bar = a;
    std::cout << "Assigned string: " << std17::any_cast<std::string>(bar) __ENDL
}

#undef __ENDL
