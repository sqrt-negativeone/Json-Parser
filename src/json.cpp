#if !defined(JSON_CPP) && defined(JSON_H)
#define JSON_CPP

//~Json_String_t

b32 operator==(const Json_String_t &lhs, const Json_String_t &rhs)
{
    if (lhs.length !=rhs.length) return 0;
    u32 i = 0;
    u32 len = lhs.length;
    for (i = 0; i < len && (lhs[i] == rhs[i]); ++i);
    return (i == len);
}

internal Json_String_Ptr_t
make_json_string(char *begin, char *end)
{
    Json_String_Ptr_t json_string = (Json_String_Ptr_t)reserve_in_main_memory(sizeof(Json_String_t));
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
// djb2 hash internal from http://www.cse.yorku.ca/~oz/hash.html
internal u32
hash_internal(const Json_String_t &json_string)
{
    u32 hash = 5381;
    u32 c = 0;
    
    u8 *str = (u8 *)json_string.str;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash % JSON_OBJECT_HASH_SIZE;
}

void
Json_Object_t::add_key_value(Json_String_Ptr_t json_key, Json_Ptr_t json_value)
{
    Json_Object_Slot_Ptr_t json_object_item = (Json_Object_Slot_Ptr_t) reserve_in_main_memory(sizeof(Json_Object_Slot_t));
    json_object_item->key = json_key;
    json_object_item->value = json_value;
    
    // NOTE(fakhri): calculate the hash
    u32 hash_value = hash_internal(*json_key);
    
    // NOTE(fakhri): insert the value
    json_object_item->next = this->slots[hash_value];
    this->slots[hash_value] = json_object_item;
}

Json_Ptr_t
Json_Object_t::operator[](const Json_String_t &json_key)
{
    u32 hash_value = hash_internal(json_key);
    Json_Object_Slot_Ptr_t json_item = this->slots[hash_value];
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
internal Json_Array_Ptr_t
make_json_array(u32 size)
{
    Json_Array_Ptr_t json_array = (Json_Array_Ptr_t)reserve_in_main_memory(sizeof(Json_Array_t));
    if (json_array)
    {
        json_array->size = size;
        json_array->data = (Json_Ptr_t *)reserve_in_main_memory(size * sizeof(Json_Ptr_t));
        if (!json_array->data)
        {
            // NOTE(fakhri): probably not enough memory
            AssertBreak;
            return 0;
        }
        *json_array->data = {};
    }
    return json_array;
}

//~ Json_t
inline b32 
Json_t::is_object()
{
    return type == JSON_OBJECT;
}

inline b32
Json_t::is_array()
{
    return type == JSON_ARRAY;
}

inline b32 
Json_t::is_string()
{
    return type == JSON_STRING;
}

inline b32
Json_t::is_number()
{
    return type == JSON_NUM;
}

inline b32
Json_t::is_boolean()
{
    return type == JSON_BOOL;
}

inline b32
Json_t::is_null()
{
    return type == JSON_NULL;
}

//~Parsers
internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_array)
{
    Json_Array_Ptr_t json_array = 0;
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
            Json_Ptr_t json_value = 0;
            // NOTE(fakhri): parse result
            {
                Parse_Json_Result_t parse_result = parse_json_value(json);
                if (parse_result.valid_return)
                {
                    json_value = (Json_Ptr_t)parse_result.json_data;
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

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_object)
{
    
    Json_Object_Ptr_t json_object = (Json_Object_Ptr_t)reserve_in_main_memory(sizeof(Json_Object_t));
    
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
            Json_String_Ptr_t json_key = 0;
            // NOTE(fakhri): parse key string
            {
                Parse_Json_Result_t parse_result = parse_json_string(json);
                if (parse_result.valid_return)
                {
                    json_key = (Json_String_Ptr_t)parse_result.json_data;
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
                Json_Ptr_t json_value;
                
                // NOTE(fakhri): parse results
                {
                    Parse_Json_Result_t parse_result = parse_json_value(json);
                    if (parse_result.valid_return)
                    {
                        json_value = (Json_Ptr_t)parse_result.json_data;
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

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_string)
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
    
    Json_String_Ptr_t json_string = make_json_string(start_of_string, end_of_string);
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

internal Parse_Json_Result_t
JSON_PARSER_SIG(parse_json_value)
{
    Json_Ptr_t json_value_result = 0;
    Parse_Json_Result_t result = {};
    result.valid_return = 1;
    json = ignore_whitespaces(json);
    switch(*json)
    {
        case '{' :
        {
            consume_characater(json);
            Json_Object_Ptr_t json_object;
            
            // NOTE(fakhri): parse json object
            {
                Parse_Json_Result_t parse_result = parse_json_object(json);
                if (parse_result.valid_return)
                {
                    json_object = (Json_Object_Ptr_t)parse_result.json_data;
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
            Json_Array_Ptr_t json_array;
            
            // NOTE(fakhri): parse json array
            {
                Parse_Json_Result_t parse_result = parse_json_array(json);
                if (parse_result.valid_return)
                {
                    json_array = (Json_Array_Ptr_t)parse_result.json_data;
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
            Json_String_Ptr_t json_string = 0;
            
            // NOTE(fakhri): parse json string
            {
                Parse_Json_Result_t parse_result = parse_json_string(json);
                if (parse_result.valid_return)
                {
                    json_string = (Json_String_Ptr_t)parse_result.json_data;
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
            Json_String_Ptr_t number = make_json_string(begin_number, json);
            Json_Number_Ptr_t json_number = (Json_Number_Ptr_t)reserve_in_main_memory(sizeof(Json_Number_t));
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

internal Json_Ptr_t
API_FUNCTION_DECLARE(any)
{
    Parse_Json_Result_t parse_result = parse_json_value(json);
    Json_Ptr_t result = 0;
    if (parse_result.valid_return)
    {
        result = (Json_Ptr_t)parse_result.json_data;
        // NOTE(fakhri): the stack should be empty by now
        AssertTrue(is_stack_empty());
    }
    return result;
}

#endif //JSON_CPP