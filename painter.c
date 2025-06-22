/* 
CGA Painter - A paint program for my 3-bit color cga file format
Copyright (C) 2025 Connor Thomson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or 
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Compile: gcc painter.c -o painter `sdl2-config --cflags --libs`
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_WIDTH 128
#define MAX_HEIGHT 128

SDL_Color palette[8] = {
    {0, 0, 0, 255},       // 1 - Black
    {255, 0, 0, 255},     // 2 - Red
    {0, 255, 0, 255},     // 3 - Green
    {0, 0, 255, 255},     // 4 - Blue
    {255, 255, 0, 255},   // 5 - Yellow
    {255, 0, 255, 255},   // 6 - Magenta
    {0, 255, 255, 255},   // 7 - Cyan
    {255, 255, 255, 255}  // 8 - White
};

void save_cga_file(char pixels[MAX_HEIGHT][MAX_WIDTH], int width, int height, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            fputc(pixels[y][x], f);
        }
        fputc(';', f);
        fputc('\n', f);
    }
    fclose(f);
}

int main() {
    int width, height;
    char filename[256];

    printf("Enter canvas width (max %d): ", MAX_WIDTH);
    scanf("%d", &width);
    printf("Enter canvas height (max %d): ", MAX_HEIGHT);
    scanf("%d", &height);
    printf("Enter output filename (e.g., art): ");
    scanf("%s", filename);

    if (strlen(filename) < 4 || strcmp(filename + strlen(filename) - 4, ".cga") != 0) {
        strcat(filename, ".cga");
    }

    if (width <= 0 || height <= 0 || width > MAX_WIDTH || height > MAX_HEIGHT) {
        printf("Invalid size.\n");
        return 1;
    }

    char pixels[MAX_HEIGHT][MAX_WIDTH];
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            pixels[y][x] = '8';

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("Computer Graphics Array Painter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int zoom = WINDOW_WIDTH / width;
    if (WINDOW_HEIGHT / height < zoom) zoom = WINDOW_HEIGHT / height;
    int canvasW = width * zoom;
    int canvasH = height * zoom;
    int offsetX = (WINDOW_WIDTH - canvasW) / 2;
    int offsetY = (WINDOW_HEIGHT - canvasH) / 2;

    int brush = 1;
    int mouse_down_left = 0;
    int mouse_down_right = 0;
    int running = 1;
    save_cga_file(pixels, width, height, filename);

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_8)
                    brush = e.key.keysym.sym - SDLK_0;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) mouse_down_left = 1;
                if (e.button.button == SDL_BUTTON_RIGHT) mouse_down_right = 1;
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) mouse_down_left = 0;
                if (e.button.button == SDL_BUTTON_RIGHT) mouse_down_right = 0;
            }

            if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = (e.type == SDL_MOUSEMOTION) ? e.motion.x : e.button.x;
                int my = (e.type == SDL_MOUSEMOTION) ? e.motion.y : e.button.y;
                int cx = (mx - offsetX) / zoom;
                int cy = (my - offsetY) / zoom;
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    if (mouse_down_left)
                        pixels[cy][cx] = '0' + brush;
                    else if (mouse_down_right)
                        pixels[cy][cx] = '8';
                    save_cga_file(pixels, width, height, filename);
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        SDL_Color bc = palette[brush - 1];
        SDL_SetRenderDrawColor(ren, bc.r, bc.g, bc.b, 255);
        SDL_Rect border = {offsetX - 1, offsetY - 1, canvasW + 2, canvasH + 2};
        SDL_RenderDrawRect(ren, &border);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = pixels[y][x] - '1';
                if (index >= 0 && index < 8) {
                    SDL_Color c = palette[index];
                    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
                    SDL_Rect rect = {
                        offsetX + x * zoom,
                        offsetY + y * zoom,
                        zoom,
                        zoom
                    };
                    SDL_RenderFillRect(ren, &rect);
                }
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
