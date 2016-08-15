#ifndef INTERFACE_H
#define INTERFACE_H

#include <curses.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>

static const float ROOT2INV = 0.70710678118;

struct Float2D {float x; float y;} railCollPoint;

struct {
	int globalW;
	int globalH; 
	int statusX; 
	int statusY;
	int gameX;
	int gameY;
	int gameW;
	int gameH;
	int paddleH;
	int gravityX;
	int gravityY;
	int railX1;
	int railX2;
	int railY1;
	int railY2;
	int railW;
	float spawnV;
} window;


struct {
	int x;
	int y;
	int v;
} userPaddle, botPaddle;

struct Projectile {
	float x;
	float y;
	float dx;
	float dy;	
} ball;

struct Hitbox { 
	int x1;
	int y1;
	int x2;
	int y2;
} topWall, leftWall, bottomWall, rightWall, topRailHb, botRailHb;

struct {
	int lives;
	int score;
} stats;

enum {HELP, OVER, LEVEL1, LEVEL2, LEVEL3, LEVEL4} gameStatus;
extern bool game_over;

bool *topRailArray;
bool *botRailArray;

int keyboardTimeout;
int countdownTimeout;
int gravityTimeout;
int gravityIconSwitcher;
int ignorePaddle;
int elapsedTime;
time_t prevEpochTime;
bool ignoreTime;

void initWindow();
void handleKey(int c);
void nextLevel();
void initGame(bool preserveStats);	
void stepGame();

void drawGame();
void drawBorder();
void drawStatus();
void drawPaddles();
void drawBall();
void drawRails();
void drawGravity();

void showHelp();
void showOver();
void quitGame();


float magnitude(float x, float y);
bool whereColliding(struct Hitbox hb, struct Projectile vec, struct Float2D *point);
bool isColliding(struct Hitbox hb, struct Projectile vec);
bool isIntersecting(struct Hitbox hb, float x, float y);

#endif