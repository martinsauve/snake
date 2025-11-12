#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>


#define APPLE_MAX 32
#define BOARD_WIDTH 16
#define BOARD_HEIGHT 16
#define CASE_SIZE 32
#define SCREEN_WIDTH BOARD_WIDTH * CASE_SIZE
#define SCREEN_HEIGHT BOARD_HEIGHT * CASE_SIZE
#define TARGET_FPS 8.0
#define TARGET_SPF 1.0/TARGET_FPS

#define LEFTVEC   (Vector2){-1, 0}
#define RIGHTVEC  (Vector2){ 1, 0}
#define UPVEC     (Vector2){ 0,-1}
#define DOWNVEC   (Vector2){ 0, 1}

#define NOT_APPLE (Vector2){-1,-1}


typedef enum direction {
   LEFT,
   RIGHT,
   UP,
   DOWN
} Direction;


typedef struct Node {
   Vector2 position;
   struct Node* next;
   struct Node* prev;
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
   while (temp->next != NULL) {
      temp = temp->next;
   }
   temp->next = newNode;
   newNode->prev = temp;
}

void deleteAtBeginning(Node** head)
{
   if (*head == NULL) {
      printf("list already empty\n");
      return;
   }
   Node* temp = *head;
   *head = (*head)->next;
   if (*head != NULL) {
      (*head)->prev = NULL;
   }
   free(temp);
}

void deleteAtEnd(Node** head)
{
   if (*head == NULL) {
      printf("The list is already empty.\n");
      return;
   }

   Node* temp = *head;
   if (temp->next == NULL) {
      *head = NULL;
      free(temp);
      return;
   }
   while (temp->next != NULL) {
      temp = temp->next;
   }
   temp->prev->next = NULL;
   free(temp);
}

void drawSnake(Node* head)
{
   Node* temp = head;
   int i = 0;
   while (temp != NULL) {
      DrawRectangle(
            temp->position.x*CASE_SIZE, temp->position.y*CASE_SIZE,
            CASE_SIZE-1, CASE_SIZE-1,
            (Color){10*i, 228, 48, 255} // TODO: Lerp me
            );

      temp=temp->next;
      i++;
   }
}

void drawApples(Vector2* apples){
   for (int i = 0; i < APPLE_MAX; i++) {
      if (!Vector2Equals(apples[i], NOT_APPLE)){
         DrawCircle(apples[i].x*CASE_SIZE+CASE_SIZE/2.0f, apples[i].y*CASE_SIZE+CASE_SIZE/2.0f, CASE_SIZE/2.0f, RED);
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



int main(void)
{

   Node* snake = NULL;
   insertAtEnd(&snake, (Vector2){15,15});

   Vector2 apples[APPLE_MAX];
   for (int i = 0; i < APPLE_MAX; i++) {
      apples[i] = NOT_APPLE;
   }

   apples[0] = (Vector2){17,17};
   apples[1] = (Vector2){12,12};

   Direction direction = RIGHT;

   InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SNAKE");

   double oldTime = 0;
   double newTime = 0;
   double dt = 0;
   int key;
   while (!WindowShouldClose())
   {
      key = GetKeyPressed();
      if (key == KEY_RIGHT && direction != LEFT ) direction = RIGHT;
      if (key == KEY_UP    && direction != DOWN ) direction = UP;
      if (key == KEY_LEFT  && direction != RIGHT) direction = LEFT;
      if (key == KEY_DOWN  && direction != UP   ) direction = DOWN;

      oldTime = newTime;
      newTime = GetTime();
      dt += newTime-oldTime;
      BeginDrawing();
      DrawFPS(15,15);

      if (dt >= TARGET_SPF) dt = 0;
      else goto endofloop;

      ClearBackground(RAYWHITE);

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

      for (int i = 0; i < APPLE_MAX; i++) {
         if (!Vector2Equals(apples[i], (Vector2){-1, -1})){
            if (Vector2Equals(apples[i], snake->position)) {
               apples[i] = (Vector2){GetRandomValue(0, BOARD_WIDTH-1), GetRandomValue(0, BOARD_HEIGHT-1)};
               goto draw;
            }
         }
      }
      deleteAtEnd(&snake);
draw:
      handleWrapAround(&snake);
      drawApples(apples);
      drawSnake(snake);
      EndDrawing();

endofloop:
   }

   CloseWindow();
   return (0);
}
