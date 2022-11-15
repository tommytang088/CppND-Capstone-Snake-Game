#include "game.h"
#include "SDL.h"
#include <algorithm>
#include <future>
#include <iostream>
#include <random>

// Initialize the game with empty field and a starting piece
Game::Game(std::size_t gridWidth, std::size_t gridHeight)
    : _gridWidth(gridWidth), _gridHeight(gridHeight) {
  _field = std::make_shared<Field>(_gridWidth, _gridHeight);
  generator = PieceGenerator();
  _piece = generator.GeneratePiece(_gridWidth, _gridHeight, _baseDescendSpeed,
                                   _field);
}

void Game::Run(Controller const &controller, Renderer &renderer,
               std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  int rowCleared = 0;
  bool running = true;
  bool alive = true; // whether the current piece is alive or not

  SimulatePiece(); // simulates the starting piece

  // Input, Update, Render - the main game loop.
  while (running) {
    frame_start = SDL_GetTicks();
    // keeps generating new pieces and simulating its descent with a child
    // thread, repeately checking the status of the future, the promis would
    // return when the piece reaches the field in the child thread
    if (alive and _future.wait_for(std::chrono::milliseconds(0)) ==
                      std::future_status::ready) {
      // updates score when a new piece is generated, and uses the score to
      // determine next piece's speed
      UpdateScore();
      _piece = generator.GeneratePiece(_gridWidth, _gridHeight,
                                       ComputePieceDescendSpeed(), _field);
      // game ends if the new piece cannot be placed
      if (_piece->IsPlaceble()) {
        std::cout << _piece->GetName() << " cannot be created. Game Over."
                  << std::endl;
        alive = false;
      } else {
        // otherwise, simulates the new piece's descent
        SimulatePiece();
      }
    }
    controller.HandleInput(running, *_piece);
    std::unique_lock<std::mutex> lck(_mutex);
    renderer.Render(*_piece, *_field);
    lck.unlock();

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle
    // takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(_score, _level, frame_count);
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to
    // achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }
  }
}

// Simulates the piece's descent in the child thread. Uses promise and future
// communiate between child thread and the main thread.
void Game::SimulatePiece() {
  std::promise<void> prms = std::promise<void>();
  _future = prms.get_future();
  _piece->Simulate(std::move(prms));
}

// Adds the square of the additional rows cleared to the total score, and
// updates current level
void Game::UpdateScore() {
  int cleared = _field->GetRowsCleared() - _rowsCleared;
  _rowsCleared = _field->GetRowsCleared();
  _score += cleared * cleared;
  _level = std::min(_maxLevel, 1 + static_cast<int>(_score / _scorePerLevel));
};

// Increases descending speed linearly at each level until reaching max level.
float Game::ComputePieceDescendSpeed() { return _baseDescendSpeed * _level; }

int Game::GetScore() const { return _score; }
int Game::GetLevel() const { return _level; }