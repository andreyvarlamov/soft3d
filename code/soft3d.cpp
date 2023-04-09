#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int32_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef float f64;

typedef i32 bool32;

#define Pi32 3.14159265359f

#define internal static
#define local_persist static
#define global_variable static

#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0])

struct game_offscreen_buffer
{
    void *Data;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct game_state
{
};

struct game_input
{
    f32 SecondsPerFrame;

    bool32 MouseLeft;
    bool32 MouseRight;
    i32 MouseDX;
    i32 MouseDY;
    i32 MouseDZ;

    bool32 Forward;
    bool32 StrafeLeft;
    bool32 StrafeRight;
    bool32 Back;
    
    bool32 Up;
    bool32 Down;
    bool32 Left;
    bool32 Right;
};

struct bitmap
{
    void *Pixels;
    i32 Width;
    i32 Height;
    i32 BytesPerPixel;
    i32 Pitch;
};

struct platform_read_file_result
{
    u32 ContentsSize;
    void *Contents;
};

internal void
DEBUGPrintString(const char *Format, ...);

internal void
PLATFORMFreeFileMemory(void *Memory);

internal platform_read_file_result
PLATFORMReadEntireFile(char *Filename);

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

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
};
#pragma pack(pop)

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
GameStateInit(game_state *State)
{
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

internal void
GameUpdateAndRender(game_state *State, game_input *Input, game_offscreen_buffer *Buffer)
{
    DrawRectangle(Buffer, 0.0f, 0.0f, (f32)Buffer->Width, (f32)Buffer->Height, 0xFF0000FF, 0xFF0000FF);

    ProcessInput(State, Input);
}
