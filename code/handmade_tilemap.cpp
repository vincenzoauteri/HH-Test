#include "handmade.h"
#include "handmade_tilemap.h"

inline uint32_t getTileValueUnchecked(TileMap *tileMap,
        TileChunk *chunk,
        uint32_t tileX,
        uint32_t tileY)
{
    ASSERT(chunk);
    ASSERT(tileX < tileMap->chunkDim);
    ASSERT(tileY < tileMap->chunkDim);

    uint32_t tileValue = chunk->tiles[tileMap->chunkDim*tileY + tileX];

    return tileValue;
}


static uint32_t getTileValue(TileMap *tileMap,
        TileChunk *tileChunk, 
        uint32_t testTileX, 
        uint32_t testTileY)
{
    uint32_t tileValue = 0;

    if (tileChunk && tileChunk->tiles){
        tileValue = 
            getTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY);
    }

    return tileValue;
}


inline TileChunk* getTileChunk(TileMap *tileMap,
        uint32_t tileChunkX,
        uint32_t tileChunkY,
        uint32_t tileChunkZ)
{
    TileChunk *result = 0; 

    if ((tileChunkX >=0 && tileChunkX < tileMap->tileChunkCountX) &&
            (tileChunkY >=0 && tileChunkY < tileMap->tileChunkCountY) &&
            (tileChunkZ >=0 && tileChunkZ < tileMap->tileChunkCountZ)){
            uint32_t planePitch = tileMap->tileChunkCountX*
            tileMap->tileChunkCountY;
            uint32_t linePitch = tileMap->tileChunkCountX;
        result = &tileMap->tileChunks[
            tileChunkZ*planePitch + 
            tileChunkY*linePitch + 
            tileChunkX];
    }
    return result ;
}

inline TileChunkPosition getChunkPositionFor(
        TileMap *tileMap,
        uint32_t absTileX,
        uint32_t absTileY,
        uint32_t absTileZ)
{
    TileChunkPosition result;
    result.tileChunkX = absTileX >> tileMap->chunkShift;
    result.tileChunkY = absTileY >> tileMap->chunkShift;
    result.tileChunkZ = absTileZ;
    result.relTileX = absTileX & tileMap->chunkMask;
    result.relTileY = absTileY & tileMap->chunkMask;
    return result;
}
static uint32_t getTileValue(TileMap *tileMap, 
        uint32_t absTileX,
        uint32_t absTileY,
        uint32_t absTileZ)
{
    TileChunkPosition chunkPos = getChunkPositionFor(tileMap, 
            absTileX,absTileY,absTileZ);

    TileChunk *tileChunk= getTileChunk(tileMap, 
            chunkPos.tileChunkX, 
            chunkPos.tileChunkY, 
            chunkPos.tileChunkZ);

    return  getTileValue(tileMap,
            tileChunk,
            chunkPos.relTileX,
            chunkPos.relTileY);
}

static uint32_t getTileValue(TileMap *tileMap,  TileMapPosition pos) 
{
  uint32_t tileChunkValue = getTileValue(tileMap, 
          pos.absTileX,
          pos.absTileY,
          pos.absTileZ);
    return tileChunkValue;
}

static bool32 isTileValueEmpty(uint32_t tileValue)  
{
    return (tileValue== 1 || 
            tileValue== 3 ||
            tileValue== 4); 
}

static bool32 isTileMapPointEmpty(TileMap *tileMap, 
        TileMapPosition pos)
{
    uint32_t tileChunkValue = getTileValue(tileMap, pos); 

    return isTileValueEmpty(tileChunkValue); 
}

inline void setTileValueUnchecked(TileMap *tileMap,
        TileChunk *chunk,
        uint32_t tileX,
        uint32_t tileY,
        uint32_t tileValue
        )
{
    ASSERT(chunk);
    ASSERT(tileX < tileMap->chunkDim);
    ASSERT(tileY < tileMap->chunkDim);

    chunk->tiles[tileMap->chunkDim*tileY + tileX] = tileValue;
}



static void setTileValue(TileMap *tileMap,
        TileChunk *tileChunk, 
        uint32_t testTileX, 
        uint32_t testTileY,
        uint32_t tileValue
        )
{
    if (tileChunk && tileChunk->tiles){
        setTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY,tileValue);
    }

}

static void setTileValue(MemoryArena *arena,
        TileMap *tileMap,
        uint32_t absTileX,
        uint32_t absTileY,
        uint32_t absTileZ,
        uint32_t tileValue) 
{
    TileChunkPosition chunkPos = getChunkPositionFor(tileMap, 
            absTileX,absTileY,absTileZ);

    TileChunk *tileChunk= getTileChunk(tileMap, 
            chunkPos.tileChunkX, 
            chunkPos.tileChunkY,
            chunkPos.tileChunkZ);

    if (tileChunk && !tileChunk->tiles) {
        uint32_t tileCount = tileMap->chunkDim * tileMap->chunkDim;
        tileChunk->tiles = pushArray(arena,
                    tileCount, 
                    uint32_t);
        for (uint32_t tileIndex = 0; 
                tileIndex < tileCount ; 
                tileIndex++){
            tileChunk->tiles[tileIndex] = 1;
        }
    }


    setTileValue(tileMap,
            tileChunk,
            chunkPos.relTileX,
            chunkPos.relTileY,
            tileValue);
}

inline void recanonicalizeCoord(TileMap *tileMap,
        uint32_t *tile,
        float *tileRel)
{
    int32_t offset = roundFloatToInt(*tileRel / tileMap->tileSizeInMeters); 
    *tile += offset;
    *tileRel -= offset*tileMap->tileSizeInMeters;
    //ASSERT(*tileRel >= -0.5f * tileMap->tileSizeInMeters);
    //ASSERT(*tileRel <= 0.5 * tileMap->tileSizeInMeters);
}


inline TileMapPosition recanonicalizePosition(TileMap *tileMap, TileMapPosition pos) 
{
    TileMapPosition result = pos;
    recanonicalizeCoord(tileMap, &result.absTileX, &result.offset_.x);
    recanonicalizeCoord(tileMap, &result.absTileY, &result.offset_.y);

    return result;
}
static void iniatilizeArena(MemoryArena *arena, 
        size_t size,
        uint8_t *base) 
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

inline bool32 areOnSameTile(TileMapPosition *a, TileMapPosition *b) 
{
    bool32 result = (a->absTileX == b->absTileX &&
            a->absTileY == b->absTileY &&
            a->absTileZ == b->absTileZ);
    return result;
}

inline TileMapPosition centeredTilePoint(
        uint32_t absTileX, 
        uint32_t absTileY,
        uint32_t absTileZ) 
{
    TileMapPosition  result  = {};
    result.absTileX = absTileX;
    result.absTileY = absTileY;
    result.absTileZ = absTileZ;
    return result;
}

inline TileMapDifference subtract(TileMap *tileMap, TileMapPosition *a, TileMapPosition *b) 
{
    TileMapDifference result;

    V2 dTileXY = {(float)a->absTileX - (float)b->absTileX,
        (float)a->absTileY - (float)b->absTileY};

    result.dXY =   (dTileXY * tileMap->tileSizeInMeters) + (a->offset_ - b->offset_); 
    result.dZ = (float)a->absTileZ - (float)b->absTileZ;
    return result;
}
inline TileMapPosition offset(TileMap *tileMap, TileMapPosition pos,V2 offset){
    pos.offset_ += offset;
    pos = recanonicalizePosition(tileMap,pos);
    return pos;
}
