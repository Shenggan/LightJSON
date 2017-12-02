#ifndef LIGHTJSON_H__
#define LIGHTJSON_H__

#include <string>
#include <vector>
#include <cstddef>

namespace ljson {

typedef enum { LJSON_NULL, LJSON_FALSE, LJSON_TRUE, LJSON_NUMBER, LJSON_STRING, LJSON_ARRAY, LJSON_OBJECT } ljson_type;

typedef struct ljson_value ljson_value;
typedef struct ljson_member ljson_member;

struct ljson_value {
    union {
        std::vector<ljson_member> * mobject;
        std::string * mstring;                  /* string */
        std::vector<ljson_value> * marray;      /* array */
        double mdouble;                         /* number */
    } data;
    ljson_type type;
};

struct ljson_member {
    std::string key;
    ljson_value value;
};

typedef struct {
    char* name;
    ljson_value value;
} ljson_object;

typedef enum {
    LJSON_PARSE_OK = 0,
    LJSON_STRINGIFY_OK,

    LJSON_PARSE_EXPECT_VALUE,
    LJSON_PARSE_INVALID_VALUE,
    LJSON_PARSE_ROOT_NOT_SINGULAR,

    LJSON_PARSE_NUMBER_TOO_BIG,

    LJSON_PARSE_MISS_QUOTATION_MARK,
    LJSON_PARSE_INVALID_STRING_ESCAPE,
    LJSON_PARSE_INVALID_STRING_CHAR,
    LJSON_PARSE_INVALID_UNICODE_HEX,
    LJSON_PARSE_INVALID_UNICODE_SURROGATE,

    LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,

    LJSON_PARSE_MISS_KEY,
    LJSON_PARSE_MISS_COLON,
    LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
} ljson_state;

inline void ljson_init(ljson_value* v) { v->type = LJSON_NULL; }

void ljson_free(ljson_value* v);
#define ljson_set_null(v) ljson_free(v);

int ljson_parse(ljson_value* v, const char* json);
int ljson_parse(ljson_value* v, const std::string & json);

int ljson_stringify(const ljson_value* v, std::string & json);
int ljson_stringify(const ljson_value* v, char* json);


ljson_type ljson_get_type(const ljson_value* v);

void ljson_set_number(ljson_value* v, double n);
double ljson_get_number(const ljson_value* v);

void ljson_set_boolean(ljson_value* v, int b);
int ljson_get_boolean(const ljson_value* v);

void ljson_set_string(ljson_value* v, const char* s, size_t len);
void ljson_set_string(ljson_value* v, const std::string & s);
const char* ljson_get_string(const ljson_value* v);
size_t ljson_get_string_length(const ljson_value* v);

void ljson_set_array(ljson_value* v, std::vector<ljson_value> & vec);
ljson_value* ljson_get_array_element(const ljson_value* v, size_t index);
size_t ljson_get_array_size(const ljson_value* v);

void ljson_set_object(ljson_value* v, std::vector<ljson_member> & vec);
size_t ljson_get_object_size(const ljson_value* v);
const char* ljson_get_object_key(const ljson_value* v, size_t index);
size_t ljson_get_object_key_length(const ljson_value* v, size_t index);
ljson_value* ljson_get_object_value(const ljson_value* v, size_t index);

} /*namespace ljson*/

#endif /* LIGHTJSON_H__ */
