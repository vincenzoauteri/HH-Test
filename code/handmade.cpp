#include "handmade.h"
#include "handmade_random.h"
#include "handmade_intrinsics.h"
#include "handmade_tilemap.cpp"
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

static void renderPlayer(FrameBuffer *buffer, int playerP.tileRelX, int playerP.tileRelY)
{
    uint32_t color = 0xFFFFFFFF;
    int  bufferSize= buffer->width*buffer->height*buffer->bytesPerPixel;
    uint8_t *byte = (uint8_t *) buffer->bitmapMemory+ 
        playerP.tileRelY*buffer->pitch  + 
        playerP.tileRelX*buffer->bytesPerPixel;

    for (int y=playerP.tileRelY;y<playerP.tileRelY+10;y++){
        uint32_t *pixel = (uint32_t*)byte;


        for (int x=playerP.tileRelX;x<playerP.tileRelX+10;x++){
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


static void iniatilizeArena(MemoryArena *arena, 
        size_t size,
        uint8_t *base) 
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}



extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) 
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;

    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);

    uint32_t tileSizeInPixels = 60;
    float metersToPixels = 0; 

    if (!gameMemory->isInitialized){

        gameState->playerP.absTileX = 1;
        gameState->playerP.absTileY = 3;
        gameState->playerP.absTileZ = 0;
        gameState->playerP.tileRelX = 5.0f;
        gameState->playerP.tileRelY = 5.0f; 
        gameMemory->isInitialized = true;

        const int TILE_MAP_COUNT_X = 256;
        const int TILE_MAP_COUNT_Y = 256;

        iniatilizeArena(&gameState->worldArena, 
                gameMemory->permanentStorageSize - sizeof(GameState),
                (uint8_t *) gameMemory->permanentStorage + sizeof(GameState));

        gameState->world = pushStruct(&gameState->worldArena, World);
        World *world = gameState->world;
        world->tileMap = pushStruct(&gameState->worldArena, TileMap);
        TileMap *tileMap = world->tileMap;

        tileMap->chunkShift = 4;
        tileMap->chunkMask = (1 << tileMap->chunkShift) - 1 ;
        tileMap->chunkDim = (1 << tileMap->chunkShift);
        tileMap->tileChunkCountX = 128;
        tileMap->tileChunkCountY = 128;
        tileMap->tileChunkCountZ = 2;
        tileMap->tileSizeInMeters = 2.0f;

        tileMap->tileChunks = pushArray(&gameState->worldArena, 
                tileMap->tileChunkCountX* 
                tileMap->tileChunkCountY* 
                tileMap->tileChunkCountZ, 
                TileChunk); 

        metersToPixels =  (float)tileSizeInPixels/tileMap->tileSizeInMeters;

        float lowerLeftX= -(float)tileSizeInPixels/2;
        float lowerLeftY= (float)buffer->height; 

        uint32_t tilesPerWidth = 17;
        uint32_t tilesPerHeight = 9;
        uint32_t screenX = 0;
        uint32_t screenY = 0;
        uint32_t randomNumberIndex = 0; 

        bool32 doorLeft = false;
        bool32 doorRight= false;
        bool32 doorTop= false;
        bool32 doorBottom= false;
        bool32 doorUp= false;
        bool32 doorDown= false;

        uint32_t absTileZ = 0; 
        for (uint32_t screenNumber = 0;
                screenNumber < 64;
                screenNumber++){

            uint32_t randomChoice = 0;
            if (!doorUp && !doorDown) {
                randomChoice = 
                    randomNumberTable[randomNumberIndex++ % 1000] % 3; 
            } else {
                randomChoice = 
                    randomNumberTable[randomNumberIndex++ % 1000] % 2 ;
            }

            if (randomChoice == 0) {
                if (absTileZ == 0){
                    doorUp = 1;
                } else {
                    doorDown = 1;
                }
            }

            else if (randomChoice == 1) {
                doorRight = true;
            } else {
                doorTop = true;
            }

            for (uint32_t tileY = 0;
                    tileY < tilesPerHeight;
                    ++tileY){
                for (uint32_t tileX = 0;
                        tileX< tilesPerWidth;
                        ++tileX){

                    uint32_t absTileX = screenX*tilesPerWidth  + tileX;
                    uint32_t absTileY = screenY*tilesPerHeight + tileY;
                    uint32_t tileValue = 1;

                    if ((tileX == 0) && 
                            (!doorLeft || (tileY != (tilesPerHeight/2)))){
                        tileValue = 2;
                    }
                    if ((tileX == (tilesPerWidth -1)) &&   
                            (!doorRight|| (tileY != (tilesPerHeight/2)))){
                        tileValue = 2;
                    }
                    if ((tileY == 0) && 
                            (!doorBottom|| (tileX != (tilesPerWidth/2)))){
                        tileValue = 2;
                    }
                    if ((tileY == (tilesPerHeight-1)) &&   
                            (!doorTop || (tileX != (tilesPerWidth/2)))){
                        tileValue = 2;
                    }
                    if ((tileX == 3) &&  (tileY == 3)) {
                        if (doorUp) {
                            tileValue = 3;
                        } else if (doorDown) {
                            tileValue = 4;
                        }
                    }

                    setTileValue(&gameState->worldArena,
                            tileMap,
                            absTileX,
                            absTileY,
                            absTileZ,
                            tileValue);
                }
            }

            if (randomChoice == 2) {
                if (absTileZ == 0) {
                    absTileZ = 1;
                } else {
                    absTileZ = 0;
                }
            }
            if (randomChoice == 1) {
                screenX += 1;
            } else {
                screenY += 1;
            }
            doorLeft = doorRight ? true : false;
            doorBottom =  doorTop ? true : false;
            doorRight = false;
            doorTop= false;

            if (!doorUp && !doorDown){
                doorUp= false;
                doorDown= false;
            } else if (doorUp) {
                doorUp = false;
                doorDown = true;
            } else if (doorDown) {
                doorUp = true;
                doorDown = false;
            }
        }
    }

    float playerHeight = 1.4f;
    float playerWidth = 0.75f*playerHeight;

    World *world = gameState->world;
    TileMap *tileMap = world->tileMap;
    metersToPixels =  (float)tileSizeInPixels/tileMap->tileSizeInMeters;


    for (int controllerIndex=0;
            controllerIndex < ARRAY_COUNT(gameInput->controllers);
            controllerIndex++){

        GameControllerInput *controller = 
            getController(gameInput,controllerIndex);
        if (controller->isAnalog) {
        } else  {
            float dPlayerY = 0.0f;
            float dPlayerX = 0.0f;
            if (controller->moveUp.endedDown) {
                dPlayerY = 1.0f;
            }
            if (controller->moveDown.endedDown) {
                dPlayerY = -1.0f;
            }
            if (controller->moveLeft.endedDown) {
                dPlayerX = -1.0f;
            }
            if (controller->moveRight.endedDown) {
                dPlayerX = 1.0f;
            }                                     

            float playerSpeed = 2.0f;
            if (controller->actionUp.endedDown) {
                playerSpeed = 10.0f;
            } 
            dPlayerX *= 2.0f * playerSpeed;
            dPlayerY *= 2.0f * playerSpeed;

            TileMapPosition newPlayerP = gameState->playerP;
            newPlayerP.tileRelX += (gameInput->dtForFrame*dPlayerX);
            newPlayerP.tileRelY += (gameInput->dtForFrame*dPlayerY);
            newPlayerP = recanonicalizePosition(tileMap,newPlayerP);

            TileMapPosition playerLeft = newPlayerP;
            playerLeft.tileRelX -= 0.5f*playerWidth;
            playerLeft= recanonicalizePosition(tileMap,playerLeft);

            TileMapPosition playerRight= newPlayerP;
            playerRight.tileRelX += 0.5f*playerWidth;
            playerRight = recanonicalizePosition(tileMap,playerRight);

            //Checks lower-left, lower-center and lower-right corners of 
            //the sprite for collisions
            if (isTileMapPointEmpty(tileMap, newPlayerP) && 
                    isTileMapPointEmpty(tileMap,playerLeft) && 
                    isTileMapPointEmpty(tileMap,playerRight)) {
                gameState->playerP = newPlayerP;
            } else {
            }
        }


        drawRect(buffer,0.0f,0.0f,(float)buffer->width
                ,(float)buffer->height,1.0f,0.0f,0.1f);

        float screenCenterX = 0.5f *(float)buffer->width;
        float screenCenterY = 0.5f *(float)buffer->height;

        for (int32_t relRow= -10; 
                relRow <  10;
                ++relRow) {
            for (int32_t relColumn= -10;
                    relColumn < 10;
                    ++relColumn ) {

                uint32_t column = relColumn + gameState->playerP.absTileX;
                uint32_t row = relRow + gameState->playerP.absTileY;

                uint32_t tileValue = getTileValue(tileMap,
                        column,
                        row,
                        gameState->playerP.absTileZ);

                float gray = 0.0f;
                if (tileValue > 0) {
                    switch (tileValue) {
                        case 1:
                            gray = 0.5f;
                            break;
                        case 2:
                            gray = 1.0f;
                            break;
                        case 3:
                            gray = 0.75f;
                            break;
                        case 4:
                            gray = 0.25f;
                            break;
                        default: 
                            gray = 0.0f;
                            break;
                    }
                    if( column == gameState->playerP.absTileX &&
                            row == gameState->playerP.absTileY) {
                        //gray = 0.0f;
                    }

                    float tileCenterX = screenCenterX- 
                        metersToPixels*gameState->playerP.tileRelX + 
                        ((float)relColumn)*tileSizeInPixels; 
                    float tileCenterY= screenCenterY+ 
                        metersToPixels*gameState->playerP.tileRelY - 
                        ((float)relRow)*tileSizeInPixels; 
                    float minX = tileCenterX - 0.5f*tileSizeInPixels; 
                    float minY = tileCenterY - 0.5f*tileSizeInPixels; 

                    float maxX =  tileCenterX + 0.5f * tileSizeInPixels; 
                    float maxY =  tileCenterY + 0.5f * tileSizeInPixels; 


                    drawRect(buffer,minX,minY,maxX,maxY,gray,gray,gray);
                }
            }
            float playerR = 1.0f;
            float playerG = 1.0f;
            float playerB = 0.0f;
            float playerLeft = screenCenterX-  
                //metersToPixels*gameState->playerP.tileRelX 
                0.5f*metersToPixels*playerWidth;
            float playerTop = screenCenterY-
                //metersToPixels*gameState->playerP.tileRelY
                metersToPixels*playerHeight;

            drawRect(buffer,
                    playerLeft, playerTop,
                    playerLeft + metersToPixels*playerWidth,
                    playerTop + metersToPixels*playerHeight,
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

