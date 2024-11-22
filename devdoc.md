## Developer Documentation
### Overview
This project is a simulation of Conway's Game of Life. It includes a menu system for interacting with the simulation, functions for managing the game board, and functions for saving and loading the game state.
Modules
1. main.c
This module contains the main function, the menu system, and the input handling thread.
2. Board.c
This module contains functions for managing the game board, including creating, updating, and freeing the board, as well as saving and loading the game state.
3. state.txt
This file contains the initial state of the game board, including the height, width, and the initial configuration of cells.
### Data Structures
enum MenuState
Represents the different states of the menu:
•	MENU
•	SETSTATE
•	LIFE
•	LIFESTEP
•	LOAD
•	SAVE
•	QUIT
•	ERRORMESSAGE
enum KeyState
Represents the different key states:
•	KEY_EMPTY
•	KEY_UP
•	KEY_DOWN
•	KEY_LEFT
•	KEY_RIGHT
•	KEY_ENTER
•	KEY_ESCAPE
struct InputThreadParams
Used to pass pointers to variables to the input thread:
•	enum MenuState* state
•	enum KeyState* key
struct Board
Represents the game board:
•	size_t height
•	size_t width
•	int generation
•	char* data
•	char* next_data
### Algorithms
Menu Navigation
The menu system allows the user to navigate through different options using the 'w' and 's' keys. The selected option is highlighted, and the user can confirm their selection with the 'Enter' key.
Board Management
The board management functions handle creating, updating, and freeing the game board. The board is represented as a 2D array of characters, where each character represents a cell that is either alive or dead.
Saving and Loading State
The game state can be saved to and loaded from a file. The state includes the height, width, and configuration of cells on the board.
### Functions
main.c
DWORD WINAPI InputThread(LPVOID lpParam)
Handles input from the user.
•	Input: LPVOID lpParam - Pointer to InputThreadParams.
•	Output: DWORD - Exit code.
int listTextFiles(char* fileNames[MAX_SAVE_FILES])
Stores all text file names found in the SAVES directory in an array.
•	Input/Output: char* fileNames[MAX_SAVE_FILES] - Array of strings to store the file names.
•	Returns: int - Number of files found.
void DrawBorders()
Draws the borders of the board.
void DrawBoard()
Draws the cells to the screen.
Board.c
void FreeBoard()
Deallocates data and next_data.
int NewBoardFromState(char* errorString)
Calls NewBoard with height and width set to -1 to load the state from state.txt.
•	Input/Output: char* errorString - Error message if an error occurs.
•	Returns: int - 0 if successful.
int NewBoard(char* errorString, int height, int width)
Creates and initializes data and next_data to store the new board, then loads cells from state.txt if height and width are -1.
•	Input/Output: char* errorString - Error message if an error occurs.
•	Input: int height, int width - Dimensions of the board.
•	Returns: int - 0 if successful.
int SetState(char* errorString)
Writes a new state to state.txt.
•	Input/Output: char* errorString - Error message if an error occurs.
•	Returns: int - 0 if successful.
int SaveState(char* filename, char* errorString)
Copies state.txt to a new or existing file named filename.
•	Input: char* filename - Name of the file.
•	Input/Output: char* errorString - Error message if an error occurs.
•	Returns: int - 0 if successful.
int LoadState(char* filename, char* errorString)
Copies an existing file named filename to state.txt.
•	Input: char* filename - Name of the file.
•	Input/Output: char* errorString - Error message if an error occurs.
•	Returns: int - 0 if successful.
void UpdateBoard()
Advances the simulation.
char Get(int row, int column)
Returns the character at [row][column] of the board data without the padding row and column.
•	Input: int row, int column - Coordinates of the cell.
•	Returns: char - Character at the specified coordinates.
void Set(int row, int column, char value)
Sets the character at [row][column] of the board next_data without the padding row and column.
•	Input: int row, int column - Coordinates of the cell.
•	Input: char value - Value to set.
void Start_Set(int row, int column, char value)
Used for setting the initial state. Sets the character at [row][column] of the board data without the padding row and column.
•	Input: int row, int column - Coordinates of the cell.
•	Input: char value - Value to set.
int Get_Height()
Returns the height of the board.
•	Returns: int - Height of the board.
int Get_Width()
Returns the width of the board.
•	Returns: int - Width of the board.
int GetGeneration()
Returns the generation number.
•	Returns: int - Generation number.
int GetPopulation()
Returns the population.
•	Returns: int - Population.