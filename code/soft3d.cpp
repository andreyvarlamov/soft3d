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
 
internal void
DrawBetweenVerticalLines(game_offscreen_buffer *Buffer,
                 vec4_f32 StartLineA, vec4_f32 EndLineA,
                 vec4_f32 StartLineB, vec4_f32 EndLineB,
                 f32 RowStart, f32 RowEnd)
{
    Assert (RowEnd >= RowStart);
    // NOTE: Make sure neither of the lines is horizontal
    Assert(!AreEqualF32(StartLineA.Y, EndLineA.Y));
    Assert(!AreEqualF32(StartLineB.Y, EndLineB.Y));

    bool32 IsVerticalA = AreEqualF32(StartLineA.X, EndLineA.X);
    f32 InvSlopeA = (IsVerticalA ?
                     0.0f :
                     ((EndLineA.X - StartLineA.X) / (EndLineA.Y - StartLineA.Y)));
    f32 VerticalOffsetA = (IsVerticalA ?
                           0.0f :
                           EndLineA.Y - EndLineA.X/InvSlopeA);
    f32 HorizontalOffsetA = (IsVerticalA ? StartLineA.X : 0.0f);

    bool32 IsVerticalB = AreEqualF32(StartLineB.X, EndLineB.X);
    f32 InvSlopeB = (IsVerticalB ?
                     0.0f :
                     ((EndLineB.X - StartLineB.X) / (EndLineB.Y - StartLineB.Y)));
    f32 VerticalOffsetB = (IsVerticalB ?
                           0.0f :
                           EndLineB.Y - EndLineB.X/InvSlopeB);
    f32 HorizontalOffsetB = (IsVerticalB ? StartLineB.X : 0.0f);
    
    if (RowStart < 0) RowStart = 0;
    if (RowEnd > Buffer->Height) RowEnd = (f32)Buffer->Height;

    for (f32 RowIndex = RowStart;
         RowIndex < RowEnd;
         RowIndex += 1.0f)
    {
        if (RowIndex >= 0 && RowIndex < Buffer->Height)
        {
            i32 PointA = TruncateF32ToI32(GetLineXForY(RowIndex, InvSlopeA, VerticalOffsetA, HorizontalOffsetA));
            i32 PointB = TruncateF32ToI32(GetLineXForY(RowIndex, InvSlopeB, VerticalOffsetB, HorizontalOffsetB));
            
            i32 LeftBoundary;
            i32 RightBoundary;
            
            if (PointA < PointB)
            {
                LeftBoundary = PointA;
                RightBoundary = PointB;
            }
            else
            {
                LeftBoundary = PointB;
                RightBoundary = PointA;
            }

            if (LeftBoundary < 0) LeftBoundary = 0;
            if (RightBoundary >= Buffer->Width ) RightBoundary = Buffer->Width - 1;
            
            for (i32 ColumnIndex = LeftBoundary;
                 ColumnIndex <= RightBoundary;
                 ++ColumnIndex)
            {
                if (ColumnIndex >= 0 && ColumnIndex < Buffer->Width)
                {
                    i32 RowTruncated = TruncateF32ToI32(RowIndex);
                    DrawPixel(Buffer, ColumnIndex, RowTruncated, 0xFFFFFFFF);
                }
            }
        }
    }
}

internal void
DrawScreenSpaceTriangle(game_offscreen_buffer *Buffer, vec4_f32 VertexMinY, vec4_f32 VertexMidY, vec4_f32 VertexMaxY)
{
    if (VertexMinY.Y > VertexMidY.Y)
    {
        vec4_f32 Temp = VertexMinY;
        VertexMinY = VertexMidY;
        VertexMidY = Temp;
    }
    if (VertexMidY.Y > VertexMaxY.Y)
    {
        vec4_f32 Temp = VertexMidY;
        VertexMidY = VertexMaxY;
        VertexMaxY = Temp;
    }
    if (VertexMinY.Y > VertexMidY.Y)
    {
        vec4_f32 Temp = VertexMinY;
        VertexMinY = VertexMidY;
        VertexMidY = Temp;
    }

    f32 TriangleTop = VertexMinY.Y;
    f32 TriangleMid = VertexMidY.Y;
    f32 TriangleBot = VertexMaxY.Y;
    
    if (TriangleTop < TriangleMid)
    {
        DrawBetweenVerticalLines(Buffer, VertexMinY, VertexMidY, VertexMinY, VertexMaxY, TriangleTop, TriangleMid);
    }
    
    if (TriangleMid < TriangleBot)
    {
        DrawBetweenVerticalLines(Buffer, VertexMidY, VertexMaxY, VertexMinY, VertexMaxY, TriangleMid, TriangleBot);
    }

    // DEBUG Points Draw
    DrawRectangle(Buffer, VertexMinY.X-3.0f, VertexMinY.Y-3.0f, VertexMinY.X+3.0f, VertexMinY.Y+3.0f, 0xFFFF5555, 0xFFFF5555);
    DrawRectangle(Buffer, VertexMidY.X-3.0f, VertexMidY.Y-3.0f, VertexMidY.X+3.0f, VertexMidY.Y+3.0f, 0xFF55FF55, 0xFF55FF55);
    DrawRectangle(Buffer, VertexMaxY.X-3.0f, VertexMaxY.Y-3.0f, VertexMaxY.X+3.0f, VertexMaxY.Y+3.0f, 0xFF5555FF, 0xFF5555FF);
}

internal void
Draw3DTriangle(game_offscreen_buffer *Buffer, vec4_f32 Vertex1, vec4_f32 Vertex2, vec4_f32 Vertex3)
{
    mat44_f32 ProjectionTransform = GetProjectionMatrix(90.0f, (f32)Buffer->Width, (f32)Buffer->Height, 1.0f, 1000.0f);
    mat44_f32 ScreenSpaceTransform = GetScreenSpaceTransform((f32)Buffer->Width, (f32)Buffer->Height);

    vec4_f32 ProjectedVertex1 = TransformVec4F32(ProjectionTransform, Vertex1);
    vec4_f32 ProjectedVertex2 = TransformVec4F32(ProjectionTransform, Vertex2);
    vec4_f32 ProjectedVertex3 = TransformVec4F32(ProjectionTransform, Vertex3);

    vec4_f32 PerspectiveVertex1 = PerspectiveDivideVec4F32(ProjectedVertex1);
    vec4_f32 PerspectiveVertex2 = PerspectiveDivideVec4F32(ProjectedVertex2);
    vec4_f32 PerspectiveVertex3 = PerspectiveDivideVec4F32(ProjectedVertex3);

    vec4_f32 ScreenSpaceVertex1 = TransformVec4F32(ScreenSpaceTransform, PerspectiveVertex1);
    vec4_f32 ScreenSpaceVertex2 = TransformVec4F32(ScreenSpaceTransform, PerspectiveVertex2);
    vec4_f32 ScreenSpaceVertex3 = TransformVec4F32(ScreenSpaceTransform, PerspectiveVertex3);

    DrawScreenSpaceTriangle(Buffer, ScreenSpaceVertex1, ScreenSpaceVertex2, ScreenSpaceVertex3);
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

    State->V1_X = 0.0f;
    State->V1_Y = 1.0f;
    State->V1_Z = -1.0f;

    State->V2_X = -1.0f;
    State->V2_Y = -1.0f;
    State->V2_Z = -1.0f;

    State->V3_X = 1.0f;
    State->V3_Y = -1.0f;
    State->V3_Z = -1.0f;

}

internal void
ProcessInput(game_state *State, game_input *Input)
{
    // i32 MouseXDeltaRange = 700;
    // State->PlayerAngle += -((f32)Input->MouseDX/(f32)MouseXDeltaRange) * Input->SecondsPerFrame * 20.0f;
    // if (State->PlayerAngle >= 2*Pi32)\
    // {
    //     State->PlayerAngle -= 2*Pi32;
    // }
    // else if (State->PlayerAngle < 0.0f)
    // {
    //     State->PlayerAngle += 2*Pi32;
    // }

    f32 RotationDelta = 1.0f*Input->SecondsPerFrame;
    if (Input->Up)
    {
        State->RotationX += RotationDelta;
        if (State->RotationX >= 2.0f*Pi32) State->RotationX -= 2.0f*Pi32;
    }
    else if (Input->Down)
    {
        State->RotationX -= RotationDelta;
        if (State->RotationX <= 0.0f) State->RotationX += 2.0f*Pi32;
    }
    
    if (Input->Right)
    {
        State->RotationY += RotationDelta;
        if (State->RotationY >= 2.0f*Pi32) State->RotationY -= 2.0f*Pi32;
    }
    else if (Input->Left)
    {
        State->RotationY -= RotationDelta;
        if (State->RotationY <= 0.0f) State->RotationY += 2.0f*Pi32;
    }

    if (Input->StrafeRight)
    {
        State->RotationZ -= RotationDelta;
        if (State->RotationZ >= 2.0f*Pi32) State->RotationZ -= 2.0f*Pi32;
    }
    else if (Input->StrafeLeft)
    {
        State->RotationZ += RotationDelta;
        if (State->RotationZ <= 0.0f) State->RotationZ += 2.0f*Pi32;
    }
}

internal void
GameUpdateAndRender(game_state *State, game_input *Input, game_offscreen_buffer *Buffer)
{
    ProcessInput(State, Input);
    
    DrawRectangle(Buffer, 0.0f, 0.0f, (f32)Buffer->Width, (f32)Buffer->Height, 0xFF000000, 0xFF000000);

    // Local
    vec4_f32 V1 = { -1.0f, -1.0f, 0.0f, 1.0f };
    vec4_f32 V2 = {  0.0f,  1.0f, 0.0f, 1.0f };
    vec4_f32 V3 = {  1.0f, -1.0f, 0.0f, 1.0f };

    // World
    // 1. Rotate in local space

    // State->RotationY = 0.0200000014f;
    mat44_f32 Rotation = GetRotationMatrix(State->RotationX, State->RotationY, State->RotationZ);
    V1 = TransformVec4F32(Rotation, V1);
    V2 = TransformVec4F32(Rotation, V2);
    V3 = TransformVec4F32(Rotation, V3);
    // 2. Scale to world
    mat44_f32 Scale = GetScaleMatrix(1.0f, 1.0f, 1.0f);
    V1 = TransformVec4F32(Scale, V1);
    V2 = TransformVec4F32(Scale, V2);
    V3 = TransformVec4F32(Scale, V3);
    // 3. Translate to world
    mat44_f32 Translation = GetTranslationMatrix(0.0f, 0.0f, -2.0f);
    V1 = TransformVec4F32(Translation, V1);
    V2 = TransformVec4F32(Translation, V2);
    V3 = TransformVec4F32(Translation, V3);

    // State->RotationY += 1.2f*Input->SecondsPerFrame;
    // if (State->RotationY >= 2.0f*Pi32) State->RotationY -= 2.0f*Pi32;
    
    Draw3DTriangle(Buffer, V1, V2, V3);

    // DEBUGPrintString("", );
}
