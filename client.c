#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "raylib.h"
#define BOARD_SIZE 8
#define TILE_SIZE 80
#define WINDOW_SIZE (BOARD_SIZE * TILE_SIZE)

extern int errno;

int port;
char chess_board[8][8];
int color; // 1=alb, 2=negru
char move[10];
char final_board[256];
Vector2 selectedTile = {-1, -1};

void string_to_matrix(char *string)
{
    int count = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            chess_board[i][j] = string[count++];
}
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

void DrawChessBoard(char board[8][8], Texture2D piecesTextures[12])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            Color tileColor = ((i + j) % 2 == 0) ? BEIGE : BROWN;
            DrawRectangle(j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE, tileColor);

            if (selectedTile.x == j && selectedTile.y == i)
            {
                DrawRectangleLinesEx((Rectangle){j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE}, 3, YELLOW);
            }

            if (board[i][j] != '.')
            {
                char piece = board[i][j];
                Texture2D pieceTexture;

                switch (piece)
                {
                case 'r':
                    pieceTexture = piecesTextures[0];
                    break;
                case 'q':
                    pieceTexture = piecesTextures[1];
                    break;
                case 't':
                    pieceTexture = piecesTextures[2];
                    break;
                case 'n':
                    pieceTexture = piecesTextures[3];
                    break;
                case 'c':
                    pieceTexture = piecesTextures[4];
                    break;
                case 'p':
                    pieceTexture = piecesTextures[5];
                    break;
                case 'R':
                    pieceTexture = piecesTextures[6];
                    break;
                case 'Q':
                    pieceTexture = piecesTextures[7];
                    break;
                case 'T':
                    pieceTexture = piecesTextures[8];
                    break;
                case 'N':
                    pieceTexture = piecesTextures[9];
                    break;
                case 'C':
                    pieceTexture = piecesTextures[10];
                    break;
                case 'P':
                    pieceTexture = piecesTextures[11];
                    break;
                default:
                    break;
                }
                float scaleFactor = 0.3f;

                Rectangle destRect = {
                    j * TILE_SIZE + TILE_SIZE / 2 - pieceTexture.width * scaleFactor / 2,
                    i * TILE_SIZE + TILE_SIZE / 2 - pieceTexture.height * scaleFactor / 2,
                    pieceTexture.width * scaleFactor,
                    pieceTexture.height * scaleFactor};

                DrawTextureEx(pieceTexture, (Vector2){destRect.x, destRect.y}, 0.0f, scaleFactor, WHITE);
            }
        }
    }
}

Vector2 GetTileFromMousePosition(Vector2 mousePosition)
{
    int x = mousePosition.x / TILE_SIZE;
    int y = mousePosition.y / TILE_SIZE;

    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE)
        return (Vector2){x, y};
    return (Vector2){-1, -1};
}
char *oglinda_tabla_de_sah(char linie, char coloana)
{
    char *oglinda = (char *)malloc(3 * sizeof(char));
    if (!oglinda)
    {
        exit(EXIT_FAILURE);
    }
    oglinda[0] = 'h' - (linie - 'a');
    oglinda[1] = '8' - (coloana - '1');
    oglinda[2] = '\0';
    return oglinda;
}
void FormatMove(Vector2 start, Vector2 end, char *move)
{
    if (color == 1)
    {
        sprintf(move, "%c%c%c%c%c%c",
                chess_board[(int)start.y][(int)start.x],
                'a' + (int)start.x, '8' - (int)start.y,
                'x',
                'a' + (int)end.x, '8' - (int)end.y);
        printf("%d, %d\n", (int)start.x, (int)start.y);
    }
    else
    {

        sprintf(move, "%c%s%c%s",
                chess_board[(int)start.y][(int)start.x],
                oglinda_tabla_de_sah('a' + (int)start.x, '8' - (int)start.y),
                'x',
                oglinda_tabla_de_sah('a' + (int)end.x, '8' - (int)end.y));
        printf("%d, %d\n", (int)start.x, (int)start.y);
    }
}

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    char buffer[256];
    build_start_board(chess_board);

    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    read(sd, &color, sizeof(int));
    printf("Joci cu %s!\n", (color == 1) ? "alb" : "negru");

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess Client");
    Texture2D pieces[12];
    pieces[0] = LoadTexture("wk.png");
    pieces[1] = LoadTexture("wq.png");
    pieces[2] = LoadTexture("wr.png");
    pieces[3] = LoadTexture("wb.png");
    pieces[4] = LoadTexture("wn.png");
    pieces[5] = LoadTexture("wp.png");
    pieces[6] = LoadTexture("bk.png");
    pieces[7] = LoadTexture("bq.png");
    pieces[8] = LoadTexture("br.png");
    pieces[9] = LoadTexture("bb.png");
    pieces[10] = LoadTexture("bn.png");
    pieces[11] = LoadTexture("bp.png");
    SetTargetFPS(60);
    while (1)
    {
        bzero(buffer, sizeof(buffer));
        read(sd, buffer, 65);
        buffer[strlen(buffer)] = '\0';
        printf("Buffer:%s\n", buffer);
        if (buffer[0] == 'A' && buffer[1] == 'i')
        {
            printf("%s\n", buffer);
            break;
        }
        else
            string_to_matrix(buffer);
        while (!WindowShouldClose())
        {

            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawChessBoard(chess_board, pieces);

            EndDrawing();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 tile = GetTileFromMousePosition(GetMousePosition());
                if (selectedTile.x == -1 && chess_board[(int)tile.y][(int)tile.x] != '.')
                {
                    selectedTile = tile;
                }
                else if (selectedTile.x != -1)
                {
                    FormatMove(selectedTile, tile, move);
                    if (move[1] == move[4] && move[2] == move[5])
                    {
                        bzero(move, sizeof(move));
                        selectedTile = (Vector2){-1, -1};
                    }
                    else
                    {
                        write(sd, move, 10);
                        selectedTile = (Vector2){-1, -1};
                        break;
                    }
                }
            }
        }
        bzero(buffer, sizeof(buffer));
        read(sd, buffer, 65);
        buffer[strlen(buffer)] = '\0';
        if (buffer[0] == 'A' && buffer[1] == 'i')
        {
            printf("%s\n", buffer);
            break;
        }
        else
            string_to_matrix(buffer);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawChessBoard(chess_board, pieces);
        EndDrawing();
    }

    read(sd, final_board, 65);
    printf("final_board:%s\n", final_board);
    buffer[11] = '\0';
    string_to_matrix(final_board);
    int fontSize = 80;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), buffer, fontSize, 2);
    float textX = (WINDOW_SIZE - textSize.x) / 2;
    float textY = (WINDOW_SIZE - textSize.y) / 2;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawChessBoard(chess_board, pieces);

    DrawText(buffer, textX, textY, fontSize, RED);

    EndDrawing();

    sleep(5);
    CloseWindow();
    close(sd);
    return 0;
}
