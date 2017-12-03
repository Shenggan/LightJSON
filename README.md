# LightJSON: A Lightweight JSON CPP Library

[![Build Status](https://travis-ci.com/Shenggan/LightJSON.svg?token=eMRNqxQT1YuqVS1nXpE5&branch=master)](https://travis-ci.com/Shenggan/LightJSON)
[![Documentation Status](https://readthedocs.org/projects/lightjson/badge/?version=latest)](http://lightjson.readthedocs.io/en/latest/?badge=latest)
![Release version](https://img.shields.io/badge/release-v0.1.0-blue.svg)

[English README](./README.en.md)

### 概览

LightJSON是一个轻量级的Json C++库。

### 使用

下面是LightJSON的使用方法，更具体的例子请参见：[example/example.cc](example/example.cc)

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

### 编译安装

The build of the LightJSON need cmake.

```shell
bash ./scripts/build.sh
```

You can run the UnitTest to validate the success of the build.

```shell
bash ./scripts/test.sh
```

### Reference
The project is base on [miloyip/json-tutorial](https://github.com/miloyip/json-tutorial).
