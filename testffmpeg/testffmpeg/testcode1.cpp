#include "stdafx.h"
#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "SDL\SDL.h"
}

#define REFRESH_EVENT (SDL_USEREVENT + 1)
#define BREAK_EVENT (SDL_USEREVENT + 2)

const int pixel_w = 320;
const int pixel_h = 180;
int screen_w = 500, screen_h = 500;
const int bpp = 12;

unsigned char buffer[pixel_w*pixel_h*bpp / 8];

int thread_exit = 0;
int refresh_video(void* opaque)
{
	thread_exit = 0;
	while (!thread_exit)
	{
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);

		SDL_Delay(40);
	}

	thread_exit = 0;
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* screen = SDL_CreateWindow("sample sdl window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		printf("create sdl window : %d", SDL_GetError());
		return -1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(screen, -1, 0);

	int pixformat = SDL_PIXELFORMAT_IYUV;

	SDL_Texture* texture = SDL_CreateTexture(renderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

	FILE* pYUV;
	pYUV = fopen("test_yuv420p_320x180.yuv", "rb+");
	if (pYUV == NULL)
	{
		printf("open file error");
		return -1;
	}
	SDL_Rect rect;
	SDL_Event event;
	SDL_Thread* sdl_thread = SDL_CreateThread(refresh_video, NULL, NULL);
	while (1)
	{
		SDL_WaitEvent(&event);

		if (event.type == REFRESH_EVENT)
		{
			if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, pYUV) != pixel_w*pixel_h*bpp / 8)
			{
				fseek(pYUV, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*bpp / 8, pYUV);
			}
			rect.x = 0;
			rect.y = 0;
			rect.w = screen_w;
			rect.h = screen_h;
			SDL_UpdateTexture(texture, NULL, buffer, pixel_w);

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_RenderPresent(renderer);
		}
		else if (event.type == BREAK_EVENT)
		{
			break;
		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			SDL_GetWindowSize(screen, &screen_w, &screen_h);
		}


	}

	SDL_Quit();
	return 0;
}