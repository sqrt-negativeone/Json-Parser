/* date = August 22nd 2021 8:31 pm */

#ifndef MEMORY_H
#define MEMORY_H

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

//~main memory internals
internal
void *reserve_in_main_memory(u64 bytes_to_reserve)
{
    // TODO(fakhri): good memory alignmentas
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


//~stack internals
// NOTE(fakhri): the stack will keep temporare informations about the length of json arrays

internal
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

internal
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

internal
b32 is_stack_empty()
{
    return (global_memory.bytes_reserved_in_stack_memory == 0);
}

internal
b32 clear_stack()
{
    return free_from_stack(global_memory.bytes_reserved_in_stack_memory);
}

internal
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

internal
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
internal void
setup_stack()
{
    global_memory.stack_memory = global_memory.memory + global_memory.capacity;
}

internal
void setup_memory(u8 *buffer, u32 size)
{
    global_memory.memory = buffer;
    global_memory.capacity = size;
    global_memory.available_bytes = size;
    setup_stack();
}


#endif //MEMORY_H
