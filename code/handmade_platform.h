#ifndef HANDMADE_PLATFORM

#include <stdint.h>

//bool32 behaves like in C not like bool in C++:
#define bool32 int32_t

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif 

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif 

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else 
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif


struct ThreadContext{ 
    int placeHolder;
};

struct debugReadFileResult {
    uint32_t contentSize;
    void *fileContent;
};

#define DEBUG_READ_ENTIRE_FILE(name) debugReadFileResult name(ThreadContext *context,char *filename)
typedef DEBUG_READ_ENTIRE_FILE(debug_read_entire_file);
DEBUG_READ_ENTIRE_FILE(DEBUGReadEntireFile);

#define DEBUG_WRITE_ENTIRE_FILE(name) bool32 name(ThreadContext *context,char *filename, \
        uint32_t memorySize, void *memory)
typedef DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file);
DEBUG_WRITE_ENTIRE_FILE(DEBUGWriteEntireFile);

#define DEBUG_FREE_FILE_MEMORY(name) void name(ThreadContext *context,void *memory)
typedef DEBUG_FREE_FILE_MEMORY(debug_free_file_memory);
DEBUG_FREE_FILE_MEMORY(DEBUGFreeFileMemory);

#define HANDMADE_PLATFORM
#endif
