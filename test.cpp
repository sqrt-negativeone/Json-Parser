#include "json.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// TODO(fakhri): do some profilings and optimizations, this is just a quick first implementation
// TODO(fakhri): do some stress tests 

int main()
{
    //NOTE(fakhri): set up the memory
    {
        u64 memory_size = MB(1);
        u8 *buffer = (u8 *)malloc(memory_size);
        setup_memory(buffer, memory_size);
    }
    
    char *json_to_parse;
    
    FILE *fh = fopen("test.json", "r");
    if ( fh != NULL )
    {
        fseek(fh, 0L, SEEK_END);
        long file_size = ftell(fh);
        rewind(fh);
        json_to_parse = (char *)malloc(file_size);
        if ( json_to_parse != NULL )
        {
            fread(json_to_parse, file_size, 1, fh);
            fclose(fh); fh = 0;
            
            auto result = parse_json(json_to_parse);
            if (result->type == JSON_OBJECT)
            {
                Json_Object_t &json_object = *result->json_object;
                char key[] = "glossary";
                Json_String_t json_string;
                json_string.str = key;
                json_string.length = sizeof(key) / sizeof(char) - 1;
                Json_t *json_value = json_object[json_string];
                if (json_value)
                {
                    AssertTrue(json_value->type == JSON_OBJECT);
                }
            }
            printf("%s", json_to_parse);
        }
        if (fh != NULL) fclose(fh);
    }
    
    
    return 0;
}