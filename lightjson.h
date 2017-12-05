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
#include <map>
#include <cassert>
#include <iostream>

namespace ljson {

/*! \brief the basic type of the json struct*/
typedef enum { LJSON_NULL, LJSON_FALSE, LJSON_TRUE, LJSON_NUMBER, LJSON_STRING, LJSON_ARRAY, LJSON_OBJECT } ljson_type;

typedef struct ljson_value ljson_value;
typedef struct ljson_member ljson_member;

/////////////////////////
/* The C Stype API     */
/////////////////////////

/*!
 * \brief free the memory of a ljson_value, 
 *          use it if you will not use it or initailize it again
 * \param v the pointer of ljson_value you want to free
 */
void ljson_free(ljson_value* v);

/*!
 * \brief initailize a ljson_value, use it after declaration
 * \param v the pointer of ljson_value you want to initailize
 */
inline void ljson_init(ljson_value* v);
/*! \brief the inner struct of a json, can present all kind of ljson_type*/
struct ljson_value {
    union __data {
        std::map<std::string, ljson_value> * mobject; /*!< object */
        std::string * mstring;                  /*!< string */
        std::vector<ljson_value> * marray;      /*!< array */
        double mdouble;                         /*!< number */
    } data;                         /*!< data part of ljson_value */
    ljson_type type;                /*!< typr of this ljson_value */

    void free() {
        switch(type) {
            case LJSON_STRING:
                assert(data.mstring != nullptr);
                delete data.mstring;
                break;
            case LJSON_ARRAY:
                assert(data.marray != nullptr);
                for (auto iter = (data.marray)->begin(); iter != (data.marray)->end(); iter++)
                    ljson_free(&(*iter));
                delete data.marray;
                break;
            case LJSON_OBJECT:
                assert(data.mobject != nullptr);
                for (auto iter = (data.mobject)->begin(); iter != (data.mobject)->end(); iter++)
                    ljson_free(&(*iter).second);
                delete data.mobject;
                break;
            default:
                break;
        }
        type = LJSON_NULL;
    }

    void copyfrom(const ljson_value & copy) {
        size_t sz;
        free();
        switch (copy.type) {
            case LJSON_NUMBER:
                data.mdouble = copy.data.mdouble; break;
            case LJSON_STRING:
                data.mstring = new std::string();
                data.mstring->assign(*(copy.data.mstring));
                break;
            case LJSON_ARRAY:
                sz = copy.data.marray->size();
                data.marray = new std::vector<ljson_value>(sz);
                //data.marray->assign(copy.data.marray->begin(), copy.data.marray->end());
                for (size_t i = 0; i < sz; i++) {
                    ljson_init(&(*data.marray)[i]);
                    (*data.marray)[i].copyfrom((*copy.data.marray)[i]);
                }
                break;
            case LJSON_OBJECT:
                data.mobject = new std::map<std::string, ljson_value>;
                for (auto iter = (copy.data.mobject)->begin(); iter != (copy.data.mobject)->end(); iter++) {
                    // ljson_value tmp;
                    // ljson_init(&tmp);
                    // (*data.mobject)[iter->first] = iter->second;
                    ljson_init(&(*data.mobject)[iter->first]);
                    (*data.mobject)[iter->first].copyfrom(iter->second);
                }
                //data.mobject->insert(copy.data.mobject->begin(), copy.data.mobject->end());
                break;
            default:
                break;
        }
        type = copy.type;
    }
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

inline void ljson_init(ljson_value* v) { v->type = LJSON_NULL; }

/*!
 * \brief the same as ljson_free(ljson_value* v)
 */
inline void setNull(ljson_value* v) { ljson_free(v); }
inline void setNull(ljson_value & v) { ljson_free(&v); }

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


ljson_type getType(const ljson_value* v);
ljson_type getType(const ljson_value & v);

void setNumber(ljson_value* v, double n);
void setNumber(ljson_value & v, double n);
double getNumber(const ljson_value* v);
double getNumber(const ljson_value & v);

void setBool(ljson_value* v, ljson_type b);
void setBool(ljson_value* v, bool b);
bool getBool(const ljson_value* v);
void setBool(ljson_value & v, ljson_type b);
void setBool(ljson_value & v, bool b);
bool getBool(const ljson_value & v);

void setString(ljson_value* v, const char* s, size_t len);
void setString(ljson_value* v, const std::string & s);
std::string & getString(const ljson_value* v);
size_t getStringLength(const ljson_value* v);
void setString(ljson_value & v, const char* s, size_t len);
void setString(ljson_value & v, const std::string & s);
std::string & getString(const ljson_value & v);
size_t getStringLength(const ljson_value & v);

void setArray(ljson_value* v, const std::vector<ljson_value> & vec, bool deep_copy = 1);
std::vector<ljson_value> & getArray(const ljson_value* v);
ljson_value & getArrayElement(const ljson_value* v, const size_t index);
void setArrayElement(ljson_value* v, const size_t index, const ljson_value & content);
size_t getArraySize(const ljson_value* v);
void setArray(ljson_value & v, const std::vector<ljson_value> & vec, bool deep_copy = 1);
std::vector<ljson_value> & getArray(const ljson_value & v);
ljson_value & getArrayElement(const ljson_value & v, const size_t index);
void setArrayElement(ljson_value & v, const size_t index, const ljson_value & content);
size_t getArraySize(const ljson_value & v);

void setObject(ljson_value* v, const std::map<std::string, ljson_value> & vec, bool deep_copy = 1);
bool objectFindKey(const ljson_value* v, const std::string & mkey);
bool objectFindKey(const ljson_value & v, const std::string & mkey);
std::map<std::string, ljson_value> & getObject(const ljson_value* v);
ljson_value & getObjElement(const ljson_value* v, const std::string & key);
void setObjElement(ljson_value* v, const std::string key, const ljson_value & content);
size_t getObjectSize(const ljson_value* v);
ljson_value & objectAccess(ljson_value* v, const std::string & mkey);
void setObject(ljson_value & v, const std::map<std::string, ljson_value> & vec, bool deep_copy = 1);
std::map<std::string, ljson_value> & getObject(const ljson_value* v);
ljson_value & getObjElement(const ljson_value & v, const std::string key);
void setObjElement(ljson_value & v, const std::string key, const ljson_value & content);
size_t getObjectSize(const ljson_value & v);
ljson_value & objectAccess(ljson_value & v, const std::string & mkey);

void ljson_reset(ljson_value* v_old, const ljson_value & v_new);
void ljson_reset(ljson_value* v_old, const ljson_value * v_new);

/////////////////////////
/* The Class Stype API */
/////////////////////////

class Value {
protected:
    ljson_value * mvalue;
public:
    Value() {  }
    Value(ljson_value* v) { mvalue = v; }
    ~Value() {  }
    Value operator[](const std::string key) const {
        assert(getType(mvalue) == LJSON_OBJECT);
        return GetObjElement(key);
    }

    Value operator[](const size_t index) const {
        assert(getType(mvalue) == LJSON_ARRAY);
        return GetArrayElement(index);
    }

    ljson_value * GetValue() const { return mvalue; };
    void SetValue(const Value & content) {
        ljson_value mv;
        ljson_init(&mv);
        mv.copyfrom(*content.GetValue());
        mvalue->copyfrom(mv);
        ljson_free(&mv);
    }

    void SetNumber(const double a_num) { setNumber(mvalue, a_num); }
    double GetNumber() const { return getNumber(mvalue); }
    void SetBool(const bool a_bool) { setBool(mvalue, a_bool); }
    bool GetBool() const { return getBool(mvalue); }
    void SetString(const std::string & a_str) { setString(mvalue, a_str); }
    std::string & GetString() const { return getString(mvalue); }
    void SetArrayElement(const size_t index, const Value & content) { setArrayElement(mvalue, index, *(content.GetValue())); }
    Value GetArrayElement(const size_t index) const { return Value(&(getArrayElement(mvalue, index)));}
    void SetObjectElement(const std::string & key, const Value & content) { setObjElement(mvalue, key, *(content.GetValue())); }
    Value GetObjElement(const std::string & key) const { return Value(&(getObjElement(mvalue, key))); }

    std::string cpp_str() const {
        std::string mjson;
        ljson_stringify(mvalue, mjson);
        return mjson;
    }

    friend std::ostream& operator<<(std::ostream& out, const Value & v) {
        out << v.cpp_str() << std::endl;
        return out;
    }

}; /*class Value*/

class Document : public Value {
public:
    Document():Value() { mvalue = new ljson_value;ljson_init(mvalue); };
    ~Document() { ljson_free(mvalue);delete mvalue; }
    int Parse(std::string & json) {
        return ljson_parse(mvalue, json);
    }
}; /*class Document*/

} /*namespace ljson*/

#endif /* LIGHTJSON_H__ */
