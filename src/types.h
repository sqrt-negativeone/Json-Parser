/* date = August 23rd 2021 10:53 am */

#ifndef TYPES_H
#define TYPES_H

using u64 = unsigned long long int;
using u32 = unsigned int;
using b32 = int;
using u8 = unsigned char;

#define consume_characater(str) ++(str)
#define global static
#define internal static

#define AssertBreak  (*((u32*)0) = 0x1234);
#define AssertTrue(expr) if (!(expr)) { AssertBreak; }


#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)


//- json string literal constructor
#define json_string_lit(str_lit) { (str_lit), sizeof(str_lit) - 1}

//- json parser signatures
#define JSON_PARSER_SIG(name)  name(char *json)

//~ helper defines 
//- json value constructors signature and definition
#define JSON_VALUE_CONSTRUCTOR_DECLARE(name, arg_type) make_json_value_from_##name(arg_type name)
#define JSON_VALUE_CONSTRUCTOR_DECLARE_NO_PARAM(name) make_json_value_from_##name()

#define JSON_VALUE_CONSTRUCTOR_BODY(name, enum_type) \
{ \
	Json_Ptr_t json_value = (Json_Ptr_t) reserve_in_main_memory(sizeof(Json_t)); \
	if (json_value) \
	{ \
		json_value->type = enum_type; \
		json_value->name = name; \
	} \
	return json_value; \
}
#define JSON_VALUE_CONSTRUCTOR_BODY_NO_PARAM(name, enum_type, val) \
{ \
	Json_Ptr_t json_value = (Json_Ptr_t) reserve_in_main_memory(sizeof(Json_t)); \
	if (json_value) \
	{ \
		json_value->type = enum_type; \
		json_value->bool_value = val; \
	} \
	return json_value; \
}

#define JSON_VALUE_CONSTRUCTOR_DEFINE(name, arg_type, enum_type) \
JSON_VALUE_CONSTRUCTOR_DECLARE(name, arg_type) JSON_VALUE_CONSTRUCTOR_BODY(name, enum_type)

#define JSON_VALUE_CONSTRUCTOR_DEFINE_NO_PARAM(name, enum_type, val) \
JSON_VALUE_CONSTRUCTOR_DECLARE_NO_PARAM(name) JSON_VALUE_CONSTRUCTOR_BODY_NO_PARAM(name, enum_type, val)


//- parser API signatures and defintions
#define API_FUNCTION_DECLARE(name) start_parsing_json_##name(char *json)

#define API_FUNCTION_BODY(type) \
{\
	Parse_Json_Result_t parse_result = parse_json_value(json); \
	if (parse_result.valid_return) \
	{ \
		Json_Ptr_t json_value = (Json_Ptr_t)parse_result.json_data; \
		if (json_value && json_value->is_##type()) \
		{ \
			AssertTrue(is_stack_empty()); \
			return json_value->json_##type; \
		} \
	} \
	return 0; \
}

#define API_FUNCTION_DEFINE(api_internal) API_FUNCTION_DECLARE(api_internal) API_FUNCTION_BODY(api_internal)

#endif //TYPES_H
