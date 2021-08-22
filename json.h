/* date = August 20th 2021 1:48 pm */

#ifndef JSON_H
#define JSON_H

//~general types and definitions
using u64 = unsigned long long int;
using u32 = unsigned int;
using b32 = int;
using u8 = unsigned char;

#define consume_characater(str) ++(str)
#define global static
#define function static

#define AssertBreak  (*((u32*)0) = 0x1234);
#define AssertTrue(expr) if (!(expr)) { AssertBreak; }


#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)

//~includes
#include "memory.h"

//~ forward declarations
struct Json_t;

//~ Json_String_t
struct Json_String_t
{
    char *str;
    u32 length;
    
    // IMPORANT(fakhri): check for bounds bfore using this!!
    char& operator[](u64 index){ return str[index]; }
};

b32 operator==(Json_String_t &lhs, Json_String_t &rhs)
{
    if (lhs.length !=rhs.length) return 0;
    u32 i = 0;
    u32 len = lhs.length;
    for (i = 0; i < len && (lhs[i] == rhs[i]); ++i);
    return (i == len);
}

//constructor
function Json_String_t *
make_json_string(char *start_of_key, char *end_of_key);

//~ Json_Object_t
struct Json_Object_Slot_t
{
    Json_String_t *key;
    Json_t *value;
    
    Json_Object_Slot_t *next;
};

// TODO(fakhri): try dynamic size hash table ?
constexpr int JSON_OBJECT_HASH_SIZE = 100;
struct Json_Object_t
{
    Json_Object_Slot_t *slots[JSON_OBJECT_HASH_SIZE];
    
    // member functions
    void add_key_value(Json_String_t *json_key, Json_t *json_value);
    
    Json_t *operator[](Json_String_t &json_key);
};


//~ Json_Array_t
struct Json_Array_t
{
    Json_t **data;
    u32 size;
    
    // IMPORANT(fakhri): check for bounds bfore using this!!
    Json_t* &operator[](u64 index){ return data[index]; }
};

// constructor
function Json_Array_t *
make_json_array(u32 size);

//~
struct Json_Number_t
{
    Json_String_t *data;
};

//~ Json_t

enum Json_Value_t
{
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL,
    JSON_NUM,
};

struct Json_t
{
    union
    {
        Json_Number_t *json_number;
        Json_String_t *json_string;
        Json_Object_t *json_object;
        Json_Array_t  *json_array;
    };
    Json_Value_t type;
};

//constructors
function Json_t *
make_json_value_from_json_object(Json_Object_t *json_object);

function Json_t *
make_json_value_from_json_array(Json_Array_t *json_array);

function Json_t *
make_json_value_from_json_string(Json_String_t *json_srting);

function Json_t *
make_json_value_from_json_number(Json_Number_t *json_number);

function Json_t *
make_json_value_from_true();

function Json_t *
make_json_value_from_false();

function Json_t *
make_json_value_from_null();

//~ parsers
struct Parse_Json_Result_t
{
    void *json_data;
    char *json_str;
    b32 valid_return;
};

function Parse_Json_Result_t
parse_json_object(char *json);

function Parse_Json_Result_t
parse_json_string(char *json);

function Parse_Json_Result_t
parse_json_array(char *json);

function Parse_Json_Result_t
parse_json_value(char *json);

function Json_t *
parse_json(char *json);

//~implementation
#include "helper_functions.cpp"
#include "json.cpp"

#endif //JSON_H