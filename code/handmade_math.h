#ifndef HANDMADE_MATH
#define HANDMADE_MATH

struct V2 {
    union {
        struct {
            float x;
            float y;
        };
        float e[2];
    };

    inline V2 &operator*=(float a);
    inline V2 &operator+=(V2 a);
};

inline V2 v2(float a, float b)
{
    V2 result = { a, b };
    return result;
};


inline V2 operator-(V2 a) 
{
    V2 result = { -a.x, -a.y };
    return result;
}

inline V2 operator*(V2 v, float a) 
{
    V2 result = { v.x, v.y };

    result.x *= a;
    result.y *= a;

    return result;
}

inline V2 operator+(V2 a, V2 b) 
{
    V2 result = { a.x, a.y };
    result.x += b.x;
    result.y += b.y;
    return result;
}

inline V2& V2::operator*=(float a) 
{
    *this = *this * a;
    return *this;
}

inline V2& V2::operator+=(V2 v) 
{
    *this = *this + v;
    return *this;
}

inline V2 operator-(V2 a, V2 b) 
{
    V2 result = { a.x, a.y };
    result.x -= b.x;
    result.y -= b.y;
    return result;
}

inline float square(float a) 
{
    return a*a;
}

inline float inner(V2 v1, V2 v2) 
{
   return (v1.x*v2.x + v1.y*v2.y); 
}

inline float lengthSq(V2 v) 
{
    return inner(v,v);
}
#endif
