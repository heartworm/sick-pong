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
	int singularityX;
	int singularityY;
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
} topWall, leftWall, bottomWall, rightWall, topRail, bottomRail;

struct {
	int lives;
	int score;
} stats;

enum {HELP, OVER, LEVEL1, LEVEL2, LEVEL3, LEVEL4} gameStatus;

int keyboardTimeout;
int countdownTimeout;

void initWindow();
void handleKey(char c);
void nextLevel();
void initGame(bool preserveStats);	
void stepGame();

void drawGame();
void drawBorder();
void drawStatus();
void drawPaddles();
void drawBall();

void showHelp();
void showOver();

// bool isColliding(struct Hitbox hb, float x, float y);
// bool isIntersecting(int hX, int hY, float x, float y);
bool isColliding(struct Hitbox hb, struct Projectile vec);
bool isIntersecting(struct Hitbox hb, float x, float y);

//void stepPositions();

#endif