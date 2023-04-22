#if !defined(SOFT3D_MATH_H)

#define Pi32 3.14159265359f

/*
  ------------------
  NOTE: GENERAL MATH
  ------------------
 */

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

#define EpsilonF32 0.000000119f // 1.19e-7
inline bool
AreEqualF32(f32 A, f32 B)
{
    f32 AbsDiff = AbsoluteF32(A - B);
    f32 AbsA = AbsoluteF32(A);
    f32 AbsB = AbsoluteF32(B);
    f32 Largest = AbsB > AbsA ? AbsB : AbsA;

    return AbsDiff <= Largest*EpsilonF32;
}

/*
  ------------------
  NOTE: TRIGONOMETRY
  ------------------
 */

// TODO: Implement without math
#include <math.h>
inline f32
TanF32(f32 Angle)
{
    f32 Result = tanf(Angle);

    return Result;
}

inline f32
SinF32(f32 Angle)
{
    f32 Result = sinf(Angle);

    return Result;
}

inline f32
CosF32(f32 Angle)
{
    f32 Result = cosf(Angle);

    return Result;
}

/*
  --------------------
  NOTE: LINEAR ALGEBRA
  --------------------
 */

struct vec4_f32
{
    f32 X;
    f32 Y;
    f32 Z;
    f32 W;
};

struct mat44_f32
{
    f32 Components[4][4];
};

inline vec4_f32
TransformVec4F32(mat44_f32 Transform, vec4_f32 Vector)
{
    vec4_f32 Result = {0};

    Result.X = (Transform.Components[0][0]*Vector.X +
                Transform.Components[0][1]*Vector.Y +
                Transform.Components[0][2]*Vector.Z +
                Transform.Components[0][3]*Vector.W);
    Result.Y = (Transform.Components[1][0]*Vector.X +
                Transform.Components[1][1]*Vector.Y +
                Transform.Components[1][2]*Vector.Z +
                Transform.Components[1][3]*Vector.W);
    Result.Z = (Transform.Components[2][0]*Vector.X +
                Transform.Components[2][1]*Vector.Y +
                Transform.Components[2][2]*Vector.Z +
                Transform.Components[2][3]*Vector.W);
    Result.W = (Transform.Components[3][0]*Vector.X +
                Transform.Components[3][1]*Vector.Y +
                Transform.Components[3][2]*Vector.Z +
                Transform.Components[3][3]*Vector.W);

    return Result;
}

inline mat44_f32
GetIdentityMatrix()
{
    mat44_f32 Result = {0};

    Result.Components[0][0] = 1.0f;
    Result.Components[1][1] = 1.0f;
    Result.Components[2][2] = 1.0f;
    Result.Components[3][3] = 1.0f;

    return Result;
}

inline mat44_f32
GetTranslationMatrix(f32 X, f32 Y, f32 Z)
{
    mat44_f32 Result = GetIdentityMatrix();

    Result.Components[0][3] = X;
    Result.Components[1][3] = Y;
    Result.Components[2][3] = Z;

    return Result;
}

inline mat44_f32
GetScaleMatrix(f32 X, f32 Y, f32 Z)
{
    mat44_f32 Result = {0};

    Result.Components[0][0] = X;
    Result.Components[1][1] = Y;
    Result.Components[2][2] = Z;

    Result.Components[3][3] = 1.0f;

    return Result;
}

/*
         |  1  0  0 |
    X  = |  0  A -B |
         |  0  B  A |

         |  C  0  D |
    Y  = |  0  1  0 |
         | -D  0  C |

         |  E -F  0 |
    Z  = |  F  E  0 |
         |  0  0  1 |

    M  = X . Y . Z

         |  CE      -CF       D   0 |
    M  = |  BDE+AF  -BDF+AE  -BC  0 |
         | -ADE+BF   ADF+BE   AC  0 |
         |  0        0        0   1 |

    where A,B are the cosine and sine of the X-axis rotation axis,
          C,D are the cosine and sine of the Y-axis rotation axis,
          E,F are the cosine and sine of the Z-axis rotation axis.
 */
inline mat44_f32
GetRotationMatrix(f32 AngleX, f32 AngleY, f32 AngleZ)
{
    mat44_f32 Result = {0};
    
    f32 CosX = CosF32(AngleX);
    f32 SinX = SinF32(AngleX);
    f32 CosY = CosF32(AngleY);
    f32 SinY = SinF32(AngleY);
    f32 CosZ = CosF32(AngleZ);
    f32 SinZ = CosF32(AngleZ);

    Result.Components[0][0] =  CosY*CosZ;
    Result.Components[0][1] = -CosY*SinZ;
    Result.Components[0][2] =  SinY;

    Result.Components[1][0] =  SinX*SinY*CosZ + CosX*SinZ;
    Result.Components[1][1] = -SinX*SinY*SinZ + CosX*CosZ;
    Result.Components[1][2] = -SinX*CosY;

    Result.Components[2][0] = -CosX*SinY*CosZ + SinX*SinZ;
    Result.Components[2][1] =  CosX*SinY*SinZ + SinX*CosZ;
    Result.Components[2][2] =  CosX*CosY;

    Result.Components[3][3] = 1;

    return Result;
}

/*
  ------------------
  NOTE: GRAPHICS
  ------------------
 */

inline f32
GetLineXForY(f32 Y, f32 InvSlope, f32 VerticalOffset, f32 HorizontalOffset)
{
    // NOTE: y = mx + b -> x = (y - b) / m
    // Vertical line: x = a
    // Generic: x = n*(y - b) + a; n = 1/m; if n != 0, a = 0

    bool32 IsVerticalLine = AreEqualF32(InvSlope, 0.0f);
    HorizontalOffset = IsVerticalLine ? HorizontalOffset : 0.0f;
    
    f32 Result = InvSlope*(Y - VerticalOffset) + HorizontalOffset;
    return Result;
}

inline mat44_f32
GetProjectionMatrix(f32 FOV, f32 Width, f32 Height, f32 NearZ, f32 FarZ)
{
    mat44_f32 Result = {0};

    f32 HalfFOVTan = TanF32(FOV/2.0f);
    f32 AspectRatio = Width/Height;

    Result.Components[0][0] = 1 / (AspectRatio*HalfFOVTan);
    Result.Components[1][1] = 1 / HalfFOVTan;

    Result.Components[2][2] = -(FarZ+NearZ) / (FarZ-NearZ);
    Result.Components[2][3] = (2.0f*FarZ*NearZ) / (NearZ-FarZ);

    Result.Components[3][2] = -1;

    return Result;
}

inline vec4_f32
PerspectiveDivideVec4F32(vec4_f32 Vector)
{
    vec4_f32 Result;

    Result.X = Vector.X/Vector.W;
    Result.Y = Vector.Y/Vector.W;
    Result.Z = Vector.Z/Vector.W;
    Result.W = 1.0f;

    return Result;
}

inline mat44_f32
GetScreenSpaceTransform(f32 Width, f32 Height)
{
    f32 HalfWidth = Width / 2.0f;
    f32 HalfHeight = Height / 2.0f;

    mat44_f32 Result = GetIdentityMatrix();

    Result.Components[0][0] = HalfWidth; // Scale by HalfWidth
    Result.Components[0][3] = HalfWidth; // Offset by HalfWidth 
    Result.Components[1][1] = -HalfHeight;
    Result.Components[1][3] = HalfHeight;
    // --> (0.0f,  0.0f) -> (HalfWidth, HalfHeight)
    //     (1.0f, 1.0f) -> (Width, 0)

    return Result;
}

#define SOFT3D_MATH_H
#endif
