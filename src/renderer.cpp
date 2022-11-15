#include "renderer.h"
#include <iostream>
#include <string>

Renderer::Renderer(const std::size_t screen_width,
                   const std::size_t screen_height,
                   const std::size_t grid_width, const std::size_t grid_height)
    : screen_width(screen_width), screen_height(screen_height),
      grid_width(grid_width), grid_height(grid_height) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create Window
  sdl_window = SDL_CreateWindow("Tetris Game", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, screen_width,
                                screen_height, SDL_WINDOW_SHOWN);

  if (nullptr == sdl_window) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create renderer
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (nullptr == sdl_renderer) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }
}

Renderer::~Renderer() {
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

void Renderer::Render(Piece const &piece, Field const &field) {
  SDL_Rect block;
  // leave one extra space outside space to represent border
  block.w = screen_width / grid_width - 2;
  block.h = screen_height / grid_height - 2;

  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);

  // Render piece
  const std::vector<int> &color = piece.GetColorCodes();
  SDL_SetRenderDrawColor(sdl_renderer, color[0], color[1], color[2], color[3]);

  for (auto &c : piece.GetBody()) {
    if (!piece.CellOutsideScreen(c[0], c[1])) {
      block.x = 1 + c[0] * (block.w + 2);
      block.y = 1 + c[1] * (block.h + 2);
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render field
  SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

  auto &grid = field.GetGrid();
  for (int x = 0; x < field.GetWidth(); x++) {
    for (int y = 0; y < field.GetHeight(); y++) {
      if (grid[y][x] == 1) {
        block.x = 1 + x * (block.w + 2);
        block.y = 1 + y * (block.h + 2);
        SDL_RenderFillRect(sdl_renderer, &block);
      }
    }
  }

  // Update Screen
  SDL_RenderPresent(sdl_renderer);
}

void Renderer::UpdateWindowTitle(int score, int level, int fps) {
  std::string title{"Tetris Score: " + std::to_string(score) + " Level: " +
                    std::to_string(level) + " FPS: " + std::to_string(fps)};
  SDL_SetWindowTitle(sdl_window, title.c_str());
}
