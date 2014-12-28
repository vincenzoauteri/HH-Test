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

#include <math.h>
inline int32_t floorFloatToInt(float floatNumber) 
{
    int32_t result = (int32_t)floorf(floatNumber);
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

inline uint32_t getTileValue(World *world,
        TileMap *tileMap,
        uint32_t tileX,
        uint32_t tileY)
{
    uint32_t result = 
        *(uint32_t *)(tileMap->tiles + 
                (world->countX*tileY+ tileX));
    return result ;
}


static bool32 isTileInMapEmpty(World *world,
        TileMap *tileMap, 
        int32_t tileX, 
        int32_t tileY)
{
    bool32 result= false;

    if (tileX<= world->countX && tileY>=0 &&
            tileY<= world->countY && tileY>=0) {
        result = (getTileValue(world,tileMap,tileX,tileY) == 0);
    }

    return result;
}


inline TileMap* getTileMapInWorld(World *world,
        int32_t tileX,
        int32_t tileY)
{
    TileMap *result = 0; 

    if (tileX >=0 && tileX < world->countX &&
            tileY>=0 && tileY < world->countY){
        result = &world->tileMaps[world->worldCountX*tileY+ tileX];
    }
    return result ;
}

inline WorldLocation getWorldLocation (World *world, RawLocation pos) 
{
    WorldLocation result;
    // Get to which tile in the current tile map would end the player
    // being with the current movement
    result.tileMapX = pos.tileMapX; 
    result.tileMapY = pos.tileMapY; 

    float X = pos.X -world->upperLeftX;
    float Y = pos.Y -world->upperLeftY;

    result.tileX = 
        floorFloatToInt((X)/
                world->tileWidth);
    result.tileY= 
        floorFloatToInt((Y)/
                world->tileHeight);

    result.X = X - result.tileX*world->tileWidth;
    result.Y = Y - result.tileY*world->tileHeight;
    //Check if the test tile is still inside the current tile map 
    //If not, change the coordinates so that it rolls over 
    //Also, update what the current tile map in the world would be
    if (result.tileX < 0) {
        result.tileX= world->countX + result.tileX;
        --result.tileMapX;
    } else  if (result.tileX >= world->countX) {
        result.tileX= world->countX - world->countX;
        ++result.tileMapX;
    }
    if (result.tileY < 0) {
        result.tileY = world->countY + result.tileY ;
        --result.tileMapY;
    } else if (result.tileY >= world->countY) {
        result.tileY = world->countY - world->countY;
        ++result.tileMapY;
    }
    return result;
}

static bool32 isWorldTileMapEmpty(World *world, 
        RawLocation rawPos)
{
    bool32 empty = false;

    WorldLocation worldPos = getWorldLocation(world,rawPos); 

    //Check if there is a defined tile map for the new world coordinates
    TileMap *tileMap = getTileMapInWorld(world,
            worldPos.tileMapX,
            worldPos.tileMapY);
    //Then checks if the tile in the tile map is empty (passage)
    if (tileMap) {
        empty = isTileInMapEmpty(world,
                tileMap,
                worldPos.tileX,
                worldPos.tileY);
    }

    return empty;
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) 
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;

    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);
    const int MAP_WIDTH = 17;
    const int MAP_HEIGHT= 9;

    TileMap tileMap[2][2]; 

    uint32_t tiles00[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,0},
        {1,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1}
    };
     uint32_t tiles01[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1}
    };
    uint32_t tiles10[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,0},
        {1,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1}
    };
    uint32_t tiles11[MAP_HEIGHT][MAP_WIDTH] =  
    {
        {1,1,1,1, 1,1,1,1, 0,1,1,1, 1,1,1,1,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,1},
        {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0,1},
        {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0,1},
        {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,1},
        {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1}
    };

    tileMap[0][0].tiles = (uint32_t *)tiles00;
    tileMap[0][1].tiles = (uint32_t *)tiles01;
    tileMap[1][0].tiles = (uint32_t *)tiles10;
    tileMap[1][1].tiles = (uint32_t *)tiles11;

    World world;
    world.worldCountX = 2;
    world.worldCountY = 2;
    world.tileMaps = (TileMap *) tileMap;
    world.countX = MAP_WIDTH;
    world.countY = MAP_HEIGHT;
    world.tileWidth = 60.0f;
    world.tileHeight= 60.0f;
    world.upperLeftX= 0.0f;
    world.upperLeftY= 0.0f;


    if (!gameMemory->isInitialized){
 
        gameState->playerTileMapX = 0;
        gameState->playerTileMapY = 0;
        gameState->playerX= (world.tileWidth*(float)world.countX)/ 2.0f;
        gameState->playerY= (world.tileHeight*(float)world.countY)/ 2.0f;
        gameMemory->isInitialized = true;
    }

    TileMap *currentTileMap 
        = getTileMapInWorld(&world,
                gameState->playerTileMapX,gameState->playerTileMapY);

    ASSERT(currentTileMap);
    float playerWidth = 0.75f*world.tileWidth;
    float playerHeight= world.tileHeight;
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
                dPlayerY = -1.0f;
            }
            if (controller->moveDown.endedDown) {
                dPlayerY = 1.0f;
            }
            if (controller->moveLeft.endedDown) {
                dPlayerX = -1.0f;
            }
            if (controller->moveRight.endedDown) {
                dPlayerX = 1.0f;
            }                                     
            dPlayerX *= 128.0f;
            dPlayerY *= 128.0f;

            float newPlayerX = gameState->playerX +  
                gameInput->dtForFrame*dPlayerX;
            float newPlayerY = gameState->playerY +  
                gameInput->dtForFrame*dPlayerY;

            RawLocation playerCenter= { gameState->playerTileMapX,
            gameState->playerTileMapY,
            newPlayerX,
            newPlayerY};

            RawLocation playerLeft = playerCenter; 
            playerLeft.X-= 0.5f*playerWidth;
            RawLocation playerRight = playerCenter; 
            playerRight.X+= 0.5f*playerWidth;
            

            //Checks lower-left, lower-center and lower-right corners of 
            //the sprite for collisions
            if (isWorldTileMapEmpty(&world,playerCenter) && 
                    isWorldTileMapEmpty(&world,playerLeft) && 
                    isWorldTileMapEmpty(&world,playerRight)) {
                WorldLocation newPos = getWorldLocation(&world,playerCenter); 
                gameState->playerTileMapX= newPos.tileMapX;
                gameState->playerTileMapY= newPos.tileMapY;

                gameState->playerX =  world.upperLeftX + 
                    world.tileWidth*newPos.tileX + 
                    newPos.X;
                gameState->playerY =  world.upperLeftY + 
                    world.tileWidth*newPos.tileY + 
                    newPos.Y;

            } else {
            }
        }


        drawRect(buffer,0.0f,0.0f,(float)buffer->width
                ,(float)buffer->height,1.0f,0.0f,0.1f);
        for (int row=0; row<world.countY;++row) {
            for (int column=0;column<world.countX;++column){
                float gray = (getTileValue (&world,currentTileMap,column,row) == 1) 
                    ? 1.0f : 0.5f ;
                float minX = world.upperLeftX + 
                    ((float)column)*world.tileWidth; 
                float minY=world.upperLeftY + 
                    ((float)row)*world.tileHeight; 
                float maxX=  minX + world.tileWidth; 
                float maxY=  minY + world.tileHeight; 

                drawRect(buffer,minX,minY,maxX,maxY,gray,gray,gray);
            }
            float playerR = 1.0f;
            float playerG = 1.0f;
            float playerB = 0.0f;
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

