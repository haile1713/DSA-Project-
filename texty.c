#define STB_IMAGE_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "./stb_image.h"

void scc(int code){
	if(code == -1){
		fprintf(stderr,"sdl error %s \n",SDL_GetError());
		exit(1);
	}
}

void *scp(void *ptr){
	if(ptr == NULL){
		fprintf(stderr,"sdl error %s \n",SDL_GetError());
		exit(1);
	}
	return ptr;
}
int main(void){
	scc(SDL_Init(SDL_INIT_VIDEO));
	SDL_Window * window = scp(
		SDL_CreateWindow(
			"Texty",
			 0, 0, 
			800, 600, 
			SDL_WINDOW_RESIZABLE
		));

        SDL_Renderer * renderer =  scp(
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)
	);

	bool quit = false;
	while (!quit){
		SDL_Event event = {0};
		while (SDL_PollEvent (&event) ) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
				}
				break;
			}
		}
		scc(SDL_SetRenderDrawColor( renderer, 100,0,100,0));
		scc(SDL_RenderClear(renderer));
	  
		SDL_RenderPresent(renderer);
	}
	SDL_Quit();
	return 0;
}
