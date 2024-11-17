#pragma once

#define DEAD ' '
#define ALIVE '@'
#define MAX_WIDTH 80
#define MAX_HEIGHT 50

// DESCRIPTION:
// Deallocates data, and next_data
void FreeBoard();

// DESCRIPTION:
// Calls NewBoard with height=-1 and width=-1 to load the state from state.txt
// INPUT/OUTPUT:
// @errorstring is printed at the error screen if an error occurs
int NewBoardFromState(char* errorString);

// DESCRIPTION:
// Creates, initializes data and next_data (with dead cells / 0-s) to store the new board
// then loads cells from state.txt if height=-1 and width=-1
// Automatically deallocates the previous memory (if needed)
// INPUT/OUTPUT:
// @errorstring is printed at the error screen if an error occurs
// and returns 0 if board initialization was successful
int NewBoard(char* errorString, int height, int width);

// DESCRIPTION:
// Writes a new state to state.txt
// INPUT/OUTPUT:
// @width is the width of the board, @height is the height of the board, @cellData is the board data (without padding)
// Returns 0 if board initialization was successful
int SetState(char* errorString);

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
// Sets the character at [@row][@column] of the board next_data without the padding row and column
void Set(int row, int column, char value);

// INPUT/OUTPUT:
// Returns the height of the board
int Get_Height();

// INPUT/OUTPUT:
// Returns the width of the board
int Get_Width();

// INPUT/OUTPUT:
// Returns the generation number
int GetGeneration();

// INPUT/OUTPUT:
// Returns the population
int GetPopulation();