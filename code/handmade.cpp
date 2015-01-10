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
    if (maxX > buffer->width){
        maxX = buffer->width;
    }
    if (maxY > buffer->height){
        maxY = buffer->height;
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

#pragma pack(push, 1)
struct BitmapHeader {
    uint16_t fileType;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t bitmapOffset;
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t sizeOfBitmap;
    uint32_t horzResolution;
    uint32_t vertResolution;
    uint32_t colorUsed;
    uint32_t colorsImportant;

    //Masks determines the order in which RGBA are stored for each pixel
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
};
#pragma pack(pop)


static LoadedBitmap DEBUGloadBMP(ThreadContext *thread, 
        debug_read_entire_file *readEntireFile,
        char *fileName) 
{
    LoadedBitmap result = {} ;

    debugReadFileResult readResult = readEntireFile(thread, fileName);

    if (readResult.contentSize != 0) {
        BitmapHeader *header = (BitmapHeader *) readResult.fileContent;
        uint32_t *pixels = (uint32_t *)((uint8_t *)readResult.fileContent + header->bitmapOffset);
        result.pixels = pixels;
        result.width = header->width;
        result.height = header->height;

        uint32_t redMask = header->redMask;
        uint32_t greenMask = header->greenMask;
        uint32_t blueMask = header->blueMask;
        uint32_t alphaMask= ~(redMask | greenMask | blueMask);

        BitScanResult redShift = findLeastSignificantBit(redMask); 
        BitScanResult greenShift = findLeastSignificantBit(greenMask); 
        BitScanResult blueShift= findLeastSignificantBit(blueMask); 
        BitScanResult alphaShift = findLeastSignificantBit(alphaMask); 

        for(int32_t y = 0;
            y < header->height;
            ++y)
        {
            for(int32_t x = 0;
                x < header->width;
                ++x)
            {
                uint32_t cc= *pixels;
                *pixels++ = ((((cc >> alphaShift.index) & 0xff) << 24) |
                        (((cc >> redShift.index) & 0xff) << 16) | 
                        (((cc >> greenShift.index) & 0xff) << 8) |
                        ((cc >> blueShift.index) & 0xFF));
        }
    }
    }
    return result;
}



static void drawBitmap(FrameBuffer *buffer, 
        LoadedBitmap *bitmap, 
        float x,
        float y,
        float alignX = 0,
        float alignY = 0 )
{
    x -= alignX;
    y -= alignY;

    int32_t minX = roundFloatToInt(x);
    int32_t minY = roundFloatToInt(y);
    int32_t maxX = roundFloatToInt(x + bitmap->width);
    int32_t maxY = roundFloatToInt(y + bitmap->height);

    int32_t sourceOffsetX = 0;
    if (minX < 0){
        sourceOffsetX = -minX;
        minX = 0;
    }
    int32_t sourceOffsetY = 0;
    if (minY < 0){
		sourceOffsetY = -minY;
        minY = 0;
    }
    if (maxX > buffer->width){
        maxX = buffer->width;
    }
    if (maxY > buffer->height){
        maxY = buffer->height;
    }

    uint32_t *sourceRow = bitmap->pixels + bitmap->width*(bitmap->height -1) - (sourceOffsetY*bitmap->width) + sourceOffsetX;

    uint8_t *destRow = ((uint8_t *)buffer->bitmapMemory + 
            minX*buffer->bytesPerPixel + 
            minY*buffer->pitch);

    for (int y=minY;y<maxY;y++){
        uint32_t *dest = (uint32_t *)destRow;
        uint32_t *source= sourceRow;

        for (int x=minX; x<maxX; x++) {
            float A= (float) ((*source >> 24) & 0xFF) / 255.0f;
            float sR= (float) ((*source >> 16) & 0xFF);
            float sG= (float) ((*source >> 8) & 0xFF);
            float sB= (float) ((*source >> 0) & 0xFF);

            float dR= (float) ((*dest>> 16) & 0xFF);
            float dG= (float) ((*dest>> 8) & 0xFF);
            float dB= (float) ((*dest>> 0) & 0xFF);

            float R  = (1.0f - A)*dR + A*sR;
            float G  = (1.0f - A)*dG + A*sG;
            float B  = (1.0f - A)*dB + A*sB;

            *dest = (((uint32_t) (R + 0.5f) << 16) |
                    ((uint32_t) (G + 0.5f) << 8) | 
                    ((uint32_t) (B + 0.5f) << 0));
            dest++;
            source++;


        }
        destRow += buffer->pitch;
        sourceRow -= bitmap->width;
    }
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) 
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;

    ASSERT (sizeof(*gameState) <= gameMemory->permanentStorageSize);

    uint32_t tileSizeInPixels = 60;
    float metersToPixels = 0; 

    if (!gameMemory->isInitialized){

        gameState->backDrop = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_background.bmp");

        HeroBitmaps *bitmap = &gameState->heroBitmaps[0] ;

        bitmap->head = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_right_head.bmp");
        bitmap->torso = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_right_torso.bmp");
        bitmap->cape = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_right_cape.bmp");
        bitmap->alignX = 72;
        bitmap->alignY = 182;
        bitmap++;

        bitmap->head = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_back_head.bmp");
        bitmap->torso = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_back_torso.bmp");
        bitmap->cape = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_back_cape.bmp");
        bitmap->alignX = 72;
        bitmap->alignY = 182;
        bitmap++;

        bitmap->head = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_left_head.bmp");
        bitmap->torso = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_left_torso.bmp");
        bitmap->cape = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_left_cape.bmp");
        bitmap->alignX = 72;
        bitmap->alignY = 182;
        bitmap++;

        bitmap->head = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_front_head.bmp");
        bitmap->torso = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_front_torso.bmp");
        bitmap->cape = DEBUGloadBMP(context, 
                gameMemory->DEBUGReadEntireFile,
                "test/test_hero_front_cape.bmp");
        bitmap->alignX = 72 ;
        bitmap->alignY = 182;

        gameState->cameraP.absTileX = 17/2;
        gameState->cameraP.absTileY = 9/2;

        gameState->playerP.absTileX = 1;
        gameState->playerP.absTileY = 3;
        gameState->playerP.absTileZ = 0;
        gameState->playerP.offsetX= 5.0f;
        gameState->playerP.offsetY= 5.0f; 

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
        tileMap->tileSizeInMeters = 1.4f;

        tileMap->tileChunks = pushArray(&gameState->worldArena, 
                tileMap->tileChunkCountX* 
                tileMap->tileChunkCountY* 
                tileMap->tileChunkCountZ, 
                TileChunk); 

        metersToPixels =  (float)tileSizeInPixels/tileMap->tileSizeInMeters;


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

            bool32 createdZDoor = false;
            if (randomChoice == 2) {
                createdZDoor = true;
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

            doorLeft = doorRight ? true : false;
            doorBottom =  doorTop ? true : false;
            doorRight = false;
            doorTop= false;

            if (createdZDoor) {
                doorUp= !doorUp;
                doorDown= !doorDown;
            } else  {
                doorUp = false;
                doorDown = false;
            }

            if (randomChoice == 2) {
                if (absTileZ == 0) {
                    absTileZ = 1;
                } else {
                    absTileZ = 0;
                }
            } else if (randomChoice == 1) {
                screenX += 1;
            } else {
                screenY += 1;
            }


        }
        gameMemory->isInitialized = true;
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
                gameState->heroFacingDirection = 1;
            }
            if (controller->moveDown.endedDown) {
                dPlayerY = -1.0f;
                gameState->heroFacingDirection = 3;
            }
            if (controller->moveLeft.endedDown) {
                dPlayerX = -1.0f;
                gameState->heroFacingDirection = 2;
            }
            if (controller->moveRight.endedDown) {
                dPlayerX = 1.0f;
                gameState->heroFacingDirection = 0;
            }                                     

            float playerSpeed = 2.0f;
            if (controller->actionUp.endedDown) {
                playerSpeed = 10.0f;
            } 

            dPlayerX *= playerSpeed;
            dPlayerY *= playerSpeed;

            TileMapPosition newPlayerP = gameState->playerP;
            newPlayerP.offsetX += (gameInput->dtForFrame*dPlayerX);
            newPlayerP.offsetY += (gameInput->dtForFrame*dPlayerY);
            newPlayerP = recanonicalizePosition(tileMap,newPlayerP);

            TileMapPosition playerLeft = newPlayerP;
            playerLeft.offsetX -= 0.5f*playerWidth;
            playerLeft= recanonicalizePosition(tileMap,playerLeft);

            TileMapPosition playerRight= newPlayerP;
            playerRight.offsetX += 0.5f*playerWidth;
            playerRight = recanonicalizePosition(tileMap,playerRight);

            //Checks lower-left, lower-center and lower-right corners of 
            //the sprite for collisions
            if (isTileMapPointEmpty(tileMap, newPlayerP) && 
                    isTileMapPointEmpty(tileMap,playerLeft) && 
                    isTileMapPointEmpty(tileMap,playerRight)) {

                if (!areOnSameTile(&gameState->playerP, &newPlayerP)) {
                    uint32_t newTileValue = getTileValue(tileMap, newPlayerP);
                    if (newTileValue == 3) {
                        ++newPlayerP.absTileZ;
                    } else if (newTileValue == 4) {
                        --newPlayerP.absTileZ;
                    }
                }
                gameState->playerP = newPlayerP;
            } else {
            }

            gameState->cameraP.absTileZ = gameState->playerP.absTileZ;

            TileMapDifference diff = subtract(tileMap,
                    &gameState->playerP,
                    &gameState->cameraP);

            if (diff.dX > (9.0f*tileMap->tileSizeInMeters)){
                gameState->cameraP.absTileX += 17 ;
            }
            if (diff.dX < -(9.0f*tileMap->tileSizeInMeters)){
                gameState->cameraP.absTileX -= 17 ;
            }
            if (diff.dY > (5.0f*tileMap->tileSizeInMeters)){
                gameState->cameraP.absTileY += 9 ;
            }
            if (diff.dY < -(5.0f*tileMap->tileSizeInMeters)){
                gameState->cameraP.absTileY -= 9 ;
            }
        }
        }

        //drawRect(buffer,0.0f,0.0f,(float)buffer->width
        //        ,(float)buffer->height,1.0f,0.0f,0.1f);

        drawBitmap(buffer, &gameState->backDrop, 0,0);

        float screenCenterX = 0.5f *(float)buffer->width;
        float screenCenterY = 0.5f *(float)buffer->height;

        for (int32_t relRow= -10; 
                relRow <  10;
                ++relRow) {
            for (int32_t relColumn= -20;
                    relColumn < 20;
                    ++relColumn ) {

                uint32_t column = relColumn + gameState->cameraP.absTileX;
                uint32_t row = relRow + gameState->cameraP.absTileY;

                uint32_t tileValue = getTileValue(tileMap,
                        column,
                        row,
                        gameState->cameraP.absTileZ);

                float gray = 0.0f;
                if (tileValue > 1) {
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

                    float tileCenterX = screenCenterX - 
                        metersToPixels*gameState->cameraP.offsetX + 
                        ((float)relColumn)*tileSizeInPixels; 
                    float tileCenterY= screenCenterY+ 
                        metersToPixels*gameState->cameraP.offsetY - 
                        ((float)relRow)*tileSizeInPixels; 
                    float minX = tileCenterX - 0.5f*tileSizeInPixels; 
                    float minY = tileCenterY - 0.5f*tileSizeInPixels; 

                    float maxX =  tileCenterX + 0.5f * tileSizeInPixels; 
                    float maxY =  tileCenterY + 0.5f * tileSizeInPixels; 


                    drawRect(buffer,minX,minY,maxX,maxY,gray,gray,gray);
                }
            }
        }
        TileMapDifference diff = subtract(tileMap,&gameState->playerP, &gameState->cameraP);

        float playerR = 1.0f;
        float playerG = 1.0f;
        float playerB = 0.0f;

        float playerGroundPointX = screenCenterX
            + metersToPixels*diff.dX;
        float playerGroundPointY = screenCenterY
            - metersToPixels*diff.dY;

        float playerLeft = playerGroundPointX-  
            0.5f*metersToPixels*playerWidth;
        float playerTop = playerGroundPointY-
            metersToPixels*playerHeight;

        drawRect(buffer,
                playerLeft, playerTop,
                playerLeft + metersToPixels*playerWidth,
                playerTop + metersToPixels*playerHeight,
                playerR,playerG,playerB);

        HeroBitmaps *heroBitmaps = &gameState->heroBitmaps[gameState->heroFacingDirection];

        drawBitmap(buffer, 
                &heroBitmaps->torso,
                playerGroundPointX, 
                playerGroundPointY,
                heroBitmaps->alignX,
                heroBitmaps->alignY);
        drawBitmap(buffer, 
                &heroBitmaps->cape,
                playerGroundPointX, 
                playerGroundPointY,
                heroBitmaps->alignX,
                heroBitmaps->alignY);
        drawBitmap(buffer, 
                &heroBitmaps->head,
                playerGroundPointX, 
                playerGroundPointY,
                heroBitmaps->alignX,
                heroBitmaps->alignY);
    }



extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples)
{
    GameState *gameState = (GameState *)gameMemory->permanentStorage;
    //gameOutputSound(soundBuffer,gameState->tSine,gameState->toneHz);
    gameOutputSound(soundBuffer);
}

