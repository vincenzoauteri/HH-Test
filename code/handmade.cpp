#include "handmade.h"

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

            *pixel++ = (uint32_t)(greenValue << GREEN) | (uint32_t) (blueValue);
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
        //float t = 2.0 * pi * ((float)soundOutput->runningSampleIndex / ((float) soundOutput->samplesPerSecond/(soundOutput->toneHz+g_pitch)));
        //float sineValue = sinf(t);
        int16_t sampleValue = int16_t(sineValue * toneVolume); 
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        tSine += 
            (2.0f * pi * (1.0f))/(float)wavePeriod;

    }
}

static void GameUpdateAndRender(GameMemory *memory, FrameBuffer *frameBuffer, 
        SoundBuffer *soundBuffer,
        GameInput *gameInput)
{

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized){
        gameState->toneHz = 256;
        gameState->offsetX= 0;
        gameState->offsetY= 0;
    }

    Input *player1Input= &gameInput->controllers[0];
    if (player1Input->isAnalog) {
        toneHz = 256 + (int)(128.0f * (player1Input->endX));
        offsetY = (int) 4.0f*(player1Input->endY);
    } else {
    }

    if( player1Input->down.endedDown) {
        offsetX +=1;
    }



    GameOutputSound( soundBuffer,toneHz);
    RenderWeirdGradient(frameBuffer, offsetX,offsetY);
}

