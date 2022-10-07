#include "variant.hpp"
#include <iostream>
#define __ENDL ;std::cout << std::endl;

int main() {
	typedef std17::variant<int, float, bool, std::string> typed_var;
	typed_var val;
	val.emplace<int>(7);
	std::cout << "Integer: " << val.get<int>() __ENDL
    val.emplace<std::string>("hello");
    std::cout << "Direct string: " << val.get<std::string>() __ENDL
	val.emplace<float>(3.14);
	std::cout << "Floating point: " << val.get<float>() __ENDL
	std::string str("test");
	val.emplace<std::string>(str);
    std::cout << "Variable string: " << val.get<std::string>() __ENDL
    bool b = true;
    val.emplace<bool>(b);
    std::cout << "Boolean: ";
    val.get<bool>() ? std::cout << "true" : std::cout << "false" __ENDL
    std17::variant<int, float, bool, std::string> sec(move(val));
    std::cout << "Bool again: " << sec.get<bool>() __ENDL
    std17::variant<int, float, bool, std::string> bar(sec);
    std::cout << "Bool again: " << bar.get<bool>() __ENDL
    std::string a("world");
    bar = a;
    std::cout << "Assigned string: " << typed_var::get<std::string>(bar) __ENDL
    return 0;
}

#undef __ENDL
