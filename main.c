#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include "raylib.h"
#include "raymath.h"

#define RAYLIB_TITLE "IDK"
#define IMAGE_SIZE 112

typedef enum {
	GRAYSCALE = 0,
	RGB,
} ImageColorEnum;

int list_dir(char* folder, char** buffer, int *len)
{
	DIR *dir;
	struct dirent *ep;

	dir = opendir(folder);

	if (dir == NULL) {
		return 0;
	}

	*len = 0;
	while((ep = readdir(dir)) != NULL) {
		bool is_invalid = strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0;
		if (is_invalid) continue;

		printf("[%i]: %s(%u/%lu)\n", *len, ep->d_name, ep->d_reclen, strlen(ep->d_name));
		(*len)++;
	}

	printf("Len: %i(%lu)\n", *len, sizeof(char**) * (*len));
	seekdir(dir, 0);
	int index = 0;
	while((ep = readdir(dir)) != NULL) {
		bool is_invalid = strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0;
		if (is_invalid) continue;

		buffer[index] = malloc(sizeof(char*) * strlen(ep->d_name));
		strncpy(buffer[index], ep->d_name, strlen(ep->d_name));
		printf("%s\n", buffer[index]);
		index++;
	}

	closedir(dir);
	return 1;
}

void save_image_grayscale(uint8_t* img_bytes, const size_t img_size, const Color* og_colors)
{
	for (int i = 0; i < img_size; i++) {
		uint8_t gray = (og_colors[i].r + og_colors[i].g + og_colors[i].b) / 3;
		img_bytes[i] = gray;
	}
}

void save_image_rgb(uint8_t* img_bytes, const size_t img_size, const uint8_t* og_img_colors)
{
	int img_idx = 0;
	for (int i = 0; i < img_size*4; i++) {
		img_bytes[img_idx++] = og_img_colors[i++];
		img_bytes[img_idx++] = og_img_colors[i++];
		img_bytes[img_idx++] = og_img_colors[i++];
	}
}

void save_image(const char* input_file, const char* output_file, const int img_format)
{
	Image img = LoadImage(input_file);

	printf("[Old] Image width: %d\n", img.width);
	printf("[Old] Image height: %d\n", img.height);

	ImageResize(&img, IMAGE_SIZE, IMAGE_SIZE);
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	printf("[New] Image width: %d\n", img.width);
	printf("[New] Image height: %d\n", img.height);

	size_t img_channels = 1;
	if (img_format == RGB) img_channels = 3;
	const size_t img_size = IMAGE_SIZE * IMAGE_SIZE;

	const size_t img_bytes_len = img_size * img_channels;
	uint8_t img_bytes[IMAGE_SIZE*IMAGE_SIZE*3] = {0};

	if (img_format == RGB) save_image_rgb(img_bytes, img_size, img.data);
	else save_image_grayscale(img_bytes, img_size, img.data);


	FILE* file = fopen(output_file, "wb");
	fwrite(img_bytes, 1, img_bytes_len, file);
	fclose(file);
}

void view_image(const char* filename, int img_format)
{
	int format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	if (img_format == RGB) format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

	Image img = LoadImageRaw(filename, IMAGE_SIZE, IMAGE_SIZE, format, 0);
	// Image img = LoadImage(filename);
	// ImageResize(&img, IMAGE_SIZE, IMAGE_SIZE);
	// ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

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
	char *folder_buffer[1024] = {};
	int len;
	int img_format = GRAYSCALE;

	if (argc > 2 && strcmp(argv[1], "-save-image") == 0) {
		for (int i = 0 ; i < argc; i++) printf("argv[%i]: %s\n", i, argv[i]);
		if (argc > 4 && strcmp(argv[5], "rgb") == 0) img_format = RGB;

		if (argc > 2 && strcmp(argv[2], "folder") == 0) {
			if (!list_dir(argv[3], folder_buffer, &len)) printf("To read folder '%s'\n", argv[5]);

			for (int i = 0; i < len; i++) {
				char buffer_folder_input[128] = "";
				strcpy(buffer_folder_input, argv[3]);
				char buffer_folder_output[128] = "";
				strcpy(buffer_folder_output, argv[4]);

				char buffer_in[1024] = "";
				strcat(strcat(buffer_in, buffer_folder_input), folder_buffer[i]);
				char buffer_out[1024] = "";
				strcat(strcat(buffer_out, strcat(buffer_folder_output, folder_buffer[i])), ".raw");

				printf("Buffer in: %s\n", buffer_in);
				printf("Buffer out: %s\n", buffer_out);
				save_image(buffer_in, buffer_out, img_format);
			}
		} else {
			save_image(argv[3], argv[4], img_format);
		}
	}

	if (argc > 2 && strcmp(argv[1], "-view-image") == 0) {
		if (argc > 3 && strcmp(argv[3], "rgb") == 0) img_format = RGB;
		view_image(argv[2], img_format);
	}

	return 0;
}
