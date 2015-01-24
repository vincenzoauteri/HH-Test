#ifndef HANDMADE_H

#include "handmade_platform.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "handmade_tilemap.h"
#include "handmade_math.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))

#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))

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

            GameButtonState start;
            GameButtonState back;
        };
    };
};

struct GameInput {
    GameButtonState mouseButtons[5];
    float dtForFrame;
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

struct MemoryArena {
    size_t size ;
    uint8_t *base;
    size_t used;
};

struct World {
    TileMap *tileMap;
};

struct LoadedBitmap {
    uint32_t width;
    uint32_t height;
    uint32_t *pixels;
};

struct HeroBitmaps {
    float alignX;
    float alignY;

    LoadedBitmap head;
    LoadedBitmap torso;
    LoadedBitmap cape;
};

struct Entity{
    bool32 exists;
    TileMapPosition position;
    V2 dP;
    float width;
    float height;
    uint32_t facingDirection;
};

struct GameState {
    MemoryArena worldArena;
    World *world;
    uint32_t cameraFollowingEntityIndex;
    TileMapPosition cameraP;

    LoadedBitmap backDrop;
    HeroBitmaps heroBitmaps[4];
    uint32_t playerIndexForController[ARRAY_COUNT(((GameInput*)0)->controllers)];
    uint32_t entityCount;
    Entity entities[256];
};

#define pushStruct(arena, type) (type *)pushSize_(arena,sizeof(type))
#define pushArray(arena, count, type) \
    (type *)pushSize_(arena,(count)*sizeof(type))
void *pushSize_(MemoryArena *arena, size_t size) 
{
    ASSERT((arena->used + size) <= arena->size );
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}


#define HANDMADE_H
#endif
