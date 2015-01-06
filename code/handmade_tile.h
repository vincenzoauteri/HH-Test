
struct TileMapPosition{
    //Cordinates relative to the tileMap position in the world 
    uint32_t absTileX;
    uint32_t absTileY;
    //coordinates relative to the tile
    float tileRelX;
    float tileRelY;
};

struct TileMap {
    uint32_t chunkShift;
    uint8_t chunkMask;
    uint32_t chunkDim;

    uint32_t tileChunkCountX;
    uint32_t tileChunkCountY;

    float tileSizeInMeters;
    int32_t tileSizeInPixels;
    float tileRadiusInMeters;
    float metersToPixel;

    TileChunk *tileChunks;
};

struct TileChunk {
    uint32_t  *tiles;
};

struct TileChunkPosition {
    uint32_t tileChunkX;
    uint32_t tileChunkY;

    uint32_t relTileX;
    uint32_t relTileY;
};
