#include "lightjson.h"
#include "gtest/gtest.h"

using namespace ljson;

inline void test_number(double expect, const char* json) {
    ljson_value v;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_NUMBER, ljson_get_type(&v));
    EXPECT_DOUBLE_EQ(expect, ljson_get_number(&v));
    ljson_free(&v);
}

inline void test_error(ljson_state error, const char* json, ljson_type done = LJSON_NULL) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(error, ljson_parse(&v, json));
    EXPECT_EQ(done, ljson_get_type(&v));
    ljson_free(&v);
}

inline void test_string(const char* expect, const char* json) {
    ljson_value v;
    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, json));
    EXPECT_EQ(LJSON_STRING, ljson_get_type(&v));
    EXPECT_STREQ(expect, ljson_get_string(&v));
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
    EXPECT_EQ(json, json2);
    ljson_free(&v);
}

TEST(test_parse, parse_null) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "null"));
    EXPECT_EQ(LJSON_NULL, ljson_get_type(&v));
    ljson_free(&v);
}

TEST(test_parse, parse_true) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "true"));
    EXPECT_EQ(LJSON_TRUE, ljson_get_type(&v));
    ljson_free(&v);
}

TEST(test_parse, parse_false) {
    ljson_value v;
    ljson_init(&v);
    v.type = LJSON_FALSE;
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "false"));
    EXPECT_EQ(LJSON_FALSE, ljson_get_type(&v));
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
    EXPECT_EQ(LJSON_ARRAY, ljson_get_type(&v));
    EXPECT_EQ(size_t(0), ljson_get_array_size(&v));
    ljson_free(&v);

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(LJSON_ARRAY, ljson_get_type(&v));
    EXPECT_EQ(size_t(5), ljson_get_array_size(&v));
    EXPECT_EQ(LJSON_NULL,   ljson_get_type(ljson_get_array_element(&v, 0)));
    EXPECT_EQ(LJSON_FALSE,  ljson_get_type(ljson_get_array_element(&v, 1)));
    EXPECT_EQ(LJSON_TRUE,   ljson_get_type(ljson_get_array_element(&v, 2)));
    EXPECT_EQ(LJSON_NUMBER, ljson_get_type(ljson_get_array_element(&v, 3)));
    EXPECT_EQ(LJSON_STRING, ljson_get_type(ljson_get_array_element(&v, 4)));
    EXPECT_DOUBLE_EQ(123.0, ljson_get_number(ljson_get_array_element(&v, 3)));
    EXPECT_STREQ("abc", ljson_get_string(ljson_get_array_element(&v, 4)));
    ljson_free(&v);

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(LJSON_ARRAY, ljson_get_type(&v));
    EXPECT_EQ(size_t(4), ljson_get_array_size(&v));
    for (int i = 0; i < 4; i++) {
        ljson_value* a = ljson_get_array_element(&v, i);
        EXPECT_EQ(LJSON_ARRAY, ljson_get_type(a));
        EXPECT_EQ(size_t(i), ljson_get_array_size(a));
        for (int j = 0; j < i; j++) {
            ljson_value* e = ljson_get_array_element(a, j);
            EXPECT_EQ(LJSON_NUMBER, ljson_get_type(e));
            EXPECT_DOUBLE_EQ((double)j, ljson_get_number(e));
        }
    }
    ljson_free(&v);
}

TEST(test_parse, parse_object) {
    ljson_value v;

    ljson_init(&v);
    EXPECT_EQ(LJSON_PARSE_OK, ljson_parse(&v, " { } "));
    EXPECT_EQ(LJSON_OBJECT, ljson_get_type(&v));
    EXPECT_EQ(size_t(0), ljson_get_object_size(&v));
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
    EXPECT_EQ(LJSON_OBJECT, ljson_get_type(&v));
    EXPECT_EQ(size_t(7), ljson_get_object_size(&v));
    EXPECT_STREQ("n", ljson_get_object_key(&v, 0));
    EXPECT_EQ(LJSON_NULL,   ljson_get_type(ljson_get_object_value(&v, 0)));
    EXPECT_STREQ("f", ljson_get_object_key(&v, 1));
    EXPECT_EQ(LJSON_FALSE,  ljson_get_type(ljson_get_object_value(&v, 1)));
    EXPECT_STREQ("t", ljson_get_object_key(&v, 2));
    EXPECT_EQ(LJSON_TRUE,   ljson_get_type(ljson_get_object_value(&v, 2)));
    EXPECT_STREQ("i", ljson_get_object_key(&v, 3));
    EXPECT_EQ(LJSON_NUMBER, ljson_get_type(ljson_get_object_value(&v, 3)));
    EXPECT_DOUBLE_EQ(123.0, ljson_get_number(ljson_get_object_value(&v, 3)));
    EXPECT_STREQ("s", ljson_get_object_key(&v, 4));
    EXPECT_EQ(LJSON_STRING, ljson_get_type(ljson_get_object_value(&v, 4)));
    EXPECT_STREQ("abc", ljson_get_string(ljson_get_object_value(&v, 4)));
    EXPECT_STREQ("a", ljson_get_object_key(&v, 5));
    EXPECT_EQ(LJSON_ARRAY, ljson_get_type(ljson_get_object_value(&v, 5)));
    EXPECT_EQ(size_t(3), ljson_get_array_size(ljson_get_object_value(&v, 5)));
    for (size_t i = 0; i < 3; i++) {
        ljson_value* e = ljson_get_array_element(ljson_get_object_value(&v, 5), i);
        EXPECT_EQ(LJSON_NUMBER, ljson_get_type(e));
        EXPECT_DOUBLE_EQ(i + 1.0, ljson_get_number(e));
    }
    EXPECT_STREQ("o", ljson_get_object_key(&v, 6));
    {
        ljson_value* o = ljson_get_object_value(&v, 6);
        EXPECT_EQ(LJSON_OBJECT, ljson_get_type(o));
        for (size_t i = 0; i < 3; i++) {
            ljson_value* ov = ljson_get_object_value(o, i);
            EXPECT_TRUE(char('1' + i) == ljson_get_object_key(o, i)[0]);
            EXPECT_EQ(size_t(1), ljson_get_object_key_length(o, i));
            EXPECT_EQ(LJSON_NUMBER, ljson_get_type(ov));
            EXPECT_DOUBLE_EQ(i + 1.0, ljson_get_number(ov));
        }
    }
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
    ljson_set_string(&v, "a", 1);
    ljson_set_null(&v);
    EXPECT_EQ(LJSON_NULL, ljson_get_type(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_boolean) {
    ljson_value v;
    ljson_init(&v);
    ljson_set_string(&v, "a", 1);
    ljson_set_boolean(&v, 1);
    EXPECT_EQ(ljson_get_boolean(&v), true);
    ljson_set_boolean(&v, 0);
    EXPECT_FALSE(ljson_get_boolean(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_number) {
    ljson_value v;
    ljson_init(&v);
    ljson_set_string(&v, "a", 1);
    ljson_set_number(&v, 1234.5);
    EXPECT_DOUBLE_EQ(1234.5, ljson_get_number(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_string) {
    ljson_value v;
    ljson_init(&v);
    ljson_set_string(&v, "", 0);
    EXPECT_STREQ("", ljson_get_string(&v));
    ljson_set_string(&v, "Hello", 5);
    EXPECT_STREQ("Hello", ljson_get_string(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_string_2) {
    ljson_value v;
    ljson_init(&v);
    ljson_set_string(&v, "");
    EXPECT_STREQ("", ljson_get_string(&v));
    ljson_set_string(&v, "Hello");
    EXPECT_STREQ("Hello", ljson_get_string(&v));
    ljson_free(&v);
}

TEST(test_set_get, test_access_array) {
    ljson_value v1;
    ljson_init(&v1);
    ljson_set_string(&v1, "Hello", 5);
    EXPECT_STREQ("Hello", ljson_get_string(&v1));

    std::vector<ljson_value> vec;
    vec.push_back(v1);

    ljson_value v;
    ljson_init(&v);
    ljson_set_array(&v, vec);
    EXPECT_STREQ("Hello", ljson_get_string(ljson_get_array_element(&v, 0)));

    ljson_free(&v);
}