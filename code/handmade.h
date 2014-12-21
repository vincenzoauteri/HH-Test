#ifndef HANDMADE_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

//bool32 behaves like in C not like bool in C++:
#define bool32 int32_t
#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))

#define TERABYTES(tb) ((uint64_t)1024*1024*1024*1024*tb)
#define GIGABYTES(gb) ((uint64_t)1024*1024*1024*gb)
#define MEGABYTES(mb) ((uint64_t)1024*1024*mb)
#define KILOBYTES(kb) ((uint64_t)1024*kb)

#ifdef DEBUG_MODE
#define ASSERT(expression)  \
    if (!(expression)) {*(int*)0 = 0;}
#else
#define ASSERT(expression)  
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

inline int32_t safeTruncateUint64(uint64_t value) {
    ASSERT(value<= 0xFFFFFFFF);
    uint32_t result = (uint32_t) value;
    return result;
}

//these two fields define a bitmap memory area (dib) for windows
struct FrameBuffer {
    void *bitmapMemory;
    int bytesPerPixel; 
    int width; 
    int height; 
    int pitch;  
};

//these two fields define a bitmap memory area (dib) for windows

struct SoundBuffer{
    int sampleCount;
    int samplesPerSecond;
    int16_t *samples;  
};

struct GameButtonState {
   int halfTransitionCount; 
   bool32 endedDown;
};


struct GameControllerInput{
    bool32 isAnalog;
    bool32 isConnected;
    float stickAverageX;
    float stickAverageY;

    union {
        GameButtonState buttons[10];
        struct {
            GameButtonState moveUp;
            GameButtonState moveDown;
            GameButtonState moveLeft;
            GameButtonState moveRight;

            GameButtonState actionUp;
            GameButtonState actionDown;
            GameButtonState actionLeft;
            GameButtonState actionRight;

            GameButtonState leftShoulder;
            GameButtonState rightShoulder;
        };
    };
};

struct GameInput {
    GameButtonState mouseButtons[5];
    int32_t mouseX;
    int32_t mouseY;
    int32_t mouseZ;
    GameControllerInput controllers[5];
};


struct GameMemory{
    bool32 isInitialized;

    uint64_t permanentStorageSize;
    void *permanentStorage;

    uint64_t transientStorageSize;
    void *transientStorage;
    debug_read_entire_file *DEBUGReadEntireFile;
    debug_write_entire_file *DEBUGWriteEntireFile;
    debug_free_file_memory *DEBUGFreeFileMemory;
};

struct GameState {
      int toneHz;
      float tSine;
      int offsetX;
      int offsetY;
      int playerX;
      int playerY;
      float tJumpTimer;
};

inline GameControllerInput *getController(GameInput *input, int controllerIndex){
    ASSERT(controllerIndex < ARRAY_COUNT(input->controllers));
    GameControllerInput *gameControllerInput = &input->controllers[controllerIndex];
    return gameControllerInput;
}
    

void RenderWeirdGradient(FrameBuffer *buffer,int offsetX,int offsetY);

#define GAME_UPDATE_AND_RENDER(name) void name(ThreadContext *context,GameMemory *gameMemory,FrameBuffer *buffer ,GameInput *gameInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
//Function stubs


#define GAME_GET_SOUND_SAMPLES(name) void name(ThreadContext *context,GameMemory *gameMemory, SoundBuffer *soundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
//Function stubs



#define HANDMADE_H
#endif
