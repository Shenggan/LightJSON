#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <streambuf>
#include "lightjson.h"

int main() {
	
	std::ifstream json_file("../example.json");
	std::ofstream out_file("../output.json");
	std::string str((std::istreambuf_iterator<char>(json_file)),  
					std::istreambuf_iterator<char>()); 
	std::cout << "Origin json:" << std::endl;
    std::cout << str << std::endl << std::endl;
    
    ljson::Document js;
    int ret = js.Parse(str);
	if (ret != 0){
		std::cerr << "Parse Fail!" << std::endl;
		return 1;
 	}
	std::cout << "Success!" << std::endl;

	ljson::Value v = js["i"];
	v.SetNumber(v.GetNumber() + 1);

	ljson::Value v2 = js["a"];

	ljson::Value v3 = js["o"];
	v3["1"].SetString("20");
	
	v3["2"].SetValue(v3);
	v3["2"]["1"].SetBool(true);
	v2[2].SetValue(v2);
	std::cout << v2[2] << std::endl;

	std::cout << "Converted json:" << std::endl;
	std::cout << js << std::endl;
	out_file << js << std::endl;

	return 0;
}