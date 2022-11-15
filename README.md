# CPPND: Capstone Project

The project implements a simple Tetris game using the framework from the Snake Game example. The game starts with a random piece (out of 7) displayed at the top of the screen. The piece automatically descends at an increasing speed based on stage/level of the game. The player can move and rotate the piece before it lands on the bottom field. Wherever a full line is filled, the field clears that row and the player gets points. Every 10 points increases the speed of the next piece, capping at level 5. The game then regenerates the random piece agian, and game continues until the player quits the game or a new piece cannot be placed.

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./Tetris`.

## Controls:
* Arrow Key UP: rotate piece clockwise
* Arrow Key Down: drop the piece
* Arrow Key Left: move the piece to the left
* Arrow Key RightL: move the piece to the right

## Scores:
* Clears 1 row: 1 point
* Clears 2 rows: 4 points
* Cleers 3 rows: 9 points
* Clears 4 rows: 16 points

## Levels:
* Level 1: 1 - 9 points
* Level 2: 10 - 19 points, 2x speed
* Level 3: 20 - 29 points, 3x speed
* Level 4: 30 - 39 points, 4x speed
* Level 5: 40 - 49 points, 5x speed


## File and Class Structure
1. main.cpp
Main program of the project, similar to the original Snake Game. Initiates a renderer, a controller, and a game object, than runs the Tetris game.

2. game.h / game.cpp
Implements the game class, which includes a unique pointer to a Piece object and a shared pointer to a Field object. The `run` method runs in the main thread, which generates new pieces, handles user inputs, and renders objects on screen. Each piece is simulated in a child thread. The `run` methods returns when a new piece cannot be placed at its starting position or the player exits the game.

3. controller.h / controller.cpp
Implements the controller, similar to the orignal Snake Game. Allows users to control the current piece with arrow keys.

4. renderer.h / renderr.cpp
Implements the renderer, similar to the origial Snake Game. Renders the body of the current piece and the bottom field as grids. Displays current score, level, and FPS in the title.

5. piece.h / piece.cpp
Includes a abstract parent Piece class, individual child specif Piece clasess (7 of them), and a factory class PieceGenerator that can generate a randome piece.

The Piece class implments most behaviors of a generic piece, except leaving the unqiue features of a specific piece undefined: shapes, name, and RGBA code codes.

A piece includes 2D vector `_body` that contains the x-y positions of indiviual cells. `_body` is computed by adding each coordiate defined in the map, `_shapes`, to the center cell of the piece. Everytime the piece changes its location, the center changes, and `_body` is updated.

Each piece is ran by a new thread in the `Simulate` method to simulate the piece's automatic descent.

6. field.h / field.cpp
Implemtns the field class, which is the bottom grids in the Tetris game. A Field includes a 2D vector `_grid` matching the number of rows and columns defined by the game. Each element is either 0 or 1, with 1 repreenting the cell is occupied. Every time `AddPiece` method is called, the cells included in the piece's `_body` is added to the field. This is done by updating the corresponding element in `_grid` to 1. The field then tries to clear any complete rows.

## Rubric items
### Loops, Functions, I/O
* The project demonstrates an understanding of C++ functions and control structures.
For example, piece.cpp defines individual functions of the piece class, and uses `for`, `while`, and `swith`.

* The project accepts user input and processes the input.
controller.cpp line 7: takes arrow key or mouse click on the exit button as input

### Object Oriented Programming
* The project uses Object Oriented Programming techniques.
* Classes use appropriate access specifiers for class members
* Class constructors utilize member initialization lists.
* Classes abstract implementation details from their interfaces
* Classes encapsulate behavior.
* Classes follow an appropriate inheritance hierarchy.
* Derived class functions override virtual base class functions.

These should all have been met in Piece.h and Piece.cpp.

## Memory Management
* The project makes use of references in function declarations.

For example, piece.cpp line 173-220, the `Move` and `Rotate` methods.

* The project uses scope / Resource Acquisition Is Initialization (RAII) where appropriate.
game.h line 31, `Piece` desturctor will be called when the unqiue pointer `_piece` is out of scope (when the game ends).

piece.cpp line 69, the lock's destrutor will be called at each iteration in the while loop.

* The project uses move semantics to move data, instead of copying it, where possible.

game.cpp line 86, piece.cpp line 48-57, prmoise is moved from Game to Piece.

* The project uses smart pointers instead of raw pointers.
game.h line 31-32, uses unqiue pointer `_piece` and shared pointer `_field`. No raw pointers are used.

## Concurrency
* The project uses multithreading.

piece.cpp line 48 - 53, each piece runs its `Descend` method in a new thread.

* A promise and future is used in the project.

game.cpp line 83 - 87, promise and future are used for each child piece thread to signal its termination to the main thread.

* A mutex or lock is used in the project.

game.cpp line 54 - 56, piece.cpp 69 - 89, mutex and lock are used when the renderer in the main thread is rendering the piece's `_body` or child piece thread is modifying `_body` when it moves.

piece.cpp line 173 - 251, When the controler in the main thread is calling `Move`, `Rotate`, and `Drop`, mutex and locks are also used.
