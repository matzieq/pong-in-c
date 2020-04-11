#include <GLFW/glfw3.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UP -1
#define DOWN 1
#define TRUE 1
#define FALSE 0
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

// const int screenWidth = 1280;
// const int screenHeight = 720;
const int gameWidth = 800;
const int gameHeight = 600;

typedef struct Paddle {
  Vector2 pos;
  Vector2 size;
  float speed;
} Paddle;

typedef struct Ball {
  Vector2 pos;
  Vector2 size;
  Vector2 speed;
} Ball;

typedef struct Game {
  Paddle player1;
  Paddle player2;
  Ball ball;
  int p1Score;
  int p2Score;
  int isGameOVer;
} Game;

Game initGame();
void drawObject(Vector2 pos, Vector2 size);
void drawPaddle(Paddle *paddle);
void drawBall(Ball *ball);
void movePaddle(Paddle *paddle, int dir);
void movePaddleWithModdedSpeed(Paddle *paddle, int dir, int speedMod);
void moveBall(Ball *ball);
void drawGame(Game *game);
void drawGameOver(Game *game);
void drawScore(Game *game);
void updateGame(Game *game);
void ai(Game *game);
void checkVictory(Game *game);
int isCollision(Paddle paddle, Ball ball);

int main() {
  time_t t;
  RenderTexture2D target;

  srand((unsigned)time(&t));

  InitWindow(gameWidth, gameHeight, "Pong");

  Game game = initGame();
  SetTargetFPS(60);

  SetTextureFilter(target.texture, 0);

  while (!WindowShouldClose()) {
    updateGame(&game);
    BeginDrawing();
    drawGame(&game);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}

Game initGame() {
  Paddle player1 = {{20, 20}, 10, 100, 6};
  Paddle player2 = {{gameWidth - 40, 20}, 10, 100, 6};
  Ball ball = {{gameWidth / 2, gameHeight / 2}, {10, 10}, {5, 5}};
  Game g = {player1, player2, ball, 0, 0, FALSE};
  return g;
}

void drawObject(Vector2 pos, Vector2 size) {
  DrawRectangleV(pos, size, RAYWHITE);
}

void drawPaddle(Paddle *paddle) { drawObject((*paddle).pos, (*paddle).size); }

void drawBall(Ball *ball) { drawObject((*ball).pos, (*ball).size); }

void movePaddle(Paddle *paddle, int dir) {
  (*paddle).pos.y += (*paddle).speed * ((dir > 0) - (dir < 0));
  (*paddle).pos.y = Clamp((*paddle).pos.y, 0, gameHeight - (*paddle).size.y);
}

void movePaddleWithModdedSpeed(Paddle *paddle, int dir, int speedMod) {
  int sgnDir = (dir > 0) - (dir < 0);
  speedMod *= sgnDir;
  (*paddle).pos.y += ((*paddle).speed + speedMod) * ((dir > 0) - (dir < 0));
  (*paddle).pos.y = Clamp((*paddle).pos.y, 0, gameHeight - (*paddle).size.y);
}

void moveBall(Ball *ball) {
  (*ball).pos = Vector2Add((*ball).pos, (*ball).speed);
}

void drawScore(Game *game) {
  DrawText(FormatText("%02i", (*game).p1Score), 20, 20, 20, RAYWHITE);
  DrawText(FormatText("%02i", (*game).p2Score), gameWidth - 40, 20, 20,
           RAYWHITE);
}

void drawGame(Game *game) {
  ClearBackground(BLACK);

  if ((*game).isGameOVer) {
    drawGameOver(game);
  } else {
    drawPaddle(&(*game).player1);
    drawPaddle(&(*game).player2);
    drawBall(&(*game).ball);
    drawScore(game);
  }
}

void updateGame(Game *game) {
  float collisionPos = 0;

  if ((*game).isGameOVer) {
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER)) {
      (*game) = initGame();
    }
    return;
  }

  checkVictory(game);

  if (IsKeyDown(KEY_DOWN)) {
    movePaddle(&(*game).player1, DOWN);
  }
  if (IsKeyDown(KEY_UP)) {
    movePaddle(&(*game).player1, UP);
  }

  ai(game);

  moveBall(&(*game).ball);

  if ((*game).ball.pos.y < 0) {
    (*game).ball.speed.y *= -1;
  }
  if ((*game).ball.pos.y > gameHeight - (*game).ball.size.y) {
    (*game).ball.speed.y *= -1;
  }

  if ((*game).ball.pos.x < 0) {
    (*game).ball.pos.x = gameWidth / 2;
    (*game).ball.pos.y = gameHeight / 2;
    (*game).ball.speed.x *= -1;
    (*game).p2Score++;
  }
  if ((*game).ball.pos.x > gameWidth - (*game).ball.size.x) {
    (*game).ball.pos.x = gameWidth / 2;
    (*game).ball.pos.y = gameHeight / 2;
    (*game).ball.speed.x *= -1;
    (*game).p1Score++;
  }

  if (isCollision((*game).player1, (*game).ball) && (*game).ball.speed.x < 0) {
    (*game).ball.speed.x *= -1;
    collisionPos =
        (*game).ball.pos.y - (*game).player1.pos.y - (*game).player1.size.y / 2;
  }

  if (isCollision((*game).player2, (*game).ball) && (*game).ball.speed.x > 0) {
    (*game).ball.speed.x *= -1;
    collisionPos =
        (*game).ball.pos.y - (*game).player2.pos.y - (*game).player2.size.y / 2;
  }

  if (collisionPos) {
    (*game).ball.speed.y = collisionPos / 7;
  }
}

int isCollision(Paddle paddle, Ball ball) {
  Rectangle paddleRect = {paddle.pos.x, paddle.pos.y, paddle.size.x,
                          paddle.size.y};
  Rectangle ballRect = {ball.pos.x, ball.pos.y, ball.size.x, ball.size.y};

  return CheckCollisionRecs(paddleRect, ballRect);
}

void ai(Game *game) {
  float ballY = (*game).ball.pos.y;
  float computerY = (*game).player2.pos.y;
  float paddleH = (*game).player2.size.y;
  // int speedMod = (rand() % 6) - 3;
  int speedMod = 0;

  int dir = 0;
  if (computerY < ballY) {
    dir = DOWN;
  }

  if (computerY + paddleH > ballY) {
    dir = UP;
  }

  movePaddleWithModdedSpeed(&(*game).player2, dir, speedMod);
}

void checkVictory(Game *game) {
  if ((*game).p1Score > 2 || (*game).p2Score > 2) {
    (*game).isGameOVer = TRUE;
  }
}

void drawGameOver(Game *game) {
  int whoWon = (*game).p1Score > 2 ? 1 : 2;
  DrawText(FormatText("Player %d won!", whoWon), gameWidth / 2 - 50,
           gameHeight / 2, 30, RAYWHITE);
}