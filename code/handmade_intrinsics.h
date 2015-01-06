#include <math.h>
inline int32_t roundFloatToInt(float floatNumber) 
{
    int32_t result= (int32_t) roundf(floatNumber);
    return result;
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
