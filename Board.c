#include <stdio.h>
#include <stdlib.h>
#include "Board.h"
#include <assert.h>

// Creates a static instance of the Board struct
static struct Board
{
	size_t M; // # of logical rows
	size_t N; // # of logical columns
	int generation;
	char* data;
	char* next_data;
} board;

// Functions for "internal" use

// DESCRIPTION:
// Used by the UpdateBoard function which needs to access the board with padding
// INPUT/OUTPUT:
// Returns the character at [@row][@column] of the board with the padding row and column
// Allows extended indexing from -1 to M, -1 to N
char GetExt(int row, int column);

//

void FreeBoard()
{
	free(board.data);
	free(board.next_data);
}

int NewBoard(char* errorString)
{
	FreeBoard(); // we only need to deallocate board data when quitting
	board.generation = 0;

	// Open state.txt
	FILE* state = fopen("SAVES//state.txt", "r");
	if (!state)
	{
		sprintf_s(errorString, 256, "Error opening state.txt!");
		return 1;
	}

	// Load board from the file
	if (fscanf_s(state, "HEIGHT=%llu\nWIDTH=%llu\n", &board.M, &board.N) != 2)
	{
		sprintf_s(errorString, 256, "Error reading height or width from state.txt!");
		return 1;
	}

	// Allocate memory for the board(s)
	board.data = (char*)malloc((Get_M()+2) * (Get_N()+2) * sizeof(char));
	if (!board.data)
	{
		sprintf_s(errorString, 256, "Memory allocation failed!");
		return 1;
	}
	board.next_data = (char*)malloc((Get_M()+2) * (Get_N()+2) * sizeof(char));
	if (!board.data)
	{
		sprintf_s(errorString, 256, "Memory allocation failed!");
		return 1;
	}

	// Initialize the board with 0-s
	const size_t trueSize = (Get_M() + 2) * (Get_N() + 2);
	for (size_t i = 0; i < trueSize; i++)
	{
		board.next_data[i] = board.data[i] = 0;
	}

	// Load live cells from the file
	char readChar;
	int cellCounter = 0;
	int numberOfCells = (int)(Get_M() * Get_N());
	// the last argument of fscanf_s is the number of characters to read
	while (fscanf_s(state, "%c", &readChar, 1) == 1 && cellCounter != numberOfCells)
	{
		if (readChar == '0' || readChar == '1')
		{
			const size_t row = cellCounter / Get_N();
			const size_t column = cellCounter % Get_N();
			Start_Set(row, column, readChar - '0');
			cellCounter++;
		}
	}
	// Check whether we have read enough cells
	if (cellCounter != numberOfCells)
	{
		sprintf_s(errorString, 256, "Only read %d cells out of the %d expected!", cellCounter, numberOfCells);
		return 1;
	}

	// Draw borders (only needed when we initialize a new board)
	for (size_t j = 0; j < Get_N() + 2; j++)
		printf("#");
	printf("\n");
	for (size_t i = 0; i < Get_M(); i++)
	{
		printf("#");
		for (size_t j = 0; j < Get_N(); j++)
			printf(" ");
		printf("#\n");
	}
	for (size_t j = 0; j < Get_N() + 2; j++)
		printf("#");

	// Close the file, and return 0 if everything went well
	fclose(state);
	return 0;
}

int SaveState(char* filename, char* errorString)
{
	return 0;
}


int LoadState(char* filename, char* errorString)
{
	return 0;
}

void UpdateBoard()
{
	board.generation++;
	for (int i = 0; i < Get_M(); i++) // row
	{
		for (int j = 0; j < Get_N(); j++) // column
		{
			char currentCell = Get(i, j);
			int neighbourSum = -currentCell; // this way the current element cancels out, since we add it in the 3x3 loop
			for (int k = i-1; k <= i + 1; k++) // row of current 3x3 adjacent grid
			{
				for (int l = j-1; l <= j + 1; l++) // column of current 3x3 adjacent grid
				{
					neighbourSum += GetExt(k, l);
				}
			}
			if (currentCell == 0) // if cell is dead
			{
				if (neighbourSum == 3)
				{
					Set(i, j, 1); // becomes live by reproduction
				}
				else
				{
					Set(i, j, 0); // remains dead
				}
			}
			else
			{
				if (neighbourSum > 3 || neighbourSum < 2)
				{
					Set(i, j, 0); // dies by under-, overpopulation
				}
				else
				{
					Set(i, j, 1); // remains live
				}
			}
		}
	}
	// swap the pointers, so data points to what next_data pointed to
	char* temp = board.data;
	board.data = board.next_data;
	board.next_data = temp;
}

char GetExt(int row, int column)
{
	assert(board.data);
	assert(row >= -1 && row <= (int)Get_M() && column >= -1 && column <= (int)Get_N());
	return board.data[(row + 1) * (board.N + 2) + column + 1];
}

char Get(int row, int column)
{
	assert(board.data);
	assert(row >= 0 && row < (int)Get_M() && column >= 0 && column < (int)Get_N());
	return board.data[(row+1) * (board.N+2) + column+1];
}

void Set(int row, int column, char value)
{
	assert(board.next_data);
	assert(row >= 0 && row < (int)Get_M() && column >= 0 && column < (int)Get_N());
	board.next_data[(row+1) * (board.N+2) + column+1] = value;
}

// used for setting the initial state
void Start_Set(int row, int column, char value)
{
	assert(board.data);
	assert(row >= 0 && row < (int)Get_M() && column >= 0 && column < (int)Get_N());
	board.data[(row + 1) * (board.N + 2) + column + 1] = value;
}

size_t Get_M()
{
	return board.M;
}

size_t Get_N()
{
	return board.N;
}

int GetGeneration()
{
	return board.generation;
}

int GetPopulation()
{
	int population = 0;
	for (int i = 0; i < board.M; i++)
	{
		for (int j = 0; j < board.N; j++)
		{
			population += Get(i, j);
		}
	}
	return population;
}
