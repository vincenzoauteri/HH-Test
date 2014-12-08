#ifndef HANDMADE_H

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

inline uint32_t safeTruncateUint64 (uint64_t value) {
    ASSERT(value<= 0xFFFFFFFF);
    uint32_t result = (uint32_t) value;
    return result;
}

//these two fields define a bitmap memory area (dib) for windows
struct FrameBuffer {
    void *bitmapMemory;
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


struct Input{
    bool32 isAnalog;
    float startX;
    float startY;
    float endX;
    float endY;
    float minX;
    float minY;
    float maxX;
    float maxY;

    union {
        GameButtonState buttons[6];
        struct {
            GameButtonState up;
            GameButtonState down;
            GameButtonState left;
            GameButtonState right;
            GameButtonState leftShoulder;
            GameButtonState rightShoulder;
        };
    };
};

struct GameInput {
    Input controllers[4];
};

struct GameMemory{
    bool32 isInitialized;

    uint64_t permanentStorageSize;
    void *permanentStorage;

    uint64_t transientStorageSize;
    void *transientStorage;
};

struct GameState {
      int toneHz;
      int offsetX;
      int offsetY;
};

    

static void RenderWeirdGradient(FrameBuffer *buffer,int offsetX,int offsetY);

static void GameUpdateAndRender(GameMemory *gameMemory,FrameBuffer *buffer, SoundBuffer *soundBuffer,GameInput *gameInput);

#define HANDMADE_H
#endif
