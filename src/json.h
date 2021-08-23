/* date = August 20th 2021 1:48 pm */

#ifndef JSON_H
#define JSON_H

//~general types and definitions
#include "types.h"

//~includes
#include "memory.h"

//~ forward declarations
struct Json_t;
typedef Json_t *Json_Ptr_t;

//~ Json_String_t
struct Json_String_t
{
    char *str;
    u32 length;
    
    // IMPORANT(fakhri): check for bounds before using this!!
    char& operator[](u64 index){ return str[index]; }
    char& operator[](u64 index) const { return str[index]; }
};

typedef Json_String_t *Json_String_Ptr_t;

//constructor
internal Json_String_Ptr_t
make_json_string(char *start_of_key, char *end_of_key);

//~ Json_Object_t
struct Json_Object_Slot_t
{
    Json_String_t *key;
    Json_t *value;
    
    Json_Object_Slot_t *next;
};

typedef Json_Object_Slot_t *Json_Object_Slot_Ptr_t;

// TODO(fakhri): try dynamic size hash table ?
constexpr int JSON_OBJECT_HASH_SIZE = 100;
struct Json_Object_t
{
    Json_Object_Slot_Ptr_t slots[JSON_OBJECT_HASH_SIZE];
    
    // member internals
    void add_key_value(Json_String_Ptr_t json_key, Json_Ptr_t json_value);
    
    Json_Ptr_t operator[](const Json_String_t &json_key);
};

typedef Json_Object_t *Json_Object_Ptr_t;

//~ Json_Array_t
struct Json_Array_t
{
    Json_Ptr_t *data;
    u32 size;
    
    // IMPORANT(fakhri): check for bounds bfore using this!!
    Json_Ptr_t &operator[](u64 index){ return data[index]; }
};

typedef Json_Array_t *Json_Array_Ptr_t;

// constructor
internal Json_Array_Ptr_t
make_json_array(u32 size);

//~
struct Json_Number_t
{
    Json_String_Ptr_t data;
};

typedef Json_Number_t *Json_Number_Ptr_t;

//~ Json_t

enum Json_Value_t
{
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_BOOL,
    JSON_NULL,
    JSON_NUM,
};

struct Json_t
{
    union
    {
        Json_Number_Ptr_t json_number;
        Json_String_Ptr_t json_string;
        Json_Object_Ptr_t json_object;
        Json_Array_Ptr_t  json_array;
        b32               bool_value;
    };
    Json_Value_t type;
    
    // type verification
    b32 is_object();
    b32 is_array();
    b32 is_string();
    b32 is_number();
    b32 is_boolean();
    b32 is_null();
};


//- constructors 
// @Signature : make_json_value_from_json_object (Json_Object_Ptr_t json_object);
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE(json_object, Json_Object_Ptr_t, JSON_OBJECT)

// @make{json_value_from_json_array (@Json_Array_Ptr_t json_array);
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE(json_array, Json_Array_Ptr_t, JSON_ARRAY); 

// @Signature : make_json_value_from_json_string  (Json_String_Ptr_t json_string);
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE(json_string, Json_String_Ptr_t, JSON_STRING); 

// @Signature : make_json_value_from_json_number (Json_Number_Ptr_t json_number);
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE(json_number, Json_Number_Ptr_t, JSON_NUM);

// @Signature : make_json_value_from_true ();
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE_NO_PARAM(true,JSON_BOOL, 1);

// @make{json_value_from_false ();
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE_NO_PARAM(false, JSON_BOOL, 0);

// @Signature : make_json_value_from_null ();
internal Json_Ptr_t
JSON_VALUE_CONSTRUCTOR_DEFINE_NO_PARAM(null, JSON_NULL, 0);

//~ parsers
struct Parse_Json_Result_t
{
    void *json_data;
    char *json_str;
    b32 valid_return;
};

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_object);

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_string);

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_array);

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_value);

//~ API 

//@Docs: parse the json string and return a pointer to generic json type that holds the parsed result, the actual type of the json can be queried, and then access the appropriate data, if you already know what the type of the json is then it is better to use other specific internal to get you the right type from the beginning
//@Signature: start_parsing_json_any(char *json)
internal Json_Ptr_t
API_FUNCTION_DECLARE(any);

//@Docs: parse json string and return a json object type 
//@Signature: start_parsing_json_object(char *json)
internal Json_Object_Ptr_t
API_FUNCTION_DEFINE(object)

//@Docs: parse json string and return a json array type 
//@Signature: start_parsing_json_array(char *json)
internal Json_Array_Ptr_t
API_FUNCTION_DEFINE(array)

//~implementation
#include "helper_functions.cpp"
#include "json.cpp"

#endif //JSON_H