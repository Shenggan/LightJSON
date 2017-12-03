/*!
 * \file lightjson.h
 * \brief The head file of LightJSON.
 * \author Shenggan
 */


#ifndef LIGHTJSON_H__
#define LIGHTJSON_H__

#include <string>
#include <vector>
#include <cstddef>

namespace ljson {

/*! \brief the basic type of the json struct*/
typedef enum { LJSON_NULL, LJSON_FALSE, LJSON_TRUE, LJSON_NUMBER, LJSON_STRING, LJSON_ARRAY, LJSON_OBJECT } ljson_type;

typedef struct ljson_value ljson_value;
typedef struct ljson_member ljson_member;

/*! \brief the inner struct of a json, can present all kind of ljson_type*/
struct ljson_value {
    union __data {
        std::vector<ljson_member> * mobject;    /*!< object */
        std::string * mstring;                  /*!< string */
        std::vector<ljson_value> * marray;      /*!< array */
        double mdouble;                         /*!< number */
    } data;                         /*!< data part of ljson_value */
    ljson_type type;                /*!< typr of this ljson_value */
};

/*! \brief the struct of the member of json object*/
struct ljson_member {
    std::string key;
    ljson_value value;
};

/*! \brief the struct of the json object*/
typedef struct {
    char* name;
    ljson_value value;
} ljson_object;

/*! \brief the type of the results or error*/
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

/*!
 * \brief initailize a ljson_value, use it after declaration
 * \param v the pointer of ljson_value you want to initailize
 */
inline void ljson_init(ljson_value* v) { v->type = LJSON_NULL; }

/*!
 * \brief free the memory of a ljson_value, 
 *          use it if you will not use it or initailize it again
 * \param v the pointer of ljson_value you want to free
 */
void ljson_free(ljson_value* v);
/*!
 * \brief the same as ljson_free(ljson_value* v)
 */
inline void ljson_set_null(ljson_value* v) { ljson_free(v); }

/*!
 * \brief parse a string to get the ljson_value
 * \param v the pointer of ljson_value you want to store the result of parse
 * \param json the string you want to parse
 * \return ljson_state
 */
int ljson_parse(ljson_value* v, const char* json);
/*!
 * \brief parse a string to get the ljson_value
 * \param v the pointer of ljson_value you want to store the result of parse
 * \param json the string you want to parse
 * \return ljson_state
 */
int ljson_parse(ljson_value* v, const std::string & json);

/*!
 * \brief ljson_value v to get the string os the json
 * \param v the pointer of ljson_value you want to stringify
 * \param json the string you want to store the result
 * \return ljson_state
 */
int ljson_stringify(const ljson_value* v, std::string & json);
/*!
 * \brief ljson_value v to get the string os the json
 * \param v the pointer of ljson_value you want to stringify
 * \param json the string you want to store the result
 * \return ljson_state
 */
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
