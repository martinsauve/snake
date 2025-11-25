#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

//#define DEBUG
#define SHOW_FPS

#define APPLE_MAX    32
#define BOARD_WIDTH  16
#define BOARD_HEIGHT 16
#define CASE_SIZE    32
#define APPLE_SIZE   CASE_SIZE / 2.0f * (2.0f / 3.0f)

#define SCREEN_WIDTH  BOARD_WIDTH  * CASE_SIZE
#define SCREEN_HEIGHT BOARD_HEIGHT * CASE_SIZE
#define RL_WINDOW_CONFIG_FLAGS (FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT)

#define SCREEN_FPS 120.0
#define TARGET_FPS 6.0
#define TARGET_SPF 1.0 / TARGET_FPS

#define LEFTVEC   (Vector2){-1, 0}
#define RIGHTVEC  (Vector2){ 1, 0}
#define UPVEC     (Vector2){ 0,-1}
#define DOWNVEC   (Vector2){ 0, 1}

#define NOT_APPLE (Vector2){-1,-1}

#define SNAKE_SPAWN_POINT (Vector2){8,8}

//#define SNAKE_HEAD_COLOR (Color){  0, 228,  48, 255}
//#define SNAKE_TAIL_COLOR (Color){255, 228,  48, 255}
#define SNAKE_HEAD_COLOR ORANGE
#define SNAKE_TAIL_COLOR PURPLE

//#98971a
#define APPLE_COLOR      (Color){0x98, 0x97, 0x1a, 0xff}
#define BACKGROUND_COLOR (Color){0x28, 0x28, 0x28, 0xff}
#define TEXT_COLOR       (Color){0xeb, 0xdb, 0xb2, 0xff}


#define KEYBIND_LEFT       (key == KEY_LEFT  || key == KEY_H  || key == KEY_A)
#define KEYBIND_DOWN       (key == KEY_DOWN  || key == KEY_J  || key == KEY_S)
#define KEYBIND_UP         (key == KEY_UP    || key == KEY_K  || key == KEY_W)
#define KEYBIND_RIGHT      (key == KEY_RIGHT || key == KEY_L  || key == KEY_D)
typedef enum direction {
   LEFT,
   RIGHT,
   UP,
   DOWN
} Direction;

typedef struct data {
   Vector2 position;
   Direction direction;
} Data;

typedef struct Node {
   Data data;
   struct  Node* next;
   struct  Node* prev;
} Node;



Node* createNode(Data data)
{
   Node *newNode = (Node*)malloc(sizeof(Node));
   newNode->data = data;
   newNode->next = NULL;
   newNode->prev = NULL;
   return newNode;
}

void insertAtBeginning(Node** head, Data data)
{
   Node * newNode = createNode(data);

   if (*head == NULL) {
      *head = newNode;
      return;
   }
   newNode->next = *head;
   (*head)->prev = newNode;
   *head = newNode;
}

void insertAtEnd(Node** head, Data data)
{
   Node* newNode = createNode(data);

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


Color lerpColor(Color col1, Color col2, float amount)
{
   return (Color) {
      .r = Lerp(col1.r, col2.r, amount),
      .g = Lerp(col1.g, col2.g, amount),
      .b = Lerp(col1.b, col2.b, amount),
      .a = Lerp(col1.a, col2.a, amount)
   };
}

void drawEyes(Vector2 position, Direction direction)
{
   Vector2 eyeOffset1, eyeOffset2;
   switch (direction) {
      case UP:
         eyeOffset1 = (Vector2){CASE_SIZE * 0.2f, CASE_SIZE * 0.4f};
         eyeOffset2 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.4f};
         break;
      case DOWN:
         eyeOffset1 = (Vector2){CASE_SIZE * 0.2f, CASE_SIZE * 0.6f};
         eyeOffset2 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.6f};
         break;
      case LEFT:
         eyeOffset1 = (Vector2){CASE_SIZE * 0.4f, CASE_SIZE * 0.2f};
         eyeOffset2 = (Vector2){CASE_SIZE * 0.4f, CASE_SIZE * 0.6f};
         break;
      case RIGHT:
         eyeOffset1 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.2f};
         eyeOffset2 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.6f};
         break;
      default:
         eyeOffset1 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.2f};
         eyeOffset2 = (Vector2){CASE_SIZE * 0.6f, CASE_SIZE * 0.6f};
         break;
   }
   DrawCircle(
         position.x * CASE_SIZE + eyeOffset1.x,
         position.y * CASE_SIZE + eyeOffset1.y,
         CASE_SIZE * 0.1f,
         BLACK
         );
   DrawCircle(
         position.x * CASE_SIZE + eyeOffset2.x,
         position.y * CASE_SIZE + eyeOffset2.y,
         CASE_SIZE * 0.1f,
         BLACK
         );
}

void drawSnake(Node* head, float dt)
{
   if (head == NULL) return;
   Node* temp = head;
   int i = 0;
   size_t len = snakeLen(&head);
   float amount = 0;
   float amountNext = 0;
   Vector2 eyePos;

   while (temp != NULL) {
      amount = 1.0f - ((float)i / (float)len);
      amountNext = 1.0f - ((float)(i + 1) / (float)len);



      // HEAD SMOOTH MOVEMENT
      if (i == 0) // HEAD
      {
         switch (temp->data.direction) {
            case LEFT:
               DrawRectangleGradientH(
                     ((temp->data.position.x * CASE_SIZE) - CASE_SIZE*dt) + CASE_SIZE+2,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE*dt,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amount),
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amountNext)
                     );
               eyePos = (Vector2){(temp->data.position.x - dt + 1), temp->data.position.y};
               break;
            case RIGHT:
               DrawRectangleGradientH(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE*dt,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amountNext),
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amount)
                     );
               eyePos = (Vector2){(temp->data.position.x + dt - 1), temp->data.position.y};
               break;
            case UP:
               DrawRectangleGradientV(
                     temp->data.position.x * CASE_SIZE,
                     ((temp->data.position.y * CASE_SIZE) - CASE_SIZE*dt) + CASE_SIZE+2,
                     CASE_SIZE,
                     CASE_SIZE*dt,
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amount),
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amountNext)
                     );
               eyePos = (Vector2){temp->data.position.x, (temp->data.position.y - dt + 1 )};
               break;
            case DOWN:
               DrawRectangleGradientV(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE,
                     CASE_SIZE*dt,
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amountNext),
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amount)
                     );
               eyePos = (Vector2){temp->data.position.x, (temp->data.position.y + dt - 1 )};
               break;
         }
      } else { // BODY AND TAIL
         switch (temp->data.direction) {
            case LEFT:
               DrawRectangleGradientH(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amount),
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amountNext)
                     );
               break;
            case RIGHT:
               DrawRectangleGradientH(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amountNext),
                     lerpColor(SNAKE_TAIL_COLOR,SNAKE_HEAD_COLOR, amount)
                     );
               break;
            case UP:
               DrawRectangleGradientV(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amount),
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amountNext)
                     );
               break;
            case DOWN:
               DrawRectangleGradientV(
                     temp->data.position.x * CASE_SIZE,
                     temp->data.position.y * CASE_SIZE,
                     CASE_SIZE,
                     CASE_SIZE,
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amountNext),
                     lerpColor(SNAKE_TAIL_COLOR, SNAKE_HEAD_COLOR, amount)
                     );
               break;
         }
      }


      //// No gradient version
      //DrawRectangle(
      //      temp->data.position.x * CASE_SIZE,
      //      temp->data.position.y * CASE_SIZE,
      //      CASE_SIZE,
      //      CASE_SIZE,
      //      lerpColor(SNAKE_HEAD_COLOR, SNAKE_TAIL_COLOR, amount)
      //      );

      temp=temp->next;
      i++;
   }
   drawEyes(eyePos, head->data.direction);
}



void drawApples(Vector2* apples)
{
   for (int i = 0; i < APPLE_MAX; i++) {
      if (!Vector2Equals(apples[i], NOT_APPLE)){
         DrawCircle(
               apples[i].x * CASE_SIZE + CASE_SIZE / 2.0f,
               apples[i].y * CASE_SIZE + CASE_SIZE / 2.0f,
               APPLE_SIZE,
               APPLE_COLOR
               );
      }
   }
}

void handleWrapAround(Node** head)
{
   if (*head == NULL) return;

   if ((*head)->data.position.x < 0)
      (*head)->data.position.x = BOARD_WIDTH - 1;
   else if ((*head)->data.position.x >= BOARD_WIDTH)
      (*head)->data.position.x = 0;

   if ((*head)->data.position.y < 0)
      (*head)->data.position.y = BOARD_HEIGHT - 1;
   else if ((*head)->data.position.y >= BOARD_HEIGHT)
      (*head)->data.position.y = 0;
}

bool handleColision(Node** head)
{
   if (*head == NULL) return false;

   Node* temp = (*head)->next;
   while (temp != NULL) {
      if (Vector2Equals((*head)->data.position, temp->data.position)) return true;
      temp = temp->next;
   }
   return false;
}

void spawnSnake(Node** head, Data data, size_t length)
{
   for (size_t i = 0; i < length; i++) {
      data = (Data){
         .position = (Vector2){data.position.x - i, data.position.y},
         .direction = RIGHT
      };
      insertAtEnd(head, data);
   }
}

// accepts snake to avoid spawning apple under it
Vector2 spawnApple(Node** snake)
{
   assert (snake != NULL);
   Node* temp;
   Vector2 apple;
   bool colision = false;

   if (snakeLen(snake) >= BOARD_WIDTH * BOARD_HEIGHT) return NOT_APPLE;

   do {
      colision = false;
      temp = *snake;
      apple = (Vector2){
         GetRandomValue(0, BOARD_WIDTH-1),
         GetRandomValue(0, BOARD_HEIGHT-1)
      };
      while (temp != NULL) {
         if (Vector2Equals(apple, temp->data.position)) {
            colision = true;
#ifdef DEBUG
            printf("cant spawn apple here, covered by snake: x:%f, y:%f, \n", temp->data.position.x, temp->data.position.y);
            fflush(stdout);
#endif
         }
         temp=temp->next;
      }
   } while (colision);
   return apple;
}


void computePhysics(Node **snake, Vector2* apples, Direction direction)
{
   switch (direction){
      case UP:
         insertAtBeginning(snake, (Data){Vector2Add((*snake)->data.position, UPVEC), UP});
         break;
      case DOWN:
         insertAtBeginning(snake, (Data){Vector2Add((*snake)->data.position, DOWNVEC), DOWN});
         break;
      case LEFT:
         insertAtBeginning(snake, (Data){Vector2Add((*snake)->data.position, LEFTVEC), LEFT});
         break;
      case RIGHT:
         insertAtBeginning(snake, (Data){Vector2Add((*snake)->data.position, RIGHTVEC), RIGHT});
         break;
   }

   handleWrapAround(snake);

   if (handleColision(snake)) {
      // reset snake
      while (*snake != NULL) {
         deleteAtBeginning(snake);
      }
      spawnSnake(snake, (Data){SNAKE_SPAWN_POINT, RIGHT}, 3);
      return;
   }

   for (int i = 0; i < APPLE_MAX; i++) {
      if (!Vector2Equals(apples[i], NOT_APPLE)){
         if (Vector2Equals(apples[i], (*snake)->data.position)) {
            apples[i] = spawnApple(snake);
            return;
         }
      }
   }
   deleteAtEnd(snake);
   return;
}


int main(void)
{
   // RL SETUP
#ifdef TOUCH_SUPPORT
   SetGesturesEnabled(
         GESTURE_SWIPE_DOWN &
         GESTURE_SWIPE_LEFT &
         GESTURE_SWIPE_UP   &
         GESTURE_SWIPE_RIGHT
         );
#endif
   SetRandomSeed(time(NULL));
   SetConfigFlags(RL_WINDOW_CONFIG_FLAGS);
   InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SNAKE");
   SetTargetFPS(SCREEN_FPS);

   Node* snake = NULL;
   spawnSnake(&snake, (Data){SNAKE_SPAWN_POINT, RIGHT}, 3);

   Vector2 apples[APPLE_MAX];
   for (int i = 0; i < APPLE_MAX; i++) apples[i] = NOT_APPLE;
   apples[0] = spawnApple(&snake);
   apples[1] = spawnApple(&snake);
   apples[2] = spawnApple(&snake);

   Direction direction = RIGHT;

   double oldTime = 0;
   double newTime = 0;
   double dt      = 0;

   int key;

   while (!WindowShouldClose())
   {
      key = GetKeyPressed();
      if (KEYBIND_RIGHT && snake->data.direction != LEFT ) direction = RIGHT;
      if (KEYBIND_UP    && snake->data.direction != DOWN ) direction = UP;
      if (KEYBIND_LEFT  && snake->data.direction != RIGHT) direction = LEFT;
      if (KEYBIND_DOWN  && snake->data.direction != UP   ) direction = DOWN;
#ifdef TOUCH_SUPPORT
      if (IsGestureDetected(GESTURE_SWIPE_RIGHT) && direction != LEFT)
         direction = RIGHT;
      if (IsGestureDetected(GESTURE_SWIPE_UP) && direction != DOWN)
         direction = UP;
      if (IsGestureDetected(GESTURE_SWIPE_LEFT) && direction != RIGHT)
         direction = LEFT;
      if (IsGestureDetected(GESTURE_SWIPE_DOWN) && direction != UP)
         direction = DOWN;
#endif

      oldTime =  newTime;
      newTime =  GetTime();
      dt     +=  newTime - oldTime;

      if (dt >= TARGET_SPF) {
         computePhysics(&snake, apples, direction);
         dt = 0;
      }
      float ndt = dt * TARGET_FPS; // normalized dt for smooth rendering

      BeginDrawing();
      {
         //DrawText(TextFormat("dt: %f", dt), 50, 50, 10, BEIGE);
         ClearBackground(BACKGROUND_COLOR);
         //ClearBackground((Color){255*dt*6, 255-255*dt*6, 255, 255}); // EPILEPSY MODE
         drawApples(apples);
         drawSnake(snake, ndt);
         DrawText(TextFormat("Length: %zu", snakeLen(&snake)), 200, 15, 20, TEXT_COLOR);
#ifdef SHOW_FPS
         DrawFPS(15,15);
#endif
      }
      EndDrawing();

   }

   CloseWindow();
   return 0;
}
