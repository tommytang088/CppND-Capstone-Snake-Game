#ifndef PIECE_H
#define PIECE_H

#include "SDL.h"
#include "field.h"
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

class Field;

enum class Rotation { kForward = 0, kBackward };
enum class Direction { kDown = 0, kLeft, kRight };

class Piece {
public:
  // constructor / destructor
  Piece(int gridWdth, int gridHeight);
  ~Piece();

  // getter and setter
  int GetSize() { return size; };
  int GetCenterCellX() { return _centerCellX; };
  int GetCenterCellY() { return _centerCellY; };
  float GetDescendSpeed() { return _descendSpeed; };
  void SetDesecendSpeed(float s) { _descendSpeed = s; };
  const std::vector<std::vector<int>> &GetBody() const { return _body; };

  // abstract methods

  virtual const std::map<int, std::vector<std::vector<int>>> &
  GetShapes() = 0; // returns a map defining all shapes of the piece

  virtual const std::string GetName() = 0; // returns the name of the piece

  virtual const std::vector<int> &
  GetColorCodes() const = 0; // returns the RGBA code of the piece

  // typical behavior methods
  bool IsFree() { return _free; };
  void SetField(std::shared_ptr<Field> field);
  void Simulate(std::promise<void> &&prms);
  void InitBody();
  void Descend(std::promise<void> &&prms);
  void Move(const Direction &d);
  void Rotate(const Rotation &r);
  void Drop();
  bool CellOutsideScreen(const int &x, const int &y) const;
  bool CellOccupied(const int &x, const int &y) const;
  bool IsPlaceble();

  static const int size{4}; // number of cells a piece would occupy

private:
  // private behavior methods
  void UpdateBody();
  bool IsBlocked(const Direction &d);
  void AddToField();

  int _gridWidth;    // number of columns in the screen
  int _gridHeight;   // number of rows in the screen
  bool _free{false}; // true if the piece is allowed to move, rotate or descend
  float _centerX;   // floating value of the x-coordindate of the piece's center
  float _centerY;   // floating value of the y-coordiate of the piece's center
  int _centerCellX; // displayed x-coordidate of the piece's center
  int _centerCellY; // displayed y-cooridate of the piece's center
  int _currentShape{0};       // current shape of the piece
  float _descendSpeed{0.1f};  // initial descend speed, number of cells
  int _descendCycleTime{100}; // descend every 100 milliseconds
  std::vector<std::vector<int>>
      _body; // vecotr of cells representing the current shape of the piece
  std::shared_ptr<Field> _field; // pointer to the field object
  std::thread _thread;           // thread to simulate the piece's descent
  std::mutex _mutex; // mutex to protect modifying private variables from
                     // different threads
};

// define each invididual piece class by filling out the missing virtual methods

class LongPiece : public Piece {
public:
  LongPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class SquarePiece : public Piece {
public:
  SquarePiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  void Rotate(const Rotation &r){}; // square piece does not rotate
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class JPiece : public Piece {
public:
  JPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class LPiece : public Piece {
public:
  LPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class SPiece : public Piece {
public:
  SPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class TPiece : public Piece {
public:
  TPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

class ZPiece : public Piece {
public:
  ZPiece(int gridWidth, int gridHeight) : Piece(gridWidth, gridHeight){};
  const std::string GetName() { return name; };
  const std::map<int, std::vector<std::vector<int>>> &GetShapes() {
    return shapes;
  };
  const std::vector<int> &GetColorCodes() const { return color_codes; };

  static const std::string name;
  static const std::map<int, std::vector<std::vector<int>>> shapes;
  static const std::vector<int> color_codes;
};

// factory class able to generate random piece objects

class PieceGenerator {
public:
  PieceGenerator() : engine(std::random_device{}()), random_t(0, 6){};
  std::unique_ptr<Piece> GeneratePiece(int gridWidth, int gridHeight,
                                       float speed,
                                       std::shared_ptr<Field> field);

private:
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_t;
};

#endif