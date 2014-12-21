#ifndef W32HANDMADE_H
#define W32HANDMADE_H 

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>


struct win32offscreenBuffer {
    BITMAPINFO info;
    void *bitmapMemory;
    int width; 
    int height; 
    int bytesPerPixel ;  
    int pitch;  
};

struct win32windowDimensions{
    int width;
    int height;
};

struct win32_soundOutput {
    int samplesPerSecond ;
    uint32_t runningSampleIndex ;
    int wavePeriod ;
    int bytesPerSample ;
    int secondaryBufferSize ;
    DWORD safetyBytes;
    float tSine;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_ReplayBuffer 
{
    HANDLE fileHandle;
    HANDLE memoryMap;
    char fileName[WIN32_STATE_FILE_NAME_COUNT];
    void *memoryBlock;
};
struct win32_State{
    uint64_t totalSize;
    void *gameMemoryBlock; 
    win32_ReplayBuffer replayBuffers[4];

    HANDLE recordingHandle;
    HANDLE playbackHandle;
    int inputRecordingIndex;
    int inputPlayingIndex;
    char fileName[WIN32_STATE_FILE_NAME_COUNT];
    char *lastSlash ; 
};

#ifdef DEBUG_MODE 

struct win32DebugTimeMarker {
    DWORD outputPlayCursor;
    DWORD outputWriteCursor;
    DWORD outputLocation;
    DWORD outputByteCount;
    DWORD expectedFrameBoundary;
    DWORD flipPlayCursor;
    DWORD flipWriteCursor;
};


#endif 

#endif
