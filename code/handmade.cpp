#include "handmade.h"
#include "win32_handmade.h"


//bool32 behaves like in C not like bool in C++:
#define bool32 int32_t
#define pi 3.14159265359f
#define GREEN_COLOR  0x0000FF00
#define RED_COLOR  0x00FF0000
#define BLUE_COLOR  0x000000FF



#if 0
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
            uint32_t scaledSampleIndex = 
                (uint32_t)((float)x +(i*frameBuffer->width)) % 48000;
            int16_t sampleValue = 
                soundBuffer->samples[scaledSampleIndex*2];
            uint32_t scaledSampleValue = 
                (uint32_t) (cY*((float)(sampleValue + VOLUME)));

            int pixelPosition =  
                (scaledSampleValue/10 + 
                 (i*(frameBuffer->height/10)))*(frameBuffer->pitch/4) ;

            pixel = (uint32_t *)row + pixelPosition + x; 
            *pixel = 0xFF00FF00;
        }
    }
}


void renderWeirdGradient(FrameBuffer *buffer,int xOffset,int yOffset)      
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
            *pixel++ = ((uint32_t) (redValue << GREEN ) | 
                    (uint32_t) (greenValue << GREEN )  | 
                    (uint32_t) (blueValue << BLUE));

        }
        row += buffer->pitch;
    }

}

static void renderPlayer(FrameBuffer *buffer, int playerX, int playerY)
{
    uint32_t color = 0xFFFFFFFF;
    int  bufferSize= buffer->width*buffer->height*buffer->bytesPerPixel;
    uint8_t *byte = (uint8_t *) buffer->bitmapMemory+ 
        playerY*buffer->pitch  + 
        playerX*buffer->bytesPerPixel;

    for (int y=playerY;y<playerY+10;y++){
        uint32_t *pixel = (uint32_t*)byte;


        for (int x=playerX;x<playerX+10;x++){
            if ((uint8_t *)pixel>= buffer->bitmapMemory && 
                    (uint8_t*)pixel <= ((uint8_t *)(buffer->bitmapMemory) 
                        + bufferSize)){
                *pixel++ = color;
            }
        }
        byte += buffer->pitch;

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
#endif

void gameOutputSound(SoundBuffer *soundBuffer)
{ 


    int16_t *sampleOut = soundBuffer->samples;

    for (int sampleIndex = 0; 
            sampleIndex < soundBuffer->sampleCount; 
            sampleIndex++) {
        *sampleOut++ = 0;
        *sampleOut++ = 0;
    }
}


inline int32_t roundFloatToInt(float floatNumber) 
{
    int32_t result= (int32_t) (floatNumber + 0.5f);
    return result;
}

inline int32_t truncateFloatToInt(float floatNumber) 
{
    int32_t result = (int32_t) (floatNumber);
    return result;
}

static void drawRect(FrameBuffer *buffer, 
        float fMinX, float fMinY, float fMaxX, float fMaxY,
        float R, float G, float B)
{
    int32_t minX = roundFloatToInt(fMinX);
    int32_t minY = roundFloatToInt(fMinY);
    int32_t maxX = roundFloatToInt(fMaxX);
    int32_t maxY = roundFloatToInt(fMaxY);

    if (minX < 0){
        minX = 0;
    }
    if (minY < 0){
        minY = 0;
    }
    if (minX > buffer->width){
        minX = buffer->width;
    }
    if (minY > buffer->height){
        minY = buffer->height;
    }

    uint32_t color = roundFloatToInt(R*255.0f) << 16 |
        roundFloatToInt(G*255.0f) << 8 |
        roundFloatToInt(B*255.0f);

    int  bufferSize= buffer->width*buffer->height*buffer->bytesPerPixel;

    uint8_t *byte = (uint8_t *) buffer->bitmapMemory+ 
        minY*buffer->pitch  + 
        minX*buffer->bytesPerPixel; 
    for (int y=minY;y<maxY;y++){
        uint32_t *pixel = (uint32_t*)byte;

        for (int x=minX;x<maxX;x++){
            if ((uint8_t *)pixel>= buffer->bitmapMemory && 
                    (uint8_t*)pixel <= ((uint8_t *)(buffer->bitmapMemory) 
                        + bufferSize)){
                *pixel++ = color;
            }
        }
        byte += buffer->pitch;

    }
}

inline uint32_t getTileValue(TileMap *tileMap,
        uint32_t tileX,
        uint32_t tileY)
{
    uint32_t result = 
        *(uint32_t *)(tileMap->tiles + 
                (tileMap->countX*tileY+ tileX));
    return result ;
}

static bool32 isMapTileEmpty(TileMap *tileMap, 
        float newPlayerX, 
        float newPlayerY)
{

    int32_t playerTileX = 
        truncateFloatToInt((newPlayerX - tileMap->upperLeftX)/
                tileMap->tileWidth);
    int32_t playerTileY = 
        truncateFloatToInt((newPlayerY- tileMap->upperLeftY)/
                tileMap->tileHeight);

    bool32 result= false;

    if (playerTileX <= tileMap->countX && playerTileX >=0 &&
            playerTileY <= tileMap->countY && playerTileY >=0){
        result = (getTileValue(tileMap,playerTileX,playerTileY) == 0);
    }
    return result;
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) 
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;

    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);
    const int MAP_WIDTH = 17;
    const int MAP_HEIGHT= 9;

    TileMap tileMap[2];

    uint32_t tiles0[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1}
    };
    uint32_t tiles1[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1}
    };

    tileMap[0].countX = MAP_WIDTH;
    tileMap[0].countY = MAP_HEIGHT;
    tileMap[0].tileWidth = 60.0f;
    tileMap[0].tileHeight= 60.0f;
    tileMap[0].upperLeftX= -30.0f;
    tileMap[0].upperLeftY= 0.0f;


    tileMap[0].tiles = (uint32_t *)tiles0;
    tileMap[1] = tileMap[0];
    tileMap[1].tiles = (uint32_t *)tiles1;

    TileMap *currentTileMap = &tileMap[0];


    if (!gameMemory->isInitialized){

        gameState->playerX=100.0f;
        gameState->playerY=150.0f;;
        gameMemory->isInitialized = true;
    }

    for (int controllerIndex=0;
            controllerIndex < ARRAY_COUNT(gameInput->controllers);
            controllerIndex++){

        GameControllerInput *controller = 
            getController(gameInput,controllerIndex);
        if (controller->isAnalog) {
        } else  {
            float dPlayerY = 0;
            float dPlayerX = 0;
            if (controller->moveUp.endedDown) {
                dPlayerY = -128.0f;
            }
            if (controller->moveDown.endedDown) {
                dPlayerY = 128.0f;
            }
            if (controller->moveLeft.endedDown) {
                dPlayerX = -128.0f;
            }
            if (controller->moveRight.endedDown) {
                dPlayerX = 128.0f;
            }                                     
            float newPlayerX = gameState->playerX +  gameInput->dtForFrame*dPlayerX;
            float newPlayerY = gameState->playerY +  gameInput->dtForFrame*dPlayerY;
            if (isMapTileEmpty(currentTileMap,newPlayerX,newPlayerY)) {
                gameState->playerX +=  gameInput->dtForFrame*dPlayerX;
                gameState->playerY +=  gameInput->dtForFrame*dPlayerY;
            } else {
                gameState->playerX = gameState->playerX;
                gameState->playerY = gameState->playerY;
            }
        }


        drawRect(buffer,-10.0f,-10.0f,(float)buffer->width+10.0f
                ,(float)buffer->height+10.0f,0.5f,0.0f,0);
        for (int row=0; row<MAP_HEIGHT;++row) {
            for (int column=0;column<MAP_WIDTH;++column){
                float gray = (getTileValue (currentTileMap,column,row) == 1) ? 0.5f : 1.0f ;
                float minX = currentTileMap->upperLeftX + 
                    ((float)column)*currentTileMap->tileWidth; 
                float minY= currentTileMap->upperLeftY + 
                    ((float)row)*currentTileMap->tileHeight; 
                float maxX=  minX + currentTileMap->tileWidth; 
                float maxY=  minY + currentTileMap->tileHeight; 

                drawRect(buffer,minX,minY,maxX,maxY,gray,gray,gray);
            }
            float playerR = 1.0f;
            float playerG = 1.0f;
            float playerB = 0.0f;
            float playerWidth = 0.75f*currentTileMap->tileWidth;
            float playerHeight= currentTileMap->tileHeight;
            float playerLeft =  gameState->playerX - 0.5f*playerWidth;
            float playerTop = gameState->playerY - playerHeight;

            drawRect(buffer,playerLeft,playerTop,
                    playerLeft+playerWidth ,
                    playerTop + playerHeight,
                    playerR,playerG,playerB);
        }

    }
}


extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;
    //gameOutputSound(soundBuffer,gameState->tSine,gameState->toneHz);
    gameOutputSound(soundBuffer);
}

