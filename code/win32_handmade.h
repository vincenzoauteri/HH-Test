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
#endif
