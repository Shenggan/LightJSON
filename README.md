# LightJSON: A Lightweight JSON CPP Library

[![Build Status](https://travis-ci.org/Shenggan/LightJSON.svg?branch=master)](https://travis-ci.org/Shenggan/LightJSON)
[![Documentation Status](https://readthedocs.org/projects/lightjson/badge/?version=latest)](http://lightjson.readthedocs.io/en/latest/?badge=latest)
![Release version](https://img.shields.io/badge/release-v0.1.0-blue.svg)

[English README](./README.en.md)

### 概览

LightJSON是一个轻量级的Json C++库。

### 使用

下面是LightJSON的使用方法，LightJSON向用户提供两种风格的API接口，以下为Class Style API。另一种API和具体的例子请参见：[example](example/)或[文档](http://lightjson.readthedocs.io/en/latest/usage.html)。

```cpp
// Class Style API
#include <iostream>
#include <string>
#include "lightjson.h"

int main() {
	ljson::Document js;

	std::string str("{ \"1\" : 1 }");
	js.Parse(str);

	ljson::Value v = js["1"];
	v.SetNumber(v.GetNumber() + 1);

	std::string str2;
	ljson_stringify(&v, str2);
	std::cout << js << std::endl;

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
