#ifndef W32HANDMADE_H
#define W32HANDMADE_H 


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
    int sampleLatency;
    float tSine;
};

#ifdef DEBUG_MODE 
struct debugReadFileResult {
    uint32_t contentSize;
    void *fileContent;
};
static debugReadFileResult DEBUGplatformReadEntireFile(char *filename);
static bool32 DEBUGplatformWriteEntireFile(char *filename, uint32_t memorySize, void *memory);
static void DEBUGplatformFreeFileMemory(void *memory);
#endif 

#endif
