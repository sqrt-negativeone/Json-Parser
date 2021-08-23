#include "src/json.h"

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
        }
        else
        {
            return 1;
        }
        if (fh != NULL) fclose(fh);
    }
    else return 1;
    
    auto result = start_parsing_json_object(json_to_parse);
    if (result)
    {
        auto glossary = (*result)[json_string_lit("glossary")];
        if (glossary && glossary->is_object())
        {
            auto title = (*glossary->json_object)[json_string_lit("title")];
            if (title && title->is_string())
            {
                printf("glossary->title : \"%s\"\n", title->json_string->str);
            }
        }
    }
    printf("%s", json_to_parse);
    
    return 0;
}