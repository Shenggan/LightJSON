#include <iostream>
#include <fstream>
#include "lightjson.h"
#include "gtest/gtest.h"

using namespace ljson;

inline void test_number(double expect, const char* json) {
    ljson_value v;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_NUMBER, getType(&v));
    EXPECT_DOUBLE_EQ(expect, getNumber(&v));
    ljson_free(&v);
}

inline void test_error(ljson_state error, const char* json, ljson_type done = LJSON_NULL) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(error, ljson_parse(&v, json));
    EXPECT_EQ(done, getType(&v));
    ljson_free(&v);
}

inline void test_string(std::string expect, const char* json) {
    ljson_value v;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_STRING, getType(&v));
    EXPECT_STREQ(expect.c_str(), getString(&v).c_str());
    ljson_free(&v);
}

inline void test_roundtrip(const std::string & json) {
    ljson_value v;
    std::string json2;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_STRINGIFY_OK, ljson_stringify(&v, json2));
    EXPECT_EQ(json, json2);
    ljson_free(&v);
}

inline void test_roundtrip(const char* json) {
    ljson_value v;
    std::string json2;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_STRINGIFY_OK, ljson_stringify(&v, json2));
    // EXPECT_EQ(json, json2);
    ljson_free(&v);
}

TEST(test_api, c_api) {
    ljson::ljson_value v;
    ljson_init(&v);
	
	std::ifstream json_file("../example.json");
	std::ofstream out_file("../output.json");
	std::string str((std::istreambuf_iterator<char>(json_file)),  
					std::istreambuf_iterator<char>()); 
	std::cout << "Origin json:" << std::endl;
	std::cout << str << std::endl << std::endl;
    ljson_parse(&v, str);

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
	ljson_stringify(&v, str2);
	std::cout << "Converted json:" << std::endl;
	std::cout << str2 << std::endl;
	out_file << str2 << std::endl;
	ljson_free(&n);
    ljson_free(&v);
}

TEST(test_api, class_api) {
    std::ifstream json_file("../example.json");
	std::ofstream out_file("../output.json");
	std::string str((std::istreambuf_iterator<char>(json_file)),  
					std::istreambuf_iterator<char>()); 
    ljson::Document js;
    js.Parse(str);
	ljson::Value v = js["i"];
	v.SetNumber(v.GetNumber() + 1);
	ljson::Value v_t = js["t"];
	ljson::Value v2 = js["a"];
	ljson::Value v3 = js["o"];
	v3["1"].SetString("20");
	v3["3"].SetValue(v_t);
	v3["2"].SetValue(v3);
	v3["2"]["1"].SetBool(true);
	v2[2].SetValue(v2);
    std::cout << v2[2] << std::endl;
    std::cout << js << std::endl;
}

TEST(test_parse, parse_free) {
    ljson_value v;
    ljson_init(&v);
    ljson_parse(&v, "\"hello\"");
    v.free();
    ljson_parse(&v, "[ 1, \"2\", 3.23 ]");
    v.free();
    ljson_parse(&v, "{ \"1\": 1, \"2\" : \"342\", \"3.23\": [1,[1, 2]]}");
    v.free();
    ljson_free(&v);
}

TEST(test_parse, parse_null) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "null"));
    EXPECT_EQ(LJSON_NULL, getType(&v));
    ljson_free(&v);
}

TEST(test_parse, parse_true) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "true"));
    EXPECT_EQ(LJSON_TRUE, getType(&v));
    ljson_free(&v);
}

TEST(test_parse, parse_false) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "false"));
    EXPECT_EQ(LJSON_FALSE, getType(&v));
    ljson_free(&v);
}

TEST(test_parse, parse_number) {
    test_number(0.0, "0");
    test_number(0.0, "-0");
    test_number(0.0, "-0.0");
    test_number(1.0, "1");
    test_number(-1.0, "-1");
    test_number(1.5, "1.5");
    test_number(-1.5, "-1.5");
    test_number(3.1416, "3.1416");
    test_number(1E10, "1E10");
    test_number(1e10, "1e10");
    test_number(1E+10, "1E+10");
    test_number(1E-10, "1E-10");
    test_number(-1E10, "-1E10");
    test_number(-1e10, "-1e10");
    test_number(-1E+10, "-1E+10");
    test_number(-1E-10, "-1E-10");
    test_number(1.234E+10, "1.234E+10");
    test_number(1.234E-10, "1.234E-10");
    test_number(0.0, "1e-10000"); /* must underflow */

    test_number(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    test_number( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    test_number(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    test_number( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    test_number(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    test_number( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    test_number(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    test_number( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    test_number(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

TEST(test_parse, parse_string) {
    test_string("", "\"\"");
    test_string("Hello", "\"Hello\"");
    test_string("Hello\nWorld", "\"Hello\\nWorld\"");
    test_string("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

    test_string("Hello\0World", "\"Hello\\u0000World\"");
    test_string("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    test_string("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    test_string("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    test_string("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    test_string("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

TEST(test_parse, parse_array) {
    ljson_value v;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "[ ]"));
    EXPECT_EQ(LJSON_ARRAY, getType(&v));
    EXPECT_EQ(size_t(0), getArraySize(&v));
    ljson_free(&v);

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(LJSON_ARRAY, getType(&v));
    EXPECT_EQ(size_t(5), getArraySize(&v));
    EXPECT_EQ(LJSON_NULL,   getType(getArrayElement(&v, 0)));
    EXPECT_EQ(LJSON_FALSE,  getType(getArrayElement(&v, 1)));
    EXPECT_EQ(LJSON_TRUE,   getType(getArrayElement(&v, 2)));
    EXPECT_EQ(LJSON_NUMBER, getType(getArrayElement(&v, 3)));
    EXPECT_EQ(LJSON_STRING, getType(getArrayElement(&v, 4)));
    EXPECT_DOUBLE_EQ(123.0, getNumber(getArrayElement(&v, 3)));
    EXPECT_STREQ("abc", getString(getArrayElement(&v, 4)).c_str());
    ljson_free(&v);

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(LJSON_ARRAY, getType(&v));
    EXPECT_EQ(size_t(4), getArraySize(&v));
    for (int i = 0; i < 4; i++) {
        ljson_value a = getArrayElement(&v, i);
        EXPECT_EQ(LJSON_ARRAY, getType(a));
        EXPECT_EQ(size_t(i), getArraySize(a));
        for (int j = 0; j < i; j++) {
            ljson_value e = getArrayElement(a, j);
            EXPECT_EQ(LJSON_NUMBER, getType(e));
            EXPECT_DOUBLE_EQ((double)j, getNumber(e));
        }
    }
    ljson_free(&v);
}

TEST(test_parse, parse_object) {
    ljson_value v;

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, " { } "));
    EXPECT_EQ(LJSON_OBJECT, getType(&v));
    EXPECT_EQ(size_t(0), getObjectSize(&v));
    ljson_free(&v);

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v,
        " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [ 1, 2, 3 ],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "
    ));
    EXPECT_EQ(LJSON_OBJECT, getType(&v));
    EXPECT_EQ(size_t(7), getObjectSize(&v));
    // EXPECT_STREQ("n", getObject_key(&v, 0));
    // EXPECT_EQ(LJSON_NULL,   getType(getObject_value(&v, 0)));
    // EXPECT_STREQ("f", getObject_key(&v, 1));
    // EXPECT_EQ(LJSON_FALSE,  getType(getObject_value(&v, 1)));
    // EXPECT_STREQ("t", getObject_key(&v, 2));
    // EXPECT_EQ(LJSON_TRUE,   getType(getObject_value(&v, 2)));
    // EXPECT_STREQ("i", getObject_key(&v, 3));
    // EXPECT_EQ(LJSON_NUMBER, getType(getObject_value(&v, 3)));
    // EXPECT_DOUBLE_EQ(123.0, getNumber(getObject_value(&v, 3)));
    // EXPECT_STREQ("s", getObject_key(&v, 4));
    // EXPECT_EQ(LJSON_STRING, getType(getObject_value(&v, 4)));
    // EXPECT_STREQ("abc", getString(getObject_value(&v, 4)));
    // EXPECT_STREQ("a", getObject_key(&v, 5));
    // EXPECT_EQ(LJSON_ARRAY, getType(getObject_value(&v, 5)));
    // EXPECT_EQ(size_t(3), getArraySize(getObject_value(&v, 5)));
    // for (size_t i = 0; i < 3; i++) {
    //     ljson_value* e = getArrayElement(getObject_value(&v, 5), i);
    //     EXPECT_EQ(LJSON_NUMBER, getType(e));
    //     EXPECT_DOUBLE_EQ(i + 1.0, getNumber(e));
    // }
    // EXPECT_STREQ("o", getObject_key(&v, 6));
    // {
    //     ljson_value* o = getObject_value(&v, 6);
    //     EXPECT_EQ(LJSON_OBJECT, getType(o));
    //     for (size_t i = 0; i < 3; i++) {
    //         ljson_value* ov = getObject_value(o, i);
    //         EXPECT_TRUE(char('1' + i) == getObject_key(o, i)[0]);
    //         EXPECT_EQ(size_t(1), getObject_key_length(o, i));
    //         EXPECT_EQ(LJSON_NUMBER, getType(ov));
    //         EXPECT_DOUBLE_EQ(i + 1.0, getNumber(ov));
    //     }
    // }
    ljson_free(&v);
}




TEST(test_parse_error, expect_value) {
    test_error(LJSON_PARSE_EXPECT_VALUE, "");
    test_error(LJSON_PARSE_EXPECT_VALUE, " ");
}

TEST(test_parse_error, root_not_singular) {
    test_error(LJSON_PARSE_ROOT_NOT_SINGULAR, "null x");
    test_error(LJSON_PARSE_ROOT_NOT_SINGULAR, "false !S", LJSON_FALSE);
    
    test_error(LJSON_PARSE_ROOT_NOT_SINGULAR, "\"hello\" !S", LJSON_STRING);
}

TEST(test_parse_error, invalid_value) {
    test_error(LJSON_PARSE_INVALID_VALUE, "nul");
    test_error(LJSON_PARSE_INVALID_VALUE, "?");
}

TEST(test_parse_error, invalid_number) {
    /* invalid number */
    test_error(LJSON_PARSE_INVALID_VALUE, "+0");
    test_error(LJSON_PARSE_INVALID_VALUE, "+1");
    test_error(LJSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    test_error(LJSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    test_error(LJSON_PARSE_INVALID_VALUE, "INF");
    test_error(LJSON_PARSE_INVALID_VALUE, "inf");
    test_error(LJSON_PARSE_INVALID_VALUE, "NAN");
    test_error(LJSON_PARSE_INVALID_VALUE, "nan");
}

TEST(test_parse_error, test_parse_number_too_big) {
    test_error(LJSON_PARSE_NUMBER_TOO_BIG, "1e309");
    test_error(LJSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

TEST(test_parse_error, string_missing_quotation_mark) {
    test_error(LJSON_PARSE_MISS_QUOTATION_MARK, "\"");
    test_error(LJSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

TEST(test_parse_error, invalid_string_escape) {
    test_error(LJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    test_error(LJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    test_error(LJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    test_error(LJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

TEST(test_parse_error, invalid_string_char) {
    test_error(LJSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    test_error(LJSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

TEST(test_parse_error, invalid_array) {
    test_error(LJSON_PARSE_INVALID_VALUE, "[1,]");
    test_error(LJSON_PARSE_INVALID_VALUE, "[\"a\", nul]");
}

TEST(test_parse_error, invalid_unicode_hex) {
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

TEST(test_parse_error, invalid_unicode_surrogate) {
    test_error(LJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    test_error(LJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

TEST(test_parse_error, miss_comma_or_square_bracket) {
    test_error(LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    test_error(LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    test_error(LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    test_error(LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

TEST(test_parse_error, miss_key) {
    test_error(LJSON_PARSE_MISS_KEY, "{:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{1:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{true:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{false:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{null:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{[]:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{{}:1,");
    test_error(LJSON_PARSE_MISS_KEY, "{\"a\":1,");
}

TEST(test_parse_error, miss_colon) {
    test_error(LJSON_PARSE_MISS_COLON, "{\"a\"}");
    test_error(LJSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

TEST(test_parse_error, miss_comma_or_curly_bracket) {
    test_error(LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    test_error(LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    test_error(LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    test_error(LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}


TEST(test_stringify, null_false_true) {
    test_roundtrip("null");
    test_roundtrip("false");
    test_roundtrip("true");
}

TEST(test_stringify, stringify_string) {
    test_roundtrip("\"\"");
    test_roundtrip("\"Hello\"");
    test_roundtrip("\"Hello\\nWorld\"");
    test_roundtrip("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    test_roundtrip("\"Hello\\u0000World\"");
}

TEST(test_stringify, stringify_array) {
    test_roundtrip("[]");
    test_roundtrip("[null,false,true,123,\"abc\",[1,2,3]]");
}

TEST(test_stringify, stringify_object) {
    test_roundtrip("{}");
    test_roundtrip("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

TEST(test_set_get, test_access_null) {
    ljson_value v;
    ljson_init(&v);
    setString(&v, "a", 1);
    setNull(&v);
    EXPECT_EQ(LJSON_NULL, getType(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_boolean) {
    ljson_value v;
    ljson_init(&v);
    setString(&v, "a", 1);
    setBool(&v, 1);
    EXPECT_EQ(getBool(&v), true);
    setBool(&v, 0);
    EXPECT_FALSE(getBool(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_number) {
    ljson_value v;
    ljson_init(&v);
    setString(&v, "a", 1);
    setNumber(&v, 1234.5);
    EXPECT_DOUBLE_EQ(1234.5, getNumber(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_string) {
    ljson_value v;
    ljson_init(&v);
    setString(&v, "", 0);
    EXPECT_STREQ("", getString(&v).c_str());
    setString(&v, "Hello", 5);
    EXPECT_STREQ("Hello", getString(&v).c_str());
    ljson_free(&v);
}

TEST(test_set_get, test_access_string_2) {
    ljson_value v;
    ljson_init(&v);
    setString(&v, "");
    EXPECT_STREQ("", getString(&v).c_str());
    setString(&v, "Hello");
    EXPECT_STREQ("Hello", getString(&v).c_str());
    ljson_free(&v);
}

TEST(test_set_get, test_access_array) {
    std::vector<ljson_value> vec(1);
    ljson_init(&vec[0]);
    setString(&vec[0], "Hello", 5);
    EXPECT_STREQ("Hello", getString(&vec[0]).c_str());

    ljson_value v;
    ljson_init(&v);
    setArray(v, vec);
    
    //EXPECT_STREQ("Hello", getString(getArrayElement(&v, 0)).c_str());
    ljson_free(&(vec[0]));
    ljson_free(&v);
}