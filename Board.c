#include <stdio.h>
#include <stdlib.h>
#include "Board.h"
#include <stdbool.h>
#include <assert.h>

// Creates a static instance of the Board struct
static struct Board
{
	size_t height; // # of logical rows
	size_t width; // # of logical columns
	int generation;
	char* data;
	char* next_data;
} board;

// FUNCTIONS FOR "INTERNAL" USE

// DESCRIPTION:
// Used by the UpdateBoard function which needs to access the board with padding
// INPUT/OUTPUT:
// Returns the character at [@row][@column] of the board with the padding row and column
// Allows extended indexing from -1 to M, -1 to N
char GetExt(int row, int column);

// DESCRIPTION:
// Copies the content of the source file to the destination file
// INPUT/OUTPUT:
// @source is the path to the source file, @dest is the path to the destination file
// @errorString is printed at the error screen if an error occurs
// Returns 0 if the copy was successful
int CopyFile(char* source, char* dest, char* errorString);

//

void FreeBoard()
{
	free(board.data);
	free(board.next_data);
}

int NewBoardFromState(char* errorString)
{
	return NewBoard(errorString, -1, -1);
}

int NewBoard(char* errorString, int height, int width)
{
	FreeBoard(); // we only need to deallocate board data when quitting
	board.generation = 0;

	if (height == -1 && width == -1)
	{
		// Open state.txt
		FILE* state = fopen("SAVES//state.txt", "r");
		if (!state)
		{
			sprintf_s(errorString, 256, "Error opening state.txt!");
			return 1;
		}

		// Load board from the file
		if (fscanf_s(state, "HEIGHT=%llu\nWIDTH=%llu\n", &board.height, &board.width) != 2)
		{
			sprintf_s(errorString, 256, "Error reading height or width from state.txt!");
			return 1;
		}

		// Allocate memory for the board(s), and initialize it with 0-s
		board.data = (char*)calloc((Get_Height() + 2) * (Get_Width() + 2), sizeof(char));
		board.next_data = (char*)calloc((Get_Height() + 2) * (Get_Width() + 2), sizeof(char));
		if (!board.data || !board.next_data)
		{
			sprintf_s(errorString, 256, "Memory allocation failed!");
			return 1;
		}

		//// Initialize the board with 0-s
		//const size_t trueSize = (Get_Height() + 2) * (Get_Width() + 2);
		//for (size_t i = 0; i < trueSize; i++)
		//{
		//	board.next_data[i] = board.data[i] = 0;
		//}

		// Load live cells from the file
		char readChar;
		int cellCounter = 0;
		int numberOfCells = (int)(Get_Height() * Get_Width());
		// the last argument of fscanf_s is the number of characters to read
		while (fscanf_s(state, "%c", &readChar, 1) == 1 && cellCounter != numberOfCells)
		{
			if (readChar == '0' || readChar == '1')
			{
				const size_t row = cellCounter / Get_Width();
				const size_t column = cellCounter % Get_Width();
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

		// Close the file, and return 0 if everything went well
		fclose(state);
	}
	else
	{
		board.height = height;
		board.width = width;
		board.data = (char*)calloc((Get_Height() + 2) * (Get_Width() + 2), sizeof(char));
		board.next_data = (char*)calloc((Get_Height() + 2) * (Get_Width() + 2), sizeof(char));
		if (!board.data || !board.next_data)
		{
			sprintf_s(errorString, 256, "Memory allocation failed!");
			return 1;
		}
	}
	

	return 0;
}

int SetState(char* errorString)
{
	int height = Get_Height();
	int width = Get_Width();
	// Rewrite the content of state.txt with the new state in the appropriate format
	FILE* state = fopen("SAVES//state.txt", "w");
	if (!state)
	{
		sprintf_s(errorString, 256, "Error opening state.txt!");
		return 1;
	}
    fprintf(state, "HEIGHT=%d\nWIDTH=%d\n", height, width);
    // Print top border
    for (int j = 0; j < width + 2; j++)
    {
    fprintf(state, "#");
    }
    fprintf(state, "\n");
    for (int i = 0; i < height; i++)
    {
    // Print left border
    fprintf(state, "#");
    for (int j = 0; j < width; j++)
    {
    fprintf(state, "%d", Get(i, j));
    }
    // Print right border
    fprintf(state, "#\n");
    }
    // Print bottom border
    for (int j = 0; j < width + 2; j++)
    {
    fprintf(state, "#");
    }
    fprintf(state, "\n");
    fclose(state);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Board.h"

int SaveState(char* filename, char* errorString)
{
	return CopyFile("SAVES//state.txt", filename, errorString);
}

int LoadState(char* filename, char* errorString)
{
	return CopyFile(filename, "SAVES//state.txt", errorString);
}

void UpdateBoard()
{
	board.generation++;
	for (int i = 0; i < Get_Height(); i++) // row
	{
		for (int j = 0; j < Get_Width(); j++) // column
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
	assert(row >= -1 && row <= (int)Get_Height() && column >= -1 && column <= (int)Get_Width());
	return board.data[(row + 1) * (Get_Width() + 2) + column + 1];
}

int CopyFile(char* source, char* dest, char* errorString)
{
	// Open the source file
	FILE* sourceFile = fopen(source, "r");
	if (!sourceFile)
	{
		sprintf_s(errorString, 256, "Error opening %s!", source);
		return 1;
	}

	// Open the destination file
	FILE* destFile = fopen(dest, "w");
	if (!destFile)
	{
		sprintf_s(errorString, 256, "Error opening %s!", dest);
		fclose(sourceFile);
		return 1;
	}

	// Copy content from source to destination
	char buffer[256];
	size_t bytes;
	while ((bytes = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0)
	{
		fwrite(buffer, 1, bytes, destFile);
	}

	// Close the files
	fclose(sourceFile);
	fclose(destFile);

	return 0;
}

char Get(int row, int column)
{
	assert(board.data);
	assert(row >= 0 && row < (int)Get_Height() && column >= 0 && column < (int)Get_Width());
	return board.data[(row+1) * (Get_Width() + 2) + column + 1];
}

void Set(int row, int column, char value)
{
	assert(board.next_data);
	assert(row >= 0 && row < (int)Get_Height() && column >= 0 && column < (int)Get_Width());
	board.next_data[(row + 1) * (Get_Width() + 2) + column + 1] = value;
}

void Start_Set(int row, int column, char value)
{
	assert(board.data);
	assert(row >= 0 && row < (int)Get_Height() && column >= 0 && column < (int)Get_Width());
	board.data[(row + 1) * (Get_Width() + 2) + column + 1] = value;
}

int Get_Height()
{
	return board.height;
}

int Get_Width()
{
	return board.width;
}

int GetGeneration()
{
	return board.generation;
}

int GetPopulation()
{
	int population = 0;
	for (int i = 0; i < Get_Height(); i++)
	{
		for (int j = 0; j < Get_Width(); j++)
		{
			population += Get(i, j);
		}
	}
	return population;
}
