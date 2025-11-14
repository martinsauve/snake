#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


#define APPLE_MAX    32
#define BOARD_WIDTH  16
#define BOARD_HEIGHT 16
#define CASE_SIZE    32
#define APPLE_SIZE   CASE_SIZE / 2.0f * (2.0f / 3.0f)

#define SCREEN_WIDTH  BOARD_WIDTH  * CASE_SIZE
#define SCREEN_HEIGHT BOARD_HEIGHT * CASE_SIZE

#define SCREEN_FPS 60.0
#define TARGET_FPS 6.0
#define TARGET_SPF 1.0 / TARGET_FPS

#define LEFTVEC   (Vector2){-1, 0}
#define RIGHTVEC  (Vector2){ 1, 0}
#define UPVEC     (Vector2){ 0,-1}
#define DOWNVEC   (Vector2){ 0, 1}

#define NOT_APPLE (Vector2){-1,-1}

#define SNAKE_SPAWN_POINT (Vector2){8,8}

#define SNAKE_HEAD_COLOR (Color){  0, 228,  48, 255}
#define SNAKE_TAIL_COLOR (Color){255, 228,  48, 255}


typedef enum direction {
   LEFT,
   RIGHT,
   UP,
   DOWN
} Direction;

typedef struct Node {
   Vector2 position;
   struct  Node* next;
   struct  Node* prev;
} Node;



Node* createNode(Vector2 position)
{
   Node *newNode = (Node*)malloc(sizeof(Node));
   newNode->position = position;
   newNode->next = NULL;
   newNode->prev = NULL;
   return newNode;
}

void insertAtBeginning(Node** head, Vector2 position)
{
   Node * newNode = createNode(position);

   if (*head == NULL) {
      *head = newNode;
      return;
   }
   newNode->next = *head;
   (*head)->prev = newNode;
   *head = newNode;
}

void insertAtEnd(Node** head, Vector2 position)
{
   Node* newNode = createNode(position);

   if (*head == NULL) {
      *head = newNode;
      return;
   }

   Node* temp = *head;
   while (temp->next != NULL) temp = temp->next;
   temp->next = newNode;
   newNode->prev = temp;
}

void deleteAtBeginning(Node** head)
{
   if (*head == NULL) return;
   Node* temp = *head;
   *head = (*head)->next;
   if (*head != NULL) {
      (*head)->prev = NULL;
   }
   free(temp);
}

size_t snakeLen(Node** head)
{
   size_t i = 1;
   Node *temp = *head;

   while (temp->next != NULL) {
      temp = temp->next;
      i++;
   }
   return i;
}

void deleteAtEnd(Node** head)
{
   if (*head == NULL) return;

   Node* temp = *head;
   if (temp->next == NULL) {
      *head = NULL;
      free(temp);
      return;
   }
   while (temp->next != NULL) temp = temp->next;
   temp->prev->next = NULL;
   free(temp);
}

Color lerpColor(Color col1, Color col2, float amount)
{
   Color out;

   out.r = Lerp(col1.r, col2.r, amount);
   out.g = Lerp(col1.g, col2.g, amount);
   out.b = Lerp(col1.b, col2.b, amount);
   out.a = Lerp(col1.a, col2.a, amount);
   return out;
}

void drawSnake(Node* head)
{
   Node* temp = head;
   int i = 0;
   size_t len = snakeLen(&head);
   float amount = 0;

   while (temp != NULL) {
      amount = (float)i / (float)len;
      DrawRectangle(
            temp->position.x * CASE_SIZE,
            temp->position.y * CASE_SIZE,
            CASE_SIZE,
            CASE_SIZE,
            lerpColor(SNAKE_HEAD_COLOR, SNAKE_TAIL_COLOR, amount)
            );

      temp=temp->next;
      i++;
   }
}

void drawApples(Vector2* apples)
{
   for (int i = 0; i < APPLE_MAX; i++) {
      if (!Vector2Equals(apples[i], NOT_APPLE)){
         DrawCircle(
               apples[i].x * CASE_SIZE + CASE_SIZE / 2.0f,
               apples[i].y * CASE_SIZE + CASE_SIZE / 2.0f,
               APPLE_SIZE,
               RED
               );
      }
   }
}

void handleWrapAround(Node** head)
{
   if (*head == NULL) return;

   if ((*head)->position.x < 0)
      (*head)->position.x = BOARD_WIDTH - 1;
   else if ((*head)->position.x >= BOARD_WIDTH)
      (*head)->position.x = 0;

   if ((*head)->position.y < 0)
      (*head)->position.y = BOARD_HEIGHT - 1;
   else if ((*head)->position.y >= BOARD_HEIGHT)
      (*head)->position.y = 0;
}

bool handleColision(Node** head)
{
   if (*head == NULL) return false;

   Node* temp = (*head)->next;
   while (temp != NULL) {
      if (Vector2Equals((*head)->position, temp->position)) return true;
      temp = temp->next;
   }
   return false;
}

void spawnSnake(Node** head, Vector2 position, size_t length)
{
   for (size_t i = 0; i < length; i++) {
      insertAtEnd(head, (Vector2){position.x - i, position.y});
   }
}

// accepts snake to avoid spawning apple under it
Vector2 spawnApple(Node** snake)
{
   assert (snake != NULL);
   Node* temp;
   Vector2 apple;

   // game is won
   if (snakeLen(snake) >= BOARD_WIDTH * BOARD_HEIGHT) return NOT_APPLE;
tryagain:
   temp = *snake;
   apple = (Vector2){
      GetRandomValue(0, BOARD_WIDTH-1),
      GetRandomValue(0, BOARD_HEIGHT-1)
   };

   while (temp != NULL) {
      if (Vector2Equals(apple, temp->position)) goto tryagain;
      temp=temp->next;
   }
   return apple;
}


int main(void)
{
   SetRandomSeed(time(NULL));
   SetConfigFlags(FLAG_MSAA_4X_HINT);
   InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SNAKE");
   SetTargetFPS(SCREEN_FPS);

   Vector2 apples[APPLE_MAX];
   for (int i = 0; i < APPLE_MAX; i++) apples[i] = NOT_APPLE;
   apples[0] = (Vector2){ 6, 4};
   apples[1] = (Vector2){12,12};

   Node* snake = NULL;
   spawnSnake(&snake, SNAKE_SPAWN_POINT, 3);

   Direction direction = RIGHT;

   double oldTime = 0;
   double newTime = 0;
   double dt      = 0;

   int key;

   while (!WindowShouldClose())
   {
      key = GetKeyPressed();
      if (key == KEY_RIGHT && direction != LEFT ) direction = RIGHT;
      if (key == KEY_UP    && direction != DOWN ) direction = UP;
      if (key == KEY_LEFT  && direction != RIGHT) direction = LEFT;
      if (key == KEY_DOWN  && direction != UP   ) direction = DOWN;

      oldTime =  newTime;
      newTime =  GetTime();
      dt     +=  newTime - oldTime;

      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawFPS(15,15);

      if (dt >= TARGET_SPF) dt = 0;
      else continue;


      switch (direction){
         case UP:
            insertAtBeginning(&snake, Vector2Add(snake->position, UPVEC));
            break;
         case DOWN:
            insertAtBeginning(&snake, Vector2Add(snake->position, DOWNVEC));
            break;
         case LEFT:
            insertAtBeginning(&snake, Vector2Add(snake->position, LEFTVEC));
            break;
         case RIGHT:
            insertAtBeginning(&snake, Vector2Add(snake->position, RIGHTVEC));
            break;
      }

      handleWrapAround(&snake);

      if (handleColision(&snake)) {
         // reset snake
         while (snake != NULL) {
            deleteAtBeginning(&snake);
         }
         spawnSnake(&snake, SNAKE_SPAWN_POINT, 3);
         direction = RIGHT;
         goto draw;
      }

      for (int i = 0; i < APPLE_MAX; i++) {
         if (!Vector2Equals(apples[i], NOT_APPLE)){
            if (Vector2Equals(apples[i], snake->position)) {
               apples[i] = spawnApple(&snake);
               goto draw;
            }
         }
      }
      deleteAtEnd(&snake);

draw:
      drawApples(apples);
      drawSnake(snake);
      DrawText(TextFormat("Length: %zu", snakeLen(&snake)), 200, 15, 20, BLACK);
      EndDrawing();

   }

   CloseWindow();
   return 0;
}
