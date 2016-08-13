#include "interface.h"

int keyboardTimeout = 0;
int countdownTimeout = 30;
int gravityTimeout = 100;
int ignorePaddle = 0;

void initWindow() { //values that stay static as soon as window size is known
	window.globalW = screen_width();
	window.globalH = screen_height();
	window.gameX = 1;
	window.gameY = 4;
	window.gameW = window.globalW - 2;
	window.gameH = window.globalH - 5;
	window.statusX = 1;
	window.statusY = 1;
	
	//All in game cartesian system from here relative to gameX gameY
	
	window.paddleH = window.gameH < 21 ? 
		floor(window.gameH / 2) : 7;
	window.gravityX = round((window.gameW + 1) / 2 - 1);
	window.gravityY = round((window.gameH + 1) / 2 - 1);
	
	window.spawnV = magnitude(window.gameH, window.gameW) / 300.0;
	
	//excesses of 100 to deal with overshoot conditions. 	
	topWall = (struct Hitbox){window.gameX - 100, window.gameY - 100, window.gameW + 100, -1};
	rightWall = (struct Hitbox){window.gameW, -100, window.gameW + 100, window.gameH + 100};
	bottomWall = (struct Hitbox){window.gameX - 100, window.gameH, window.gameW + 100, window.gameH + 100};
	leftWall = (struct Hitbox){window.gameX - 100, window.gameY - 100, window.gameX - 1, window.gameH + 100};
	
	gameStatus = HELP;
}

void initGame(bool preserveStats) { //values to be reset upon round start. 
	countdownTimeout = 30;
	gravityTimeout = 100;

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

void handleKey(char c) { //handle keys from main.c
	switch (gameStatus) {
	case LEVEL4:
	case LEVEL3:
	case LEVEL2:
	case LEVEL1:
		if (c == 'a') userPaddle.v = -1;
		else if (c == 'z') userPaddle.v = 1;
		else if (c == 'q') gameStatus = HELP;
	case OVER:
	case HELP:
		if (c == 'l') nextLevel();
	}
}

void stepGame() {
	if (gameStatus >= LEVEL1) {
		if (countdownTimeout > 0) {
			countdownTimeout--;
			return;
		}
		if (keyboardTimeout == 0) {
			keyboardTimeout = 0;
			
			//Move user paddle		
			int maxY = window.gameH - window.paddleH;
			int newY = userPaddle.y + userPaddle.v;
			if (newY <= maxY && newY >= 0) {
				userPaddle.y = newY;
			}
			userPaddle.v = 0;
			
		} else {
			keyboardTimeout--;
		}	
		if (gameStatus == LEVEL3) {
			if (gravityTimeout > 0) {
				gravityTimeout--;
			} else {
				
				float aGravity = window.spawnV / 3;
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
				
				if (newV > 1) {
					newDy = newDy / newV;
					newDx = newDx / newV;
				}
				//allows reflections but orbit is impossible
				//since outside of a small radius a 0.6unit/s2 decelleration
				//is close to impossible. 
				if (newDx * ball.dx < 0 || fabs(newDx) < 0.3) {
					newDy = ball.dy;
					newDx = ball.dx;
				}
				
				// if (fabs(newDy / newV) > ROOT2INV &&
					// fabs(aY / aX) > ROOT2INV &&
					// aY * newDy > 0) { //limit to about 45deg from horiz
					// newDy = ball.dy;	
				// }
				
				ball.dy = newDy;
				ball.dx = newDx;
			}
		}
		//move ball 
		float newBallX = ball.x + ball.dx;
		float newBallY = ball.y + ball.dy;
		
		
		struct Hitbox paddleHb = (struct Hitbox){userPaddle.x, userPaddle.y, 
			userPaddle.x, userPaddle.y + window.paddleH - 1};
		
		struct Hitbox botHb = (struct Hitbox){botPaddle.x, botPaddle.y,
			botPaddle.x, botPaddle.y + window.paddleH - 1};
			
		bool ignorePaddleSet = false;
		
		if (isColliding(topWall, ball) ||
				isColliding(bottomWall, ball)) {
			ball.dy = -1 * ball.dy;
		} else if (isColliding(rightWall, ball)) {
			ball.dx = -1 * ball.dx;
		} else if (isColliding(paddleHb, ball) && !(ignorePaddle > 0)) {
			stats.score++;
			//hits the bottom moving upward and from below the paddle
			if (ball.dy < 0 && paddleHb.y2 <= (window.gameH - 3) && ball.y > paddleHb.y2) {
				ball.dy = -1 * ball.dy;
				ignorePaddleSet = true;
			//hits the top moving downward and from above the paddle
			} else if (ball.dy > 0 && paddleHb.y1 >= 2 && ball.y < paddleHb.y1) {
				ball.dy = -1 * ball.dy;
				ignorePaddleSet = true;
			} else {
				ball.dx = -1 * ball.dx;
			}
			ignorePaddle = 3; //there should be no cirumstance where the ball
			//collides for more than one tick. so ignore for a short while. 
			//solves ball sticking due to endpoint rules. 
		} else if (isColliding(leftWall, ball)) {
			if (--stats.lives < 0) {
				gameStatus = OVER;
				initGame(false);
			} 
			initGame(true);
		} else if (gameStatus >= LEVEL2 && isColliding(botHb, ball)) {
			ball.dx = -1 * ball.dx;
		} else {
			ball.x = newBallX;
			ball.y = newBallY;
		}
		
		if (ignorePaddle && !ignorePaddleSet) {
			ignorePaddle -= 1;
		}
		
		if (gameStatus >= LEVEL2) {
			int newBotY = ball.y - round((window.paddleH - 1) / 2);
			int maxBotY = window.gameH - window.paddleH;
			if (newBotY >= 0 && newBotY <= maxBotY) {
				botPaddle.y = newBotY;
			}
		}
	}
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
	//draw_formatted(1, 1, "x: %f, y: %f", ball.x, ball.y);
}

void drawStatus() {
	char* status = NULL;
	if (gameStatus == HELP) status = "Help Screen";
	else if (gameStatus == OVER) status = "Game Over";
	else if (gameStatus == LEVEL1) status = "Level 1";
	else if (gameStatus == LEVEL2) status = "Level 2";
	else if (gameStatus == LEVEL3) status = "Level 3";
	else if (gameStatus == LEVEL4) status = "Level 4";
	draw_string(window.statusX, window.statusY, status);	
	
	if (gameStatus >= LEVEL1) {
		if (countdownTimeout > 0) {
			draw_formatted(window.statusX, window.statusY + 1, "Game starts in... %.1f", countdownTimeout / 10.0);
		} else {
			draw_formatted(window.statusX, window.statusY + 1, "Score: %d  -  Lives: %d", stats.score, stats.lives);
		}
	}
}

void drawGravity() {
	if (gravityTimeout <= 0) draw_char(window.gravityX + window.gameX, window.gravityY + window.gameY, '~');
}

void showHelp() {
	draw_string(window.gameX, window.gameY, "Shravan Lal, n9286675");
	draw_string(window.gameX, window.gameY + 1, "Move Paddle: Up and Down Arrow Keys");
	draw_string(window.gameX, window.gameY + 2, "Change Level: L Key");
	draw_string(window.gameX, window.gameY + 3, "Pause Game / Show Help: Q Key");
	draw_string(window.gameX, window.gameY + 4, "Press any key to hide help...");
	show_screen();
	wait_char();
	gameStatus = LEVEL1;
}

void showOver() {
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
	initGame(false);
}

float magnitude(float x, float y) {
	return sqrt(x*x + y*y);
}


bool isColliding(struct Hitbox hb, struct Projectile vec) {
	//iterative collision logic required for high speeds
	// where object may be skipped by the projectile 
	// but still in the way	
	
	float x1 = vec.x;
	float y1 = vec.y;
	float x2 = vec.x + vec.dx;
	float y2 = vec.y + vec.dy;
	
	bool collX = false;
	bool collY = false;
	
	int rX1 = round(x1 < x2 ? x1 : x2);
	int rX2 = round(x1 < x2 ? x2 : x1);
	int rY1 = round(y1 < y2 ? y1: y2);
	int rY2 = round(y1 < y2 ? y2: y1);
	
	for (int x = rX1; x <= rX2; x++) {
		if (x >= hb.x1 && x <= hb.x2) collX = true;
	}
	
	for (int y = rY1; y <= rY2; y++) {
		if (y >= hb.y1 && y <= hb.y2) collY = true;
	}
	
	return collX && collY;
	
}

bool isIntersecting(struct Hitbox hb, float x, float y) {
	int rX = round(x);
	int rY = round(y);
	bool collX = rX >= hb.x1 && rX <= hb.x2;
	bool collY = rY >= hb.y1 && rY <= hb.y2;
	return collX && collY;  
}