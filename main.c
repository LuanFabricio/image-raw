#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#define RAYLIB_TITLE "IDK"
#define IMAGE_SIZE 96

void save_image(const char* input_file, const char* output_file)
{
	Image img = LoadImage(input_file);

	printf("Image width: %d\n", img.width);
	printf("Image height: %d\n", img.height);

	ImageResize(&img, IMAGE_SIZE, IMAGE_SIZE);
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	Color *data = (Color*)img.data;
	uint8_t img_bytes[IMAGE_SIZE*IMAGE_SIZE];

	for (int i = 0; i < IMAGE_SIZE*IMAGE_SIZE; i++) {
		uint8_t gray = (data[i].r + data[i].g + data[i].b) / 3;
		img_bytes[i] = gray;
	}

	FILE* file = fopen(output_file, "wb");
	fwrite(img_bytes, 1, IMAGE_SIZE*IMAGE_SIZE, file);
	fclose(file);
}

void view_image(const char* filename)
{
	Image img = LoadImageRaw(filename, IMAGE_SIZE, IMAGE_SIZE, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 0);
	printf("Image width: %d\n", img.width);
	printf("Image height: %d\n", img.height);
	printf("Image format: %d\n", img.format);

	InitWindow(1280, 720, RAYLIB_TITLE);
	SetTargetFPS(60);

	Texture2D tex = LoadTextureFromImage(img);
	Camera2D camera = {
		.offset = {0.0, 0.0},
		.target = {0.0, 0.0},
		.rotation = 0.0,
		.zoom = 1.0,
	};
	camera.offset.x = GetScreenWidth() * 0.5;
	camera.offset.y = GetScreenHeight() * 0.5;
	bool draggin = false;
	Vector2 anchor = Vector2Zero();

	printf("Camera offset: %.02f, %.02f\n", camera.offset.x, camera.offset.y);
	printf("Camera target: %.02f, %.02f\n", camera.target.x, camera.target.y);
	printf("Camera rotation: %f\n", camera.rotation);
	printf("Camera zoom: %f\n", camera.zoom);

	Vector2 vzero = Vector2Zero();
	const float MOUSE_WHEEL_SPEED = 0.1;
	while (!WindowShouldClose()) {
		float mw = GetMouseWheelMove();
		if (mw < 0) {
			camera.zoom -= MOUSE_WHEEL_SPEED;
		} else if (mw > 0) {
			camera.zoom += MOUSE_WHEEL_SPEED;
		}

		Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			draggin = true;
			anchor = mouse;
		} else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
			draggin = false;
		}

		ClearBackground(DARKBLUE);
		BeginDrawing();
		BeginMode2D(camera);
			DrawTextureV(tex, vzero, WHITE);
		EndMode2D();
		EndDrawing();

		if (draggin) {
			camera.target = Vector2Subtract(camera.target, Vector2Subtract(mouse, anchor));
		}
	}

	UnloadImage(img);
	UnloadTexture(tex);
	CloseWindow();
}

int main(int argc, char* argv[])
{
	if (argc > 2 && strcmp(argv[1], "-load-image") == 0) {
		save_image(argv[2], argv[3]);
	}

	if (argc > 2 && strcmp(argv[1], "-view-image") == 0) {
		view_image(argv[2]);
	}

	return 0;
}
