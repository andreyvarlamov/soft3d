#if !defined(SOFT3D_H)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

#define internal static
#define local_persist static
#define global_variable static

#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0])

#define STAR_COUNT 4096
#define STAR_SPREAD 64.0f

struct game_offscreen_buffer
{
    void *Data;
    i32 Width;
    i32 Height;
    i32 Pitch;
    i32 BytesPerPixel;
};

struct star
{
    f32 X;
    f32 Y;
    f32 Z;
};

struct game_state
{
    star Stars[STAR_COUNT];
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

internal void
DEBUGPrintString(const char *Format, ...);

internal void
PLATFORMFreeFileMemory(void *Memory);

internal platform_read_file_result
PLATFORMReadEntireFile(char *Filename);

#define SOFT3D_H
#endif
