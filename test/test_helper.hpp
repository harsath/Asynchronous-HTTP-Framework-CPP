#pragma once

#include <cstddef>
#include <iostream>
#include <typeinfo>
#include <type_traits>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BOOL(arg) (arg) ? "true" : "false"

template<typename T>
inline void print(T arg){
	std::cout << arg << std::endl;
}

template<typename T, typename... Targs>
inline void print(T arg, Targs... args){
	print(arg);
	print(args...);
}

static std::size_t _total_test_count = 0;
static std::size_t _fail_count = 0;
static std::size_t _ok_cout = 0;

namespace{

template<typename T1=void, typename T2=void>
inline void ASSERT_EQ(const T1& value_one, const T2& value_two, const std::string& print_console){ 
	_total_test_count += 1; 
	if(value_one == value_two){
		std::cout << GREEN << "[ OK ] -> " << RESET << print_console << "\n";
		_ok_cout += 1;
	}else{
		std::cout << RED << "[ FAILED ] -> " << RESET << print_console << " " << "Value returned: " << value_one << "\n";
		_fail_count += 1;
	}
}
template<>
inline void ASSERT_EQ<bool>(const bool& value_one, const bool& value_two, const std::string& print_console){
	_total_test_count += 1; 
	if(value_one && value_two){
		std::cout << GREEN << "[ OK ] -> " << RESET << print_console << "\n";
		_ok_cout += 1;
	}else{
		std::cout << RED << "[ FAILED ] -> " << RESET << print_console << " " << "Value returned: " << value_one << "\n";
		_fail_count += 1;
	}
}


inline void print_final_stat(void){
	std::cout << "\n\nRESULTS:- (Total Tests: " << _total_test_count << ") \n" << GREEN << "Total OK: " << _ok_cout << RESET << "\n" << RED << "Total FAILED: " << _fail_count << RESET << std::endl;
}

}
