#include "soft3d.h"
#include "soft3d_math.h"

internal bitmap
LoadBMP(char *Filename)
{
    bitmap Result = {0};
    platform_read_file_result ReadResult = PLATFORMReadEntireFile(Filename);
    if (ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        Result.Pixels = (u8 *)ReadResult.Contents + Header->BitmapOffset;
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        Result.BytesPerPixel = 4;
        Result.Pitch = Result.Width * Result.BytesPerPixel;
    }

    return Result;
}

internal void
DrawPixel(game_offscreen_buffer *Buffer,
          i32 PixelX, i32 PixelY,
          u32 Color)
{
    u32 *Pixels = (u32 *)Buffer->Data;
    Pixels[PixelY*Buffer->Width + PixelX] = Color;
}

internal void
DrawLine(game_offscreen_buffer *Buffer,
         f32 RealStartX, f32 RealStartY,
         f32 RealEndX, f32 RealEndY,
         u32 Color)
{
    f32 DistanceX = RealEndX - RealStartX;
    f32 DistanceY = RealEndY - RealStartY;
    // TODO: Use absolute function
    f32 DistanceX_Abs = ((DistanceX > 0) ? DistanceX : -DistanceX);
    f32 DistanceY_Abs = ((DistanceY > 0) ? DistanceY : -DistanceY);
    f32 SteppingDistance = ((DistanceX_Abs >= DistanceY_Abs) ? DistanceX_Abs : DistanceY_Abs);
    f32 dX = DistanceX / SteppingDistance;
    f32 dY = DistanceY / SteppingDistance;

    f32 X = RealStartX;
    f32 Y = RealStartY;
    u32 *Pixels = (u32 *)Buffer->Data;
    for (i32 SteppingIndex = 0;
         SteppingIndex < (i32)SteppingDistance;
         ++SteppingIndex)
    {
        i32 RoundedX = RoundF32ToI32(X);
        i32 RoundedY = RoundF32ToI32(Y);
        if (RoundedX >= 0 && RoundedX < Buffer->Width &&
            RoundedY >= 0 && RoundedY < Buffer->Height)
        {
            Pixels[RoundedY * Buffer->Width + RoundedX] = Color;
        }

        X += dX;
        Y += dY;
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              f32 RealMinX, f32 RealMinY,
              f32 RealMaxX, f32 RealMaxY,
              u32 Color, u32 BorderColor)
{
    i32 MinX = RoundF32ToI32(RealMinX);
    i32 MinY = RoundF32ToI32(RealMinY);
    i32 MaxX = RoundF32ToI32(RealMaxX);
    i32 MaxY = RoundF32ToI32(RealMaxY);
    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;
    if (MaxX > Buffer->Width) MaxX = Buffer->Width;
    if (MaxY > Buffer->Height) MaxY = Buffer->Height;

    u8 *Row = ((u8 *)Buffer->Data +
               MinX * Buffer->BytesPerPixel +
               MinY * Buffer->Pitch);

    for (int Y = MinY;
         Y < MaxY;
         ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        
        for (int X = MinX;
             X < MaxX;
             ++X)
        {
            u32 ColorToFill = Color;
            if (Y == MinY || Y == MaxY - 1 || X == MinX || X == MaxX - 1)
            {
                ColorToFill = BorderColor;
            }
            
            *Pixel++ = ColorToFill;
        }

        Row += Buffer->Pitch;
    }
}

internal star
PlaceStarInRandomLocation(f32 StarSpread)
{
    star Result = {0};
    
    Result.X = 2.0f * ((f32)RandomUnitF32() - 0.5f)*StarSpread;
    Result.Y = 2.0f * ((f32)RandomUnitF32() - 0.5f)*StarSpread;
    Result.Z = ((f32)RandomUnitF32() + 0.00001f)*StarSpread;

    return Result;
}

internal void
GameStateInit(game_state *State)
{
    srand((u32)time(0));

    for (i32 StarIndex = 0;
         StarIndex < STAR_COUNT;
         ++StarIndex)
    {
        State->Stars[StarIndex] = PlaceStarInRandomLocation(STAR_SPREAD);
    }
}

internal void
ProcessInput(game_state *State, game_input *Input)
{
    // i32 MouseXDeltaRange = 700;
    // State->PlayerAngle += -((f32)Input->MouseDX/(f32)MouseXDeltaRange) * Input->SecondsPerFrame * 20.0f;
    // if (State->PlayerAngle >= 2*Pi32)
    // {
    //     State->PlayerAngle -= 2*Pi32;
    // }
    // else if (State->PlayerAngle < 0.0f)
    // {
    //     State->PlayerAngle += 2*Pi32;
    // }
}

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

internal mat44_f32
GetIdentityMatrix()
{
    mat44_f32 Result = {0};

    Result.Components[0][0] = 1.0f;
    Result.Components[1][1] = 1.0f;
    Result.Components[2][2] = 1.0f;
    
    Result.Components[3][3] = 1.0f;

    return Result;
}

internal mat44_f32
GetScreenSpaceTransform()
{
    mat44_f32 Result = {0};

    Result.Components[0][0] = 1.0f;
    Result.Components[1][1] = 1.0f;
    Result.Components[2][2] = 1.0f;
    
    Result.Components[3][3] = 1.0f;

    return Result;
}
    
internal mat44_f32
GetTranslationMatrix(f32 X, f32 Y, f32 Z)
{
    mat44_f32 Result = GetIdentityMatrix();;

    Result.Components[0][3] = X;
    Result.Components[1][3] = Y;
    Result.Components[2][3] = Z;

    return Result;
}

internal mat44_f32
GetScreenSpaceTransform(f32 HalfWidth, f32 HalfHeight)
{
    mat44_f32 Result = {0};

    Result.Components[0][0] = HalfWidth; // Scale by HalfWidth
    Result.Components[0][3] = HalfWidth; // Offset by HalfWidth 
    Result.Components[1][1] = -HalfHeight;
    Result.Components[1][3] = HalfHeight;
    // --> (0.0f,  0.0f) -> (HalfWidth, HalfHeight)
    //     (1.0f, -1.0f) -> (Width, 0)
    
    Result.Components[2][2] = 1.0f;
    Result.Components[3][3] = 1.0f;

    return Result;
}

internal mat44_f32
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
internal mat44_f32
GetRotationMatrix(f32 AngleX, f32 AngleY, f32 AngleZ)
{
    mat44_f32 Result = {0};
    
    f32 CosX = cosf(AngleX);
    f32 SinX = sinf(AngleX);
    f32 CosY = cosf(AngleY);
    f32 SinY = sinf(AngleY);
    f32 CosZ = cosf(AngleZ);
    f32 SinZ = sinf(AngleZ);

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

internal vec4_f32
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

internal vec4_f32
PerspectiveDivideVec4F32(vec4_f32 Vector)
{
    vec4_f32 Result;

    Result.X = Vector.X/Vector.W;
    Result.Y = Vector.Y/Vector.W;
    Result.Z = Vector.Z/Vector.W;
    Result.W = Vector.W;

    return Result;
}

internal void
UpdateAndDrawStars(game_offscreen_buffer *Buffer, game_state *State, f32 SecondsPerFrame)
{
    f32 StarSpeed = 20.0f;
    
    f32 BufferCenterX = (f32)Buffer->Width/2.0f;
    f32 BufferCenterY = (f32)Buffer->Height/2.0f;
    f32 FOV = Pi32/2.0f;
    f32 HalfFOVTan = tanf(FOV/2.0f);
    
    for (i32 StarIndex = 0;
         StarIndex < STAR_COUNT;
         ++StarIndex)
    {
        star * CurrentStar = &State->Stars[StarIndex];
        CurrentStar->Z -= SecondsPerFrame*StarSpeed;

        if (CurrentStar->Z <= 0)
        {
            *CurrentStar = PlaceStarInRandomLocation(STAR_SPREAD);
        }

        f32 PerspectiveStarX = CurrentStar->X / (HalfFOVTan*CurrentStar->Z);
        f32 PerspectiveStarY = CurrentStar->Y / (HalfFOVTan*CurrentStar->Z);
        i32 DrawX = TruncateF32ToI32(PerspectiveStarX*BufferCenterX + BufferCenterX);
        i32 DrawY = TruncateF32ToI32(PerspectiveStarY*BufferCenterY + BufferCenterY);

        if (DrawX >= 0 && DrawX < Buffer->Width &&
            DrawY >= 0 && DrawY < Buffer->Height)
        {
            DrawPixel(Buffer, DrawX, DrawY, 0xFFFFFFFF);
        }
        else
        {
            *CurrentStar = PlaceStarInRandomLocation(STAR_SPREAD);
        }
    }
}

internal void
FillScanBufferRow(i32 *Scanbuffer, i32 Row, i32 MinX, i32 MaxX)
{
    Assert(Row < 900);
    
    Scanbuffer[Row*2] = MinX;
    Scanbuffer[Row*2 + 1] = MaxX;
}

internal void
DrawScanBufferRows(game_offscreen_buffer *Buffer, i32 *Scanbuffer, i32 MinY, i32 MaxY)
{
    Assert(MinY >= 0 && MinY < 900);
    Assert(MaxY >= 0 && MaxY < 900);
    
    for (i32 Row = MinY;
         Row < MaxY;
         ++Row)
    {
        i32 MinX = Scanbuffer[Row*2];
        i32 MaxX = Scanbuffer[Row*2 + 1];
        
        for (i32 Column = MinX;
             Column < MaxX;
             ++Column)
        {
            DrawPixel(Buffer, Column, Row, 0xFFFFFFFF);
        }
    }
}

internal void
ScanConvertLine(i32 *Scanbuffer, vec4_f32 VertexMinY, vec4_f32 VertexMaxY, i32 WhichSide)
{
    i32 StartY = TruncateF32ToI32(VertexMinY.Y);
    i32 EndY = TruncateF32ToI32(VertexMaxY.Y);
    i32 StartX = TruncateF32ToI32(VertexMinY.X);
    i32 EndX = TruncateF32ToI32(VertexMaxY.X);

    i32 dY = EndY - StartY;
    i32 dX = EndX - StartX;

    if (dY <= 0)
    {
        return;
    }

    f32 StepX = (f32)dX/(f32)dY;
    f32 CursorX = (f32)StartX;

    for (i32 Row = StartY;
         Row < EndY;
         ++Row)
    {
        Scanbuffer[Row*2 + WhichSide] = (i32)CursorX;
        CursorX += StepX;
    }
}

internal void
ScanConvertSortedTriangle(i32 *Scanbuffer, vec4_f32 VertexMinY, vec4_f32 VertexMidY, vec4_f32 VertexMaxY, i32 Handedness)
{
    ScanConvertLine(Scanbuffer, VertexMinY, VertexMaxY, 0 + Handedness);
    ScanConvertLine(Scanbuffer, VertexMinY, VertexMidY, 1 - Handedness);
    ScanConvertLine(Scanbuffer, VertexMidY, VertexMaxY, 1 - Handedness);
}

internal void
DrawTriangle(game_offscreen_buffer *Buffer, i32 *Scanbuffer, vec4_f32 Vertex1, vec4_f32 Vertex2, vec4_f32 Vertex3)
{
    mat44_f32 ScreenSpaceTransform = GetScreenSpaceTransform((f32)Buffer->Width/2.0f, (f32)Buffer->Height/2.0f);

    vec4_f32 ScreenSpaceVertex1 = TransformVec4F32(ScreenSpaceTransform, Vertex1);
    vec4_f32 ScreenSpaceVertex2 = TransformVec4F32(ScreenSpaceTransform, Vertex2);
    vec4_f32 ScreenSpaceVertex3 = TransformVec4F32(ScreenSpaceTransform, Vertex3);

    vec4_f32 VertexMinY = PerspectiveDivideVec4F32(ScreenSpaceVertex1);
    vec4_f32 VertexMidY = PerspectiveDivideVec4F32(ScreenSpaceVertex2);
    vec4_f32 VertexMaxY = PerspectiveDivideVec4F32(ScreenSpaceVertex3);

    if (VertexMaxY.Y < VertexMidY.Y)
    {
        vec4_f32 Temp = VertexMaxY;
        VertexMaxY = VertexMidY;
        VertexMidY = Temp;
    }
    if (VertexMidY.Y < VertexMinY.Y)
    {
        vec4_f32 Temp = VertexMidY;
        VertexMidY = VertexMinY;
        VertexMinY = Temp;
    }
    if (VertexMaxY.Y < VertexMinY.Y)
    {
        vec4_f32 Temp = VertexMaxY;
        VertexMaxY = VertexMinY;
        VertexMinY = Temp;
    }

    i32 Handedness = ((VertexMidY.X > VertexMaxY.X) ? 0 : 1);
    
    ScanConvertSortedTriangle(Scanbuffer, VertexMinY, VertexMidY, VertexMaxY, Handedness);
    DrawScanBufferRows(Buffer, Scanbuffer, TruncateF32ToI32(VertexMinY.Y), TruncateF32ToI32(VertexMaxY.Y));
}

internal void
GameUpdateAndRender(game_state *State, game_input *Input, game_offscreen_buffer *Buffer)
{
    ProcessInput(State, Input);
    
    DrawRectangle(Buffer, 0.0f, 0.0f, (f32)Buffer->Width, (f32)Buffer->Height, 0xFF000000, 0xFF000000);

    int Scanbuffer[1800] = {0};

    vec4_f32 Vertex1 = { -1.0f, -1.0f, 1.0f, 1.0f };
    vec4_f32 Vertex2 = {  0.0f,  1.0f, 1.0f, 1.0f };
    vec4_f32 Vertex3 = { -1.0f,  1.0f, 1.0f, 1.0f };

    DrawTriangle(Buffer, Scanbuffer, Vertex1, Vertex2, Vertex3);
}
