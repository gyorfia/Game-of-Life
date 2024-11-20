#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <string.h>
#include "econio.h"
#include "Board.h"

#define MAX_SAVE_FILES 256

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

// DESCRIPTION:
// Stores all text file names found in SAVES in an array
// INPUT/OUTPUT:
// @fileNames is an array of strings to store the file names, caller must deallocate the memory allocated for the strings
// Returns the number of files found, which is the number of strings allocated in the @fileNames array
int listTextFiles(char* fileNames[MAX_SAVE_FILES]) {
	int nFiles = 0;
	const char* folderPath = "SAVES"; // Folder name to search for text files
	WIN32_FIND_DATAA findData;
	HANDLE hFind;
	char searchPath[MAX_PATH];

	// Create a search path
	snprintf(searchPath, MAX_PATH, "%s\\*.txt", folderPath);

	// Find the first file in the directory
	hFind = FindFirstFileA(searchPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("No save files found in the folder: %s\n", folderPath);
		return 0;
	}

	// Iterate through the files
	do {
		// "SAVES//" + filename + 1 for the null terminator
		int strSize = 7 + (strlen(findData.cFileName)) + 1;
		fileNames[nFiles] = (char*)malloc(strSize * sizeof(char));
        if (fileNames[nFiles] != NULL) {
			sprintf_s(fileNames[nFiles], strSize, "SAVES//%s", findData.cFileName);
        } else {
			return -1;
        }
		nFiles++;
	} while (FindNextFileA(hFind, &findData) != 0 && nFiles < MAX_SAVE_FILES);

	FindClose(hFind); // Close the search handle
	return nFiles;
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
		clrscr(); // start the state with a cleared screen
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
		{
			char* fileNames[MAX_SAVE_FILES];
			int nFiles = listTextFiles(fileNames);
			int y = 0; // current selection
			int selected = 0; // 0 if not selected, 1 if selected, -1 if an eror occurred
			key = 1; // set the key to a non-empty value, so the menu is displayed initially
			while (key != KEY_ESCAPE && !selected)
			{
				if (key != KEY_EMPTY)
				{
					if (key == KEY_UP && y > 0) { y--; }
					else if (key == KEY_DOWN && y < nFiles - 1) { y++; }
					if (key != KEY_ESCAPE)
					{
						clrscr();
						printf("Select a save file (SPACE), or return to the menu (ESC):\n");
						for (int i = 0; i < nFiles; i++)
						{
							if (i == y) // when the selection is on the current file
							{
								printf("-> %s\n", fileNames[i] + 7); // Avoid printing "SAVES//"
								if (key == KEY_SPACE)
								{
									if (LoadState(fileNames[i], errorString))
									{
										selected = -1;
										break;
									}
									clrscr();
									printf("%s loaded successfully!", fileNames[i]+7); // Avoid printing "SAVES//"
									Sleep(1500);
									selected = 1;
									break;
								}
							}
							else
							{
								printf("   %s\n", fileNames[i]+7); // Avoid printing "SAVES//"
							}
						}
						key = KEY_EMPTY; // reset the key
					}
				}
			}
			key = KEY_EMPTY;
			state = selected == -1 ? ERRORMESSAGE : MENU;
				break;
		}

		case SAVE:
		{
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
		}

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