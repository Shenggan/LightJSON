/*!
 * \file lightjson.cc
 * \brief The cc file of LightJSON.
 * \author Shenggan
 */

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstddef>
#include <string>
#include <iostream>
#include <vector>

#include "lightjson.h"

namespace ljson{

typedef struct {
    const char* json;
} ljson_context;

static int ljson_parse_value(ljson_context* c, ljson_value* v);

void ljson_free(ljson_value* v) {
    assert(v != nullptr);
    switch(v->type) {
        case LJSON_STRING:
            assert(v->data.mstring != nullptr);
            delete v->data.mstring;
            break;
        case LJSON_ARRAY:
            assert(v->data.marray != nullptr);
            for (auto iter = (v->data.marray)->begin(); iter != (v->data.marray)->end(); iter++)
                ljson_free(&(*iter));
            delete v->data.marray;
            break;
        case LJSON_OBJECT:
            assert(v->data.mobject != nullptr);
            for (auto iter = (v->data.mobject)->begin(); iter != (v->data.mobject)->end(); iter++)
                ljson_free(&(*iter).second);
            delete v->data.mobject;
            break;
        default:
            break;
    }
    v->type = LJSON_NULL;
}

inline void ljson_init(ljson_value* v) { v->type = LJSON_NULL; }

inline void expect_char(ljson_context* c, char ch) { 
    assert(*c->json == (ch));
    c->json++;
}

inline bool isdigit1to9(char ch) {
    return isdigit(ch) && ch != '0';
}

static void ljson_parse_whitespace(ljson_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int ljson_parse_literal(ljson_context* c, ljson_value* v, const char* literal, ljson_type type) {
    expect_char(c, literal[0]);
    size_t i = 0;
    for (; literal[i + 1]; i++) {
        if (c->json[i] != literal[i + 1])
            return LJSON_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LJSON_PARSE_OK;
}

static int ljson_parse_number(ljson_context* c, ljson_value* v) {
    const char* p = c->json;

    if (*p == '-') p++;
    
    if (*p == '0') p++;
    else {
        if (!isdigit1to9(*p)) return LJSON_PARSE_INVALID_VALUE;
        for (p++; isdigit(*p); p++);
    }

    if (*p == '.') {
        p++;
        if(!isdigit(*p)) return LJSON_PARSE_INVALID_VALUE;
        for (p++; isdigit(*p); p++);
    }

    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!isdigit(*p)) return LJSON_PARSE_INVALID_VALUE;
        for (p++; isdigit(*p); p++);
    }

    errno = 0;
    v->data.mdouble = strtod(c->json, nullptr);
    if (errno == ERANGE && (v->data.mdouble == HUGE_VAL || v->data.mdouble == -HUGE_VAL)) return LJSON_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LJSON_NUMBER;
    return LJSON_PARSE_OK;
}

static const char* ljson_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
        else return nullptr;
    }
    return p;
}

static void ljson_encode_utf8(std::string & s, unsigned u) {
    if (u <= 0x7F) 
        s += u & 0xFF;
    else if (u <= 0x7FF) {
        s += 0xC0 | ((u >> 6) & 0xFF);
        s += 0x80 | ( u       & 0x3F);
    }
    else if (u <= 0xFFFF) {
        s += 0xE0 | ((u >> 12) & 0xFF);
        s += 0x80 | ((u >>  6) & 0x3F);
        s += 0x80 | ( u        & 0x3F);
    }
    else {
        assert(u <= 0x10FFFF);
        s += 0xF0 | ((u >> 18) & 0xFF);
        s += 0x80 | ((u >> 12) & 0x3F);
        s += 0x80 | ((u >>  6) & 0x3F);
        s += 0x80 | ( u        & 0x3F);
    }
}

static int ljson_parse_string_raw(ljson_context* c, std::string & cache_string) {
    unsigned u, u2;
    expect_char(c, '\"');
    cache_string = "";
    const char* head = c->json;
    while (true) {
        char ch = *head++;
        switch (ch) {
            case '\"':
                c->json = head;
                return LJSON_PARSE_OK;
            case '\0':
                return LJSON_PARSE_MISS_QUOTATION_MARK;
            case '\\':
                switch(*head++) {
                    case '\"': cache_string += '\"'; break;
                    case '\\': cache_string += '\\'; break;
                    case '/':  cache_string += '/';  break;
                    case 'b':  cache_string += '\b'; break;
                    case 'f':  cache_string += '\f'; break;
                    case 'n':  cache_string += '\n'; break;
                    case 'r':  cache_string += '\r'; break;
                    case 't':  cache_string += '\t'; break;
                    case 'u':
                        if (!(head = ljson_parse_hex4(head, &u)))
                            return LJSON_PARSE_INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*head++ != '\\')
                                return LJSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (*head++ != 'u')
                                return LJSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (!(head = ljson_parse_hex4(head, &u2)))
                                return LJSON_PARSE_INVALID_UNICODE_HEX;
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                return LJSON_PARSE_INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        ljson_encode_utf8(cache_string, u);
                        break;
                    default:
                        return LJSON_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) {
                    return LJSON_PARSE_INVALID_STRING_CHAR;
                }
                cache_string += ch;
        }
    }
}

static int ljson_parse_string(ljson_context* c, ljson_value* v) {
    int ret;
    std::string cache_string;
    if ((ret = ljson_parse_string_raw(c, cache_string)) == LJSON_PARSE_OK)
        setString(v, cache_string);
    return ret;
}

static int ljson_parse_array(ljson_context* c, ljson_value* v) {
    expect_char(c, '[');
    int ret;
    std::vector<ljson_value> vec;
    ljson_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = LJSON_ARRAY;
        v->data.marray = new std::vector<ljson_value>;
        return LJSON_PARSE_OK;
    }
    
    for (;;) {
        ljson_value e;
        ljson_init(&e);
        if ((ret = ljson_parse_value(c, &e)) != LJSON_PARSE_OK)
            break;
        vec.push_back(e);
        ljson_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            ljson_parse_whitespace(c);
        }
        else if (*c->json == ']') {
            c->json++;
            setArray(v, vec, 0);
            return LJSON_PARSE_OK;
        } 
        else{
            ret = LJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        } 
    }
    for (auto iter = vec.begin(); iter != vec.end(); iter++)
            ljson_free(&(*iter));
    return ret;
}

static int ljson_parse_object(ljson_context* c, ljson_value* v ) {
    expect_char(c, '{');
    int ret;
    std::map<std::string, ljson_value> m_map;
    ljson_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = LJSON_OBJECT;
        v->data.mobject = new std::map<std::string, ljson_value>;
        return LJSON_PARSE_OK;
    }
    for (;;) {
        ljson_member m;
        ljson_init(&m.value);
        if (*c->json != '"') {
            ret = LJSON_PARSE_MISS_KEY;
            break;
        }
        if ((ret = ljson_parse_string_raw(c, m.key) != LJSON_PARSE_OK))
            break;
        ljson_parse_whitespace(c);
        if (*c->json != ':') {
            ret = LJSON_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        ljson_parse_whitespace(c);
        if ((ret = ljson_parse_value(c, &m.value)) != LJSON_PARSE_OK)
            break;
        m_map[m.key] = m.value;

        ljson_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            ljson_parse_whitespace(c);
        } else if (*c->json == '}') {
            c->json++;
            setObject(v, m_map, 0);
            return LJSON_PARSE_OK;
        } else {
            ret = LJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }

    }
    for (auto iter = m_map.begin(); iter != m_map.end(); iter++)
        ljson_free(&(*iter).second);
    v->type = LJSON_NULL;
    return ret;
}

static int ljson_parse_value(ljson_context* c, ljson_value* v) {
    switch (*c->json) {
        case 'n':  return ljson_parse_literal(c, v, "null", LJSON_NULL);
        case 't':  return ljson_parse_literal(c, v, "true", LJSON_TRUE);
        case 'f':  return ljson_parse_literal(c, v, "false", LJSON_FALSE);
        case '\"': return ljson_parse_string(c, v);
        case '[':  return ljson_parse_array(c, v);
        case '{':  return ljson_parse_object(c, v);
        case '\0': return LJSON_PARSE_EXPECT_VALUE;
        default:   return ljson_parse_number(c, v);
    }
}

int ljson_parse(ljson_value* v, const char* json) {
    ljson_context c;
    int ret;
    assert(v != nullptr);
    c.json = json;
    v->type = LJSON_NULL;
    ljson_parse_whitespace(&c);

    if ((ret = ljson_parse_value(&c, v)) == LJSON_PARSE_OK) {
        ljson_parse_whitespace(&c);
        if (*c.json != '\0')
            ret = LJSON_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

int ljson_parse(ljson_value* v, const std::string & json) {
    return ljson_parse(v, json.c_str());
}

static void ljson_stringify_string(std::string & str, const std::string & json_str) {
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    str += '"';
    for (auto iter = json_str.begin(); iter != json_str.end(); iter++) {
        unsigned char ch = (unsigned char)(*iter);
        switch (ch) {
            case '\"': str += "\\\""; break;
            case '\\': str += "\\\\"; break;
            case '\b': str += "\\b";  break;
            case '\f': str += "\\f";  break;
            case '\n': str += "\\n";  break;
            case '\r': str += "\\r";  break;
            case '\t': str += "\\t";  break;
            default:
                if (ch < 0x20) {
                    str += "\\u00";
                    str += hex_digits[ch >> 4];
                    str += hex_digits[ch & 15];
                } else {
                    str += *iter;
                }
        }
    }
    str += '"';
}

static int ljson_stringify_value(const ljson_value* v, std::string & str) {
    switch (v->type) {
        case LJSON_NULL:    str += "null";  break;
        case LJSON_FALSE:   str += "false"; break;
        case LJSON_TRUE:    str += "true";   break;
        case LJSON_NUMBER:
            char buffer[32];
            sprintf(buffer, "%.17g", v->data.mdouble);  
            str += buffer;
            break;
        case LJSON_STRING: 
            ljson_stringify_string(str, *(v->data.mstring)); 
            break;
        case LJSON_ARRAY:
            str += '[';
            for (auto iter = v->data.marray->begin(); iter != v->data.marray->end(); iter++) {
                if (iter != v->data.marray->begin())
                    str += ',';
                ljson_stringify_value(&(*iter), str);
            }
            str += ']';
            break;
        case LJSON_OBJECT:
            str += '{';
            for (auto iter = v->data.mobject->begin(); iter != v->data.mobject->end(); iter++) {
                if (iter != v->data.mobject->begin())
                    str += ',';
                ljson_stringify_string(str, (*iter).first);
                str += ':';
                ljson_stringify_value(&(*iter).second, str);
            }
            str += '}';
            break;
        default:
            break;
    }
    return LJSON_STRINGIFY_OK;
}

int ljson_stringify(const ljson_value* v, std::string & json) {
    int ret;
    assert(v != nullptr);
    if ((ret = ljson_stringify_value(v, json)) != LJSON_STRINGIFY_OK) {
        json = "";
        return ret;
    }
    return LJSON_STRINGIFY_OK;
}

int ljson_stringify(const ljson_value* v, char* json) {
    std::string str(json);
    return ljson_stringify(v, str);
}

void ljson_reset(ljson_value* v_old, const ljson_value & v_new) {
    v_old->copyfrom(v_new);
}
void ljson_reset(ljson_value* v_old, const ljson_value * v_new) {
    ljson_reset(v_old, *v_new);
}

ljson_type getType(const ljson_value* v) {
    assert(v != nullptr);
    return v->type;
}
ljson_type getType(const ljson_value & v) { return getType(&v); }

void setNumber(ljson_value* v, double n) {
    ljson_free(v);
    v->data.mdouble = n;
    v->type = LJSON_NUMBER;
}
void setNumber(ljson_value & v, double n) { return setNumber(&v, n); }

double getNumber(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_NUMBER);
    return v->data.mdouble;
}
double getNumber(const ljson_value & v) { return getNumber(&v); };

void setBool(ljson_value* v, ljson_type b) {
    ljson_free(v);
    v->type = b;
}
void setBool(ljson_value & v, ljson_type b) { setBool(&v, b); }

void setBool(ljson_value* v, bool b) {
    ljson_free(v);
    v->type = b ? LJSON_TRUE : LJSON_FALSE;
}
void setBool(ljson_value & v, bool b) { setBool(&v, b); }

bool getBool(const ljson_value* v) {
    assert(v != nullptr && (v->type == LJSON_TRUE || v->type == LJSON_FALSE));
    return v->type == LJSON_TRUE;
}
bool getBool(const ljson_value & v) { return getBool(&v); }

void setString(ljson_value* v, const char* s, size_t len) {
    assert(v != nullptr && (s != nullptr || len == 0));
    ljson_free(v);
    v->data.mstring = new std::string();
    v->data.mstring->assign(s, len);
    v->type = LJSON_STRING;
}

void setString(ljson_value* v, const std::string & s) {
    assert(v != nullptr);
    assert(&s != v->data.mstring);
    ljson_free(v);
    v->data.mstring = new std::string();
    v->data.mstring->assign(s);
    v->type = LJSON_STRING;
}

std::string & getString(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_STRING);
    return *(v->data.mstring);
}

size_t getStringLength(const ljson_value* v){
    assert(v != nullptr && v->type == LJSON_STRING);
    return v->data.mstring->length();
}

void setString(ljson_value & v, const char* s, size_t len) { setString(&v, s, len); }
void setString(ljson_value & v, const std::string & s) { setString(&v, s); }
std::string & getString(const ljson_value & v) { return getString(&v); }
size_t getStringLength(const ljson_value& v) { return getStringLength(&v); };



void setArray(ljson_value* v, const std::vector<ljson_value> & vec, bool deep_copy) {
    assert(v != nullptr);
    ljson_free(v);
    v->data.marray = new std::vector<ljson_value>;
    v->type = LJSON_ARRAY;
    if (!deep_copy)
        v->data.marray->assign(vec.begin(), vec.end());
    else {
        size_t sz = vec.size();
        v->data.marray = new std::vector<ljson_value>(sz);
        for (size_t i = 0; i < sz; i++) {
            ljson_init(&(*v->data.marray)[i]);
            ljson_reset(&((*v->data.marray)[i]), vec[i]);
        }
    }
} 

std::vector<ljson_value> & getArray(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_ARRAY);
    return *(v->data.marray);
}

void setArrayElement(ljson_value* v, size_t index, const ljson_value & content) {
    assert(v != nullptr && v->type == LJSON_ARRAY);
    assert(index < v->data.marray->size());
    ljson_reset(&((*v->data.marray)[index]), content);
}

ljson_value & getArrayElement(const ljson_value* v, size_t index){
    assert(v != nullptr && v->type == LJSON_ARRAY);
    assert(index < v->data.marray->size());
    return (*v->data.marray)[index];
}

size_t getArraySize(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_ARRAY);
    return v->data.marray->size();
}

void setArray(ljson_value & v, const std::vector<ljson_value> & vec, bool deep_copy) { setArray(v, vec, deep_copy); }
std::vector<ljson_value> & getArray(const ljson_value& v) { return getArray(v); }
ljson_value & getArrayElement(const ljson_value & v, const size_t index) { return getArrayElement(&v, index); }
void setArrayElement(ljson_value & v, const size_t index, const ljson_value & content) { setArrayElement(&v, index, content); }
size_t getArraySize(const ljson_value & v) { return getArraySize(&v); }


void setObject(ljson_value* v, const std::map<std::string, ljson_value> & vec, bool deep_copy) {
    assert(v != nullptr);
    ljson_free(v);
    v->data.mobject = new std::map<std::string, ljson_value>;
    v->type = LJSON_OBJECT;
    if (!deep_copy) 
        v->data.mobject->insert(vec.begin(), vec.end());
    else
        for (auto iter = vec.begin(); iter != vec.end(); iter++) {
            ljson_init(&(*v->data.mobject)[iter->first]);
            ljson_reset(&(*v->data.mobject)[iter->first], iter->second);
        }
}

size_t getObjectSize(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_OBJECT);
    return v->data.mobject->size();
}

bool objectFindKey(const ljson_value* v, const std::string & mkey) {
    return v->data.mobject->find(mkey) != v->data.mobject->end();
}

ljson_value & getObjElement(const ljson_value* v, const std::string & key) {
    assert(v != nullptr && v->type == LJSON_OBJECT);
    assert(objectFindKey(v, key));
    return (*v->data.mobject)[key];
}
void setObjElement(ljson_value* v, const std::string key, const ljson_value & content) {
    assert(v != nullptr && v->type == LJSON_OBJECT);
    assert(objectFindKey(v, key));
    ljson_reset(&((*v->data.mobject)[key]), content);
}

std::map<std::string, ljson_value> & getObject(const ljson_value* v) {
    assert(v != nullptr && v->type == LJSON_OBJECT);
    return *(v->data.mobject);
}

ljson_value & objectAccess(ljson_value* v, const std::string & mkey) {
    assert(objectFindKey(v, mkey));
    return (*v->data.mobject)[mkey];
}

void setObject(ljson_value & v, const std::map<std::string, ljson_value> & vec, bool deep_copy) { setObject(&v, vec, deep_copy); }
bool objectFindKey(const ljson_value & v, const std::string & mkey) { return objectFindKey(&v, mkey); }
std::map<std::string, ljson_value> & getObject(const ljson_value & v) { return getObject(&v); }
ljson_value & getObjElement(const ljson_value & v, const std::string key) { return getObjElement(&v, key); }
void setObjElement(ljson_value & v, const std::string key, const ljson_value & content) { setObjElement(&v, key, content); }
size_t getObjectSize(const ljson_value & v) { return getObjectSize(&v); }
ljson_value & objectAccess(ljson_value & v, const std::string & mkey) { return objectAccess(v, mkey); }

}   /*namespace ljson*/