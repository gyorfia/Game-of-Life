#pragma once

#define DEAD ' '
#define ALIVE '@'

// Deallocates data, and next_data
void FreeBoard();

// DESCRIPTION:
// Creates, initializes data and next_data (with dead cells / 0-s) to store the new board
// Automatically deallocates the previous memory (if needed)
// Draws the borders of the board, with a given offset, to leave space for instructions
// INPUT/OUTPUT:
// @M is the height, @N is the width of the board, 
// and returns 0 if board initialization was successful
int NewBoard(char* errorString);

// DESCRIPTION:
// Copies state.txt to a new or existing file named @filename
// INPUT/OUTPUT:
// @filename must be a c type string
// Returns 0 if board initialization was successful
int SaveState(char* filename, char* errorString);

// DESCRIPTION:
// Copies an existing file named @filename to state.txt
// INPUT/OUTPUT:
// @filename must be a c type string
int LoadState(char* filename, char* errorString);

// Advance the simulation
void UpdateBoard();

// INPUT/OUTPUT:
// Returns the character at [@row][@column] of the board without the padding row and column
char Get(int row, int column);

// INPUT/OUTPUT:
// Sets the character at [@row][@column] of the next board without the padding row and column
void Set(int row, int column, char value);

void Start_Set(int row, int column, char value);

// INPUT/OUTPUT:
// Returns the height of the board
size_t Get_M();

// INPUT/OUTPUT:
// Returns the width of the board
size_t Get_N();

// INPUT/OUTPUT:
// Returns the generation number
int GetGeneration();

// INPUT/OUTPUT:
// Returns the population
int GetPopulation();