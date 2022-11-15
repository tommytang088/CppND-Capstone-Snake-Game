#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "controller.h"
#include "field.h"
#include "piece.h"
#include "renderer.h"
#include <future>
#include <memory>
#include <mutex>
#include <random>

class Game {
public:
  Game(std::size_t grid_width, std::size_t grid_height);
  void Run(Controller const &controller, Renderer &renderer,
           std::size_t target_frame_duration);
  int GetScore() const;
  int GetLevel() const;

private:
  // private behavior methods
  void SimulatePiece();
  void UpdateScore();
  float ComputePieceDescendSpeed();

  std::size_t _gridWidth;
  std::size_t _gridHeight;
  PieceGenerator generator;
  std::unique_ptr<Piece> _piece; // pointer to the current piece
  std::shared_ptr<Field> _field; // pointer to the field
  std::mutex _mutex;
  std::future<void> _future;

  int _score{0};
  int _rowsCleared{0}; // number of rows cleared
  int _scorePerLevel{10};
  int _maxLevel{5};
  int _level{1};                // current level of the game
  float _baseDescendSpeed{0.1}; // default descending speed, 0.1 cells per cycle

  void PlaceFood();
  void Update();
};

#endif