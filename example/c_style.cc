#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <streambuf>
#include "lightjson.h"

int main() {
	ljson::ljson_value v;
    ljson_init(&v);
	
	std::ifstream json_file("../example.json");
	std::ofstream out_file("../output.json");
	std::string str((std::istreambuf_iterator<char>(json_file)),  
					std::istreambuf_iterator<char>()); 
	std::cout << "Origin json:" << std::endl;
	std::cout << str << std::endl << std::endl;
    int ret = ljson_parse(&v, str);
	if (ret != 0) {
		std::cerr << "Parse Fail!" << std::endl;
		return 1;
 	}
	std::cout << "Success!" << std::endl;

	ljson::ljson_value & v_i = ljson::objectAccess(&v, "i");
	ljson::setNumber(&v_i, ljson::getNumber(&v_i) + 1);

	ljson::ljson_value & v_t = ljson::objectAccess(&v, "t");
	ljson::setBool(&v_t, !ljson::getBool(&v_t));

	ljson::ljson_value & v_s = ljson::objectAccess(&v, "s");
	ljson::setString(&v_s, ljson::getString(&v_s) + "def");

	ljson::ljson_value & v_s2 = ljson::objectAccess(&v, "s2");
	ljson::getString(&v_s2) += "def";

	ljson::ljson_value n;
	n.copyfrom(v_i);

	ljson::ljson_value & v_a = ljson::objectAccess(&v, "a");
	ljson::setString(ljson::getArrayElement(&v_a, 1), "10");
	ljson::getArray(&v_a).push_back(n);

	ljson::ljson_value v_sub_o = ljson::objectAccess(&v, "o");
	ljson::setNumber(ljson::getObjElement(&v_sub_o, "2"), 10);
	ljson::getObject(&v_sub_o)["haha2"] = n;
	ljson::getObject(&v)["haha"] = n;
	
	std::string str2;
	ret = ljson_stringify(&v, str2);
	if (ret != 1){
		std::cerr << "Stringify Fail! The type error is " << ret << std::endl;
		return 1;
 	}
	std::cout << "Converted json:" << std::endl;
	std::cout << str2 << std::endl;
	out_file << str2 << std::endl;
	ljson_free(&n);
    ljson_free(&v);
	return 0;
}