
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include "win32_handmade.h"
#include "handmade.cpp"


#define internal static
#define global_variable static
#define local_persist static




//GRAPHICS 

static win32offscreenBuffer g_frameBuffer;


static win32windowDimensions win32getWindowDimensions(HWND hwnd) 
{
    win32windowDimensions retDimensions;
    RECT clientRect;
    GetClientRect(hwnd,&clientRect);
    retDimensions.width= clientRect.right - clientRect.left;
    retDimensions.height = clientRect.bottom - clientRect.top;
    return retDimensions;
}

static void Win32ResizeDIBSection(win32offscreenBuffer *buffer,int width, int height)
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
    buffer->bitmapMemory = VirtualAlloc(0,bitmapMemorySize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);


}

static void Win32displayBufferInWindow(win32offscreenBuffer *buffer, HDC deviceContext, int width , int height)
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
        HMODULE xInputLibrary = LoadLibrary("xinput1_3.dll");
    }
    if (xInputLibrary){

        XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary,"XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary,"XInputSetState");
    }
}

static void win32ProcessDigitalButton(GameButtonState *newState, GameButtonState *oldState, DWORD buttonBit,DWORD xInputButtonState) {
    newState->halfTransitionCount = (newState->endedDown != oldState->endedDown) ? 1 : 0;
    newState->endedDown= (xInputButtonState & buttonBit) & buttonBit;
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
        case  WM_SYSKEYDOWN:
            {
            }break;
        case  WM_SYSKEYUP:
            {
            }break;
        case  WM_KEYDOWN:
            {
            }break;
        case  WM_KEYUP:
            {
                uint32_t vKcode = wParam;
                bool32 wasDown = (lParam & (1 << 30));
                bool32 isDown = (lParam & (1 << 31));

                if (wasDown!=isDown) {
                    switch (vKcode) {
                        case 'W':
                            OutputDebugStringA("Pressed W\n");
                            break;
                        case 'A':
                            OutputDebugStringA("Pressed A\n");
                            break;
                        case 'S':
                            OutputDebugStringA("Pressed S\n");
                            break;
                        case 'D':
                            OutputDebugStringA("Pressed D\n");
                            break;
                        case 'Q':
                            OutputDebugStringA("Pressed Q\n");
                            break;
                        case 'E':
                            OutputDebugStringA("Pressed E\n");
                            break;
                        case VK_UP: 
                            OutputDebugStringA("Pressed UP\n");
                            break;
                        case VK_DOWN: 
                            OutputDebugStringA("Pressed DOWN\n");
                            break;
                        case VK_LEFT: 
                            OutputDebugStringA("Pressed LEFT\n");
                            break;
                        case VK_RIGHT: 
                            OutputDebugStringA("Pressed RIGHT\n");
                            break;
                        case VK_SPACE:
                            OutputDebugStringA("Pressed SPACE\n");
                            break;
                        case VK_ESCAPE:
                            OutputDebugStringA("Pressed ESC\n");
                            break;
                        case VK_F4:
                            bool32 isAltDown = (lParam & (1 << 29));
                            if (isAltDown){
                                g_running=false;
                                OutputDebugStringA("Pressed ALT+F4\n");
                            }
                            break;
                    }
                }

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

    Win32ResizeDIBSection(&g_frameBuffer,1280,720);


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
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                hInstance,
                0);

        if(hwnd) {

            HDC deviceContext = GetDC(hwnd);
            MSG message;

            win32_soundOutput soundOutput;

            soundOutput.samplesPerSecond = 48000;
            soundOutput.runningSampleIndex =0;
            soundOutput.bytesPerSample = sizeof(int16_t)*2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond*soundOutput.bytesPerSample;
            soundOutput.sampleLatency= soundOutput.samplesPerSecond / 15;
            soundOutput.tSine= 0.0f;


            win32InitSound(hwnd,soundOutput.samplesPerSecond,soundOutput.secondaryBufferSize);
            int16_t *samples = (int16_t*) VirtualAlloc(0,soundOutput.secondaryBufferSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);

            win32ClearBuffer(&soundOutput);
            g_secondaryBuffer->Play(0,0,DSBPLAY_LOOPING);

            g_running = true;


            GameMemory gameMemory = {};
            gameMemory.permanentStorageSize = MEGABYTES(64); 
            gameMemory.permanentStorage = VirtualAlloc(0,gameMemory.permanentStorageSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
            gameMemory.isInitialized = false;

            GameInput gameInput[2];
            GameInput *newInput = &gameInput[0];
            GameInput *oldInput = &gameInput[1];


            LARGE_INTEGER lastCounter;
            uint64_t  lastCycleCount = __rdtsc();
            QueryPerformanceCounter(&lastCounter);

            //Game Loop
            for (;g_running;){

                GameInput input = {};

                while(PeekMessage(&message,0,0,0,PM_REMOVE)){

                    if (message.message == WM_QUIT){
                        g_running = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                //Input 
                int maxControllerCount  = XUSER_MAX_COUNT;
                if(maxControllerCount > ARRAY_COUNT(input.controllers)){
                    maxControllerCount = ARRAY_COUNT(input.controllers);
                }
                for(DWORD controllerIndex =0;controllerIndex<maxControllerCount; controllerIndex++)
                {
                    Input *oldController = &oldInput->controllers[controllerIndex];
                    Input *newController = &newInput->controllers[controllerIndex];

                    XINPUT_STATE inputState ; 
                    if (XInputGetState(controllerIndex,&inputState) == ERROR_SUCCESS){
                        //Controller is plugged in
                        //Look at inputState.dwPacketNumber
                        XINPUT_GAMEPAD *pad = &inputState.Gamepad;
                        bool dpadUp = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool dpaDown = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool dpadLeft= (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool dpadRight = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

                        newController->isAnalog = true;
                        newController->startX = oldController->endX;
                        newController->startY = oldController->endY;


                        float x;

                        if (pad->sThumbLX < 0) {
                            x = (float)pad->sThumbLX / 32768.0f;
                        } else {
                            x = (float)pad->sThumbLX / 32767.0f;
                        }

                        float y;

                        if (pad->sThumbLY < 0) {
                            y = (float)pad->sThumbLX / 32768.0f;
                        } else {
                            y = (float)pad->sThumbLX / 32767.0f;
                        }


                        newController->minX = newController->maxX = newController->endX = x;
                        newController->minY = newController->maxY = newController->endY = y;

                        win32ProcessDigitalButton(&newController->down,
                                &oldController->down,
                                XINPUT_GAMEPAD_A,
                                pad->wButtons);
                        win32ProcessDigitalButton(&newController->right,
                                &oldController->right,
                                XINPUT_GAMEPAD_B,
                                pad->wButtons);
                        win32ProcessDigitalButton(&newController->left,
                                &oldController->left,
                                XINPUT_GAMEPAD_X,
                                pad->wButtons);
                        win32ProcessDigitalButton(&newController->up,
                                &oldController->up,
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
                DWORD byteToLock;
                DWORD targetCursor;
                DWORD bytesToWrite;
                DWORD playCursor;
                DWORD writeCursor;
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

                GameUpdateAndRender(&frameBuffer, &soundBuffer,newInput);

                if (soundIsValid){
                    win32FillSoundBuffer(&soundOutput,&soundBuffer,byteToLock,bytesToWrite);
                }

                {
                    win32windowDimensions dimensions  = win32getWindowDimensions(hwnd);
                    Win32displayBufferInWindow(&g_frameBuffer,deviceContext,dimensions.width,dimensions.height);
                }
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
            }

            GameInput *temp = newInput;
            newInput = oldInput;
            oldInput = temp;
            VirtualFree(samples, 0, MEM_RELEASE);
        }
        else {
            //TODO (Logging)
        }
    }
    else {
        //TODO (Logging)
    }

    return (0);
}
