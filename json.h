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

#define AssertBreak(m)  (*((u32*)0) = 0x1234);
#define AssertTrue(expr) if (!(expr)) { AssertBreak(expr) }


#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)

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

//~memory management
struct Memory_t
{
    u8 *memory;
    u8 *stack_memory;
    
    u32 capacity;
    u32 available_bytes;
    
    u32 bytes_reserved_in_main_memory;
    u32 bytes_reserved_in_stack_memory;
};

global Memory_t global_memory = {};

//~main memory functions
function
void *reserve_in_main_memory(u64 bytes_to_reserve)
{
    void *result = 0;
    if (global_memory.available_bytes >= bytes_to_reserve)
    {
        result = (void*) (global_memory.memory + global_memory.bytes_reserved_in_main_memory);
        global_memory.bytes_reserved_in_main_memory += bytes_to_reserve;
        global_memory.available_bytes -= bytes_to_reserve;
    }
    
    AssertTrue((global_memory.bytes_reserved_in_main_memory +
                global_memory.bytes_reserved_in_stack_memory +
                global_memory.available_bytes) == global_memory.capacity);
    
    return result;
}


//~stack functions
// NOTE(fakhri): the stack will keep temporare informations about the length of json arrays
function void
setup_stack()
{
    global_memory.stack_memory = global_memory.memory + global_memory.capacity;
}

function
void *reserve_in_stack(u64 bytes_to_reserve)
{
    void *result = 0;
    if (global_memory.available_bytes >= bytes_to_reserve)
    {
        global_memory.stack_memory -= bytes_to_reserve;
        global_memory.bytes_reserved_in_stack_memory += bytes_to_reserve;
        global_memory.available_bytes -= bytes_to_reserve;
        result = (void*) global_memory.stack_memory;
    }
    AssertTrue((global_memory.bytes_reserved_in_main_memory +
                global_memory.bytes_reserved_in_stack_memory +
                global_memory.available_bytes) == global_memory.capacity);
    return result;
}

function
b32 free_from_stack(u64 bytes_to_free)
{
    b32 success = 0;
    if (global_memory.bytes_reserved_in_stack_memory >= bytes_to_free)
    {
        success = 1;
        global_memory.available_bytes += bytes_to_free;
        global_memory.stack_memory += bytes_to_free;
        global_memory.bytes_reserved_in_stack_memory -= bytes_to_free;
    }
    AssertTrue((global_memory.bytes_reserved_in_main_memory +
                global_memory.bytes_reserved_in_stack_memory +
                global_memory.available_bytes) == global_memory.capacity);
    return success;
}

function
b32 is_stack_empty()
{
    return (global_memory.bytes_reserved_in_stack_memory == 0);
}

function
b32 clear_stack()
{
    return free_from_stack(global_memory.bytes_reserved_in_stack_memory);
}

function
b32 push_integer_into_stack(u32 value)
{
    int *value_ptr = (int *)reserve_in_stack(sizeof(u32));
    b32 success = 0;
    if (value_ptr)
    {
        success = 1;
        *value_ptr = value;
    }
    return success;
}

function
b32 pop_integer_from_stack(u32 *dest)
{
    b32 success = 0;
    if (global_memory.bytes_reserved_in_stack_memory >= sizeof(u32))
    {
        success = 1;
        *dest = *((u32*)global_memory.stack_memory);
        success = free_from_stack(sizeof(u32));
    }
    return success;
}

//~ global memory setup
function
void setup_memory(u8 *buffer, u32 size)
{
    global_memory.memory = buffer;
    global_memory.capacity = size;
    global_memory.available_bytes = size;
    setup_stack();
}


//~implementations
//Helper functions
function b32
is_whitespace(char c)
{
    constexpr char carriage_return = 13;
    return (c == ' ' || c == '\n' || c == '\t' || c == carriage_return);
}

function char *
ignore_whitespaces(char *str)
{
    
    while(*str && is_whitespace(*str))
    {
        consume_characater(str);
    }
    return str;
}

function char *
compare_json_str(char *json, char *pattern)
{
    while(*pattern && *pattern == *json)
    {
        consume_characater(pattern);
        consume_characater(json);
    }
    if (*pattern)
    {
        return 0;
    }
    return json;
}

function char *
compute_nested_arrays_length(char *json)
{
    if (json == 0) return 0;
    // NOTE(fakhri): when this function is called, *json holds '[', we need to consume that charecter befor we continue
    AssertTrue(*json == '[');
    consume_characater(json);
    json = ignore_whitespaces(json);
    if (*json == ']')
    {
        // NOTE(fakhri): empty array
        push_integer_into_stack(0);
        return json;
    }
    u32 array_size = 1;
    while(*json && *json != ']')
    {
        if (*json == '[')
        {
            json = compute_nested_arrays_length(json);
            AssertTrue(*json == ']');
            if (!json)
            {
                return 0;
            }
        }
        else if (*json == ',')
        {
            ++array_size;
        }
        consume_characater(json);
    }
    
    if (*json == 0 || !push_integer_into_stack(array_size))
    {
        return 0;
    }
    return json;
}

//~Json_String_t
function Json_String_t *
make_json_string(char *begin, char *end)
{
    Json_String_t *json_string = (Json_String_t *)reserve_in_main_memory(sizeof(Json_String_t));
    if (!json_string)
    {
        return 0;
    }
    *json_string = {};
    
    json_string->length = (u32)(end - begin);
    json_string->str = (char*) reserve_in_main_memory(json_string->length + 1);
    
    char *json_string_current_char = json_string->str;
    for (char *current_char = begin; current_char != end; ++current_char)
    {
        b32 should_copy = *current_char != '\\';
        
        if (should_copy)
        {
            *json_string_current_char = *current_char;
            ++json_string_current_char;
        }
    }
    *json_string_current_char = 0;
    return json_string;
}

//~Json_Object_t
// djb2 hash function from http://www.cse.yorku.ca/~oz/hash.html
function u32
hash_function(Json_String_t *json_string)
{
    u32 hash = 5381;
    u32 c = 0;
    
    u8 *str = (u8 *)json_string->str;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash % JSON_OBJECT_HASH_SIZE;
}

void
Json_Object_t::add_key_value(Json_String_t *json_key, Json_t *json_value)
{
    Json_Object_Slot_t *json_object_item = (Json_Object_Slot_t*) reserve_in_main_memory(sizeof(Json_Object_Slot_t));
    json_object_item->key = json_key;
    json_object_item->value = json_value;
    
    // NOTE(fakhri): calculate the hash
    u32 hash_value = hash_function(json_key);
    
    // NOTE(fakhri): insert the value
    json_object_item->next = this->slots[hash_value];
    this->slots[hash_value] = json_object_item;
}

Json_t *
Json_Object_t::operator[](Json_String_t &json_key)
{
    u32 hash_value = hash_function(&json_key);
    Json_Object_Slot_t *json_item = this->slots[hash_value];
    while(json_item)
    {
        if (*json_item->key == json_key)
        {
            return json_item->value;
        }
        json_item = json_item->next;
    }
    return 0;
}

//~Json_Array_t
function Json_Array_t *
make_json_array(u32 size)
{
    Json_Array_t *json_array = (Json_Array_t*)reserve_in_main_memory(sizeof(Json_Array_t));
    if (json_array)
    {
        json_array->size = size;
        json_array->data = (Json_t **)reserve_in_main_memory(size * sizeof(Json_t*));
        if (!json_array->data)
        {
            // NOTE(fakhri): probably not enough memory
            AssertBreak(1);
            return 0;
        }
        *json_array->data = {};
    }
    return json_array;
}

//~ Json_t

function Json_t *
make_json_value_from_json_object(Json_Object_t *json_object)
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_OBJECT;
        json_value->json_object = json_object;
    }
    return json_value;
}

function Json_t *
make_json_value_from_json_array(Json_Array_t *json_array)
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_ARRAY;
        json_value->json_array = json_array;
    }
    return json_value;
}

function Json_t *
make_json_value_from_json_string(Json_String_t *json_string)
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_STRING;
        json_value->json_string = json_string;
    }
    return json_value;
}

function Json_t *
make_json_value_from_json_number(Json_Number_t *json_number)
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_NUM;
        json_value->json_number = json_number;
    }
    return json_value;
}

function Json_t *
make_json_value_from_true()
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_TRUE;
    }
    return json_value;
}

function Json_t *
make_json_value_from_false()
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_FALSE;
    }
    return json_value;
}

function Json_t *
make_json_value_from_null()
{
    Json_t *json_value = (Json_t *) reserve_in_main_memory(sizeof(Json_t));
    if (json_value)
    {
        json_value->type = JSON_NULL;
    }
    return json_value;
}

//~Parsers
function Parse_Json_Result_t
parse_json_array(char *json)
{
    Json_Array_t *json_array = 0;
    Parse_Json_Result_t result = {};
    result.valid_return = 1;
    u32 array_index = 0;
    
    if (is_stack_empty())
    {
        // NOTE(fakhri): compute the size of json array (and any json array nested inside it recursively)
        //and store the results in the stack memory
        if (!compute_nested_arrays_length(json - 1))
        {
            result.valid_return = 0;
            return result;
        }
    }
    
    // NOTE(fakhri): get json array length and reserve memory for it
    {
        u32 json_array_length;
        if (!pop_integer_from_stack(&json_array_length))
        {
            AssertTrue(0);
        }
        // NOTE(fakhri): if this error fires then something weird happen
        json_array = make_json_array(json_array_length);
        if (!json_array)
        {
            result.valid_return = 0;
            return result;
        }
    }
    
    json = ignore_whitespaces(json);
    while(true)
    {
        if (*json == 0)
        {
            result.valid_return = 0;
            return result;
        }
        else if (*json == ']')
        {
            // NOTE(fakhri): json array well parsed
            consume_characater(json);
            break;
        }
        else if (*json == ',')
        {
            // NOTE(fakhri): more json values are coming
            consume_characater(json);
        }
        else
        {
            // NOTE(fakhri): json value
            Json_t *json_value = 0;
            // NOTE(fakhri): parse result
            {
                Parse_Json_Result_t parse_result = parse_json_value(json);
                if (parse_result.valid_return)
                {
                    json_value = (Json_t *)parse_result.json_data;
                    json = parse_result.json_str;
                }
                else
                {
                    result.valid_return = 0;
                    return result;
                }
            }
            
            if (!json_value)
            {
                result.valid_return = 0;
                return result;
            }
            
            // insert json value
            json_array->data[array_index++] = json_value;
        }
    }
    result.json_data = (void *)json_array;
    result.json_str = json;
    return result;
}

function Parse_Json_Result_t
parse_json_object(char *json)
{
    
    Json_Object_t *json_object = (Json_Object_t *)reserve_in_main_memory(sizeof(Json_Object_t));
    
    Parse_Json_Result_t result = {};
    result.valid_return = 1;
    
    while(true)
    {
        json = ignore_whitespaces(json);
        // parse key/values
        // NOTE(fakhri): keys are strings
        if (*json == '"')
        {
            // NOTE(fakhri): string, it's a key
            consume_characater(json);
            Json_String_t *json_key = 0;
            // NOTE(fakhri): parse key string
            {
                Parse_Json_Result_t parse_result = parse_json_string(json);
                if (parse_result.valid_return)
                {
                    json_key = (Json_String_t *)parse_result.json_data;
                    json = parse_result.json_str;
                }
                else
                {
                    result.valid_return = 0;
                    return result;
                }
            }
            AssertTrue(*json == '"');
            consume_characater(json);
            json = ignore_whitespaces(json);
            if (*json == ':')
            {
                // NOTE(fakhri): start of a value
                consume_characater(json);
                // NOTE(fakhri): parse the value
                Json_t *json_value;
                
                // NOTE(fakhri): parse results
                {
                    Parse_Json_Result_t parse_result = parse_json_value(json);
                    if (parse_result.valid_return)
                    {
                        json_value = (Json_t *)parse_result.json_data;
                        json = parse_result.json_str;
                    }
                    else
                    {
                        result.valid_return = 0;
                        return result;
                    }
                }
                
                json_object->add_key_value(json_key, json_value);
                if (!json_object)
                {
                    result.valid_return = 0;
                    return result;
                }
                if (*json == ',')
                {
                    // more key/values to parse
                    consume_characater(json);
                }
                else if (*json == '}')
                {
                    // NOTE(fakhri): json object is well parsed
                    consume_characater(json);
                    break;
                }
                else
                {
                    // NOTE(fakhri): json is not well formed
                    result.valid_return = 0;
                    return result;
                }
            }
            else
            {
                // NOTE(fakhri): json not well formed
                result.valid_return = 0;
                return result;
            }
        }
        else if (*json == '}')
        {
            consume_characater(json);
            break;
        }
        else
        {
            // NOTE(fakhri): json not well formed
            result.valid_return = 0;
            return result;
        }
    }
    result.json_data = (void *)json_object;
    result.json_str = json;
    return result;
}

function Parse_Json_Result_t
parse_json_string(char *json)
{
    Parse_Json_Result_t result = {};
    result.valid_return = 1;
    
    char *start_of_string = json;
    char *end_of_string = 0;
    // NOTE(fakhri): find the end of the key string
    while(*json && *json != '"')
    {
        consume_characater(json);
    }
    
    if (*json == 0)
    {
        // NOTE(fakhri): json is not well formed
        result.valid_return = 0;
        return result;
    }
    end_of_string = json;
    
    Json_String_t *json_string = make_json_string(start_of_string, end_of_string);
    if (!json_string)
    {
        // NOTE(fakhri): json is not well formed
        result.valid_return = 0;
        return result;
    }
    result.json_data = (void*) json_string;
    result.json_str = json;
    return result;
}

function Parse_Json_Result_t
parse_json_value(char *json)
{
    Json_t *json_value_result = 0;
    Parse_Json_Result_t result = {};
    result.valid_return = 1;
    json = ignore_whitespaces(json);
    switch(*json)
    {
        case '{' :
        {
            consume_characater(json);
            Json_Object_t *json_object;
            
            // NOTE(fakhri): parse json object
            {
                Parse_Json_Result_t parse_result = parse_json_object(json);
                if (parse_result.valid_return)
                {
                    json_object = (Json_Object_t *)parse_result.json_data;
                    json = parse_result.json_str;
                }
                else
                {
                    result.valid_return = 0;
                    return result;
                }
            }
            
            json_value_result = make_json_value_from_json_object(json_object);
        } break;
        case '[':
        {
            // NOTE(fakhri): start of a json array
            consume_characater(json);
            Json_Array_t *json_array;
            
            // NOTE(fakhri): parse json array
            {
                Parse_Json_Result_t parse_result = parse_json_array(json);
                if (parse_result.valid_return)
                {
                    json_array = (Json_Array_t *)parse_result.json_data;
                    json = parse_result.json_str;
                }
                else
                {
                    result.valid_return = 0;
                    return result;
                }
            }
            
            json_value_result = make_json_value_from_json_array(json_array);
        } break;
        case '"':
        {
            // NOTE(fakhri): start of string
            consume_characater(json);
            Json_String_t *json_string = 0;
            
            // NOTE(fakhri): parse json string
            {
                Parse_Json_Result_t parse_result = parse_json_string(json);
                if (parse_result.valid_return)
                {
                    json_string = (Json_String_t *)parse_result.json_data;
                    json = parse_result.json_str;
                }
                else
                {
                    result.valid_return = 0;
                    return result;
                }
            }
            consume_characater(json);
            json_value_result = make_json_value_from_json_string(json_string);
        } break;
        case 't':
        {
            // NOTE(fakhri): true value
            char *true_str = "true";
            json = compare_json_str(json, true_str);
            if (!json)
            {
                result.valid_return = 0;
                return result;
            }
            json_value_result = make_json_value_from_true();
        } break;
        case 'f':
        {
            // NOTE(fakhri): false value
            char *false_str = "false";
            json = compare_json_str(json, false_str);
            if (!json)
            {
                result.valid_return = 0;
                return result;
            }
            json_value_result = make_json_value_from_false();
        } break;
        case 'n':
        {
            // NOTE(fakhri): null value
            char *null_str = "null";
            json = compare_json_str(json, null_str);
            if (!json)
            {
                result.valid_return = 0;
                return result;
            }
            json_value_result = make_json_value_from_null();
        } break;
        default :
        {
            // TODO(fakhri): parse number or report failure if can't parse
            // NOTE(fakhri): this is for debug only
            char *begin_number = json;
            while(*json && !is_whitespace(*json))
            {
                consume_characater(json);
            }
            Json_String_t *number = make_json_string(begin_number, json);
            Json_Number_t *json_number = (Json_Number_t *)reserve_in_main_memory(sizeof(Json_Number_t));
            if (json_number && number)
            {
                json_number->data = number;
            }
            else
            {
                result.valid_return = 0;
                return result;
            }
            json_value_result = make_json_value_from_json_number(json_number);
        }
    }
    json = ignore_whitespaces(json);
    
    
    result.json_data = (void *)json_value_result;
    result.json_str = json;
    
    return result;
}

function Json_t *
parse_json(char *json)
{
    Parse_Json_Result_t parse_result = parse_json_value(json);
    Json_t *result = 0;
    if (parse_result.valid_return)
    {
        result = (Json_t *)parse_result.json_data;
    }
    return result;
}

#endif //JSON_H
