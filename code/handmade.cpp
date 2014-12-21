#include "handmade.h"
#include "win32_handmade.h"


//bool32 behaves like in C not like bool in C++:
#define bool32 int32_t
#define pi 3.14159265359f




void RenderSoundSamples(FrameBuffer  *frameBuffer, SoundBuffer *soundBuffer){
    uint8_t *row = (uint8_t *) frameBuffer->bitmapMemory;

    const int PADDING= 20;
    const int VOLUME= 3000;
    float cX =  (float)soundBuffer->samplesPerSecond/(float)frameBuffer->width; 
    float cY = (float)frameBuffer->height / (((float)VOLUME*2)); 



        uint32_t *pixel =(uint32_t *) row;

        for (int x = 0;
                x < frameBuffer->width;
                ++x){

            pixel = (uint32_t*) row + x ;

            for (int y = 0;
                    y< frameBuffer->height;
                    ++y){
                *pixel= (uint32_t) 0x00000000;
                pixel += frameBuffer->pitch/4;
            }

            //uint32_t pixelPosition= (uint32_t)(cY*soundBuffer->samples[(int)(x*cX)]*frameBuffer->pitch + x) ;
            for (int i=0;i < 10; i++){
                uint32_t scaledSampleIndex = (uint32_t)((float)x+(i*frameBuffer->width)) % 48000;
                int16_t sampleValue = soundBuffer->samples[scaledSampleIndex*2];
                uint32_t scaledSampleValue = (uint32_t) (cY*((float)(sampleValue + VOLUME)));

                int pixelPosition =  (scaledSampleValue/10 + (i*(frameBuffer->height/10)))*(frameBuffer->pitch/4) ;

                pixel = (uint32_t *)row + pixelPosition + x; 
                *pixel = 0xFF00FF00;
            }
        }
}


void RenderWeirdGradient(FrameBuffer *buffer,int xOffset,int yOffset)      
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
            uint8_t redValue = (uint8_t) 128;
            uint8_t blueValue = (uint8_t)(x+xOffset);
            uint8_t greenValue = (uint8_t)(y+yOffset);

            //*pixel++ = ((uint32_t)(greenValue) << GREEN) | ((uint32_t) (blueValue));
            *pixel++ = ((uint32_t) (redValue << GREEN ) | (uint32_t) (greenValue << GREEN )  | (uint32_t) (blueValue << BLUE));

        }
        row += buffer->pitch;
    }

}

void gameOutputSound(SoundBuffer *soundBuffer,float tSine, int toneHz)
{ 

    int16_t toneVolume = 3000;

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
            2.0f*pi*1.0f/(float)wavePeriod ;

        if (tSine >= 2.0f*pi) {
            tSine-= 2.0f*pi;
        }

    }
}


static void RenderPlayer(FrameBuffer *buffer, int playerX, int playerY)
{
    uint32_t color = 0xFFFFFFFF;
    int  bufferSize= buffer->width*buffer->height*buffer->bytesPerPixel;
    uint8_t *byte = (uint8_t *) buffer->bitmapMemory+ 
        playerY*buffer->pitch  + 
        playerX*buffer->bytesPerPixel;

    for (int y=playerY;y<playerY+10;y++){
        uint32_t *pixel = (uint32_t*)byte;


        for (int x=playerX;x<playerX+10;x++){
            if ((uint8_t *)pixel>= buffer->bitmapMemory && (uint8_t*)pixel <= ((uint8_t *)(buffer->bitmapMemory) + bufferSize)){
                *pixel++ = color;
            }
        }
        byte += buffer->pitch;

    }
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) 
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;

    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);

    if (!gameMemory->isInitialized){

        gameState->toneHz = 256;
        gameState->tSine= 0.0f;
        gameState->offsetX= 0;
        gameState->offsetY= 0;
        gameState->playerX = 100;
        gameState->playerY = 100;


        char *filename = __FILE__;
        debugReadFileResult fileReadResult;
        fileReadResult = gameMemory->DEBUGReadEntireFile(context,filename);

        if(fileReadResult.fileContent){
            gameMemory->DEBUGWriteEntireFile(context,"..\\data\\temp.tmp",fileReadResult.contentSize,fileReadResult.fileContent);
            gameMemory->DEBUGFreeFileMemory(context,fileReadResult.fileContent);
        }

        gameMemory->isInitialized = true;
    }

    for (int controllerIndex=0;
            controllerIndex < ARRAY_COUNT(gameInput->controllers);
            controllerIndex++){

        GameControllerInput *controller = getController(gameInput,controllerIndex);
        if (controller->isAnalog) {
            //gameState->toneHz = 256 + (int)(128.0f * (controller->stickAverageX));
            //gameState->offsetX = (int) (100.0f*(controller->stickAverageX));
            //gameState->offsetY = (int) (100.0f*(controller->stickAverageY));

            gameState->playerX += (int) (10.0f*(controller->stickAverageX));
            gameState->playerY -= (int) (10.0f*(controller->stickAverageY));
            if (controller->actionDown.endedDown){
                gameState->tJumpTimer = 4.0f;
            }
            if (gameState->tJumpTimer > 0){
                gameState->playerY += (int)(10.0f*sinf(1.0f*pi*gameState->tJumpTimer));
            gameState->tJumpTimer -=0.033f;
            }

        } else  {
            if (controller->moveUp.endedDown){
                gameState->tJumpTimer = 4.0f;
            }
            if (gameState->tJumpTimer > 0){
                gameState->playerY += (int)(10.0f*sinf(1.0f*pi*gameState->tJumpTimer));
                gameState->tJumpTimer -=0.033f;
            }
        }

    }


    RenderWeirdGradient(buffer, gameState->offsetX,gameState->offsetY);
    //RenderPlayer(buffer,gameState->playerX,gameState->playerY);
    RenderPlayer(buffer,gameInput->mouseX,gameInput->mouseY);
    //RenderSoundSamples(frameBuffer,soundBuffer);
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;
    gameOutputSound(soundBuffer,gameState->tSine,gameState->toneHz);
}

