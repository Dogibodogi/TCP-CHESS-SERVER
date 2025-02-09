#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 2071
#define BUFFER_SIZE 256

extern int errno;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// variabile globale
int CoadaClienti[2];
int NrClientiInAsteptare = 0;
int did_white_king_move = 0;
int did_black_king_move = 0;
int a1_rook = 0, a8_rook = 0, h8_rook = 0, h1_rook = 0;
int en_passant;
char mutare[10];
typedef struct
{
  int player1;
  int player2;
} GameParams;
void build_start_board(char chess_board[8][8])
{
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      chess_board[i][j] = '.';
  for (int i = 0; i < 8; i++) // pionii
  {
    chess_board[1][i] = 'p'; // pioni albi
    chess_board[6][i] = 'P'; // pioni negri
  }
  chess_board[0][0] = chess_board[0][7] = 't'; // ture albe
  chess_board[7][7] = chess_board[7][0] = 'T'; // ture negre

  chess_board[0][1] = chess_board[0][6] = 'c'; // cai albi;
  chess_board[7][1] = chess_board[7][6] = 'C'; // cai negri;

  chess_board[0][2] = chess_board[0][5] = 'n'; // nebuni albi;
  chess_board[7][2] = chess_board[7][5] = 'N'; // nebuni negri;

  chess_board[0][3] = 'r';
  chess_board[7][3] = 'R';

  chess_board[0][4] = 'q';
  chess_board[7][4] = 'Q';
}
char *matrix_to_string(char board[8][8])
{
  char *string = malloc(65 * sizeof(char));
  int count = 0;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      string[count++] = board[i][j];

  string[count] = '\0';

  return string;
}
void string_to_matrix(char *string, char chess_board[8][8])
{
  int count = 0;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      chess_board[i][j] = string[count++];
}
int random_between_1_and_2()
{
  return 1 + rand() % 2;
}
void move_to_matrix_coords(int color, char move[10], int *line, int *row)
{
  if (color == 1) // jucatorul alb
  {
    printf("alb\n");
    *row = move[1] - 'a';
    *line = 7 - (move[2] - '0') + 1;
  }
  else
  {
    *row = 7 - (move[1] - 'a');
    *line = move[2] - '0' - 1; // jucatorul negru
  }
}
int is_capturable_square(int line, int row, char chess_board[8][8], int color)
{
  if (color == 1)
  {
    for (int i = line - 1; i > 0; i--) // tura pe linii
    {
      if (chess_board[i][row] != '.')
      {
        if (chess_board[i][row] != 'T' && chess_board[i][row] != 'Q')
        {
          break;
        }
        else
        {
          return 1;
        }
      }
    }

    for (int i = row + 1; i < 8; i++) // tura pe coloana  +
    {
      if (chess_board[line][i] != '.')
      {
        if (chess_board[line][i] != 'T' && chess_board[line][i] != 'Q')
        {

          break;
        }
        else
        {
          return 1;
        }
      }
    }
    for (int i = row - 1; i > 0; i--) // tura pe coloana  -
    {
      if (chess_board[line][i] != '.')
      {
        printf("%c\n", chess_board[line][i]);
        if (chess_board[line][i] != 'T' && chess_board[line][i] != 'Q')
        {

          break;
        }
        else
        {

          return 1;
        }
      }
    }
    if (chess_board[line - 2][row + 1] == 'C' || chess_board[line - 2][row - 1] == 'C' || chess_board[line - 1][row + 2] == 'C' || chess_board[line - 1][row - 2] == 'C')
    {

      return 1;
    }

    for (int i = 1; i + row < 8; i++)
    {
      if (chess_board[line - i][row + i] != '.')
      {
        printf("piesa:%c\n", chess_board[line - i][row + i]);
        if (chess_board[line - i][row + i] != 'N' && chess_board[line - i][row + i] != 'Q')
        {
          break;
        }
        else
        {
          return 1;
        }
      }
    }
    for (int i = 1; row >= i; i++)
    {
      printf("piesa:%c\n", chess_board[line - i][row - i]);
      if (chess_board[line - i][row - i] != '.')
      {
        if (chess_board[line - i][row - i] != 'N' && chess_board[line - i][row - i] != 'Q')
        {
          break;
        }
        else
        {
          return 1;
        }
      }
    }

    if (chess_board[line - 1][row - 1] == 'P' || chess_board[line - 1][row = 1] == 'P')
    {
      return 1;
    }
  }
  else if (color == 2)
  {
    printf("aaaaa\n");
    for (int i = line - 1; i > 0; i--) // tura pe linii
    {
      if (chess_board[i][row] != '.')
      {
        if (chess_board[i][row] != 't' && chess_board[i][row] != 'q')
        {
          printf("%c\n", chess_board[i][row]);
          printf("nu i Tura pe linie\n");
          break;
        }
        else
        {
          printf("Tura pe linie!\n");
          return 1;
        }
      }
    }

    for (int i = row + 1; i < 8; i++) // tura pe coloana  +
    {
      if (chess_board[line][i] != '.')
      {
        if (chess_board[line][i] != 't' && chess_board[line][i] != 'q')
        {
          printf("nu i Tura pe cloana\n");
          break;
        }
        else
        {
          printf("Tura pe cloana\n");
          return 1;
        }
      }
    }
    for (int i = row - 1; i > 0; i--) // tura pe coloana  -
    {
      if (chess_board[line][i] != '.')
      {
        printf("%c\n", chess_board[line][i]);
        if (chess_board[line][i] != 't' && chess_board[line][i] != 'q')
        {
          printf("nu i Tura pe cloana\n");
          break;
        }
        else
        {
          return 1;
        }
      }
    }
    if (chess_board[line - 2][row + 1] == 'c' || chess_board[line - 2][row - 1] == 'c' || chess_board[line - 1][row + 2] == 'c' || chess_board[line - 1][row - 2] == 'c')
    {
      return 1;
    }

    for (int i = 1; i + row < 8; i++)
    {
      if (chess_board[line - i][row + i] != '.')
      {
        printf("piesa:%c\n", chess_board[line - i][row + i]);
        if (chess_board[line - i][row + i] != 'n' && chess_board[line - i][row + i] != 'q')
        {
          break;
        }
        else
        {
          printf("E nebun pe linie!\n");
          return 1;
        }
      }
    }
    for (int i = 1; row >= i; i++)
    {
      printf("piesa:%c\n", chess_board[line - i][row - i]);
      if (chess_board[line - i][row - i] != '.')
      {
        if (chess_board[line - i][row - i] != 'n' && chess_board[line - i][row - i] != 'q')
        {
          break;
        }
        else
        {
          printf("E nebun pe linie!\n");
          return 1;
        }
      }
    }

    if (chess_board[line - 1][row - 1] == 'p' || chess_board[line - 1][row = 1] == 'p')
    {
      return 1;
    }
  }
  return 0;
}
void capture_move_to_matrix_coords(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow)
{
  if (color == 1) // jucatorul alb
  {
    printf("alb\n");
    *row = move[1] - 'a';
    *line = 7 - (move[2] - '0') + 1;
    *CaptureRow = move[4] - 'a'; // pe4xh6
    *CaptureLine = 7 - (move[5] - '0') + 1;
  }
  else // jucatorul negru
  {
    printf("negru\n");
    *row = 7 - (move[1] - 'a');
    *line = move[2] - '0' - 1;
    *CaptureRow = 7 - (move[4] - 'a'); // pe4xh6
    *CaptureLine = move[5] - '0' - 1;
  }
}
void check_for_rooks(char chess_board[8][8], int color)
{
  if (color == 1)
  {
    if (chess_board[7][0] != 't')
      a1_rook = 1;
    if (chess_board[7][7] != 't')
      a8_rook = 1;
  }
  else
  {
    if (chess_board[7][0] != 'T')
      h8_rook = 1;
    if (chess_board[7][7] != 'T')
      h1_rook = 1;
  }
}
void check_for_kings(char chess_board[8][8], int color)
{
  if (color == 1)
  {
    if (chess_board[7][4] != 'r')
    {
      printf("S-A MISCAT REGLE ALB!\n");
      did_white_king_move = 1;
      printf("patratul 7, 4:%c\n", (chess_board[7][4]));
    }
  }
  else if (chess_board[7][3] != 'R')
  {
    printf("patratul  negru7, 3:%c\n", (chess_board[7][3]));
    did_black_king_move = 1;
  }
}
int is_legal_pawn_move(int color, char move[10], int *line, int *row, char chess_board[8][8])
{
  if (color == 1)
  {
    if ((chess_board[*line + 2][*row] == 'p' && *line + 2 == 6) || chess_board[*line + 1][*row] == 'p') // daca mutarea de pion este valida
    {
      if (chess_board[*line][*row] == '.') // daca patratelul unde se muta este liber
      {
        if (*line == 0)
          chess_board[*line][*row] = 'q';
        else
          chess_board[*line][*row] = 'p'; // punem pionul pe pozitia ceruta

        if (chess_board[*line + 1][*row] == 'p')
        {
          en_passant = -1; // stergem pozitia anterioara a pionului
          chess_board[*line + 1][*row] = '.';
        }
        else
        {
          chess_board[*line + 2][*row] = '.';
          en_passant = 7 - *row;
          printf("en_passant:%d\n", en_passant);
        }
        return 1;
      }
    }
  }
  else if ((chess_board[*line + 2][*row] == 'P' && *line + 2 == 6) || chess_board[*line + 1][*row] == 'P') // daca mutarea de pion este valida

  {
    if (chess_board[*line][*row] == '.') // daca patratelul unde se muta este liber
    {
      if (*line == 0)
        chess_board[*line][*row] = 'Q';
      else
        chess_board[*line][*row] = 'P'; // punem pionul pe pozitia ceruta

      if (chess_board[*line + 1][*row] == 'P')
      { // stergem pozitia anterioara a pionului
        chess_board[*line + 1][*row] = '.';
        en_passant = -1;
      }
      else
      {
        chess_board[*line + 2][*row] = '.';
        en_passant = 7 - *row;
        printf("en_passant:%d\n", en_passant);
      }
      return 1;
    }
  }
  printf("tEST\n");
}
int is_legal_pawn_capture(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow, char chess_board[8][8])
{
  printf("line,row, CaptureLine, CAptureROw:%d, %d, %d, %d\n", *line, *row, *CaptureLine, *CaptureRow);
  if (color == 1)
  {
    if (chess_board[*line][*row] == 'p' && chess_board[*CaptureLine][*CaptureRow] > 'A' && chess_board[*CaptureLine][*CaptureRow] < 'Z')
    {
      printf("Intrat!\n");
      if (*CaptureLine == *line - 1 && (*CaptureRow == *row - 1 || *CaptureRow == *row + 1))
      {
        printf("Intrat!\n");
        chess_board[*line][*row] = '.';
        chess_board[*CaptureLine][*CaptureRow] = 'p';
        en_passant = -1;
        return 1;
      }
    }
    else if (*line == 3 && (en_passant + 1 == *row || en_passant - 1 == *row) && en_passant > -1)
    {
      printf("Intrat in enpassant!\n");
      if (*CaptureLine == 2 && *CaptureRow == en_passant)
      {
        en_passant = -1;
        chess_board[*CaptureLine][*CaptureRow] = 'p';
        chess_board[*CaptureLine + 1][*CaptureRow] = '.';
        chess_board[*line][*row] = '.';
        return 1;
      }
    }
  }
  else
  {
    if (chess_board[*line][*row] == 'P' && chess_board[*CaptureLine][*CaptureRow] > 'a' && chess_board[*CaptureLine][*CaptureRow] < 'z')
    {
      printf("Intrat!\n");
      if (*CaptureLine == *line - 1 && (*CaptureRow == *row - 1 || *CaptureRow == *row + 1))
      {
        printf("Intrat!\n");
        chess_board[*line][*row] = '.';
        chess_board[*CaptureLine][*CaptureRow] = 'P';
        return 1;
      }
    }
    else if (*line == 3 && (en_passant + 1 == *row || en_passant - 1 == *row) && en_passant > -1)
    {
      printf("Intrat in enpassant!\n");
      if (*CaptureLine == 2 && *CaptureRow == en_passant)
      {
        en_passant = -1;
        chess_board[*CaptureLine][*CaptureRow] = 'P';
        chess_board[*CaptureLine + 1][*CaptureRow] = '.';
        chess_board[*line][*row] = '.';
        return 1;
      }
    }
  }
}
int is_legal_bishop_move(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow, char chess_board[8][8])
{
  int a;
  if (color == 1)
  {
    a = 0;
  }
  else
  {
    a = 32;
  }
  if (chess_board[*line][*row] == 'n' - a || chess_board[*line][*row] == 'q' - a)
  {
    if (*CaptureLine < *line)
    {
      if (*CaptureRow < *row)
      {
        for (int i = 1; i < *line + 1 && i < *row + 1; i++)
        {
          if (chess_board[*line - i][*row - i] != '.' && *line - i != *CaptureLine && i != 1)
            return 0;
          else if (*line - i == *CaptureLine && *row - i == *CaptureRow && ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.'))
          {
            if (chess_board[*line][*row] == 'n' - a)
              chess_board[*CaptureLine][*CaptureRow] = 'n' - a;
            else
              chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
            chess_board[*line][*row] = '.';
            return 1;
          }
        }
      }
      else if (*CaptureRow > *row)
      {
        for (int i = 1; i < *line + 1 && i + *row < 8; i++)
        {
          if (chess_board[*line - i][*row + i] != '.' && *line - i != *CaptureLine && i != 1)
            return 0;
          else if (*line - i == *CaptureLine && *row + i == *CaptureRow && ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.'))
          {
            if (chess_board[*line][*row] == 'n' - a)
              chess_board[*CaptureLine][*CaptureRow] = 'n' - a;
            else
              chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
            chess_board[*line][*row] = '.';
            return 1;
          }
        }
      }
    }
    else if (*CaptureLine > *line)
    {
      if (*CaptureRow < *row)
      {
        for (int i = 1; *line + i < 8 && i < *row + 1; i++)
        {
          if (chess_board[*line + i][*row - i] != '.' && *line + i != *CaptureLine && i != 1)
            return 0;
          else if (*line + i == *CaptureLine && *row - i == *CaptureRow && ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.'))
          {
            if (chess_board[*line][*row] == 'n' - a)
              chess_board[*CaptureLine][*CaptureRow] = 'n' - a;
            else
              chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
            chess_board[*line][*row] = '.';
            return 1;
          }
        }
      }
      else if (*CaptureRow > *row)
      {
        for (int i = 1; *line + i < 8 && i + *row < 8; i++)
        {
          if (chess_board[*line + i][*row + i] != '.' && *line - i != *CaptureLine && i != 1)
            return 0;
          else if (*line + i == *CaptureLine && *row + i == *CaptureRow && ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.'))
          {
            if (chess_board[*line][*row] == 'n' - a)
              chess_board[*CaptureLine][*CaptureRow] = 'n' - a;
            else
              chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
            chess_board[*line][*row] = '.';
            return 1;
          }
        }
      }
    }
  }
}
int is_legal_knight_move(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow, char chess_board[8][8])
{
  int a;
  if (color == 1)
    a = 0;
  else
    a = 32;
  int x = abs(*line - *CaptureLine);
  int y = abs(*row - *CaptureRow);
  if ((x == 1 && y == 2) || (x == 2 && y == 1))
  {
    if (chess_board[*line][*row] == 'c' - a)
      if (*CaptureLine < 8 && *CaptureLine > -1 && *CaptureRow < 8 && *CaptureRow > -1)
      {
        if (chess_board[*CaptureLine][*CaptureRow] == '.' || (chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a))
        {
          chess_board[*line][*row] = '.';
          chess_board[*CaptureLine][*CaptureRow] = 'c' - a;
          return 1;
        }
        else
          return 0;
      }
      else
        return 0;
  }
  else
    return 0;
}
int is_legal_rook_move(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow, char chess_board[8][8])
{
  int a;
  if (color == 1)
    a = 0;
  else
    a = 32;
  if (*line != *CaptureLine && *row != *CaptureRow)
    return 0;
  printf("line, row, CaptureLine, CaptureRow:%d %d %d %d\n", *line, *row, *CaptureLine, *CaptureRow);
  if (chess_board[*line][*row] == 't' - a || chess_board[*line][*row] == 'q' - a)
  {
    printf("Intrat!\n");
    if (*line == *CaptureLine)
    {
      if (*row < *CaptureRow)
      {
        for (int i = *row + 1; i < *CaptureRow; i++)
        {
          if (chess_board[*line][i] != '.')
            return 0;
        }
        if ((chess_board[*line][*CaptureRow] > 'A' + a && chess_board[*line][*CaptureRow] < 'Z' + a) || chess_board[*line][*CaptureRow] == '.')
        {
          if (chess_board[*line][*row] == 't' - a)
            chess_board[*CaptureLine][*CaptureRow] = 't' - a;
          else
            chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
          chess_board[*line][*row] = '.';
          return 1;
        }
      }
      else if (*row > *CaptureRow)
      {
        for (int i = *CaptureRow + 1; i < *row; i++)
        {
          if (chess_board[*line][i] != '.')
            return 0;
        }
        if ((chess_board[*line][*CaptureRow] > 'A' + a && chess_board[*line][*CaptureRow] < 'Z' + a) || chess_board[*line][*CaptureRow] == '.')
        {
          if (chess_board[*line][*row] == 't' - a)
            chess_board[*CaptureLine][*CaptureRow] = 't' - a;
          else
            chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
          chess_board[*line][*row] = '.';
          return 1;
        }
      }
      else
        return 0;
    }
    else if (*row == *CaptureRow)
    {
      if (*line < *CaptureLine)
      {
        for (int i = *line + 1; i < *CaptureLine; i++)
        {
          if (chess_board[i][*row] != '.')
            return 0;
        }
        if ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.')
        {
          if (chess_board[*line][*row] == 't' - a)
            chess_board[*CaptureLine][*CaptureRow] = 't' - a;
          else
            chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
          chess_board[*line][*row] = '.';
          return 1;
        }
      }
      else if (*line > *CaptureLine)
      {
        for (int i = *CaptureLine + 1; i < *line; i++)
        {
          if (chess_board[i][*row] != '.')
            return 0;
        }
        if ((chess_board[*CaptureLine][*CaptureRow] > 'A' + a && chess_board[*CaptureLine][*CaptureRow] < 'Z' + a) || chess_board[*CaptureLine][*CaptureRow] == '.')
        {
          if (chess_board[*line][*row] == 't' - a)
            chess_board[*CaptureLine][*CaptureRow] = 't' - a;
          else
            chess_board[*CaptureLine][*CaptureRow] = 'q' - a;
          chess_board[*line][*row] = '.';
          return 1;
        }
      }
    }
  }
  else
    return 0;
}
int is_legal_king_move(int color, char move[10], int *line, int *row, int *CaptureLine, int *CaptureRow, char chess_board[8][8])
{
  int a;
  if (color == 1)
  {
    a = 0;
  }
  else
    a = 32;
  int x = abs(*line - *CaptureLine);
  int y = abs(*row - *CaptureRow);
  if ((x < 2 && y < 2) && (x > 0 || y > 0))
    if (chess_board[*line][*row] == 'r' - a && chess_board[*CaptureLine][*CaptureRow] == '.')
    {
      chess_board[*line][*row] = '.';
      chess_board[*CaptureLine][*CaptureRow] = 'r' - a;
      printf("Returnat 1!\n");
      return 1;
    }

  if (color == 1)
  {
    if (did_white_king_move == 0)
    {
      if (*CaptureLine == 7 && *CaptureRow == 6)
      {
        if (a8_rook == 0)
          if (is_capturable_square(7, 6, chess_board, 1) == 0 && is_capturable_square(7, 5, chess_board, 1) == 0 && is_capturable_square(7, 4, chess_board, 1) == 0)
          {
            printf("Rocada mica alb!\n");
            if (chess_board[7][6] == '.' && chess_board[7][5] == '.')
            {
              chess_board[7][5] = 't';
              chess_board[*line][*row] = '.';
              chess_board[7][6] = 'r';
              chess_board[7][7] = '.';
              printf("Returnat 12!\n");
              return 1;
            }
          }
      }
      else if (*CaptureLine == 7 && *CaptureRow == 2)
      {
        if (a1_rook == 0)
          if (is_capturable_square(7, 3, chess_board, 1) == 0 && is_capturable_square(7, 4, chess_board, 1) == 0)
          {
            printf("Rocada mare alb!\n");
            if (chess_board[7][1] == '.' && chess_board[7][2] == '.' && chess_board[7][3] == '.')
            {
              chess_board[7][3] = 't';
              chess_board[*line][*row] = '.';
              chess_board[7][2] = 'r';
              chess_board[7][0] = '.';
              printf("Returnat 13!\n");
              return 1;
            }
          }
      }
    }
  }
  else if (color == 2)
  {
    if (did_black_king_move == 0)
    {
      if (*CaptureLine == 7 && *CaptureRow == 1)
      {
        if (h1_rook == 0)
          if (is_capturable_square(7, 1, chess_board, 2) == 0 && is_capturable_square(7, 2, chess_board, 2) == 0 && is_capturable_square(7, 3, chess_board, 2) == 0)
          {
            printf("Rocada mica negru!\n");
            if (chess_board[7][1] == '.' && chess_board[7][2] == '.')
            {
              chess_board[7][2] = 'T';
              chess_board[*line][*row] = '.';
              chess_board[7][1] = 'R';
              chess_board[7][0] = '.';
              printf("Returnat 14!\n");
              return 1;
            }
          }
      }
      else if (*CaptureLine == 7 && *CaptureRow == 5)
      {
        if (h8_rook == 0)
          if (is_capturable_square(7, 5, chess_board, 1) == 0 && is_capturable_square(7, 4, chess_board, 1) == 0)
          {
            printf("Rocada mare negru!\n");
            if (chess_board[7][4] == '.' && chess_board[7][5] == '.' && chess_board[7][6] == '.')
            {
              chess_board[7][5] = 'R';
              chess_board[*line][*row] = '.';
              chess_board[7][4] = 'T';
              chess_board[7][7] = '.';
              printf("Returnat 15!\n");
              return 1;
            }
          }
      }
    }
  }
}
int is_legal_move(char move[10], char chess_board[8][8], int *line, int *row, int *CaptureLine, int *CaptureRow, int color)
{

  if (strlen(move) == 3 && move[0] != 'O')
    move_to_matrix_coords(color, move, line, row);
  else if (move[0] != 'O')
  {
    printf("strlen(move):%ld\n", strlen(move));
    capture_move_to_matrix_coords(color, move, line, row, CaptureLine, CaptureRow);
  }
  if (*CaptureLine > 7 || CaptureLine < 0)
    printf("Nu\n");
  if (*CaptureRow > 7 || CaptureRow < 0)
    printf("Nu\n");

  if (*CaptureRow == *row && *CaptureLine == *line)
  {
    return 2;
  }
  if (move[0] == 'p' || move[0] == 'P')
  {
    if (is_legal_pawn_move(color, move, CaptureLine, CaptureRow, chess_board) == 1)
      return 1;
    else
      return is_legal_pawn_capture(color, move, line, row, CaptureLine, CaptureRow, chess_board);
  }
  else
  {
    en_passant = -1;
  }
  if (move[0] == 'n' || move[0] == 'N')
  {
    return is_legal_bishop_move(color, move, line, row, CaptureLine, CaptureRow, chess_board);
  }
  if (move[0] == 'c' || move[0] == 'C')
  {
    return is_legal_knight_move(color, move, line, row, CaptureLine, CaptureRow, chess_board);
  }
  if (move[0] == 't' || move[0] == 'T')
  {
    return is_legal_rook_move(color, move, line, row, CaptureLine, CaptureRow, chess_board);
  }
  if (move[0] == 'q' || move[0] == 'Q')
  {
    printf("Reegina!\n");
    if (is_legal_rook_move(color, move, line, row, CaptureLine, CaptureRow, chess_board) == 1)
    {
      printf("regina-tura!\n");
      return 1;
    }
    else if (is_legal_bishop_move(color, move, line, row, CaptureLine, CaptureRow, chess_board) == 1)
    {
      printf("regina-nebun\n");
      return 1;
    }
    else
    {
      printf("Mutare de regina invalida!\n");
      return 0;
    }
  }
  if (move[0] == 'r' || move[0] == 'R')
  {
    if (is_legal_king_move(color, move, line, row, CaptureLine, CaptureRow, chess_board) == 1)
    {
      return 1;
    }
    else
      return 0;
  }
  return 0;
}
int check_if_game_ended(char chess_board[8][8])
{
  int BlackKing = 0, WhiteKing = 0;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      if (chess_board[i][j] == 'r')
        WhiteKing = 1;
      if (chess_board[i][j] == 'R')
        BlackKing = 1;
    }
  if (WhiteKing == 1 && BlackKing == 1)
    return 0;
  else if (WhiteKing == 1)
    return 1;
  else
    return 2;
}
void print_board(char board[8][8])
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
      printf("%c ", board[i][j]);
    printf("\n");
  }

  printf("\n");
}
void CloneBoard(char chess_board[8][8], char mirrored_board[8][8])
{
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      mirrored_board[i][j] = chess_board[i][j];
    }
}
void flip_board(char board[8][8])
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      char temp = board[i][j];
      board[i][j] = board[7 - i][7 - j];
      board[7 - i][7 - j] = temp;
    }
  }
}
void handle_game(int player1, int player2)
{
  char buffer[256];
  char chess_board[8][8];
  char mirrored_chess_board[8][8];
  int FirstMove = 0;
  int Player1Color = 0;
  int Player2Color = 0;
  int line, row, CaptureLine, CaptureRow;
  int bytes_received;
  build_start_board(chess_board);
  CloneBoard(chess_board, mirrored_chess_board);

  Player1Color = random_between_1_and_2();
  if (Player1Color == 1)
    Player2Color = 2;
  else
    Player2Color = 1;

  flip_board(chess_board);
  write(player1, &Player1Color, sizeof(Player1Color)); // trimitem culorile
  write(player2, &Player2Color, sizeof(Player2Color));

  while (1)
  {
    if (Player1Color == 1 || FirstMove > 0)
    {
      write(player1, matrix_to_string(chess_board), 65);
      read(player1, mutare, 10);
      mutare[strlen(mutare)] = '\0';
      printf("%s\n", mutare);
      is_legal_move(mutare, chess_board, &line, &row, &CaptureLine, &CaptureRow, Player1Color);
      check_for_kings(chess_board, Player1Color);
      check_for_rooks(chess_board, Player1Color);
      if (check_if_game_ended(chess_board) > 0)
      {
        printf("Gata!\n");
        if (check_if_game_ended(chess_board) == 1)
        {
          if (Player1Color == 1)
          {
            printf("1\n");
            write(player1, "Ai castigat!", strlen("Ai castigat!"));
            write(player2, "Ai pierdut!", strlen("Ai pierdut!"));
            sleep(1);
            write(player2, matrix_to_string(chess_board), 65);
            write(player1, matrix_to_string(chess_board), 65);
            break;
          }
          else
          {
            printf("2\n");
            write(player1, "Ai pierdut!", strlen("Ai pierdut!"));
            write(player2, "Ai castigat!", strlen("Ai castigat!"));
            sleep(1);
            write(player2, matrix_to_string(chess_board), 65);
            write(player1, matrix_to_string(chess_board), 65);
            break;
          }
        }
        else
        {
          if (Player1Color == 1)
          {
            write(player1, "Ai pierdut!", strlen("Ai pierdut!"));
            write(player2, "Ai castigat!", strlen("Ai castigat!"));
            sleep(1);
            write(player2, matrix_to_string(chess_board), 65);
            write(player1, matrix_to_string(chess_board), 65);
            printf("3\n");
            break;
          }
          else
          {
            write(player1, "Ai castigat!", strlen("Ai castigat!"));
            write(player2, "Ai pierdut!", strlen("Ai pierdut!"));
            sleep(1);
            write(player2, matrix_to_string(chess_board), 65);
            write(player1, matrix_to_string(chess_board), 65);
            printf("4\n");
            break;
          }
        }
      }
      else
        write(player1, matrix_to_string(chess_board), 65);
      flip_board(chess_board);
      print_board(chess_board);
    }
    if (write(player2, matrix_to_string(chess_board), 65) == -1)
    {
      perror("Eroare la write!\n");
    }
    read(player2, mutare, 10);
    mutare[strlen(mutare)] = '\0';
    printf("%s\n", mutare);
    is_legal_move(mutare, chess_board, &line, &row, &CaptureLine, &CaptureRow, Player2Color);
    check_for_rooks(chess_board, Player2Color);
    check_for_kings(chess_board, Player2Color);
    if (check_if_game_ended(chess_board) > 0)
    {
      printf("Gata!\n");
      if (check_if_game_ended(chess_board) == 1)
      {
        if (Player2Color == 1)
        {
          write(player2, "Ai castigat!", strlen("Ai castigat!"));
          write(player1, "Ai pierdut!", strlen("Ai pierdut!"));
          write(player2, matrix_to_string(chess_board), 65);
          write(player1, matrix_to_string(chess_board), 65);
          printf("5\n");
          break;
        }
        else
        {
          write(player2, "Ai pierdut!", strlen("Ai pierdut!"));
          write(player1, "Ai castigat!", strlen("Ai castigat!"));
          write(player2, matrix_to_string(chess_board), 65);
          write(player1, matrix_to_string(chess_board), 65);
          printf("6\n");
          break;
        }
      }
      else
      {
        if (Player2Color == 1)
        {
          write(player2, "Ai pierdut!", strlen("Ai pierdut!"));
          write(player1, "Ai castigat!", strlen("Ai castigat!"));
          write(player2, matrix_to_string(chess_board), 65);
          write(player1, matrix_to_string(chess_board), 65);
          printf("7\n");
          break;
        }
        else
        {
          write(player2, "Ai castigat!", strlen("Ai castigat!"));
          write(player1, "Ai pierdut!", strlen("Ai pierdut!"));
          write(player2, matrix_to_string(chess_board), 65);
          write(player1, matrix_to_string(chess_board), 65);
          printf("8\n");
          break;
        }
      }
    }
    else
      write(player2, matrix_to_string(chess_board), 65);
    flip_board(chess_board);
    print_board(chess_board);
    FirstMove = 1;
  }
}
void *thread_handle_game(void *arg)
{
  GameParams *params = (GameParams *)arg;
  handle_game(params->player1, params->player2);

  close(params->player1);
  close(params->player2);

  free(params);
  pthread_exit(NULL);
}
int main()
{
  srand(time(NULL));
  struct sockaddr_in server;
  struct sockaddr_in from;
  int sd;

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  if (listen(sd, 1) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  while (1)
  {
    int client;
    int length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    client = accept(sd, (struct sockaddr *)&from, &length);
    if (client < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }
    else
    {
      pthread_mutex_lock(&lock);
      CoadaClienti[NrClientiInAsteptare++] = client;
    }
    if (NrClientiInAsteptare == 2)
    {
      NrClientiInAsteptare = 0;

      GameParams *params = (GameParams *)malloc(sizeof(GameParams));
      if (params == NULL)
      {
        perror("[server] Eroare la alocarea memoriei pentru parametrii jocului.\n");
        close(CoadaClienti[0]);
        close(CoadaClienti[1]);
        continue;
      }
      params->player1 = CoadaClienti[0];
      params->player2 = CoadaClienti[1];

      pthread_t thread_id;
      if (pthread_create(&thread_id, NULL, thread_handle_game, (void *)params) != 0)
      {
        perror("[server] Eroare la crearea thread-ului.\n");
        close(CoadaClienti[0]);
        close(CoadaClienti[1]);
        free(params);
        continue;
      }

      pthread_detach(thread_id);

      pthread_mutex_unlock(&lock);
    }
    else
    {
      printf("%i\n", NrClientiInAsteptare);
      pthread_mutex_unlock(&lock);
      continue;
    }
  }
}
