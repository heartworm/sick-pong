#include "interface.h"

int countdownTimeout = 30;
int gravityTimeout = 500;
int ignorePaddle = 0;
int gravityIconSwitcher = 5;
int elapsedTime = 0;
time_t prevEpochTime = 0;
bool ignoreTime = false;

void initWindow() { //values that stay static as soon as window size is known
	window.globalW = screen_width();
	window.globalH = screen_height();
	window.gameX = 1;
	window.gameY = 3;
	window.gameW = window.globalW - 2;
	window.gameH = window.globalH - 4;
	window.statusX = 1;
	window.statusY = 1;
	
	//All in game cartesian system from here relative to gameX gameY
	
	window.paddleH = window.globalH < 21 ? 
		round((window.gameH - 1) / 2) : 7;
	window.gravityX = round((window.gameW + 1) / 2 - 1);
	window.gravityY = round((window.gameH + 1) / 2 - 1);
	
	window.spawnV = magnitude(window.gameH, window.gameW) / 300.0;
	
	//excesses of 100 to deal with overshoot conditions. 	
	topWall = (struct Hitbox){window.gameX - 100, window.gameY - 100, window.gameW + 100, -1};
	rightWall = (struct Hitbox){window.gameW, -100, window.gameW + 100, window.gameH + 100};
	bottomWall = (struct Hitbox){window.gameX - 100, window.gameH, window.gameW + 100, window.gameH + 100};
	leftWall = (struct Hitbox){window.gameX - 100, window.gameY - 100, window.gameX - 1, window.gameH + 100};
	
	window.railX1 = round((window.gameW / 4.0)) - 1;
	window.railX2 = round(((window.gameW / 4.0)*3.0)) -1;
	window.railY1 = round((window.gameH / 3.0)) - 1;
	window.railY2 = ceil(((window.gameH / 3.0)*2)) - 1;
	
	topRailHb = (struct Hitbox){window.railX1, window.railY1, window.railX2, window.railY1};
	botRailHb = (struct Hitbox){window.railX1, window.railY2, window.railX2, window.railY2};
	
	window.railW = window.railX2 - window.railX1 + 1;
	topRailArray = malloc(window.railW * sizeof(bool));
	botRailArray = malloc(window.railW * sizeof(bool));

	gameStatus = HELP;
}

void initGame(bool preserveStats) { //values to be reset upon round start. 
	countdownTimeout = 30;
	gravityTimeout = 500;

	userPaddle.y = round((window.gameH - window.paddleH) / 2);
	userPaddle.x = 2;	
	userPaddle.v = 0;
	
	botPaddle.y = userPaddle.y; 
	botPaddle.x = window.gameW - 3;
	
	srand(time(NULL));
	float halfRandMax = RAND_MAX / 2.0;
	float randDec = (rand() - halfRandMax) / halfRandMax;
	
	float v = window.spawnV;
	
	ball.x = round((window.gameW + 1) / 2 - 1);
	ball.y = round((window.gameH + 1) / 2 - 1);
	
	//root2inv*v is max value of dy (at 45deg)
	ball.dy = randDec * v * ROOT2INV;
	ball.dx = -sqrt(v*v - ball.dy*ball.dy);
	
	if (!preserveStats) {
		stats.score = 0;
		stats.lives = 3; 
		
		for (int x = 0; x < window.railW; x++) {
			topRailArray[x] = true;
			botRailArray[x] = true;
		}
	}
}
	
void drawBorder() { 
	unsigned char borderChar = '*'; //ascii block
	int x1 = 0;
	int x2 = window.globalW - 1;
	int y1 = 0;
	int y2 = window.globalH - 1;
	draw_line(x1, y1, x2, y1, borderChar);
	draw_line(x2, y1, x2, y2, borderChar);
	draw_line(x2, y2, x1, y2, borderChar);
	draw_line(x1, y2, x1, y1, borderChar);
	draw_line(x1, window.gameY-1, x2, window.gameY-1, borderChar);
}

void handleKey(int c) { //handle keys from main.c, needs to be int not char
	if (c == 'q') quitGame();
	switch (gameStatus) {
	case LEVEL4:
	case LEVEL3:
	case LEVEL2:
	case LEVEL1:
		if (c == KEY_UP) userPaddle.v = -1;
		else if (c == KEY_DOWN) userPaddle.v = 1;
		else if (c == 'h') gameStatus = HELP;
	case OVER:
	case HELP:
		if (c == 'l') nextLevel();
	}
}

void stepGame() {
	if (gameStatus >= LEVEL1) {
		if (countdownTimeout > 0) {
			countdownTimeout--;
			time(&prevEpochTime);
			return;
		}
		
		time_t newTime = time(NULL);
		if (!ignoreTime) elapsedTime += newTime - prevEpochTime;
		prevEpochTime = newTime;
		ignoreTime = false;
		
		if (gameStatus == LEVEL3) {
			if (gravityTimeout > 0) {
				gravityTimeout--;
			} else {
				
				float aGravity = window.spawnV / 2.0;
				float yDist = window.gravityY - ball.y;
				float xDist = window.gravityX - ball.x;
				
				float dist = magnitude(yDist, xDist);
				//limit r2 to avoid black hole effect
				float r2 = dist*dist < 1 ? 1 : dist*dist; 
				float accel = aGravity / (dist < 1 ? 1 : dist); 
				
				float aY = accel * (yDist / dist);
				float aX = accel * (xDist / dist);
				
				float newDx = ball.dx + aX;
				float newDy = ball.dy + aY;
				float newV = magnitude(newDx, newDy);
				
				//limiting v to speed of light
				if (newV > 1) {
					newDy = newDy / newV;
					newDx = newDx / newV;
				}
				
				//no reflections and no extreme slowdowns
				if (newDx * ball.dx < 0 || fabs(newDx) < 0.3) {
					newDy = ball.dy;
					newDx = ball.dx;
				}
			
				ball.dy = newDy;
				ball.dx = newDx;
			}
		}
		
		//Move user paddle		
		int maxY = window.gameH - window.paddleH;
		int newY = userPaddle.y + userPaddle.v;
		if (newY <= maxY && newY >= 0) {
			userPaddle.y = newY;
		}
		userPaddle.v = 0;
		
		//anticipate ball movement
		float newBallX = ball.x + ball.dx;
		float newBallY = ball.y + ball.dy;
		
		int rNewX = round(newBallX);
		int rNewY = round(newBallY);
		int rOldX = round(ball.x);
		int rOldY = round(ball.y);
		
		
		struct Hitbox paddleHb = (struct Hitbox){userPaddle.x, userPaddle.y, 
			userPaddle.x, userPaddle.y + window.paddleH - 1};
		
		struct Hitbox botHb = (struct Hitbox){botPaddle.x, botPaddle.y,
			botPaddle.x, botPaddle.y + window.paddleH - 1};
					
		//use lazy eval to chuck correct coord in the point variable
		bool railColl = whereColliding(botRailHb, ball, &railCollPoint) ||
			whereColliding(topRailHb, ball, &railCollPoint);
		
		if (isColliding(topWall, ball) ||
				isColliding(bottomWall, ball)) {
			ball.dy = -1 * ball.dy;
		} else if (isColliding(rightWall, ball)) {
			ball.dx = -1 * ball.dx;
		} else if (isColliding(paddleHb, ball) && !(ignorePaddle > 0)) {
			stats.score++;
			//hits the bottom moving upward and from below the paddle
			if (ball.dy < 0 && paddleHb.y2 <= (window.gameH - 3) && rOldY > paddleHb.y2) {
				ball.dy = -1 * ball.dy;
				ball.x -= 1;
			//hits the top moving downward and from above the paddle
			} else if (ball.dy > 0 && paddleHb.y1 >= 2 && rOldY < paddleHb.y1) {
				ball.dy = -1 * ball.dy;
				ball.x -= 1;
			} else {
				ball.dx = -1 * ball.dx;
			}
			ignorePaddle = 4; //there should be no cirumstance where the ball
			//collides for more than one tick. so ignore for a short while. 
			//solves ball sticking due to endpoint rules. 
			//number of ticks ignored is this value - 1
		} else if (isColliding(leftWall, ball)) {
			if (--stats.lives < 0) {
				gameStatus = OVER;
				initGame(false);
			} 
			initGame(true);
		} else if (gameStatus >= LEVEL2 && isColliding(botHb, ball)) {
			ball.dx = -1 * ball.dx;
		} else if (gameStatus == LEVEL4 && railColl) {
			int railIndex = railCollPoint.x - window.railX1;
			bool *correctRail = railCollPoint.y == window.railY1 ? topRailArray : botRailArray;
			if (correctRail[railIndex]) {
				if (rNewX >= window.railX1 && rNewX <= window.railX2 && rOldY != railCollPoint.y) {
					ball.dy = -1 * ball.dy;	
				} else {
					ball.dx = -1 * ball.dx;
				}
				correctRail[railIndex] = false;
			} else {
				ball.x = newBallX;
				ball.y = newBallY;
			}
		} else {
			//move ball
			ball.x = newBallX;
			ball.y = newBallY;
		}
		
		//ignore paddle
		if (ignorePaddle > 0) {
			ignorePaddle -= 1;
		}
		
		//move bot paddle
		if (gameStatus >= LEVEL2) {
			int newBotY = ball.y - round((window.paddleH - 1) / 2);
			int maxBotY = window.gameH - window.paddleH;
			if (newBotY >= 0 && newBotY <= maxBotY) {
				botPaddle.y = newBotY;
			}
		}
	} else time(&prevEpochTime);
}

void drawPaddles() {
	unsigned char paddleChar = 178;
	int offX = window.gameX;
	int offY  = window.gameY;
	int uX = userPaddle.x + offX;
	int uY1 = userPaddle.y + offY;
	int uY2 = uY1 - 1 + window.paddleH;
	draw_line(uX, uY1, uX, uY2, paddleChar);
	
	if (gameStatus >= LEVEL2) {	
		int bX = botPaddle.x + offX;
		int bY1 = botPaddle.y + offY;
		int bY2 = bY1 - 1 + window.paddleH;
		draw_line(bX, bY1, bX, bY2, paddleChar);
	}
}

void drawBall() {
	char ballChar = ball.dx > 0 ? '>' : '<';
	draw_char(round(ball.x + window.gameX), round(ball.y + window.gameY), ballChar);
}

void drawStatus() {
	// char* status = NULL;
	// if (gameStatus == HELP) status = "Help Screen";
	// else if (gameStatus == OVER) status = "Game Over";
	// else if (gameStatus == LEVEL1) status = "Level 1";
	// else if (gameStatus == LEVEL2) status = "Level 2";
	// else if (gameStatus == LEVEL3) status = "Level 3";
	// else if (gameStatus == LEVEL4) status = "Level 4";
	// draw_string(window.statusX, window.statusY, status);	
	
	int mins = elapsedTime/60;
	int secs = elapsedTime % 60;
	
	if (gameStatus >= LEVEL1) {
		if (countdownTimeout > 0) {
			draw_formatted(window.statusX, window.statusY, "Game starts in... %.1f", countdownTimeout / 10.0);
		} else {
			draw_formatted(window.statusX, window.statusY, "Score: %d  -  Lives: %d  -  Level: %d  -  Elapsed: %.2d:%.2d",
				stats.score, stats.lives, (int)gameStatus - OVER, mins, secs);
		}
	}
}

void drawGravity() {
	int x = window.gravityX + window.gameX;
	int y = window.gravityY + window.gameY;
	if (gravityTimeout <= 0) {
		if (gravityIconSwitcher >= 30) draw_line(x, y-3, x, y+3, '~');
		else if (gravityIconSwitcher >= 20) draw_line(x-3, y-3, x+3, y+3, '~');
		else if (gravityIconSwitcher >= 10) draw_line(x-5, y, x+5, y, '~');
		else draw_line(x+3, y-3, x-3, y+3, '~');
		gravityIconSwitcher = (gravityIconSwitcher + 1) % 40;
	}
}

void drawRails() {
	char railChar = '#';
	int x1 = window.railX1 + window.gameX;
	int x2 = window.railX2 + window.gameX;
	int y1 = window.railY1 + window.gameY;
	int y2 = window.railY2 + window.gameY;
	
	for (int n = 0; n < window.railW; n++) {
		if (topRailArray[n]) draw_char(n + x1, y1, railChar);
		if (botRailArray[n]) draw_char(n + x1, y2, railChar);
	}
	
}

void showHelp() {
	ignoreTime = true;
	clear_screen();
	draw_string(0, 0, "Shravan Lal, n9286675");
	draw_string(0, 1, "Move Paddle: Up and Down Arrow Keys");
	draw_string(0, 2, "Change Level: L Key");
	draw_string(0, 3, "Pause Game / Show Help: H Key");
	draw_string(0, 4, "Quit Game: Q Key");
	draw_string(0, 5, "Press any key to hide help...");
	show_screen();
	wait_char();
	gameStatus = LEVEL1;
}

void showOver() {
	ignoreTime = true;
	draw_string(window.gameX, window.gameY, "Game Over. Press any key to retry...");
	show_screen();
	wait_char();
	gameStatus = HELP;
}

void drawGame() { //after step, draw the arena	
	clear_screen();
	drawBorder();
	drawStatus();
	switch (gameStatus) {
	case LEVEL4:
		drawRails();
	case LEVEL3: 
		drawGravity();
	case LEVEL2: 
	case LEVEL1: 
		drawPaddles();
		drawBall();
		break;
	case OVER:
		showOver();
		return;
	case HELP:
		showHelp();
		break;
	}
	show_screen();
}

void nextLevel() {
	gameStatus++;
	if (gameStatus > LEVEL4 || gameStatus < LEVEL1) gameStatus = LEVEL1;
	initGame(true);
}

void quitGame() { //since we allocating on heap ffs
	game_over = true;
	free(topRailArray);
	free(botRailArray);
}

float magnitude(float x, float y) {
	return sqrt(x*x + y*y);
}


bool isColliding(struct Hitbox hb, struct Projectile vec) {
	return whereColliding(hb, vec, NULL);
}

bool whereColliding(struct Hitbox hb, struct Projectile vec, struct Float2D *point) {
	//iterative collision logic required for high speeds
	// where object may be skipped by the projectile 
	// but still in the way	
	
	float x1 = vec.x;
	float y1 = vec.y;
	float x2 = vec.x + vec.dx;
	float y2 = vec.y + vec.dy;
	
	bool collX = false;
	bool collY = false;
	
	int collXPoint = -100000;
	int collYPoint = -100000;
	
	int rX1 = round(x1 < x2 ? x1 : x2);
	int rX2 = round(x1 < x2 ? x2 : x1);
	int rY1 = round(y1 < y2 ? y1: y2);
	int rY2 = round(y1 < y2 ? y2: y1);
	
	for (int x = rX1; x <= rX2; x++) {
		if (x >= hb.x1 && x <= hb.x2) {
			collX = true;
			collXPoint = fabs(x - vec.x) < fabs(collXPoint - vec.x) ? x : collXPoint;
		}
	}
	
	for (int y = rY1; y <= rY2; y++) {
		if (y >= hb.y1 && y <= hb.y2) {
			collY = true;
			collYPoint = fabs(y - vec.y) < fabs(collYPoint - vec.y) ? y : collYPoint;
		}
	}
	
	if (collX && collY) {		
		if (point != NULL) {
			point->x = collXPoint;
			point->y = collYPoint;
		}
		return true;
	}
	return false;
}

bool isIntersecting(struct Hitbox hb, float x, float y) {
	int rX = round(x);
	int rY = round(y);
	bool collX = rX >= hb.x1 && rX <= hb.x2;
	bool collY = rY >= hb.y1 && rY <= hb.y2;
	return collX && collY;  
}