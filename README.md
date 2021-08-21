# Json-Parser
A single header C++ Json parser written in C-like style, implemented from scratch for learning and for fun with no use of any libraries.

the parser uses no operating system memory allocations, the caller should provide a buffer with enough size, and the parser will use this buffer for all its memory allocations.
this means that the parser will use no more memory than what you provided, and freeing the parser memory is simply freeing the initial buffer

## TODOs (for now)

- [ ] parse numbers
- [ ] figure out how much memory is approximately needed for the parser as a function of the size of json file
- [ ] optimize the parser performance
- [ ] stress tests
- [ ] close all the TODOs in the source file (this one just doesn't end :p )
