// clear && g++ -std=c++17 -Wall -O3 -s main.cpp -D_REENTRANT -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf && ./a.out

#include "Crillee_ttf.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <deque>
#include <iostream>
#include <algorithm>
#include <fstream>

#define WIDTH 100
#define HEIGHT 100
#define TILE_DIM 5

enum class dir {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

SDL_Point apple_pos = {WIDTH/2, HEIGHT/2};

struct Snake {
private:
	bool first_frame = true;
	
public:
	inline static std::deque<Snake*> snakes;
	
	dir facing = dir::RIGHT;
	std::deque<SDL_Point> points;
	SDL_Point head_pos = {2, 2};
	SDL_Color color = {255, 255, 255, 255};
	
	bool automated = false;
	
	Snake() {
		color = {static_cast<Uint8>(rand()%255), static_cast<Uint8>(rand()%255), static_cast<Uint8>(rand()%255), 0};
		snakes.push_back(this);
	}
	~Snake() {
		snakes.erase(std::find(snakes.begin(), snakes.end(), this));
	}
	
	void update() {
		if(first_frame) {
			points.push_back(head_pos);
			points.push_back({head_pos.x+1, head_pos.y});
			first_frame = false;
		}
		
		switch(Snake::facing) {
			case dir::UP:
				--head_pos.y;
				break;
			case dir::DOWN:
				++head_pos.y;
				break;
			case dir::LEFT:
				--head_pos.x;
				break;
			case dir::RIGHT:
				++head_pos.x;
				break;
		}
		
		/* Kills the snake */
		if(head_pos.x < 0 || head_pos.x > WIDTH) {
			printf("Game over! Score: %lu\n", points.size());
			this->~Snake();
		}
		if(head_pos.y < 0 || head_pos.y > HEIGHT) {
			printf("Game over! Score: %lu\n", points.size());
			this->~Snake();
		}
		for(auto it = points.begin(); it < points.end()-1; ++it) {
			if(head_pos.x == (*it).x && head_pos.y == (*it).y) {
				printf("Game over! Score: %lu\n", points.size());
				this->~Snake();
			}
		}
		
		points.push_back(head_pos);
		
		/*
		if(automated) {
			switch(facing) {
				case dir::RIGHT:
					//if(apple_pos.x < head_pos.x)
						//facing = dir::DOWN;
						
					//if(apple_pos.x == head_pos.x) {
						if(apple_pos.y > head_pos.y)
							facing = dir::DOWN;
						else
							facing = dir::UP;
					//}
					break;
					
				case dir::LEFT:
					//if(apple_pos.x > head_pos.x)
						//facing = dir::DOWN;
						
					//if(apple_pos.x == head_pos.x) {
						if(apple_pos.y > head_pos.y)
							facing = dir::DOWN;
						else
							facing = dir::UP;
					//}
					break;
				
				case dir::DOWN:
					//if(apple_pos.y < head_pos.y)
						//facing = dir::LEFT;
						
					//if(apple_pos.y == head_pos.y) {
						if(apple_pos.x > head_pos.x)
							facing = dir::RIGHT;
						else
							facing = dir::LEFT;
					//}
					break;
				
				case dir::UP:
					//if(apple_pos.y > head_pos.y)
						//facing = dir::LEFT;
						
					//if(apple_pos.y == head_pos.y) {
						if(apple_pos.x > head_pos.x)
							facing = dir::RIGHT;
						else
							facing = dir::LEFT;
					//}
					break;
			}
		}
		*/
		
		/* Resettle the apple if it's touching our head */
		if(apple_pos.x == head_pos.x && apple_pos.y == head_pos.y) {
			for(SDL_Point point : points) {
				apple_pos.x = rand() % (WIDTH-8) + 4;
				apple_pos.y = rand() % (HEIGHT-8) + 4;
				
				if(apple_pos.x != point.x && apple_pos.y != point.y) break;
			}
		} else {
			/* Do not overall enlarge the snake if it is not eating the apple */
			points.pop_front();
		}
	}
};

// The snake of our current player
Snake player;

namespace Canvas {
	SDL_Window* window;
	SDL_Renderer* renderer;
	
	inline constexpr int top_margin_h = 30;
	
	void init() {
		static bool initted = false;
		if(!initted) {
			SDL_Init(SDL_INIT_VIDEO);
	
			if(TTF_Init() < 0) {
				std::cout << "Error intializing SDL_ttf: " << TTF_GetError() << std::endl;
				exit(1);
			}
			
			SDL_CreateWindowAndRenderer(WIDTH * TILE_DIM, HEIGHT * TILE_DIM + top_margin_h, 0, &window, &renderer);
			SDL_SetWindowTitle(Canvas::window, "Fuck all niggers. Fuck atheist forilla niggers");
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
			
			initted = true;
		}
	}
	
	void draw_tile(SDL_Point pos) {
		for(int y = 0; y < TILE_DIM; ++y) {
			for(int x = 0; x < TILE_DIM; ++x) {
				SDL_RenderDrawPoint(renderer, pos.x * TILE_DIM + x, pos.y * TILE_DIM + y + top_margin_h);
			}
		}
	}
	
	void display_score() {
		static bool first_run = true;
		static TTF_Font* crillee;
		
		if(first_run) {
			// Dumps the bytes of the font's TTF into a file so that TTF_OpenFont() can read it
			std::string temp_filename = std::tmpnam(nullptr);
			std::ofstream temp_file(temp_filename, std::ios::binary);
			temp_file.write((char *)&Crillee_ttf[0], Crillee_ttf_len);
			crillee = TTF_OpenFont(temp_filename.c_str(), 12);
			
			first_run = false;
			
			if(crillee == NULL) {
				std::cout << "Error loading font" << std::endl;
				return;
			}
		}
		
		static SDL_Surface* text_surface;
		static SDL_Texture* text_texture;
		static SDL_Rect text_rect;

		const char* text = (std::string("Score: " + std::to_string(player.points.size()))).c_str();
			
		text_surface = TTF_RenderText_Shaded(crillee, text, {255, 255, 255, 255}, {0, 0, 0, 0});
		if(!text_surface) {
			std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
			return;
		}
		text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		// Once the surface is converted to texture it is no longer needed
		SDL_FreeSurface(text_surface);
		
		// Sets the width and height of the rectangle holding the text
		text_rect = {.x = 10, .y = (top_margin_h-text_surface->h)/2, .w = text_surface->w, .h = text_surface->h};

		SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
		
		// Frees the texture
		SDL_DestroyTexture(text_texture);
	}
	
	void render() {
		SDL_SetRenderDrawColor(Canvas::renderer, 0, 0, 0, 0);
		SDL_RenderClear(Canvas::renderer);
		
		/* Draws the apple */
		SDL_SetRenderDrawColor(Canvas::renderer, 255, 0, 0, 255);
		draw_tile(apple_pos);
		
		/* Draws the top margin */
		SDL_SetRenderDrawColor(Canvas::renderer, 120, 120, 120, 255);
		SDL_RenderDrawLine(renderer, 0, top_margin_h, WIDTH * TILE_DIM, top_margin_h);
		
		for(Snake* snake : Snake::snakes) {
			snake->update();
			
			/* Draws the snake */
			SDL_SetRenderDrawColor(Canvas::renderer, snake->color.r, snake->color.g, snake->color.b, 255);
			for(auto point = snake->points.begin(); point != snake->points.end(); ++point) {
				if(point == snake->points.end()-1)
					SDL_SetRenderDrawColor(Canvas::renderer, snake->color.r, snake->color.g, snake->color.b, 170);
					
				draw_tile(*point);
			}
		}
			
		display_score();
		
		SDL_RenderPresent(renderer);
	}
}

bool paused = false;

/* Listens for basic user input: closing the application, and changing direction */
void listen_events() {
	static SDL_Event event;	
	if(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			SDL_DestroyRenderer(Canvas::renderer);
			SDL_DestroyWindow(Canvas::window);
			SDL_Quit();
			exit(0);
		}
		else if(event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_RIGHT:
					if(player.facing == dir::LEFT) break;
					player.facing = dir::RIGHT;
					break;
				case SDLK_LEFT:
					if(player.facing == dir::RIGHT) break;
					player.facing = dir::LEFT;
					break;
				case SDLK_UP:
					if(player.facing == dir::DOWN) break;
					player.facing = dir::UP;
					break;
				case SDLK_DOWN:
					if(player.facing == dir::UP) break;
					player.facing = dir::DOWN;
					break;
				case SDLK_SPACE:
					paused = !paused;
					break;
			}
		}
	}
}

int main() {
	srand(time(NULL));
	
	Canvas::init();
	
	player = Snake();
	
	Snake nigger;
	nigger.automated = true;
	
	while(true) {
		listen_events();
		if(!paused) {
			Canvas::render();
		}
		usleep(50000);
	}
	
	return 0;
}
