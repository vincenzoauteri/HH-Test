#include <math.h>
inline float squareRoot(float floatNumber) {
    return sqrtf(floatNumber);
}

inline int32_t signOf(int32_t value) {
    int32_t result = (value >= 0) ? 1 : -1; 
    return result;
}

inline float absoluteValue(float floatNumber) {
    return fabs(floatNumber);
}

inline int32_t roundFloatToInt(float floatNumber) 
{
    int32_t result= (int32_t) roundf(floatNumber);
    return result;
}

inline uint32_t ceilFloatToInt(float floatNumber) {
    return(uint32_t) ceilf(floatNumber);
}

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

inline float sine(float angle) {
    return sinf(angle);
}

inline float cosine(float angle) {
    return cosf(angle);
}

inline float arctan(float angle) {
    return atanf(angle);
}
struct BitScanResult {
    bool32 found;
    uint32_t index ;
};

static BitScanResult findLeastSignificantBit(uint32_t value) {
    BitScanResult result= {};
#if COMPILER_MSVC 
    result.found =_BitScanForward((unsigned long *)&result.index,value);
#else
    for (uint32_t test = 0;
            test < 32;
            ++test){
        if (value & (1 << test)) {
            result.index= test;
            result.found = true;
            break;
        }

    }
#endif
    return result;
}
