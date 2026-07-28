#include "raylib.h"
#include <iostream>

int main(void)
{
    std::cout << "Initializing 3D Project..." << std::endl;

    InitWindow(1280, 720, "Raylib 3D C++ - Win/Switch Homebrew");
    
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawCube((Vector3){ 0, 1, 0 }, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires((Vector3){ 0, 1, 0 }, 2.0f, 2.0f, 2.0f, MAROON);
                DrawGrid(10, 1.0f);
            EndMode3D();

            DrawText("3D Project C++ (Win/Switch)", 10, 10, 20, DARKGRAY);
            DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}