#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <streambuf>
#include "../lightjson.h"

int main() {
	ljson::ljson_value v;
    ljson_init(&v);
	
	std::ifstream json_file("../example/example.json");
	std::ofstream out_file("../example/output.json");
	std::string str((std::istreambuf_iterator<char>(json_file)),  
					std::istreambuf_iterator<char>()); 
	std::cout << "Origin json:" << std::endl;
	std::cout << str << std::endl << std::endl;
    int ret = ljson_parse(&v, str);
	if (ret != 0){
		std::cerr << "Parse Fail!" << std::endl;
		return 1;
 	}
	std::cout << "Success!" << std::endl;
	std::string str2;
	ret = ljson_stringify(&v, str2);
	if (ret != 1){
		std::cerr << "Stringify Fail! The type error is " << ret << std::endl;
		return 1;
 	}
	std::cout << "Converted json:" << std::endl;
	std::cout << str2 << std::endl;
	out_file << str2 << std::endl;
    ljson_free(&v);
	return 0;
}