#if !defined(HELPER_FUNCTIONS_CPP) && defined(JSON_H)
#define HELPER_FUNCTIONS_CPP

//~Helper internals
internal b32
is_whitespace(char c)
{
    constexpr char carriage_return = 13;
    return (c == ' ' || c == '\n' || c == '\t' || c == carriage_return);
}

internal char *
ignore_whitespaces(char *str)
{
    
    while(*str && is_whitespace(*str))
    {
        consume_characater(str);
    }
    return str;
}

internal char *
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

internal char *
compute_nested_arrays_length(char *json)
{
    if (json == 0) return 0;
    // NOTE(fakhri): when this internal is called, *json holds '[', we need to consume that charecter befor we continue
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

#endif // HELPER_FUNCTIONS_CPP