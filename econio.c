/* Fait par Eric TETZ */ 
/* econio.cpp Définitions des fonctions */
  
 #include "econio.h"
  
static WORD bgcolor = BLACK;
static WORD fgcolor = WHITE << 4;
#define STDOUT GetStdHandle(STD_OUTPUT_HANDLE)
  
void gotoxy( int x, int y ) // Place le curseur en x,y
{
  COORD dwCursorPosition = { x, y };
  SetConsoleCursorPosition (STDOUT, dwCursorPosition);
}

int wherex( void ) // Renvoie la valeur de x
{
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  GetConsoleScreenBufferInfo (STDOUT, &coninfo);
  return coninfo.dwCursorPosition.X;
}

int wherey( void ) // Renvoie la valeur de y
{
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  GetConsoleScreenBufferInfo (STDOUT, &coninfo);
  return coninfo.dwCursorPosition.Y;
}

void textbackground( int newcolor ) // Change la couleur du texte
{
  fgcolor = newcolor;
  SetConsoleTextAttribute (STDOUT, fgcolor | bgcolor);
}

void textcolor( int newcolor )
{
  bgcolor = newcolor << 4;
  SetConsoleTextAttribute (STDOUT, fgcolor | bgcolor);
}

void setcursortype( int cur ) // type de curseur
{
  BOOL visible = cur != NOCURSOR;
  CONSOLE_CURSOR_INFO CursorInfo = { cur, visible };
  SetConsoleCursorInfo (STDOUT, &CursorInfo);
}

void clrscr ( void ) // efface l'écran
{
  HANDLE hstdout = STDOUT;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo (hstdout, &csbi))
  {
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter (hstdout, ' ', dwConSize, coordScreen, &cCharsWritten);
    FillConsoleOutputAttribute (hstdout, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition (hstdout, coordScreen);
  }
}
  
