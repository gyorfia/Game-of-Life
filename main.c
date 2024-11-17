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
enum KeyState {}; // used to store the key pressed by the user, using the windows virtual key codes
#define VK_EMPTY 0x0 // the empty state of the key, using the Windows naming convention

// Used to pass pointers to variables to the input thread
struct InputThreadParams {
	enum MenuState* state;
	enum KeyState* key;
};

// Input processing function
DWORD WINAPI InputThread(LPVOID lpParam);

// Function to draw the borders of the board
void DrawBorders();

// Draw the cells to the screen
void DrawBoard();

int main()
{
	// Initialize the board
	enum MenuState state = MENU;
	enum KeyState key = VK_EMPTY;
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
				if (key != VK_EMPTY)
				{
					state = key; // the keypress from (0-6) corresponds to the state
					key = VK_EMPTY; // reset the key
				}
			}
			break;

		case SETSTATE:
		{
			int boardWidth = 0, boardHeight = 0;
			char instruction[256];
			setcursortype(NORMALCURSOR);
			sprintf_s(instruction, 256, "Enter board width=[5:%d] and height=[5:%d]: ", MAX_WIDTH, MAX_HEIGHT);

			// Ask the user to enter board width and height
			while (boardWidth < 5 || boardWidth >= 80 || boardHeight < 5 || boardHeight >= 80) {
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
			DrawBorders();
			DrawBoard();
			int x = 0, y = 0;
			while (key != VK_ESCAPE)
			{
				if (key != VK_EMPTY)
				{
					if (key == VK_UP && y > 0){y--;}
					else if (key == VK_DOWN && y < Get_Height() - 1){y++;}
					else if (key == VK_LEFT && x > 0){x--;}
					else if (key == VK_RIGHT && x < Get_Width() - 1){x++;}
					else if (key == VK_SPACE)
					{
						Set(y, x, Get(y, x) == 0 ? 1 : 0);
					}
					if (key != VK_ESCAPE)
					{
						gotoxy(x + 1, y + 1); // jump to the cell, after processing an input
						DrawBoard();
						key = VK_EMPTY; // reset the key
					}
				}
			}
			key = VK_EMPTY;
			state = MENU;
			setcursortype(NOCURSOR);
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
			const size_t M = Get_Height();
			const size_t N = Get_Width();
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
			while (key != VK_ESCAPE)
			{
				if (key != VK_EMPTY)
				{
					if (key != VK_ESCAPE)
					{
						key = VK_EMPTY; // reset the key
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
			key = VK_EMPTY;
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
			const size_t M = Get_Height();
			const size_t N = Get_Width();
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
			while (key != VK_ESCAPE)
			{
				while (TRUE)
				{
					if (key != VK_EMPTY)
					{
						if (key != VK_SPACE && key != VK_ESCAPE)
						{
							key = VK_EMPTY; // reset the key
						}
						else
						{
							break;
						}
					}
				}
				if (key == VK_SPACE)
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
					key = VK_EMPTY;
				}
			}
			key = VK_EMPTY;
			state = MENU;
			break;
		}

		case LOAD:
			printf("Loading data...\n");
			printf("Press ESC to return to Menu\n");
			while (key != VK_ESCAPE)
			{
				if (key != VK_EMPTY && key != VK_ESCAPE)
				{
					key = VK_EMPTY; // reset the key
				}
			}
			key = VK_EMPTY;
			state = MENU;
			break;

		case SAVE:
			printf("Saving data...\n");
			printf("Press ESC to return to Menu\n");
			while (key != VK_ESCAPE)
			{
				if (key != VK_EMPTY && key != VK_ESCAPE)
				{
					key = VK_EMPTY; // reset the key
				}
			}
			key = VK_EMPTY;
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
			while (key != VK_ESCAPE)
			{
				if (key != VK_EMPTY)
				{
					if (key != VK_ESCAPE)
					{
						key = VK_EMPTY; // reset the key
					}
					else
					{
						break;
					}
				}
			}
			key = VK_EMPTY;
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
		if (*pKey == VK_EMPTY)
		{
			if (_kbhit())
			{
				*pKey = _getch();
			}
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