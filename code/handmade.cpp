#include "handmade.h"
#include "win32_handmade.h"

//bool32 behaves like in C not like bool in C++:
#define bool32 int32_t
#define pi 3.14159265359f



static void RenderWeirdGradient(FrameBuffer *buffer,int xOffset,int yOffset)      
{
    uint8_t *row = (uint8_t *) buffer->bitmapMemory;

    for (int y = 0;
            y< buffer->height;
            ++y){

        uint32_t *pixel =(uint32_t *) row;

        const int RED = 16;
        const int GREEN = 8;
        const int BLUE= 0;

        for (int x = 0;
                x< buffer->width;
                ++x){
            /*
             * Pixel in memory RR GG BB 00 
             *                 */
            uint8_t blueValue = (x+xOffset);
            uint8_t greenValue = (y+yOffset);

            //*pixel++ = ((uint32_t)(greenValue) << GREEN) | ((uint32_t) (blueValue));
            *pixel++ = ((uint32_t) (greenValue) << GREEN)  | ((uint32_t) blueValue);

        }
        row += buffer->pitch;
    }

}

static void GameOutputSound(SoundBuffer *soundBuffer,int toneHz)
{ 

    static float tSine;
    int toneVolume = 3000;

    int wavePeriod = 
        soundBuffer->samplesPerSecond/toneHz;

    int16_t *sampleOut = soundBuffer->samples;

    for (int sampleIndex = 0; 
            sampleIndex < soundBuffer->sampleCount; 
            sampleIndex++) {

        float sineValue = sinf(tSine);
        int16_t sampleValue = int16_t(sineValue * toneVolume); 
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        tSine += 
            (2.0f * pi * (1.0f))/(float)wavePeriod;

    }
}


static void GameUpdateAndRender(GameMemory *gameMemory, FrameBuffer *frameBuffer, 
        SoundBuffer *soundBuffer,
        GameInput *gameInput)
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;
	
    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);

    if (!gameMemory->isInitialized){

        gameState->toneHz = 256;
        gameState->offsetX= 0;
        gameState->offsetY= 0;


        char *filename = __FILE__;
        debugReadFileResult fileReadResult;
        fileReadResult = DEBUGplatformReadEntireFile(filename);

        if(fileReadResult.fileContent){
            DEBUGplatformWriteEntireFile("..\\data\\temp.tmp",fileReadResult.contentSize,fileReadResult.fileContent);
            DEBUGplatformFreeFileMemory(fileReadResult.fileContent);
        }

        gameMemory->isInitialized = true;
    }

    Input *player1Input= &gameInput->controllers[0];
    if (player1Input->isAnalog) {
        gameState->toneHz = 256 + (int)(128.0f * (player1Input->endX));
        gameState->offsetX = (int) 100.0f*(player1Input->endX);
        gameState->offsetY = (int) 100.0f*(player1Input->endY);
    } else {
    }

    if( player1Input->down.endedDown) {
        gameState->offsetX +=1;
    }



    GameOutputSound( soundBuffer,gameState->toneHz);
    RenderWeirdGradient(frameBuffer, gameState->offsetX,gameState->offsetY);
}

