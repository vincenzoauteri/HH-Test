#include "handmade.h"
#include "win32_handmade.h"


#define internal static
#define global_variable static
#define local_persist static


DEBUG_FREE_FILE_MEMORY(DEBUGFreeFileMemory)
{
    if(memory){ 
        VirtualFree(memory,0,MEM_RELEASE);
    }
}

DEBUG_READ_ENTIRE_FILE(DEBUGReadEntireFile)
{
    debugReadFileResult result = {};

    HANDLE fileHandle = CreateFileA(
            filename,
            GENERIC_READ,
            FILE_SHARE_READ,
            0,
            OPEN_EXISTING,
            0,
            0);

    if (fileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER  fileSize ; 
        if (GetFileSizeEx(fileHandle,&fileSize)){

            DWORD bytesRead =0;

            //Guard against files  > 4GB
            uint32_t fileSize32 = safeTruncateUint64(fileSize.QuadPart);

            result.fileContent = VirtualAlloc(0,fileSize32,MEM_RESERVE 
                    | MEM_COMMIT,PAGE_READWRITE); 
            if (ReadFile(
                        fileHandle, result.fileContent,fileSize32,&bytesRead,0) 
                    && (fileSize32 == bytesRead)){
                result.contentSize = fileSize32;
            } else {
                //File not read
                DEBUGFreeFileMemory(context,result.fileContent);
                result.fileContent = 0;
                result.contentSize = 0;
            }

        } else {
            //File size not got
        }
        CloseHandle(fileHandle);
    } else {
        //File handle not got
    }
    return result;
}


DEBUG_WRITE_ENTIRE_FILE(DEBUGWriteEntireFile)
{
    bool32 result = false ; 
    HANDLE fileHandle = CreateFileA(
            filename,
            GENERIC_WRITE,
            0,
            0,
            CREATE_ALWAYS,
            0, 
            0);

    if (fileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER  fileSize ; 
        if (GetFileSizeEx(fileHandle,&fileSize)){

            DWORD bytesWritten=0;

            //Guard against files  > 4GB
            if (WriteFile(fileHandle, memory,memorySize,&bytesWritten,0) 
                    && (memorySize== bytesWritten)){
                result=true;
            } else {
                //File not read
            }

        } else {
            //File size not got
        }
        CloseHandle(fileHandle);
    } else {
        //File handle not got
    }
    return result;
}

//GRAPHICS 

static win32offscreenBuffer g_frameBuffer;


static win32windowDimensions win32getWindowDimensions(HWND hwnd) 
{
    win32windowDimensions retDimensions;
    RECT clientRect;
    GetClientRect(hwnd,&clientRect);
    retDimensions.width = clientRect.right - clientRect.left;
    retDimensions.height = clientRect.bottom - clientRect.top;
    return retDimensions;
}

static void Win32ResizeDIBSection(
        win32offscreenBuffer *buffer,int width, int height)
{
    //Free first 

    if (buffer->bitmapMemory) {
        VirtualFree(buffer->bitmapMemory, 0, MEM_RELEASE);
    } 


    buffer->width  =  width; 
    buffer->height =  height; 

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth  = buffer->width;
    //Negative value in biHeighti to get top-left origin of the window
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;
    //Can avoid init to 0 some field because static
    //buffer->info.bmiHeader.biSizeImage = 0;
    //buffer->info.bmiHeader.biXPelsPerMeter = 0;
    //buffer->info.bmiHeader.biYPelsPerMeter = 0;
    //buffer->info.bmiHeader.biCrlUsed = 0;
    //buffer->info.bmiHeader.biCrlImportant = 0;


    buffer->bytesPerPixel = 4;
    buffer->pitch = buffer->width*buffer->bytesPerPixel;

    int bitmapMemorySize =  buffer->bytesPerPixel*width*height;
    //Alloc memory. VirtualAlloc will pad the memory reserved to align size 4k 
    buffer->bitmapMemory = VirtualAlloc(
            0,bitmapMemorySize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
}

static void Win32displayBufferInWindow(
        win32offscreenBuffer *buffer, HDC deviceContext, int width, int height)
{

    //TODO Aspect ratio correction pending.
    int offsetX = 10;
    int offsetY = 10;

    PatBlt(deviceContext, 0,0, 
            width, 
            offsetY,
            BLACKNESS);

    PatBlt(deviceContext, 0,offsetY + buffer->height, 
            width, 
            height,
            BLACKNESS);

    PatBlt(deviceContext, 0,0,
            offsetX, 
            height,
            BLACKNESS);
    PatBlt(deviceContext, 
            offsetX + buffer->width,
            0,
            width, 
            height, 
            BLACKNESS);
    //StretchDIBits stretches our backbuffer up/down to the actual window's 
    //dimensions                                                     n
    StretchDIBits(
            deviceContext,
            /*
               top,left,width,height,
               top,left,width,height,
               */
            offsetX,offsetY,buffer->width,buffer->height,
            0,0,buffer->width,buffer->height,
            buffer->bitmapMemory,
            &buffer->info,
            DIB_RGB_COLORS,
            SRCCOPY);
}
//END GRAPHICS 
//INPUT
//Pointer to function
#define X_INPUT_GET_STATE(name) \
    DWORD WINAPI name(DWORD  dwUserIndex,  XINPUT_STATE* pState )
typedef X_INPUT_GET_STATE(x_input_get_state);
//Function stubs
X_INPUT_GET_STATE(xInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state *pXinputGetState = xInputGetStateStub;
//Alias fot the function pointer 
#define XInputGetState pXinputGetState

//Pointer to function
#define X_INPUT_SET_STATE(name) \
    DWORD WINAPI name(DWORD  dwUserIndex,XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
//Function stubs
X_INPUT_SET_STATE(xInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state *pXinputSetState = xInputSetStateStub;
//Alias fot the function pointer 
#define XInputSetState pXinputSetState

static void win32LoadInput(void) 
{
    HMODULE xInputLibrary = LoadLibrary("xinput1_4.dll");

    if (!xInputLibrary){ 
        xInputLibrary = LoadLibrary("xinput1_3.dll");
    }

    if (xInputLibrary){

        XInputGetState = (x_input_get_state *)
            GetProcAddress(xInputLibrary,"XInputGetState");
        XInputSetState = (x_input_set_state *)
            GetProcAddress(xInputLibrary,"XInputSetState");
    }
}

struct win32_GameCode{
    HMODULE gameCodeDLL;
    FILETIME lastWriteTime;
    game_update_and_render *updateAndRender;
    game_get_sound_samples *getSoundSamples;
    bool32 isValid;
};

void win32UnloadGameCode(win32_GameCode *gameCode) 
{
    if (gameCode->gameCodeDLL){
        FreeLibrary(gameCode->gameCodeDLL);
        gameCode->gameCodeDLL = 0;
    }
    gameCode->isValid = false;
    gameCode->updateAndRender = 0;
    gameCode->getSoundSamples = 0;
}

inline FILETIME win32_getFileDate(char *fileName) {
    FILETIME lastWriteTime = {};
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(fileName,GetFileExInfoStandard,&data)){ 

        lastWriteTime = data.ftLastWriteTime;
    }
    return lastWriteTime;
}

win32_GameCode win32LoadGameCode(char * sourceDllName,char *tempDllName)
{
    win32_GameCode result = {};

    win32_getFileDate(sourceDllName);


    CopyFile(sourceDllName,tempDllName,FALSE);

    result.gameCodeDLL = LoadLibraryA(tempDllName);


    if (result.gameCodeDLL){
        result.updateAndRender= (game_update_and_render*)
            GetProcAddress(result.gameCodeDLL,"gameUpdateAndRender");
        result.getSoundSamples= (game_get_sound_samples*)
            GetProcAddress(result.gameCodeDLL,"gameGetSoundSamples");
        result.isValid = (result.updateAndRender && result.getSoundSamples);
    }

    if (!result.isValid) {
        result.updateAndRender = 0;
        result.getSoundSamples=  0;
    }
    return result;
}

static void win32ProcessDigitalButton(
        GameButtonState *newState, 
        GameButtonState *oldState, 
        DWORD buttonBit,
        DWORD xInputButtonState) {
    newState->endedDown = (xInputButtonState & buttonBit) == buttonBit;
    newState->halfTransitionCount = 
        (newState->endedDown != oldState->endedDown) ? 1 : 0;
}

static void win32ProcessKeyboardMessage(
        GameButtonState *newState, bool32 isDown) 
{
    if (newState->endedDown != isDown){
        newState->endedDown = isDown;
        ++newState->halfTransitionCount;
    }
}
//END INPUT 

//SOUND

static LPDIRECTSOUNDBUFFER g_secondaryBuffer;

//Pointer to function
#define DIRECTSOUND_CREATE(name) \
    HRESULT WINAPI name(LPCGUID pcGuidDevice, \
            LPDIRECTSOUND *ppDS, \
            LPUNKNOWN pUnkOuter)

typedef DIRECTSOUND_CREATE(directsound_create);

static void win32InitSound(HWND hwnd, 
        int32_t samplesPerSecond, 
        int32_t bufferSize){
    //Load the library and init 
    HMODULE dSoundLibrary = LoadLibrary("dsound.dll");

    if (dSoundLibrary){
        directsound_create *DirectSoundCreate = 
            (directsound_create*) GetProcAddress(dSoundLibrary, 
                    "DirectSoundCreate");
        //Get a DirectSound object
        LPDIRECTSOUND directSound;

        if (DirectSoundCreate && 
                SUCCEEDED(DirectSoundCreate(0,&directSound,0))){

            WAVEFORMATEX waveFormat = {};
            waveFormat.wFormatTag = WAVE_FORMAT_PCM;
            waveFormat.nChannels = 2;
            waveFormat.nSamplesPerSec = samplesPerSecond;
            waveFormat.wBitsPerSample = 16;
            waveFormat.nBlockAlign = 
                (waveFormat.nChannels*waveFormat.wBitsPerSample)/8;
            waveFormat.nAvgBytesPerSec = 
                waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;

            if (SUCCEEDED(directSound->SetCooperativeLevel(hwnd,
                            DSSCL_PRIORITY))){

                DSBUFFERDESC bufferDesc = {}; 
                bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                bufferDesc.dwSize= sizeof(bufferDesc);

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if (SUCCEEDED(directSound->
                            CreateSoundBuffer(&bufferDesc,
                                &primaryBuffer,0))){

                    if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))){
                        OutputDebugStringA("Primary Sound Buffer Set\n");
                    }else{
                        OutputDebugStringA
                            ("Primary Sound Buffer Format Not Set\n");

                    }
                } else {
                    OutputDebugStringA("Primary Sound Buffer Not Created \n");
                }

                DSBUFFERDESC secBufferDesc = {}; 
                secBufferDesc.dwSize= sizeof(bufferDesc);
                secBufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
                secBufferDesc.dwBufferBytes= bufferSize;
                secBufferDesc.lpwfxFormat= &waveFormat;

                if(SUCCEEDED(directSound->
                            CreateSoundBuffer(&secBufferDesc,
                                &g_secondaryBuffer,0))){
                    OutputDebugStringA("Secondary Sound Buffer Created \n");
                } else {
                    OutputDebugStringA("Secondary Sound Buffer Not Created\n");
                }

            } else {
                OutputDebugStringA("Cooperative Level Not Set\n");
            }
        } else {
            OutputDebugStringA("Direct Sound Create Failed\n");

        }
    } else {
        OutputDebugStringA("Direct Sound Library Not Loaded\n");
    }
}


static void win32ClearBuffer(win32_soundOutput *soundOutput)
{
    VOID *region1;
    DWORD region1Size;
    VOID *region2;
    DWORD region2Size;


    if (SUCCEEDED(g_secondaryBuffer->Lock(
                    0,
                    soundOutput->secondaryBufferSize,
                    &region1,&region1Size,
                    &region2,&region2Size,
                    0))){

        int8_t *destByte= (int8_t *)region1;

        for (DWORD byteIndex= 0; byteIndex< region1Size; byteIndex++){
            *destByte++ = 0 ; 
        }

        destByte= (int8_t *)region2;
        for (DWORD byteIndex= 0; byteIndex< region2Size; byteIndex++){
            *destByte++ = 0 ; 
        }
        g_secondaryBuffer->Unlock(region1,region1Size,region2,region2Size);
    }
}

static void win32FillSoundBuffer(win32_soundOutput *soundOutput, 
        SoundBuffer *soundBuffer, 
        DWORD byteToLock, 
        DWORD bytesToWrite)
{
    VOID *region1;
    DWORD region1Size;
    VOID *region2;
    DWORD region2Size;


    if (SUCCEEDED(g_secondaryBuffer->Lock(
                    byteToLock,
                    bytesToWrite,
                    &region1,&region1Size,
                    &region2,&region2Size,
                    0))){

        DWORD regionSample1Count = region1Size/soundOutput->bytesPerSample;
        int16_t *destSample= (int16_t *)region1;
        int16_t *srcSample = soundBuffer->samples;

        for (DWORD sampleIndex = 0;
                sampleIndex<regionSample1Count;
                sampleIndex++){
            *destSample++ = *srcSample++;
            *destSample++ = *srcSample++;
            soundOutput->runningSampleIndex++;
        }

        DWORD regionSample2Count = region2Size/soundOutput->bytesPerSample;
        destSample = (int16_t *)region2;
        srcSample = soundBuffer->samples;
        for (DWORD sampleIndex = 0;
                sampleIndex< regionSample2Count;
                sampleIndex++){
            *destSample++ = *srcSample++;
            *destSample++ = *srcSample++;
            soundOutput->runningSampleIndex++;
        }
        g_secondaryBuffer->Unlock(region1,region1Size,region2,region2Size);
    }

}
//END SOUND 






//TODO Global for now
static bool g_running;
static bool g_pause;
static int64_t g_perfCountFrequency;

void catStrings( size_t countA, char *sourceA, 
        size_t countB, char *sourceB, 
        size_t destCount, char *dest)
{
    for (int index=0; index<countA; index++){
        *dest++ = *sourceA++;
    }

    for (int index=0; index<countB; index++){
        *dest++ = *sourceB++;
    }
    *dest++ = 0;
}


static void win32_getFileName(win32_State *state) {
    DWORD sizeOfFileName = 
        GetModuleFileNameA(0, state->fileName,sizeof(state->fileName));

    state->lastSlash = state->fileName; 
    for (char *scan = state->fileName;
            *scan;
            ++scan){
        if (*scan == '\\'){
            state->lastSlash = scan + 1;
        }
    }
}
static int stringLength(char *string) {
    int length = 0;
    while (string[length++]);
    return length;
}

static  void win32_buildPathFileName (win32_State *state, char *fileName,
        int destCount, char *dest) 
{
    catStrings(state->lastSlash - state->fileName,state->fileName, 
            stringLength(fileName), fileName,
            destCount,dest);
}
static void win32_getInputFileLocation (win32_State *state ,
        bool32 isInput, 
        int slotIndex,
        int destCount,
        char *dest) 
{
    char temp[64];
    wsprintf(temp,"loop_edit_%d_%s.hmi",slotIndex,isInput ? "input":"state");
    win32_buildPathFileName(state, temp,destCount,dest);
}

static win32_ReplayBuffer *win32_getReplayBuffer(win32_State *state, 
        int unsigned index)
{
    ASSERT(index < ARRAY_COUNT(state->replayBuffers));
    win32_ReplayBuffer *replayBuffer = &state->replayBuffers[index];
    return replayBuffer;
}

void win32_beginRecordingInput(win32_State *state,int inputRecordingIndex)
{ 
    win32_ReplayBuffer *replayBuffer = 
        win32_getReplayBuffer(state,inputRecordingIndex); 

    if (replayBuffer->memoryBlock) {
        state->inputRecordingIndex = inputRecordingIndex;

        char fileName[WIN32_STATE_FILE_NAME_COUNT];
        win32_getInputFileLocation(state,
                true,
                inputRecordingIndex,
                sizeof(fileName),
                fileName);
        state->recordingHandle= CreateFileA(fileName,
                GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);

        /*
           DWORD bytesReturned;
           DeviceIoControl( state->recordingHandle,            // handle to a file
           FSCTL_SET_SPARSE,                     // dwIoControlCode
           0,0,0,0,// input buffer
           &bytesReturned ,            // number of bytes returned
           0);        // OVERLAPPED structure

*/
        //DWORD bytesWritten;
        //WriteFile(state->recordingHandle, state->gameMemoryBlock, (DWORD)state->totalSize,&bytesWritten,0);
        /*
           LARGE_INTEGER filePosition;
           filePosition.QuadPart = state->totalSize;
           SetFilePointerEx(state->recordingHandle, filePosition, 0, FILE_BEGIN);
           */
        CopyMemory(replayBuffer->memoryBlock,
                state->gameMemoryBlock,
                state->totalSize);

    }
}

void win32_endRecordingInput(win32_State *state)
{
    CloseHandle(state->recordingHandle);
    state->inputRecordingIndex= 0;
}

void win32_beginInputPlayback(win32_State *state, int inputPlayingIndex)
{ 
    win32_ReplayBuffer *replayBuffer = 
        win32_getReplayBuffer(state,inputPlayingIndex); 

    if (replayBuffer->memoryBlock) {
        state->inputPlayingIndex= inputPlayingIndex;
        //DWORD bytesRead;
        //ReadFile(state->playbackHandle, 
        //state->gameMemoryBlock, 
        //(DWORD)state->totalSize,&bytesRead,0);
        //
        char fileName[WIN32_STATE_FILE_NAME_COUNT];
        win32_getInputFileLocation(state,true,inputPlayingIndex,sizeof(fileName),fileName);
        state->playbackHandle= CreateFileA(fileName,
                GENERIC_READ,0,0,OPEN_EXISTING,0,0);

        /*
           LARGE_INTEGER filePosition;
           filePosition.QuadPart = state->totalSize;
           SetFilePointerEx(state->playbackHandle, filePosition, 0, FILE_BEGIN);

*/

        CopyMemory(state->gameMemoryBlock,
                replayBuffer->memoryBlock,
                state->totalSize);
    }
}

void  win32_endInputPlayback(win32_State *state)
{
    CloseHandle(state->playbackHandle);
    state->inputPlayingIndex= 0;
}

void win32_recordInput(win32_State *state,GameInput *newInput)
{
    DWORD bytesWritten;
    WriteFile(state->recordingHandle, 
            newInput, 
            sizeof(*newInput), 
            &bytesWritten,0);
}

void win32_playbackInput(win32_State *state,GameInput *newInput)
{
    DWORD bytesRead;
    ReadFile(state->playbackHandle, newInput, sizeof(*newInput), &bytesRead,0);
    if(bytesRead>0){

        OutputDebugStringA("bytes Read");
    } else {
        int inputPlayingIndex = state->inputPlayingIndex;
        win32_endInputPlayback(state);
        win32_beginInputPlayback(state,inputPlayingIndex);
        ReadFile(state->playbackHandle, 
                newInput, 
                sizeof(*newInput), 
                &bytesRead,0);
    }
}


static void win32ProcessPendingMessages(
        win32_State *state,
        GameControllerInput *keyboardController)
{
    MSG message;
    while(PeekMessage(&message,0,0,0,PM_REMOVE)){
        switch (message.message) {
            case WM_QUIT:
                g_running = false;
                break;
            case  WM_SYSKEYDOWN:
            case  WM_SYSKEYUP:
            case  WM_KEYDOWN:
            case  WM_KEYUP:
                {
                    uint32_t vKcode = (uint32_t)message.wParam;

                    bool32 wasDown = ((message.lParam & (1 << 30))!=0);
                    bool32 isDown  = ((message.lParam & (1 << 31))==0);

                    if (wasDown != isDown) {
                        switch (vKcode) {
                            case 'W':
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->moveUp,isDown);
                                OutputDebugStringA("Pressed W\n");
                                break;
                            case 'A':
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->moveLeft,isDown);
                                OutputDebugStringA("Pressed A\n");
                                break;
                            case 'S':
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->moveDown,isDown);
                                OutputDebugStringA("Pressed S\n");
                                break;
                            case 'D':
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->moveRight,isDown);
                                OutputDebugStringA("Pressed D\n");
                                break;
                            case 'Q':
                                OutputDebugStringA("Pressed Q\n");
                                break;
                            case 'E':
                                OutputDebugStringA("Pressed E\n");
                                break;
                            case VK_UP: 
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->actionUp,isDown);
                                OutputDebugStringA("Pressed UP\n");
                                break;
                            case VK_DOWN: 
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->actionDown,isDown);
                                OutputDebugStringA("Pressed DOWN\n");
                                break;
                            case VK_LEFT: 
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->actionLeft,isDown);
                                OutputDebugStringA("Pressed LEFT\n");
                                break;
                            case VK_RIGHT: 
                                win32ProcessKeyboardMessage(&keyboardController
                                        ->actionRight,isDown);
                                OutputDebugStringA("Pressed RIGHT\n");
                                break;
                            case VK_SPACE:
                                OutputDebugStringA("Pressed SPACE\n");
                                break;
                            case VK_ESCAPE:
                                OutputDebugStringA("Pressed ESC\n");
                                break;
#if DEBUG_MODE
                            case 'P':
                                if (isDown){
                                    g_pause = !g_pause; 
                                }
                                break;
                            case 'L':
                                if (isDown){
                                    if (state->inputPlayingIndex == 0) {
                                        if (state->inputRecordingIndex == 0) {
                                            win32_beginRecordingInput(state,1);
                                        } else {
                                            win32_endRecordingInput(state);
                                            win32_beginInputPlayback(state,1);
                                        }
                                    } else {
                                        win32_endInputPlayback(state);
                                    }
                                }
                                break;
#endif
                            case VK_F4:
                                bool32 isAltDown = 
                                    (message.lParam & (1 << 29));
                                if (isAltDown){
                                    g_running=false;
                                    OutputDebugStringA("Pressed ALT+F4\n");
                                }
                                break;
                        }
                    }

                }break;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

static float win32ProcessXInputStickPosition(SHORT value,
        SHORT deadzone) 
{
    float result = 0;
    if (value < -deadzone ) {
        result = (float)value / 32768.0f;
    } else if (value > deadzone){
        result = (float)value/ 32767.0f;
    }
    return result;
}

static LRESULT CALLBACK MainWindowCallback(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    LRESULT result = 0;
    switch (uMsg) {
        case WM_SIZE:
            {
                OutputDebugStringA("WM_SIZE\n");
            }break;
        case WM_DESTROY:
            {
                g_running = false;
                OutputDebugStringA("WM_DESTROY\n");
            }break;
        case WM_CLOSE:
            {
                //TODO Handle with a message to he user ?
                g_running = false;
                OutputDebugStringA("WM_CLOSE\n");
            }break;
        case WM_ACTIVATEAPP:
            { 
                if(wParam) {
                    SetLayeredWindowAttributes(hwnd,
                            RGB(0,0,0),
                            255,
                            LWA_ALPHA);
                } else {
                    SetLayeredWindowAttributes(hwnd,
                            RGB(0,0,0),
                            64,
                            LWA_ALPHA);
                }
                OutputDebugStringA("WM_ACTIVATE\n");
            }break;
        case WM_PAINT:
            { 
                PAINTSTRUCT paint;
                HDC deviceContext = BeginPaint(hwnd,&paint);
                win32windowDimensions dimensions = 
                    win32getWindowDimensions(hwnd);

                Win32displayBufferInWindow(&g_frameBuffer,
                        deviceContext,
                        dimensions.width,
                        dimensions.height);
                EndPaint(hwnd,&paint);     
                OutputDebugStringA("WM_PAINT\n");
            }break;
        default:
            {
                result = DefWindowProc(hwnd,uMsg,wParam,lParam); 
            }break;
    }
    return (result);
}


inline LARGE_INTEGER win32getWallClock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline float win32getSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) 
{
    float result =((end.QuadPart - start.QuadPart)  / 
            (float) g_perfCountFrequency); 
    return result;
}

#if 0
#ifdef DEBUG_MODE

static void win32DebugDrawVertical(win32offscreenBuffer *buffer, int x, int top, int bottom, uint32_t color)
{
    uint8_t *pixel = ((uint8_t *) buffer->bitmapMemory + 
            x* (buffer->bytesPerPixel) + 
            top * (buffer->pitch));

    if (top < 0) {
        top=0;
    }

    if (bottom > buffer->height) {
        bottom = buffer->height;
    }

    if (x >0 && x<buffer->width){
        for (int y = top; y< bottom; ++y) {
            *(uint32_t *) pixel = color; 
            pixel += buffer->pitch;
        }
    }
}

inline void win32DrawSoundBufferMarker ( win32offscreenBuffer *buffer,
        win32_soundOutput *soundOutput,
        float c, int padX,int top,int bottom,DWORD value, uint32_t color)
{
    int x = padX + (int)(c * (float)value);
    win32DebugDrawVertical(buffer, x, top,bottom,color);
}

static void win32DebugDisplay( win32offscreenBuffer *buffer,
        int markerCount, win32DebugTimeMarker *markers,
        int currentMarkerIndex,
        win32_soundOutput *soundOutput,
        float targetSecondsPerFrame
        )
{

    int padX = 16;
    int padY = 16;

    int lineHeight = 64;
    float c = (float)(buffer->width - 2*padX) / (float)soundOutput->secondaryBufferSize;

    for (int markerIndex= 0; 
            markerIndex < markerCount; 
            markerIndex++) {
        DWORD playColor = 0xFFFFFFFF;
        DWORD writeColor = 0xFFFF0000;
        DWORD flipColor = 0xFFFFFF00;
        int top = padY;
        int bottom=  padY + lineHeight;

        win32DebugTimeMarker *marker = &markers[markerIndex];
        if (markerIndex == currentMarkerIndex){

            top += lineHeight + padY;
            bottom +=  padY + lineHeight;
            int firstTop = top;

            win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->outputPlayCursor,playColor);
            win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->outputWriteCursor,writeColor);

            top += lineHeight + padY;
            bottom +=  padY + lineHeight;

            win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->outputLocation,playColor);
            win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->outputLocation + marker->outputByteCount,writeColor);

            top += lineHeight + padY;
            bottom +=  padY + lineHeight;
            win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,firstTop,bottom,marker->expectedFrameBoundary,flipColor);

        }
        win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->flipPlayCursor,playColor);
        win32DrawSoundBufferMarker(buffer,soundOutput,c,padX,top,bottom,marker->flipWriteCursor,writeColor);
    }
}

#endif
#endif


int CALLBACK WinMain(
        HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        int         nCmdShow
        )
{

    win32_State state = {};
    win32_getFileName(&state);
    ThreadContext context = {};

    char sourceDllFullPath[WIN32_STATE_FILE_NAME_COUNT];
    win32_buildPathFileName(&state, 
            "handmade.dll", 
            sizeof(sourceDllFullPath),
            sourceDllFullPath);


    char tempDllFullPath[WIN32_STATE_FILE_NAME_COUNT];
    win32_buildPathFileName(&state, 
            "handmade_temp.dll", 
            sizeof(tempDllFullPath),
            tempDllFullPath);



    win32LoadInput();
    win32_GameCode gameCode = 
        win32LoadGameCode(sourceDllFullPath,tempDllFullPath); 

    WNDCLASS WindowClass =  {}; 

    const int WINDOW_WIDTH = 1270 ;
    const int WINDOW_HEIGHT = 800;
    Win32ResizeDIBSection(&g_frameBuffer,WINDOW_WIDTH,WINDOW_HEIGHT);


    WindowClass.style =  CS_HREDRAW | CS_VREDRAW;// | CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";


    LARGE_INTEGER perfFrequency;
    QueryPerformanceFrequency(&perfFrequency);
    g_perfCountFrequency = perfFrequency.QuadPart;
    UINT desiredSchedulerMS = 1;
    bool32 schedulerConfigured  = 
        (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);



    if (RegisterClass(&WindowClass))    {


        HWND hwnd= CreateWindowEx (
                //WS_EX_TOPMOST | 
                WS_EX_LAYERED,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                WINDOW_WIDTH,
                WINDOW_HEIGHT,
                0,
                0,
                hInstance,
                0);

        if(hwnd) {

            HDC refreshDC = GetDC(hwnd); 
            int monitorRefreshHz = GetDeviceCaps(refreshDC,VREFRESH);
            monitorRefreshHz = (monitorRefreshHz > 1) ? 
                monitorRefreshHz : 60;
            float gameUpdateHz = (float)monitorRefreshHz / 2.0f;
            float targetSecondsPerFrame = 1.0f/ (float) gameUpdateHz;

            win32_soundOutput soundOutput;

            soundOutput.samplesPerSecond = 48000;
            soundOutput.bytesPerSample = sizeof(int16_t)*2;
            soundOutput.secondaryBufferSize = 
                soundOutput.samplesPerSecond*soundOutput.bytesPerSample;
            soundOutput.safetyBytes 
                = (int)((float)(soundOutput.bytesPerSample*
                            soundOutput.samplesPerSecond)/ 
                        gameUpdateHz/3.0f);
            win32InitSound(hwnd,
                    soundOutput.samplesPerSecond,
                    soundOutput.secondaryBufferSize);
            win32ClearBuffer(&soundOutput);
            g_secondaryBuffer->Play(0,0,DSBPLAY_LOOPING);
            //soundOutput.tSine= 0.0f;

#if 0  
            for (;;){
                DWORD playCursor;
                DWORD writeCursor;

                g_secondaryBuffer->GetCurrentPosition(&playCursor,&writeCursor); 
                DWORD bytesBetween = (writeCursor > playCursor) ? (writeCursor-playCursor) : writeCursor + (soundOutput.secondaryBufferSize - playCursor); 
                char textBuffer[256];
                _snprintf_s(textBuffer, sizeof(textBuffer),
                        "PC:%u WC:%u BB:%u \n",playCursor,writeCursor,bytesBetween);
                OutputDebugStringA(textBuffer);
            }
#endif


            g_running = true;


            int16_t *samples = 
                (int16_t*) VirtualAlloc(0,
                        soundOutput.secondaryBufferSize,
                        MEM_RESERVE | MEM_COMMIT,
                        PAGE_READWRITE);

#ifdef DEBUG_MODE
            LPVOID baseAddress = (LPVOID)TERABYTES((uint64_t)2);
#else 
            LPVOID baseAddress = 0;
#endif 
            GameMemory gameMemory = {};

            gameMemory.DEBUGReadEntireFile = DEBUGReadEntireFile;
            gameMemory.DEBUGWriteEntireFile = DEBUGWriteEntireFile;
            gameMemory.DEBUGFreeFileMemory = DEBUGFreeFileMemory;

            gameMemory.permanentStorageSize = MEGABYTES(64); 
            gameMemory.transientStorageSize = GIGABYTES(1);

            state.totalSize = 
                gameMemory.permanentStorageSize + 
                gameMemory.transientStorageSize;

            state.gameMemoryBlock =
                VirtualAlloc(baseAddress,
                        state.totalSize,
                        MEM_RESERVE | MEM_COMMIT,
                        PAGE_READWRITE);

            gameMemory.permanentStorage = state.gameMemoryBlock; 
            gameMemory.transientStorage = 
                (uint8_t *)state.gameMemoryBlock + 
                gameMemory.permanentStorageSize ;
            gameMemory.isInitialized = false;

            for (int replayIndex =1; 
                    replayIndex < ARRAY_COUNT(state.replayBuffers);
                    replayIndex++){
                win32_ReplayBuffer *replayBuffer = 
                    &state.replayBuffers[replayIndex];

                win32_getInputFileLocation(&state, 
                        false,
                        replayIndex,
                        sizeof(replayBuffer->fileName),
                        replayBuffer->fileName);


                replayBuffer->fileHandle
                    = CreateFileA(replayBuffer->fileName,
                            GENERIC_WRITE | GENERIC_READ,
                            0,0,
                            CREATE_ALWAYS,
                            0,0);

                DWORD error = GetLastError();

                DWORD maxSizeHigh = state.totalSize >> 32;
                DWORD maxSizeLow= state.totalSize & 0xFFFFFFFF;
                replayBuffer->memoryMap = 
                    CreateFileMapping(replayBuffer->fileHandle
                            ,0,PAGE_READWRITE,
                            maxSizeHigh ,
                            maxSizeLow,

                            0);

                replayBuffer->memoryBlock = 
                    MapViewOfFile(replayBuffer->memoryMap,
                            FILE_MAP_ALL_ACCESS, 
                            0,0, 
                            state.totalSize);
                if (replayBuffer->memoryBlock){

                }

            }


            if(samples &&
                    gameMemory.permanentStorage && 
                    gameMemory.transientStorage){

                GameInput gameInput[2] ={};
                GameInput *newInput = &gameInput[0];
                GameInput *oldInput = &gameInput[1];


                LARGE_INTEGER lastCounter  = win32getWallClock();

                uint64_t  lastCycleCount = __rdtsc();

                //Game Loop
                DWORD lastWriteCursor= 0;
                DWORD lastPlayCursor = 0;
                LARGE_INTEGER flipWallClock = win32getWallClock();

#ifdef DEBUG_MODE
                int timeMarkersIndex= 0;
                win32DebugTimeMarker markers[30] ={};
#endif


                for (;g_running;){
                    newInput->dtForFrame= targetSecondsPerFrame;

                    FILETIME newDllWriteTime = 
                        win32_getFileDate(sourceDllFullPath);
                    if (CompareFileTime(&newDllWriteTime ,
                                &gameCode.lastWriteTime) != 0)
                    {
                        win32UnloadGameCode(&gameCode);
                        gameCode = win32LoadGameCode(sourceDllFullPath,
                                tempDllFullPath); 
                    }


                    GameControllerInput *newKeyboardController = 
                        getController(newInput,0);
                    GameControllerInput *oldKeyboardController = 
                        getController(oldInput,0);

                    *newKeyboardController = {};

                    newKeyboardController->isConnected = true;
                    newKeyboardController->isAnalog = false;

                    for (int buttonIndex=0;
                            buttonIndex<ARRAY_COUNT(newKeyboardController->
                                buttons);
                            ++buttonIndex){
                        newKeyboardController->
                            buttons[buttonIndex].endedDown =
                            oldKeyboardController->
                            buttons[buttonIndex].endedDown;
                    }


                    win32ProcessPendingMessages(&state,
                            newKeyboardController);

                    //Input 
                    DWORD maxControllerCount  = XUSER_MAX_COUNT ;

                    if(maxControllerCount > (ARRAY_COUNT(newInput->
                                    controllers) - 1)){
                        maxControllerCount = (ARRAY_COUNT(newInput->
                                    controllers) - 1);
                    }

                    for(DWORD controllerIndex =0; 
                            controllerIndex < maxControllerCount; 
                            controllerIndex++)
                    {
                        DWORD ourControllerIndex = controllerIndex + 1;
                        GameControllerInput *oldController = 
                            getController(oldInput,ourControllerIndex);
                        GameControllerInput *newController = 
                            getController(newInput,ourControllerIndex);

                        XINPUT_STATE inputState ; 
                        if (XInputGetState(controllerIndex,&inputState) == 
                                ERROR_SUCCESS){
                            //Controller is plugged in
                            //Look at inputState.dwPacketNumber
                            newController->isConnected = true;
                            newController->isAnalog = 
                                oldController->isAnalog;

                            newController->isAnalog = true;
                            XINPUT_GAMEPAD *pad = &inputState.Gamepad;

                            newController->stickAverageX = 
                                win32ProcessXInputStickPosition(pad->sThumbLX,
                                        XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);


                            newController->stickAverageY = 
                                win32ProcessXInputStickPosition(pad->sThumbLY,
                                        XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                            float treshold = 0.5f;


                            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP){
                                newController->stickAverageY +=1.0f;
                            }

                            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN){
                                newController->stickAverageY -=1.0f;
                            } 

                            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT){
                                newController->stickAverageX -=1.0f;
                            }

                            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT){
                                newController->stickAverageX +=1.0f;
                            }




                            win32ProcessDigitalButton
                                (&newController->moveLeft,
                                 &oldController->moveLeft,
                                 1,
                                 (newController->stickAverageX < 
                                  -treshold) ? 1 : 0
                                );

                            win32ProcessDigitalButton
                                (&newController->moveRight,
                                 &oldController->moveRight,
                                 1,
                                 (newController->stickAverageX > 
                                  treshold) ? 1 : 0
                                );

                            win32ProcessDigitalButton
                                (&newController->moveUp,
                                 &oldController->moveUp,
                                 1,
                                 (newController->stickAverageY 
                                  < -treshold) ? 1 : 0
                                );

                            win32ProcessDigitalButton
                                (&newController->moveDown,
                                 &oldController->moveDown,
                                 1,
                                 (newController->stickAverageY > 
                                  treshold) ? 1 : 0
                                );

                            win32ProcessDigitalButton
                                (&newController->actionDown,
                                 &oldController->actionDown,
                                 XINPUT_GAMEPAD_A,
                                 pad->wButtons);
                            win32ProcessDigitalButton
                                (&newController->actionRight,
                                 &oldController->actionRight,
                                 XINPUT_GAMEPAD_B,
                                 pad->wButtons);
                            win32ProcessDigitalButton
                                (&newController->actionLeft,
                                 &oldController->actionLeft,
                                 XINPUT_GAMEPAD_X,
                                 pad->wButtons);
                            win32ProcessDigitalButton
                                (&newController->actionUp,
                                 &oldController->actionUp,
                                 XINPUT_GAMEPAD_Y,
                                 pad->wButtons);
                            win32ProcessDigitalButton
                                (&newController->leftShoulder,
                                 &oldController->leftShoulder,
                                 XINPUT_GAMEPAD_LEFT_SHOULDER,
                                 pad->wButtons);
                            win32ProcessDigitalButton
                                (&newController->rightShoulder,
                                 &oldController->rightShoulder,
                                 XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                 pad->wButtons);


                        } else {
                            newController->isConnected = false;
                            //Controller not available
                        }
                    }
                    /* rumble
                       XINPUT_VIBRATION vibration;
                       vibration.wLeftMotorSpeed = 60000;
                       vibration.wRightMotorSpeed = 60000;
                       XInputSetState(0,&vibration);
                       */

                    if (!g_pause){
                        POINT mouseP;
                        GetCursorPos(&mouseP);
                        ScreenToClient(hwnd,&mouseP);
                        newInput->mouseX = mouseP.x;
                        newInput->mouseY = mouseP.y;
                        newInput->mouseZ = 0; 
                        win32ProcessKeyboardMessage
                            (&newInput->mouseButtons[0], 
                             GetKeyState(VK_LBUTTON) & (1 <<15));
                        win32ProcessKeyboardMessage
                            (&newInput->mouseButtons[1], 
                             GetKeyState(VK_RBUTTON) & (1 <<15));
                        win32ProcessKeyboardMessage
                            (&newInput->mouseButtons[2], 
                             GetKeyState(VK_MBUTTON) & (1 <<15));
                        FrameBuffer frameBuffer = {};
                        frameBuffer.bitmapMemory = 
                            g_frameBuffer.bitmapMemory; 
                        frameBuffer.width= g_frameBuffer.width; 
                        frameBuffer.height= g_frameBuffer.height; 
                        frameBuffer.pitch= g_frameBuffer.pitch; 
                        frameBuffer.bytesPerPixel= 4; 

                        //Rendering

                        //DirectSound Output test 

                        bool32 soundIsValid = false;


                        if (state.inputRecordingIndex){
                            win32_recordInput(&state,newInput);
                        }
                        if (state.inputPlayingIndex){
                            win32_playbackInput(&state,newInput);
                        }
                        if (gameCode.updateAndRender) {
                            gameCode.updateAndRender
                                (&context,&gameMemory, &frameBuffer,newInput);
                        }

                        DWORD playCursor;
                        DWORD writeCursor;
                        LARGE_INTEGER AudioWallClock = win32getWallClock();
                        float fromBeginToAudioSeconds = 
                            1000.0f*win32getSecondsElapsed
                            (flipWallClock,AudioWallClock);

                        if (g_secondaryBuffer->
                                GetCurrentPosition(&playCursor,&writeCursor) == 
                                DS_OK) {

                            if (!soundIsValid){
                                soundOutput.runningSampleIndex = 
                                    writeCursor / soundOutput.bytesPerSample; 
                                soundIsValid = true;
                            }

                            DWORD byteToLock = 
                                ((soundOutput.runningSampleIndex * 
                                  soundOutput.bytesPerSample)  %
                                 soundOutput.secondaryBufferSize);

                            DWORD expectedSoundBytesPerFrame = (int) 
                                ((float)(soundOutput.samplesPerSecond *
                                    soundOutput.bytesPerSample) 
                                 /gameUpdateHz);

                            float secondsLeftUntilFlip = 
                                (targetSecondsPerFrame -
                                 fromBeginToAudioSeconds);
                            DWORD expectedBytesUntilFlip =
                                (DWORD)((secondsLeftUntilFlip /
                                            targetSecondsPerFrame) * 
                                        (float) expectedSoundBytesPerFrame ); 

                            DWORD expectedFrameBoundary = 
                                playCursor + expectedBytesUntilFlip;
                            DWORD safetyWriteCursor = writeCursor;

                            if (safetyWriteCursor < playCursor) {
                                safetyWriteCursor +=
                                    soundOutput.secondaryBufferSize;
                            }
                            safetyWriteCursor  += soundOutput.safetyBytes;

                            bool32 audioCardIsLowLatency = 
                                (safetyWriteCursor < expectedFrameBoundary); 

                            DWORD targetCursor=0;
                            if (audioCardIsLowLatency) {
                                targetCursor = (expectedFrameBoundary + 
                                        expectedSoundBytesPerFrame) ;
                            } else {
                                targetCursor = (safetyWriteCursor+ 
                                        expectedSoundBytesPerFrame + 
                                        soundOutput.safetyBytes); 
                            }
                            targetCursor =
                                (targetCursor % 
                                 soundOutput.secondaryBufferSize);

                            DWORD bytesToWrite=0;

                            if (byteToLock > targetCursor){
                                bytesToWrite = 
                                    (soundOutput.secondaryBufferSize-
                                     byteToLock);
                                bytesToWrite += targetCursor;
                            } else {
                                bytesToWrite = targetCursor - byteToLock;
                            } 

                            SoundBuffer soundBuffer = {};
                            soundBuffer.samplesPerSecond = 
                                soundOutput.samplesPerSecond; 
                            soundBuffer.sampleCount = 
                                bytesToWrite / soundOutput.bytesPerSample; 
                            soundBuffer.samples = samples;  
                            if (gameCode.getSoundSamples) {
                                gameCode.getSoundSamples
                                    (&context,&gameMemory,&soundBuffer);
                            }

#if 0 
                            win32DebugTimeMarker *marker = &markers[timeMarkersIndex];
                            marker->outputPlayCursor = playCursor;
                            marker->outputWriteCursor = writeCursor;
                            marker->outputLocation = byteToLock;
                            marker->outputByteCount = bytesToWrite;
                            marker->expectedFrameBoundary = expectedFrameBoundary;

                            g_secondaryBuffer->GetCurrentPosition(&playCursor,&writeCursor); 
                            char textBuffer[256];
                            DWORD bytesBetween = (writeCursor > playCursor) ? (writeCursor-playCursor) : writeCursor + (soundOutput.secondaryBufferSize - playCursor); 
                            float audioLatencySeconds = ((bytesBetween / soundOutput.bytesPerSample)/(float)soundOutput.samplesPerSecond);
                            _snprintf_s(textBuffer, sizeof(textBuffer),
                                    "BTL %u TC:%u BTW:%u AL:%f \n",byteToLock,targetCursor,bytesToWrite,audioLatencySeconds);
                            OutputDebugStringA(textBuffer);
#endif
                            win32FillSoundBuffer
                                (&soundOutput,
                                 &soundBuffer,
                                 byteToLock,
                                 bytesToWrite);
                        } else {
                            soundIsValid = false;
                        }

                        float secondElapsedPerFrame = 
                            win32getSecondsElapsed(lastCounter,
                                    win32getWallClock()); 

                        if (secondElapsedPerFrame < targetSecondsPerFrame) {
                            if (schedulerConfigured){
                                DWORD ms = 
                                    (DWORD) (1000.0f*(targetSecondsPerFrame - 
                                                secondElapsedPerFrame));
                                if (ms >0) { 
                                    Sleep (ms);
                                }
                            }

                            while (secondElapsedPerFrame < 
                                    targetSecondsPerFrame) {
                                secondElapsedPerFrame = 
                                    win32getSecondsElapsed(lastCounter,
                                            win32getWallClock()); 
                            }
                        } else {
                            // Missed frame rate!!! 
                        }
                        LARGE_INTEGER endCounter = win32getWallClock();
                        float msPerFrame = 
                            1000.0f*win32getSecondsElapsed
                            (lastCounter,endCounter);
                        lastCounter = endCounter;

#if 0 

                        win32DebugDisplay(&g_frameBuffer, ARRAY_COUNT(markers), markers, timeMarkersIndex -1, &soundOutput, targetSecondsPerFrame);
#endif
                        win32windowDimensions dimensions  = 
                            win32getWindowDimensions(hwnd);
                        HDC deviceContext = GetDC(hwnd);
                        Win32displayBufferInWindow
                            (&g_frameBuffer,
                             deviceContext,
                             dimensions.width,
                             dimensions.height);
                        ReleaseDC(hwnd, deviceContext);


#if DEBUG_MODE
                        {
                            DWORD playCursor;
                            DWORD writeCursor;
                            if (g_secondaryBuffer->GetCurrentPosition
                                    (&playCursor,&writeCursor) == DS_OK) {
                                win32DebugTimeMarker *marker =
                                    &markers[timeMarkersIndex];
                                marker->flipPlayCursor = playCursor;
                                marker->flipWriteCursor = writeCursor;

                            }
                            flipWallClock = win32getWallClock();
                        }
#endif


#if 0
                        uint64_t endCycleCount = __rdtsc();
                        uint64_t cyclesElapsed = endCycleCount - 
                            lastCycleCount;
                        double MCPF = 
                            ((double) cyclesElapsed /(1000.0f *1000.0f));
                        double FPS  = 
                            (double)g_perfCountFrequency /
                            (double)win32getSecondsElapsed
                            (lastCounter,win32getWallClock()); 
                        char buffer[256];
                        sprintf_s
                            (buffer,
                             "ms/loop %0.2f FPS %0.2f MegaCycles %0.2f\n",
                             msPerFrame,FPS,MCPF);
                        //OutputDebugStringA(buffer);

#endif
                    }
                    GameInput *temp = newInput;
                    newInput = oldInput;
                    oldInput = temp;

#if DEBUG_MODE  
                    timeMarkersIndex++;
                    if (timeMarkersIndex == ARRAY_COUNT(markers)){
                        timeMarkersIndex= 0;
                    }
#endif
                } //End Main Loop

                VirtualFree(samples, 0, MEM_RELEASE);
                VirtualFree(gameMemory.permanentStorage, 0, MEM_RELEASE);
            } else {
                //Menory allocation failed 
            }
        } else{
            // Window handle not received
        }   
    } else {
        // Window class not registerd
    }

    return (0);
}

