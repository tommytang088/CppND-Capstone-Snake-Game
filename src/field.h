#ifndef FIELD_H
#define FIELD_H

#include "piece.h"
#include <vector>

class Piece;

class Field {
public:
  Field(int gridWidth, int gridHeight);

  // getters
  int GetWidth() const { return _gridWidth; };
  int GetHeight() const { return _gridHeight; };
  const std::vector<std::vector<int>> &GetGrid() const { return _grid; };

  // behavior methods
  void AddPiece(const Piece &piece);
  int GetRowsCleared() { return _rowsCleared; };

private:
  void ClearFrom(const int &row);

  int _gridWidth;
  int _gridHeight;
  int _rowsCleared{0};
  std::vector<std::vector<int>>
      _grid; // 2d vector with 0 and 1, 0 = empty, 1 = occupied
};

#endif