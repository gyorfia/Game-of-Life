#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include "econio.h"
#include "Board.h"

enum MenuState 
{
	MENU = '0', // start at '0' so we can use _getch() to get the state
	SETSTATE,
	LIFE,
	LIFESTEP,
	LOAD,
	SAVE,
	QUIT,
	ERRORMESSAGE
};

// Enum to represent the key states, read from _getch()
enum KeyState
{
	KEY_EMPTY = 0, // empty state of the key defined as 0
	KEY_UP = 'w',
	KEY_DOWN = 's',
	KEY_LEFT = 'a',
	KEY_RIGHT = 'd',
	KEY_SPACE = 32,
	KEY_ESCAPE = 27
};

// Used to pass pointers to variables to the input thread
struct InputThreadParams {
	enum MenuState* state;
	enum KeyState* key;
};

// Input processing function
DWORD WINAPI InputThread(LPVOID lpParam);

void listTextFiles(const char* folderPath) {
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char searchPath[MAX_PATH];

	// Create a search path (e.g., "C:\\Folder\\*.txt")
	snprintf(searchPath, MAX_PATH, "%s\\*.txt", folderPath);

	// Find the first file in the directory
	hFind = FindFirstFileA(searchPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("No text files found in the folder: %s\n", folderPath);
		return;
	}

	// Iterate through the files
	do {
		printf("Found: %s\n", findData.cFileName);
	} while (FindNextFile(hFind, &findData) != 0);

	FindClose(hFind); // Close the search handle
}


// Function to draw the borders of the board
void DrawBorders();

// Draw the cells to the screen
void DrawBoard();

int main()
{
	SaveState("SAVES//TEST.txt", "Error message!");
	// Initialize the board
	enum MenuState state = MENU;
	enum KeyState key = KEY_EMPTY;
	char errorString[256] = "Error message!";

	// Create a thread to handle the input
	HANDLE hThread;
	DWORD threadId;
	DWORD exitCode;
    struct InputThreadParams threadParams = { &state, &key };
	hThread = CreateThread(NULL, 0, InputThread, &threadParams, 0, &threadId);
	if (hThread == NULL)
	{
		printf("Failed to create input handler thread.\n");
		return 1;
	}

	// Hide the cursor
	setcursortype(NOCURSOR);

	while (true)
	{
		switch (state)
		{
		case MENU:
			// Display the main menu
			printf("Select menu option (1-6):\n");
            printf("1. SET STATE\n");
            printf("2. LIFE\n");
            printf("3. LIFE STEP\n");
            printf("4. LOAD\n");
            printf("5. SAVE\n");
            printf("6. QUIT\n");
			while (state < '1' || state > '6')
			{
				if (key != KEY_EMPTY)
				{
					state = key; // the keypress from (0-6) corresponds to the state
					key = KEY_EMPTY; // reset the key
				}
			}
			break;

		case SETSTATE:
		{
			int boardWidth = 0, boardHeight = 0;
			char instruction[256];
			setcursortype(SOLIDCURSOR);
			sprintf_s(instruction, 256, "Enter board width=[5:%d] and height=[5:%d]: ", MAX_WIDTH, MAX_HEIGHT);

			// Ask the user to enter board width and height
			while (boardWidth < 5 || boardWidth > MAX_WIDTH || boardHeight < 5 || boardHeight > MAX_HEIGHT) {
				printf("%s", instruction);
				scanf_s("%d %d", &boardWidth, &boardHeight);
				sprintf_s(instruction, 256, "Invalid width or height!\nEnter board width=[5:%d] and height=[5:%d]: ", MAX_WIDTH, MAX_HEIGHT);
				clrscr();
			}
			if (NewBoard(errorString, boardHeight, boardWidth))
			{
				state = ERRORMESSAGE;
				break;
			}
			setcursortype(NOCURSOR);
			DrawBorders();
			DrawBoard();
			const int N = Get_Width();
			gotoxy(N + 3, 1);
			printf("Controls:\n");
			gotoxy(N + 3, 2);
			printf("\tESC   - Save state and return to Menu\n");
			gotoxy(N + 3, 3);
			printf("\tSPACE - Toggle cell\n");
			gotoxy(N + 3, 4);
			printf("\tWASD  - Move selection\n");
			int x = 0, y = 0; // y corresponds to the row, x corresponds to the column
			gotoxy(1, 1); // jump to the top left cell initially
			_putch('X');
			while (key != KEY_ESCAPE)
			{
				if (key != KEY_EMPTY)
				{
					if (key == KEY_UP && y > 0){y--;}
					else if (key == KEY_DOWN && y < Get_Height() - 1){y++;}
					else if (key == KEY_LEFT && x > 0){x--;}
					else if (key == KEY_RIGHT && x < Get_Width() - 1){x++;}
					else if (key == KEY_SPACE)
					{
						Start_Set(y, x, Get(y, x) == 0 ? 1 : 0);
					}
					if (key != KEY_ESCAPE)
					{
						DrawBoard();
						gotoxy(x + 1, y + 1); // jump to the cell, after processing an input
						_putch('X');
						key = KEY_EMPTY; // reset the key
					}
				}
			}
			key = KEY_EMPTY;
			state = MENU;
			// Save the state to state.txt
			if (SetState(errorString))
			{
				state = ERRORMESSAGE;
				break;
			}
			break;
		}

		case LIFE:
		{
			if (NewBoardFromState(errorString))
			{
				state = ERRORMESSAGE;
				break;
			}
			const int M = Get_Height();
			const int N = Get_Width();
			DrawBorders();
			DrawBoard();
			// print controls and properties of the board
			gotoxy(N + 3, 1);
			printf("Controls:\n");
			gotoxy(N + 3, 2);
			printf("\tESC   - Return to Menu\n");
			gotoxy(N + 3, 3);
			printf("Generation: %d\n", GetGeneration());
			gotoxy(N + 3, 4);
			printf("Population: %d\n", GetPopulation());
			while (key != KEY_ESCAPE)
			{
				if (key != KEY_EMPTY)
				{
					if (key != KEY_ESCAPE)
					{
						key = KEY_EMPTY; // reset the key
					}
					else
					{
						break;
					}
				}
				UpdateBoard();
				DrawBoard();
				gotoxy(N + 3, 3);
				printf("Generation: %d\n", GetGeneration());
				gotoxy(N + 3, 4);
				// clear the population printed, because it can be smaller than the previous one
				printf("Population:      ");
				gotoxy(N + 3, 4);
				printf("Population: %d\n", GetPopulation());
				Sleep(100);
			}
			key = KEY_EMPTY;
			state = MENU;
			break;
		}

		case LIFESTEP:
		{
			if (NewBoardFromState(errorString))
			{
				state = ERRORMESSAGE;
				break;
			}
			const int M = Get_Height();
			const int N = Get_Width();
			DrawBorders();
			DrawBoard();
			// print controls and properties of the board
			gotoxy(N + 3, 1);
			printf("Controls:\n");
			gotoxy(N + 3, 2);
			printf("\tESC   - Return to Menu\n");
			gotoxy(N + 3, 3);
			printf("\tSPACE - Next generation\n");
			gotoxy(N + 3, 4);
			printf("Generation: %d\n", GetGeneration());
			gotoxy(N + 3, 5);
			printf("Population: %d\n", GetPopulation());
			while (key != KEY_ESCAPE)
			{
				while (TRUE)
				{
					if (key != KEY_EMPTY)
					{
						if (key != KEY_SPACE && key != KEY_ESCAPE)
						{
							key = KEY_EMPTY; // reset the key
						}
						else
						{
							break;
						}
					}
				}
				if (key == KEY_SPACE)
				{
					UpdateBoard();
					DrawBoard();
					gotoxy(N + 3, 4);
					printf("Generation: %d\n", GetGeneration());
					gotoxy(N + 3, 5);
					// clear the population printed, because it can be smaller than the previous one
					printf("Population:      ");
					gotoxy(N + 3, 5);
					printf("Population: %d\n", GetPopulation());
					key = KEY_EMPTY;
				}
			}
			key = KEY_EMPTY;
			state = MENU;
			break;
		}

		case LOAD:
			listTextFiles("SAVES");
			printf("Press ESC to return to Menu\n");
			while (key != KEY_ESCAPE)
			{
				if (key != KEY_EMPTY && key != KEY_ESCAPE)
				{
					key = KEY_EMPTY; // reset the key
				}
			}
			key = KEY_EMPTY;
			state = MENU;
			break;

		case SAVE:
			printf("Saving data...\n");
			printf("Press ESC to return to Menu\n");
			while (key != KEY_ESCAPE)
			{
				if (key != KEY_EMPTY && key != KEY_ESCAPE)
				{
					key = KEY_EMPTY; // reset the key
				}
			}
			key = KEY_EMPTY;
			state = MENU;
			break;

		case QUIT:
			FreeBoard();
			clrscr();

			// Wait 1s for the thread to complete
			WaitForSingleObject(hThread, 1000);

			// Get the exit code of the thread
			if (GetExitCodeThread(hThread, &exitCode))
			{
				if (exitCode == 0)
				{
					// Close the thread handle
					CloseHandle(hThread);
					printf("Program exited successfully!\n");
				}
				else // if the thread didn't exit successfully
				{
					printf("Program exited, InputThread exited with code %d\n", exitCode);
				}
			}
			else
			{
				printf("Program exited, failed to get InputThread exit code.\n");
			}
			return 0;
			break;

		case ERRORMESSAGE:
			printf("An error occurred: %s\n", errorString);
			printf("Press ESC to return to Menu\n");
			while (key != KEY_ESCAPE)
			{
				if (key != KEY_EMPTY)
				{
					if (key != KEY_ESCAPE)
					{
						key = KEY_EMPTY; // reset the key
					}
					else
					{
						break;
					}
				}
			}
			key = KEY_EMPTY;
			state = MENU;
			break;

		default:
			printf("Unknown state!\n");
			return 1;
			break;
		}
		clrscr(); // clear the screen after changing states
	}
}
DWORD WINAPI InputThread(LPVOID lpParam)
{
	struct InputThreadParams* params = (struct InputThreadParams*)lpParam;
	enum MenuState* pState = params->state;
	enum KeyState* pKey = params->key;

	while (*pState != QUIT)
	{
		if (_kbhit() && *pKey == KEY_EMPTY)
		{
			*pKey = _getch();
		}
	}
	return 0;
}

void DrawBorders()
{
	const int height = Get_Height();
	const int width = Get_Width();
	// Draw borders (only needed when we initialize a new board)
	for (int j = 0; j < width + 2; j++)
		printf("#");
	printf("\n");
	for (int i = 0; i < height; i++)
	{
		printf("#");
		for (int j = 0; j < width; j++)
			printf(" ");
		printf("#\n");
	}
	for (int j = 0; j < width + 2; j++)
		printf("#");
}

void DrawBoard()
{
	for (int i = 0; i < Get_Height(); i++)
	{
		gotoxy(1, i + 1);
		for (int j = 0; j < Get_Width(); j++)
			printf("%c", (Get(i, j) == 0 ? DEAD : ALIVE));
	}
}