# Usage

A example of the usage of the LightJSON is in [example/example.cc](https://github.com/Shenggan/LightJSON/blob/master/example/example.cc)

```cpp
#include <iostream>
#include <string>
#include "lightjson.h"

int main() {
	ljson::ljson_value v;
	ljson_init(&v);

	std::string str("{ \"1\" : 1 }");
	ljson_parse(&v, str);

	std::string str2;
	ljson_stringify(&v, str2);
	std::cout << str2 << std::endl;

	ljson_free(&v);
	return 0;
}

```
