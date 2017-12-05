# Usage

### Class Style API

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

### C Style API

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
