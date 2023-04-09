#if !defined(SOFT3D_MATH_H)

#define Pi32 3.14159265359f

inline u32
SafeTruncateU64(u64 Value)
{
    // TODO: Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return Result;
}

inline i32
RoundF32ToI32(f32 Real)
{
    i32 Result = (i32)(Real + 0.5f);
    return Result;
}

inline i32
TruncateF32ToI32(f32 Real)
{
    i32 Result = (i32)Real;
    return Result;
}

inline f32
AbsoluteF32(f32 Value)
{
    f32 Result = ((Value > 0) ? Value : -Value);
    return Result;
}

inline f32
RandomUnitF32()
{
    u32 Resolution = 256;
    u32 Random = (u32)rand() % Resolution;
    
    f32 Result = (f32)Random / (f32)(Resolution - 1);

    return Result;
}

// TODO: Implement without math
#include <math.h>
inline f32
TanF32(f32 Angle)
{
    f32 Result = tanf(Angle);

    return Result;
}

#define SOFT3D_MATH_H
#endif
