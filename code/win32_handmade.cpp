
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include "handmade.cpp"

#include "win32_handmade.h"


#define internal static
#define global_variable static
#define local_persist static

#ifdef DEBUG_MODE 


static debugReadFileResult DEBUGplatformReadEntireFile(char *filename)
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
                DEBUGplatformFreeFileMemory(result.fileContent);
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

static void DEBUGplatformFreeFileMemory(void *memory)
{
    if(memory){ 
        VirtualFree(memory,0,MEM_RELEASE);
    }
}

static bool32 DEBUGplatformWriteEntireFile(
        char *filename, uint32_t memorySize, void *memory)
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
#endif 



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
    //Alloc memory. VirtualAlloc will pad the memory reserved to align with memory page size (4k) 
    buffer->bitmapMemory = VirtualAlloc(
            0,bitmapMemorySize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
}

static void Win32displayBufferInWindow(
        win32offscreenBuffer *buffer, HDC deviceContext, int width, int height)
{

    //TODO Aspect ratio correction pending.

    //StretchDIBits stretches our backbuffer up/down to the actual window's dimensions
    StretchDIBits(
            deviceContext,
            /*
               top,left,width,height,
               top,left,width,height,
               */
            0,0,width,height,
            0,0,buffer->width,buffer->height,
            buffer->bitmapMemory,
            &buffer->info,
            DIB_RGB_COLORS,
            SRCCOPY);
}
//END GRAPHICS 
//INPUT
//Pointer to function
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD  dwUserIndex,  XINPUT_STATE* pState )
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
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD  dwUserIndex,XINPUT_VIBRATION* pVibration)
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

static void win32ProcessDigitalButton(
        GameButtonState *newState, GameButtonState *oldState, DWORD buttonBit,DWORD xInputButtonState) {
    newState->halfTransitionCount = (newState->endedDown != oldState->endedDown) ? 1 : 0;
    newState->endedDown= (xInputButtonState & buttonBit) & buttonBit;
}

static void win32ProcessKeyboardMessage(
        GameButtonState *newState, bool32 isDown) {
    //ASSERT(newState->endedDown != isDown);
    newState->endedDown = isDown;
    ++newState->halfTransitionCount;
}
//END INPUT 

//SOUND

static LPDIRECTSOUNDBUFFER g_secondaryBuffer;

//Pointer to function
#define DIRECTSOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECTSOUND_CREATE(directsound_create);

static void win32InitSound(HWND hwnd, int32_t samplesPerSecond, int32_t bufferSize){
    //Load the library and init 
    HMODULE dSoundLibrary = LoadLibrary("dsound.dll");

    if (dSoundLibrary){
        directsound_create *DirectSoundCreate = (directsound_create*) GetProcAddress(dSoundLibrary, "DirectSoundCreate");
        //Get a DirectSound object
        LPDIRECTSOUND directSound;

        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0,&directSound,0))){

            WAVEFORMATEX waveFormat = {};
            waveFormat.wFormatTag = WAVE_FORMAT_PCM;
            waveFormat.nChannels = 2;
            waveFormat.nSamplesPerSec = samplesPerSecond;
            waveFormat.wBitsPerSample = 16;
            waveFormat.nBlockAlign = (waveFormat.nChannels*waveFormat.wBitsPerSample)/8;
            waveFormat.nAvgBytesPerSec= waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;

            if (SUCCEEDED(directSound->SetCooperativeLevel(hwnd,DSSCL_PRIORITY))){

                DSBUFFERDESC bufferDesc = {}; 
                bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                bufferDesc.dwSize= sizeof(bufferDesc);

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDesc,&primaryBuffer,0))){

                    if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))){
                        OutputDebugStringA("Primary Sound Buffer Set\n");
                    }else{
                        OutputDebugStringA("Primary Sound Buffer Format Not Set\n");
                    }
                } else {
                    OutputDebugStringA("Primary Sound Buffer Not Created \n");
                }

                DSBUFFERDESC secBufferDesc = {}; 
                secBufferDesc.dwSize= sizeof(bufferDesc);
                secBufferDesc.dwFlags = 0;
                secBufferDesc.dwBufferBytes= bufferSize;
                secBufferDesc.lpwfxFormat= &waveFormat;

                if(SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDesc,&g_secondaryBuffer,0))){
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

static void win32FillSoundBuffer(win32_soundOutput *soundOutput, SoundBuffer *soundBuffer, DWORD byteToLock, DWORD bytesToWrite)
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

        for (DWORD sampleIndex = 0;sampleIndex<regionSample1Count;sampleIndex++){
            *destSample++ = *srcSample++;
            *destSample++ = *srcSample++;
            soundOutput->runningSampleIndex++;
        }

        DWORD regionSample2Count = region2Size/soundOutput->bytesPerSample;
        destSample = (int16_t *)region2;
        srcSample = soundBuffer->samples;
        for (DWORD sampleIndex = 0;sampleIndex< regionSample2Count;sampleIndex++){
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


static void win32ProcessPendingMessages(
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
                                win32ProcessKeyboardMessage(&keyboardController->moveUp,isDown);
                                OutputDebugStringA("Pressed W\n");
                                break;
                            case 'A':
                                win32ProcessKeyboardMessage(&keyboardController->moveLeft,isDown);
                                OutputDebugStringA("Pressed A\n");
                                break;
                            case 'S':
                                win32ProcessKeyboardMessage(&keyboardController->moveDown,isDown);
                                OutputDebugStringA("Pressed S\n");
                                break;
                            case 'D':
                                win32ProcessKeyboardMessage(&keyboardController->moveRight,isDown);
                                OutputDebugStringA("Pressed D\n");
                                break;
                            case 'Q':
                                OutputDebugStringA("Pressed Q\n");
                                break;
                            case 'E':
                                OutputDebugStringA("Pressed E\n");
                                break;
                            case VK_UP: 
                                win32ProcessKeyboardMessage(&keyboardController->actionUp,isDown);
                                OutputDebugStringA("Pressed UP\n");
                                break;
                            case VK_DOWN: 
                                win32ProcessKeyboardMessage(&keyboardController->actionDown,isDown);
                                OutputDebugStringA("Pressed DOWN\n");
                                break;
                            case VK_LEFT: 
                                win32ProcessKeyboardMessage(&keyboardController->actionLeft,isDown);
                                OutputDebugStringA("Pressed LEFT\n");
                                break;
                            case VK_RIGHT: 
                                win32ProcessKeyboardMessage(&keyboardController->actionRight,isDown);
                                OutputDebugStringA("Pressed RIGHT\n");
                                break;
                            case VK_SPACE:
                                OutputDebugStringA("Pressed SPACE\n");
                                break;
                            case VK_ESCAPE:
                                OutputDebugStringA("Pressed ESC\n");
                                break;
                            case VK_F4:
                                bool32 isAltDown = (message.lParam & (1 << 29));
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
    if (value < deadzone ) {
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
                OutputDebugStringA("WM_ACTIVATE\n");
            }break;
        case WM_PAINT:
            { 
                PAINTSTRUCT paint;
                HDC deviceContext = BeginPaint(hwnd,&paint);
                win32windowDimensions dimensions = win32getWindowDimensions(hwnd);
                Win32displayBufferInWindow(&g_frameBuffer,deviceContext,dimensions.width,dimensions.height);
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

int CALLBACK WinMain(
        HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        int         nCmdShow
        )
{
    win32LoadInput();
    WNDCLASS WindowClass =  {}; 

    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 768;
    Win32ResizeDIBSection(&g_frameBuffer,WINDOW_WIDTH,WINDOW_HEIGHT);


    WindowClass.style =  CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";



    LARGE_INTEGER performanceFrequency;
    QueryPerformanceFrequency(&performanceFrequency);

    if (RegisterClass(&WindowClass)) {


        HWND hwnd= CreateWindowEx (
                0,
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

            HDC deviceContext = GetDC(hwnd);

            win32_soundOutput soundOutput;

            soundOutput.samplesPerSecond = 48000;
            soundOutput.runningSampleIndex =0;
            soundOutput.bytesPerSample = sizeof(int16_t)*2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond*soundOutput.bytesPerSample;
            soundOutput.sampleLatency= soundOutput.samplesPerSecond / 15;
            soundOutput.tSine= 0.0f;


            win32InitSound(hwnd,soundOutput.samplesPerSecond,soundOutput.secondaryBufferSize);

            win32ClearBuffer(&soundOutput);
            g_secondaryBuffer->Play(0,0,DSBPLAY_LOOPING);

            g_running = true;


            int16_t *samples = (int16_t*) VirtualAlloc(0,soundOutput.secondaryBufferSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);

#ifdef DEBUG_MODE
            LPVOID baseAddress = (LPVOID)TERABYTES((uint64_t)2);
#else 
            LPVOID baseAddress = 0;
#endif 
            GameMemory gameMemory = {};

            gameMemory.permanentStorageSize = MEGABYTES(64); 
            gameMemory.transientStorageSize = GIGABYTES(1);

            gameMemory.permanentStorage = VirtualAlloc(baseAddress,gameMemory.permanentStorageSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
            gameMemory.isInitialized = false;

            gameMemory.transientStorage = VirtualAlloc((uint8_t*)gameMemory.permanentStorage + gameMemory.permanentStorageSize,gameMemory.transientStorageSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);

            if(samples && gameMemory.permanentStorage){

                GameInput gameInput[2] ={};
                GameInput *newInput = &gameInput[0];
                GameInput *oldInput = &gameInput[1];


                LARGE_INTEGER lastCounter;
                uint64_t  lastCycleCount = __rdtsc();
                QueryPerformanceCounter(&lastCounter);

                //Game Loop
                for (;g_running;){

                    GameControllerInput *newKeyboardController = getController(newInput,0);
                    GameControllerInput *oldKeyboardController = getController(oldInput,0);
                    *newKeyboardController = {};
                    newKeyboardController->isConnected = true;
                    newKeyboardController->isAnalog = false;

                    for (int buttonIndex=0;
                            buttonIndex<ARRAY_COUNT(newKeyboardController->buttons);
                            ++buttonIndex){
                        newKeyboardController->buttons[buttonIndex].endedDown =
                            oldKeyboardController->buttons[buttonIndex].endedDown;
                    }
                    
                    win32ProcessPendingMessages(newKeyboardController);

                    //Input 
                    DWORD maxControllerCount  = XUSER_MAX_COUNT ;

                    if(maxControllerCount > (ARRAY_COUNT(newInput->controllers) - 1)){
                        maxControllerCount = (ARRAY_COUNT(newInput->controllers) - 1) ;
                    }

                    for(DWORD controllerIndex =0; controllerIndex < maxControllerCount; controllerIndex++)
                    {
                        DWORD ourControllerIndex = controllerIndex + 1;
                        GameControllerInput *oldController = getController(oldInput,ourControllerIndex);
                        GameControllerInput *newController = getController(newInput,ourControllerIndex);

                        XINPUT_STATE inputState ; 
                        if (XInputGetState(controllerIndex,&inputState) == ERROR_SUCCESS){
                            //Controller is plugged in
                            //Look at inputState.dwPacketNumber
                            newController->isConnected = true;

                            newController->isAnalog = true;
                            XINPUT_GAMEPAD *pad = &inputState.Gamepad;

                            newController->stickAverageX = win32ProcessXInputStickPosition(pad->sThumbLX,
                                    XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);


                            newController->stickAverageY =  win32ProcessXInputStickPosition(pad->sThumbLY,
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

                            


                            win32ProcessDigitalButton(&newController->moveLeft,
                                    &oldController->moveLeft,1,
                                    (newController->stickAverageX < -treshold) ? 1 : 0
                                    );

                            win32ProcessDigitalButton(&newController->moveRight,&oldController->moveRight,
                                    1,(newController->stickAverageX > treshold) ? 1 : 0
                                    );

                            win32ProcessDigitalButton(&newController->moveUp,&oldController->moveUp,
                                    1,(newController->stickAverageY < -treshold) ? 1 : 0
                                    );

                            win32ProcessDigitalButton(&newController->moveDown,&oldController->moveDown,
                                    1,(newController->stickAverageY > treshold) ? 1 : 0
                                    );

                            win32ProcessDigitalButton(&newController->actionDown,
                                    &oldController->actionDown,
                                    XINPUT_GAMEPAD_A,
                                    pad->wButtons);
                            win32ProcessDigitalButton(&newController->actionRight,
                                    &oldController->actionRight,
                                    XINPUT_GAMEPAD_B,
                                    pad->wButtons);
                            win32ProcessDigitalButton(&newController->actionLeft,
                                    &oldController->actionLeft,
                                    XINPUT_GAMEPAD_X,
                                    pad->wButtons);
                            win32ProcessDigitalButton(&newController->actionUp,
                                    &oldController->actionUp,
                                    XINPUT_GAMEPAD_Y,
                                    pad->wButtons);
                            win32ProcessDigitalButton(&newController->leftShoulder,
                                    &oldController->leftShoulder,
                                    XINPUT_GAMEPAD_LEFT_SHOULDER,
                                    pad->wButtons);
                            win32ProcessDigitalButton(&newController->rightShoulder,
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

                    FrameBuffer frameBuffer = {};
                    frameBuffer.bitmapMemory = g_frameBuffer.bitmapMemory; 
                    frameBuffer.width= g_frameBuffer.width; 
                    frameBuffer.height= g_frameBuffer.height; 
                    frameBuffer.pitch= g_frameBuffer.pitch; 

                    //Rendering

                    //DirectSound Output test 
                    DWORD byteToLock=0;
                    DWORD targetCursor=0;
                    DWORD bytesToWrite=0;
                    DWORD playCursor=0;
                    DWORD writeCursor=0;
                    bool32 soundIsValid = false;


                    if (SUCCEEDED(g_secondaryBuffer->GetCurrentPosition(&playCursor,&writeCursor))) {

                        byteToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) 
                            % soundOutput.secondaryBufferSize;
                        targetCursor = (playCursor + (soundOutput.sampleLatency*soundOutput.bytesPerSample)) 
                            % soundOutput.secondaryBufferSize; 

                        bytesToWrite=0;

                        if (byteToLock > targetCursor){
                            bytesToWrite = (soundOutput.secondaryBufferSize-byteToLock);
                            bytesToWrite += targetCursor;
                        } else {
                            bytesToWrite = targetCursor - byteToLock;
                        } 
                        soundIsValid = true;
                    }

                    SoundBuffer soundBuffer = {};
                    soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond; 
                    soundBuffer.sampleCount = bytesToWrite / soundOutput.bytesPerSample; 
                    soundBuffer.samples = samples;  

                    GameUpdateAndRender(&gameMemory, &frameBuffer, &soundBuffer,newInput);

                    if (soundIsValid){
                        win32FillSoundBuffer(&soundOutput,&soundBuffer,byteToLock,bytesToWrite);
                    }

                    win32windowDimensions dimensions  = win32getWindowDimensions(hwnd);

                    Win32displayBufferInWindow(&g_frameBuffer,deviceContext,dimensions.width,dimensions.height);
#if 0
                    LARGE_INTEGER endCounter;
                    uint64_t  endCycleCount = __rdtsc();
                    QueryPerformanceCounter(&endCounter);
                    float counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
                    float timeElapsedInMs= (1000*counterElapsed) / performanceFrequency.QuadPart;
                    float cyclesElapsed = endCycleCount - lastCycleCount;
                    char buffer[256];
                    sprintf(buffer,"ms/loop %0.2f FPS %0.2f MegaCycles %0.2f\n",timeElapsedInMs,1000/timeElapsedInMs,cyclesElapsed/1000000);
                    OutputDebugStringA(buffer);
                    lastCounter = endCounter;
                    lastCycleCount = endCycleCount;
#endif
                    GameInput *temp = newInput;
                    newInput = oldInput;
                    oldInput = temp;
                    Sleep(1);
                }
                
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
