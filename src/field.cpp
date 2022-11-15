#include "field.h"
#include <algorithm>
#include <vector>

Field::Field(int gridWidth, int gridHeight)
    : _gridWidth(gridWidth), _gridHeight(gridHeight),
      _grid(gridHeight, std::vector<int>(gridWidth, 0)){};

// adds the cells of the piece to the field by updating the corresponding
// elements in the 2D vector with 1
void Field::AddPiece(const Piece &piece) {
  int row = 0;
  for (auto &c : piece.GetBody()) {
    _grid[c[1]][c[0]] = 1;
    row = std::max(row, c[1]);
  }
  ClearFrom(row);
};

// clears rows above and includes the current row in the field
void Field::ClearFrom(const int &row) {
  int cleared{0};
  int s;
  for (int i = row; i >= 0; i--) {
    // computes the number of grids in each row
    s = std::reduce(_grid[i].begin(), _grid[i].end());
    if (s == 0) {
      break;
    } else if (s == _gridWidth) {
      // the current row is full, clears it by setting all elements to 0
      for (int &v : _grid[i])
        v = 0;
      cleared++;
    } else if (cleared > 0) {
      // if the current row has grids, move them down by number of rows cleared
      // so far
      for (int j = 0; j < _gridWidth; j++) {
        _grid[i + cleared][j] = _grid[i][j];
        _grid[i][j] = 0;
      }
    }
  }
  _rowsCleared += cleared;
};