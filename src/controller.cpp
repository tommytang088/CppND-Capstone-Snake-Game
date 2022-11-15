#include "controller.h"
#include "SDL.h"
#include "piece.h"
#include <iostream>

// controls the piece with arrow keys
void Controller::HandleInput(bool &running, Piece &piece) const {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      running = false;
    } else if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.sym) {
      case SDLK_LEFT:
        piece.Move(Direction::kLeft);
        break;
      case SDLK_RIGHT:
        piece.Move(Direction::kRight);
        break;
      case SDLK_UP:
        piece.Rotate(Rotation::kForward);
        break;
      case SDLK_DOWN:
        piece.Drop();
        break;
      }
    }
  }
}
