#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <windows.h>
#include <random>
#include <ctime>
#include <time.h>
#include <mmsystem.h>

using namespace std;

// Globals
GLfloat static heigh = 600;
GLfloat static width = heigh;

GLfloat static boardHeight = 500;
GLfloat static boardWidth = boardHeight / 2;

GLfloat static blockSide = boardHeight / 20;

GLfloat static xDisplacement = width/2 - boardWidth/2;
GLfloat static yDisplacement = heigh/2 - boardHeight/2;

void myInit(void); // OpenGL initialization
void display(); // Drawing the scene
void createGameBoard(void);
void initGame();
void movePieceDown();
void update(int);
void drawBoard();
void checkGameState();
void drawBlocks();
int* toGameCoords(int, int);
void drawTetrisBlock(float x, float y, float size, int color);
void colorPicker(int color, int shade);
void getTetrisPiece(int pieceType);
void stitchPieceToGameboard(int piece[4][4], int position[2]);
void movePieceDownAndSetPieceStatus();
int getRandomPieceNumber();
void mergeDroppedPieceIntoBoard();
void handle_KeyboardFunc(GLubyte key, GLint x, GLint y);
void handleSpecialKeyPress(int key, int x, int y);
void moveLeft();
void moveRight();
void rotate();
void clearFullLines();
void displayGameOver();
int getCycleTime();

int gameBoard[10][20];
bool activePiece = false;
bool gameover = false;
bool pieceDropped = true;
int piece[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
int xpos = 0;
int ypos = 0;
int points = 0;
std::clock_t startTime;

int main(int argc, char** argv) {
    // Initialize the random number generator with a random seed
    srand(time(NULL));
    const char* filePath = "C:\\Dev\\Aulas\\TetrisTheme.wav";
    PlaySound(TEXT(filePath), NULL, SND_FILENAME | SND_ASYNC);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, heigh);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Primeiro programa");
    myInit();
    createGameBoard();
    glutDisplayFunc(display);
    glutTimerFunc(100, update, 0);
    glutKeyboardFunc(handle_KeyboardFunc);
    glutSpecialFunc(handleSpecialKeyPress);
    glutMainLoop();
    return 0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if(gameover) {
        displayGameOver();
        glutSwapBuffers();
        return;
    }

    // Draw the game grid
    drawBoard();
    // Draw the current and next piece
    drawBlocks();

    glutSwapBuffers();
}
void update(int value) {
    // Update game state: move pieces, check for collisions
    startTime = std::clock();
    if(gameover){
        glutPostRedisplay();
        glutTimerFunc(250, update, 0);
        return;
    }
    if (pieceDropped) {
        clearFullLines();
        xpos = 3;
        ypos = 17;
        getTetrisPiece(rand() % 7 + 1);
        if(gameover)glutTimerFunc(250, update, 0);
        pieceDropped = false;
    }
    movePieceDownAndSetPieceStatus();
    if (pieceDropped) {
        mergeDroppedPieceIntoBoard();
    }
    // Check game conditions: line completion, game over, etc.
    checkGameState();

    glutPostRedisplay();
    glutTimerFunc(getCycleTime(), update, 0);
}

int getCycleTime(){

    if (startTime < 10000 )return 500;
    if (startTime < 20000 )return 400;
    if (startTime < 30000 )return 300;
    if (startTime < 100000 )return 200;
    return 100;
}

void handleSpecialKeyPress(int key, int x, int y){
    switch (key){
        case GLUT_KEY_UP:
            rotate();
            break;
        case GLUT_KEY_DOWN:
            movePieceDownAndSetPieceStatus();
            if(pieceDropped) mergeDroppedPieceIntoBoard();
            break;
        mergeDroppedPieceIntoBoard();
            break;
        case GLUT_KEY_LEFT:
            moveLeft();
            break;
        case GLUT_KEY_RIGHT:
            moveRight();
            break;
        default:
            break;
    }
    glutPostRedisplay();

}

void handle_KeyboardFunc(GLubyte key, GLint x, GLint y) {
    glutPostRedisplay();
    switch (key){
        case 'w':
            rotate();
            break;
        case 's':
            movePieceDownAndSetPieceStatus();
            if(pieceDropped) mergeDroppedPieceIntoBoard();
            break;
        case 'a':
            moveLeft();
            break;
        case 'd':
            moveRight();
            break;
        case ' ':
            while (!pieceDropped) {
                movePieceDownAndSetPieceStatus();
            }
            mergeDroppedPieceIntoBoard();
        default:
            break;
    }
    glutPostRedisplay();

}

void displayGameOver() {
    char pointsMessage[50];
    sprintf(pointsMessage, "%d", points);
    int lenPoitsMsg = strlen(pointsMessage);

    char* spacer= "POINTS: ";
    int lenSpacerMsg = strlen(spacer);

    const char* message = "GAME OVER";
    int len = strlen(message);

    glColor3f(1.0, 0.0, 0.0); // Set color to red
    glRasterPos2f(width / 2 - (len * 9) / 2, heigh / 2); // Position the text

    //Gameover
    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message[i]);
    }
    glRasterPos2f(width / 2 - ((lenPoitsMsg + lenSpacerMsg)  * 9) / 2, (heigh / 4)); // Position the text
    //Spacer
    for (int i = 0; i < lenSpacerMsg; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, spacer[i]);
    }
    //Points
    for (int i = 0; i < lenPoitsMsg; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, pointsMessage[i]);
    }

    glColor3f(1.0, 1.0, 1.0); // Reset color to white
}



void clearFullLines(){
    int fullLines = 0;
    int fullBlocksInALine = 0;
    for (int j = 0; j < 20; ++j) {
        fullBlocksInALine = 0;
        for (int i = 0; i < 10; ++i) {
            if (gameBoard[i][j] != 0) fullBlocksInALine++;
        }
        if (fullBlocksInALine == 10){
            fullLines++;
            //clear full line
            for (int m = j; m<19; m++){
                for (int n=0; n<10; n++){
                    gameBoard[n][m] = gameBoard[n][m+1];
                }
            }
            j--;
        }
    }
    if (fullLines == 1) points += 100;
    if (fullLines == 2) points += 300;
    if (fullLines == 3) points += 500;
    if (fullLines == 4) points += 800;
    return;
}



void rotate() {
    int newPiece[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            newPiece[j][3 - i] = piece[i][j];
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (newPiece[i][j] != 0 && (i + xpos < 0 || i + xpos >= 10 || j + ypos >= 20 || j + ypos < 0)) {
                return;
            }
            if (newPiece[i][j] != 0 && gameBoard[xpos + i][ypos + j] != 0) {
                return;
            }
        }
    }

    // Copy the newPiece back to piece
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece[i][j] = newPiece[i][j];
        }
    }
}

void moveRight() {
    int newX = xpos + 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0 && (newX + i >= 10 || gameBoard[i + newX][j + ypos] != 0)) {
                return;
            }
        }
    }
    xpos = newX;
}

void moveLeft() {
    int newX = xpos - 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0 && (newX + i < 0 || gameBoard[i + newX][j + ypos] != 0)) {
                return;
            }
        }
    }
    xpos = newX;
}

void mergeDroppedPieceIntoBoard() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0) {
                gameBoard[xpos + i][ypos + j] = piece[i][j];
            }
        }
    }
}

void movePieceDownAndSetPieceStatus() {
    int newy = ypos - 1;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0 && (newy + j < 0 || gameBoard[xpos + i][newy + j] != 0)) {
                pieceDropped = true;
                return;
            }
        }
    }

    ypos = newy;
    pieceDropped = false;
}

void drawBlocks() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            drawTetrisBlock(i, j, blockSide, gameBoard[i][j]);
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0 && i + xpos >= 0 && i + xpos < 10 && j + ypos >= 0 && j + ypos < 20) {
                drawTetrisBlock(i + xpos, j + ypos, blockSide, piece[i][j]);
            }
        }
    }
}

void checkGameState() {
    // Implement game state checks here
}

void drawBoard() {
    int borderWidth = 10;
    int pointsAreaWidth = 100;
    int poitsAreaHeight = 75;
    int pointsAreaPos = boardHeight - 100;


    //glColor3f(0.247, 0.561, 0.973);
    glColor3f(0.1, 0.1, 0.1);
    //Border
    glBegin(GL_QUADS);
        glVertex2f(0 - borderWidth + xDisplacement, 0 - borderWidth + yDisplacement);
        glVertex2f(0 - borderWidth+ xDisplacement, boardHeight + borderWidth + yDisplacement);
        glVertex2f(boardWidth + borderWidth+ xDisplacement, boardHeight + borderWidth + yDisplacement);
        glVertex2f(boardWidth + borderWidth+ xDisplacement, 0 - borderWidth + yDisplacement);
    glEnd();
    //PointsArea
    glBegin(GL_QUADS);
        glVertex2f(boardWidth + borderWidth+ xDisplacement,  pointsAreaPos + yDisplacement);
        glVertex2f(boardWidth + borderWidth+ xDisplacement,  pointsAreaPos + poitsAreaHeight + yDisplacement);
        glVertex2f(boardWidth + borderWidth+ xDisplacement + pointsAreaWidth,  pointsAreaPos + poitsAreaHeight + yDisplacement);
        glVertex2f(boardWidth + borderWidth+ xDisplacement + pointsAreaWidth,  pointsAreaPos + yDisplacement);
    glEnd();
    //pointsText

    char message[50];
    sprintf(message, "%d", points);
    int len = strlen(message);

    glColor3f(0.2, .2, 0.2); // Set color to red
    glRasterPos2f(boardWidth + borderWidth+ xDisplacement + 20,  pointsAreaPos + yDisplacement + (poitsAreaHeight/2) - 12); // Position the text

    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, message[i]);
    }

    glColor3f(1.0, 1.0, 1.0); // Reset color to white
}

void initGame() {
    // Initialize game grid, current piece, next piece, score, etc.
}

int* toGameCoords(int linha, int coluna) {
    static int coords[2];
    coords[0] = 0;
    coords[1] = 0;
    return coords;
}

void drawTetrisBlock(float x, float y, float size, int color) {
    x = x * blockSide;
    y = y * blockSide;
    float const prop = 0.25;

    colorPicker(color, 2);
    glBegin(GL_QUADS);
    glVertex2f(x + xDisplacement, y + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + size + yDisplacement);
    glVertex2f(x + xDisplacement, y + size + yDisplacement);
    glEnd();

    colorPicker(color, 3);
    glBegin(GL_QUADS);
    glVertex2f(x + size - (size * prop) + xDisplacement, y + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + size + yDisplacement);
    glVertex2f(x + size - (size * prop) + xDisplacement, y + size - (size * prop) + yDisplacement);
    glEnd();

    colorPicker(color, 4);
    glBegin(GL_QUADS);
    glVertex2f(x + xDisplacement, y + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + yDisplacement);
    glVertex2f(x + size - (size * prop) + xDisplacement, y + (size * prop) + yDisplacement);
    glVertex2f(x + (size * prop) + xDisplacement, y + (size * prop) + yDisplacement);
    glEnd();

    colorPicker(color, 1);
    glBegin(GL_QUADS);
    glVertex2f(x + xDisplacement, y + size + yDisplacement);
    glVertex2f(x + size + xDisplacement, y + size + yDisplacement);
    glVertex2f(x + size - (size * prop) + xDisplacement, y + size - (size * prop) + yDisplacement);
    glVertex2f(x + (size * prop) + xDisplacement, y + size - (size * prop) + yDisplacement);
    glEnd();
}

void colorPicker(int color, int shade) {
    /*
    1: blue
    2: green
    3: orange
    4: cyan
    5: red
    6: pink
    7: yellow
    8: grey
    */
    switch (color) {
        case 1: //blue
            switch (shade) {
                case 1:
                    glColor3f(0.247, 0.561, 0.973);
                    break;
                case 2:
                    glColor3f(0.0, 0.337, 0.992);
                    break;
                case 3:
                    glColor3f(0.004, 0.165, 0.635);
                    break;
                case 4:
                    glColor3f(0.004, 0.055, 0.361);
                    break;
                default:
                    break;
            }
            break;
        case 2: //green
            switch (shade) {
                case 1:
                    glColor3f(0.0, 0.992, 0.024);
                    break;
                case 2:
                    glColor3f(0.0, 0.992, 0.024);
                    break;
                case 3:
                    glColor3f(0.012, 0.608, 0.035);
                    break;
                case 4:
                    glColor3f(0.0, 0.369, 0.008);
                    break;
                default:
                    break;
            }
            break;
        case 3: //orange
            switch (shade) {
                case 1:
                    glColor3f(1.0, 0.522, 0.180);
                    break;
                case 2:
                    glColor3f(1.0, 0.404, 0.004);
                    break;
                case 3:
                    glColor3f(0.812, 0.286, 0.016);
                    break;
                case 4:
                    glColor3f(0.529, 0.196, 0.004);
                    break;
                default:
                    break;
            }
            break;
        case 4: //cyan
            switch (shade) {
                case 1:
                    glColor3f(0.031, 0.894, 0.953);
                    break;
                case 2:
                    glColor3f(0.004, 0.850, 0.9);
                    break;
                case 3:
                    glColor3f(0.016, 0.550, 0.650);
                    break;
                case 4:
                    glColor3f(0.020, 0.400, 0.450);
                    break;
                default:
                    break;
            }
            break;
        case 5: //red
            switch (shade) {
                case 1:
                    glColor3f(0.973, 0.235, 0.153);
                    break;
                case 2:
                    glColor3f(0.992, 0.024, 0.008);
                    break;
                case 3:
                    glColor3f(0.757, 0.027, 0.020);
                    break;
                case 4:
                    glColor3f(0.467, 0.0, 0.0);
                    break;
                default:
                    break;
            }
            break;
        case 6: //pink
            switch (shade) {
                case 1:
                    glColor3f(1.0, 0.286, 0.988);
                    break;
                case 2:
                    glColor3f(1.0, 0.004, 0.992);
                    break;
                case 3:
                    glColor3f(0.714, 0.016, 0.592);
                    break;
                case 4:
                    glColor3f(0.427, 0.0, 0.325);
                    break;
                default:
                    break;
            }
            break;
        case 7: //yellow
            switch (shade) {
                case 1:
                    glColor3f(0.996, 1.0, 0.267);
                    break;
                case 2:
                    glColor3f(0.992, 0.992, 0.004);
                    break;
                case 3:
                    glColor3f(0.6039, 0.6039, 0.0196);
                    break;
                case 4:
                    glColor3f(0.3412, 0.3647, 0.0118);
                    break;
                default:
                    break;
            }
            break;
        case 0: //grey
            switch (shade) {
                case 1:
                    glColor3f(0.075, 0.075, 0.075);
                    break;
                case 2:
                    glColor3f(0.067, 0.067, 0.067);
                    break;
                case 3:
                    glColor3f(0.03, 0.03, 0.03);
                    break;
                case 4:
                    glColor3f(0.015, 0.015, 0.015);
                    break;
                default:
                    break;
            }
            break;
    }
}

void myInit(void) {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glColor3f(0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPointSize(4.0);
    gluOrtho2D(0.0, heigh, 0.0, width);
    glMatrixMode(GL_MODELVIEW);
}

void getTetrisPiece(int pieceType) {
    // Define the Tetris pieces
    int newPiece [4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece[i][j] = 0;
            newPiece[i][j] = 0;
        }
    }
    switch (pieceType) {
        case 1: // I piece
            newPiece[2][0] = 1;
            newPiece[2][1] = 1;
            newPiece[2][2] = 1;
            newPiece[2][3] = 1;
            break;
        case 2: // J piece
            newPiece[1][1] = 2;
            newPiece[2][1] = 2;
            newPiece[2][2] = 2;
            newPiece[2][3] = 2;
            break;
        case 3: // L piece
            newPiece[1][1] = 3;
            newPiece[2][1] = 3;
            newPiece[1][2] = 3;
            newPiece[1][3] = 3;
            break;
        case 4: // O piece
            newPiece[1][1] = 4;
            newPiece[1][2] = 4;
            newPiece[2][1] = 4;
            newPiece[2][2] = 4;
            break;
        case 5: // S piece
            newPiece[1][2] = 5;
            newPiece[1][3] = 5;
            newPiece[2][1] = 5;
            newPiece[2][2] = 5;
            break;
        case 6: // T piece
            newPiece[1][1] = 6;
            newPiece[1][2] = 6;
            newPiece[1][3] = 6;
            newPiece[2][2] = 6;
            break;
        case 7: // Z piece
            newPiece[1][1] = 7;
            newPiece[1][2] = 7;
            newPiece[2][2] = 7;
            newPiece[2][3] = 7;
            break;
        default:
            // Invalid piece type, return empty grid
            break;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = -1; j < 3; j++) {
            if(newPiece[i][j] != 0 && gameBoard[i+xpos][j+ypos] != 0){
                gameover = true;
                return;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece[i][j] = newPiece[i][j];
        }
    }
}

void createGameBoard() {
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            gameBoard[i][j] = 0;
        }
    }
}

