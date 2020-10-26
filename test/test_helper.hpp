// libhttpserver SSL HTTP Server Implementation
// Copyright © 2020 Harsath
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <cstddef>
#include <cstdlib>
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
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
		_fail_count += 1;
	}
}


inline void print_final_stat(void){
	std::cout << "\n\nRESULTS:- (Total Tests: " << _total_test_count << ") \n" << GREEN << "Total OK: " << _ok_cout << RESET << "\n" << RED << "Total FAILED: " << _fail_count << RESET << std::endl;
}

}
