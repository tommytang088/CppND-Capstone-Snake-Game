#include "piece.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

// create a new piece centered at the top
Piece::Piece(int gridWidth, int gridHeight)
    : _gridWidth(gridWidth), _gridHeight(gridHeight),
      _centerX(gridWidth / 2 - 1), _centerY(0) {
  _centerCellX = static_cast<int>(_centerX);
  _centerCellY = static_cast<int>(_centerY);
  _field = nullptr;
}

Piece::~Piece() {
  std::cout << "Piece destructor is called" << std::endl;
  _free = false;

  // if thread is running, setup thread barrier before destroying this object
  if (_thread.joinable())
    _thread.join();
}

// set the pointer to the field object
void Piece::SetField(std::shared_ptr<Field> field) { _field = field; };

void Piece::InitBody() {
  const std::map<int, std::vector<std::vector<int>>> &shapes = GetShapes();
  for (int i = 0; i < GetSize(); i++)
    _body.emplace_back(
        std::vector<int>{_centerCellX + shapes.at(_currentShape)[i][0],
                         _centerCellY + shapes.at(_currentShape)[i][1]});
}

// udpate the cooridates of each cell in the body of the piece
void Piece::UpdateBody() {
  const std::map<int, std::vector<std::vector<int>>> &shapes = GetShapes();
  for (int i = 0; i < GetSize(); i++)
    _body[i] = std::vector<int>{_centerCellX + shapes.at(_currentShape)[i][0],
                                _centerCellY + shapes.at(_currentShape)[i][1]};
};

// create a thread to simulate the piece's descent from the top of the screen
void Piece::Simulate(std::promise<void> &&prms) {
  if (_body.empty())
    InitBody();
  _free = true;
  _thread = std::thread(&Piece::Descend, this, std::move(prms));
}

// runs an infinite loop to descend the piece, returns when the piece reaches
// bottom of the screen or the field, or when the program terminates
void Piece::Descend(std::promise<void> &&prms) {
  int prevCellY;
  std::chrono::time_point<std::chrono::system_clock> cycleStartTime =
      std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> cycleEndTime =
      cycleStartTime + std::chrono::milliseconds(_descendCycleTime);
  // check piece is not blocked at the start
  if (!IsBlocked(Direction::kDown)) {
    while (true) {
      // piece is not free to move due to external termination
      if (!_free)
        break;
      std::unique_lock<std::mutex> lck(_mutex);
      // updates the piece's center every cycle, if the center moves to a new
      // cell, updates all cells in the body
      if (std::chrono::system_clock::now() > cycleEndTime) {
        cycleStartTime = cycleEndTime;
        cycleEndTime =
            cycleStartTime + std::chrono::milliseconds(_descendCycleTime);
        prevCellY = _centerCellY;
        _centerY += _descendSpeed;
        _centerCellY = static_cast<int>(_centerY);
        if (_centerCellY != prevCellY) {
          UpdateBody();
          // breaks the loop if the piece is blocked below after moving to a new
          // location
          if (IsBlocked(Direction::kDown)) {
            lck.unlock();
            break;
          }
        }
      }
      lck.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  AddToField();     // adds the piece to the field object
  prms.set_value(); // notifies the future in the main thread
}

// add cells in the piece's body to the field object
void Piece::AddToField() {
  if (_field != nullptr) {
    std::lock_guard<std::mutex> lck(_mutex);
    _field->AddPiece(*this);
  }
}

// returns true if the cell defined by x and y coordinates is outside the screen
bool Piece::CellOutsideScreen(const int &x, const int &y) const {
  return x < 0 or x == _gridWidth or y < 0 or y == _gridHeight;
};

// returns true if the cell defined by x and y coordinates is occupied by the
// field object
bool Piece::CellOccupied(const int &x, const int &y) const {
  if (_field != nullptr and x >= 0 and x < _gridWidth and y >= 0 and
      y < _gridHeight)
    return _field->GetGrid()[y][x] == 1;
  return false;
};

// returns true if the cell is blocked and therefore not allowed to move one
// cell in the input direction
bool Piece::IsBlocked(const Direction &d) {
  int dx;
  int dy;
  int x;
  int y;
  std::string dStr;
  switch (d) {
  case Direction::kDown:
    dx = 0;
    dy = 1;
    dStr = "down";
    break;
  case Direction::kLeft:
    dx = -1;
    dy = 0;
    dStr = "left";
    break;
  case Direction::kRight:
    dx = 1;
    dy = 0;
    dStr = "right";
  }
  // for each cell inside the screen, preventing it from moving outside the
  // screen or moving to an occupied cell
  for (auto &c : _body) {
    if (!CellOutsideScreen(c[0], c[1])) {
      x = c[0] + dx;
      y = c[1] + dy;
      if (CellOutsideScreen(x, y) or CellOccupied(x, y)) {
        std::cout << GetName() << " cannot move " << dStr << ", cell at ("
                  << c[0] << ", " << c[1] << ") is blocked " << std::endl;
        return true;
      }
    }
  }
  return false;
}

// returns true if the piece's current location is already occupied by the field
// object
bool Piece::IsPlaceble() {
  for (auto &c : _body) {
    if (CellOccupied(c[0], c[1])) {
      std::cout << GetName() << " is occupied at cell (" << c[0] << ", " << c[1]
                << ")" << std::endl;
      return true;
    }
  }
  return false;
}

// move the piece by one cell in the input direction
void Piece::Move(const Direction &d) {
  if (!_free)
    return;
  std::lock_guard<std::mutex> lck(_mutex);
  if (!IsBlocked(d)) {
    int dx = d == Direction::kLeft ? -1 : 1;
    std::string str = d == Direction::kLeft ? "left" : "right";
    _centerX += dx;
    _centerCellX += dx;
    UpdateBody();
    std::cout << GetName() << " moved " << str << ", new center at ("
              << _centerCellX << ", " << _centerCellY << ")" << std::endl;
  }
}

// rotate the piece in the input direction
void Piece::Rotate(const Rotation &r) {
  if (!_free)
    return;
  int x;
  int y;
  int nextShape;
  std::string rStr;
  const std::map<int, std::vector<std::vector<int>>> &shapes = GetShapes();
  if (r == Rotation::kForward) {
    nextShape = (_currentShape + 1) % shapes.size();
    rStr = "forward";
  } else {
    nextShape = (_currentShape + shapes.size() - 1) % shapes.size();
    rStr = "borward";
  }
  std::lock_guard<std::mutex> lck(_mutex);
  // if any cell of the next shape is outside screen or already occupied, then
  // the piece cannot rotate
  for (auto &s : shapes.at(nextShape)) {
    x = _centerCellX + s[0];
    y = _centerCellY + s[1];
    // rotating outside the top of the screen is allowed
    if (CellOutsideScreen(x, std::max(y, 0)) or CellOccupied(x, y)) {
      std::cout << GetName() << " cannot rotate " << rStr << ", cell at (" << x
                << ", " << y << ") is blocked" << std::endl;
      return;
    }
  }
  // update the piece body with the new shape
  _currentShape = nextShape;
  UpdateBody();
}

// instantsly drops the piece to the bottom, by calculating how many cells it is
// allowed to move down
void Piece::Drop() {
  if (!_free)
    return;
  int moves{_gridHeight - 1};
  std::lock_guard<std::mutex> lck(_mutex);
  auto &grid = _field->GetGrid();
  for (auto &c : _body) {
    // ignores any cells outside the screen, which are in fact above the top of
    // the screen
    if (CellOutsideScreen(c[0], c[1]))
      continue;
    // compute the minium number of possible moves without going outside the
    // screen or overlap with the field
    moves = std::min(moves, _gridHeight - c[1] - 1);
    for (int row = c[1]; row < _gridHeight; row++) {
      if (grid[row][c[0]] == 1) {
        moves = std::min(moves, row - c[1] - 1);
        break;
      }
    }
  }
  _centerY += moves;
  _centerCellY += moves;
  UpdateBody();
  std::cout << GetName() << " droped " << moves << " cells, new center at ("
            << _centerCellX << ", " << _centerCellY << ")" << std::endl;
  _free = false;
}

// defines static constants of each individual piece

const std::string LongPiece::name{"LongPiece"};
const std::map<int, std::vector<std::vector<int>>> LongPiece::shapes = {
    {0, {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},
    {1, {{0, -2}, {0, -1}, {0, 0}, {0, 1}}}};
const std::vector<int> LongPiece::color_codes{255, 0, 0, 255}; // red

const std::string SquarePiece::name{"SquarePiece"};
const std::map<int, std::vector<std::vector<int>>> SquarePiece::shapes = {
    {0, {{0, 0}, {1, 0}, {0, 1}, {1, 1}}}};
const std::vector<int> SquarePiece::color_codes{255, 255, 0, 255}; // yellow

const std::string JPiece::name{"J-Piece"};
const std::map<int, std::vector<std::vector<int>>> JPiece::shapes = {
    {0, {{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},
    {1, {{0, -1}, {0, 0}, {0, 1}, {-1, 1}}},
    {2, {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}},
    {3, {{1, -1}, {0, -1}, {0, 0}, {0, 1}}}};
const std::vector<int> JPiece::color_codes{0, 0, 255, 255}; // blue

const std::string LPiece::name{"L-Piece"};
const std::map<int, std::vector<std::vector<int>>> LPiece::shapes = {
    {0, {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}},
    {1, {{-1, -1}, {0, -1}, {0, 0}, {0, 1}}},
    {2, {{-1, 0}, {0, 0}, {1, 0}, {1, -1}}},
    {3, {{0, -1}, {0, 0}, {0, 1}, {1, 1}}}};
const std::vector<int> LPiece::color_codes{255, 165, 0, 255}; // orange

const std::string SPiece::name{"S-Piece"};
const std::map<int, std::vector<std::vector<int>>> SPiece::shapes = {
    {0, {{0, 0}, {1, 0}, {-1, 1}, {0, 1}}},
    {1, {{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}};
const std::vector<int> SPiece::color_codes{255, 0, 255, 255}; // pink

const std::string TPiece::name{"T-Piece"};
const std::map<int, std::vector<std::vector<int>>> TPiece::shapes = {
    {0, {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},
    {1, {{0, -1}, {-1, 0}, {0, 0}, {0, 1}}},
    {2, {{0, 0}, {-1, 1}, {0, 1}, {1, 1}}},
    {3, {{0, -1}, {0, 0}, {1, 0}, {0, 1}}}};
const std::vector<int> TPiece::color_codes{0, 255, 255, 255}; // cyan

const std::string ZPiece::name{"Z-Piece"};
const std::map<int, std::vector<std::vector<int>>> ZPiece::shapes = {
    {0, {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}},
    {1, {{1, -1}, {0, 0}, {1, 0}, {0, 1}}}};
const std::vector<int> ZPiece::color_codes{0, 255, 0, 255}; // green

// factory methods to initialize a random piece with its body centered at the
// top of the screen
std::unique_ptr<Piece>
PieceGenerator::GeneratePiece(int gridWidth, int gridHeight, float speed,
                              std::shared_ptr<Field> field) {
  std::unique_ptr<Piece> p;
  int t = random_t(engine);
  switch (t) {
  case 0:
    p = std::make_unique<LongPiece>(gridWidth, gridHeight);
    break;
  case 1:
    p = std::make_unique<SquarePiece>(gridWidth, gridHeight);
    break;
  case 2:
    p = std::make_unique<JPiece>(gridWidth, gridHeight);
    break;
  case 3:
    p = std::make_unique<LPiece>(gridWidth, gridHeight);
    break;
  case 4:
    p = std::make_unique<SPiece>(gridWidth, gridHeight);
    break;
  case 5:
    p = std::make_unique<TPiece>(gridWidth, gridHeight);
    break;
  case 6:
    p = std::make_unique<ZPiece>(gridWidth, gridHeight);
    break;
  }
  p->SetDesecendSpeed(speed);
  p->InitBody();
  p->SetField(field);
  std::cout << "Created new " << p->GetName() << " at (" << p->GetCenterCellX()
            << ", " << p->GetCenterCellY()
            << "), speed=" << p->GetDescendSpeed() << std::endl;
  return p;
}