#include "LedControlMS.h"


//defined variables
#define MATRIX_SIZE 8
#define LEVELS_NUMBER 7
#define LICKERING_DELAY 300
#define MOVING_TIME_PAUSE 300
#define TEXT_DURATION 2000
#define MAX_ENEMIES 3
#define ENEMY_SPEED 1000


//a structure that saves the coordinates of any object
typedef struct
{
  int x;
  int y;
}coordinate;


//a level structure, used to save every information we need for a specific level
typedef struct
{
  int matrix[MATRIX_SIZE][MATRIX_SIZE];
  coordinate player_start;
  coordinate player_previous_start;
  coordinate next_level;
  coordinate previous_level;
  int enemies_number = 0;
  
}level; 


//global variables
int current_level;
coordinate player;                                    //player
level levels[LEVELS_NUMBER];                          //a structure array with every level
LedControl led_control=LedControl(12,11,10,1);        //we need this for the LED matrix
unsigned long player_lickering_time = millis();       //saves the current time
int player_lickering_status;
unsigned long player_can_move = millis(); 
int running = 1;
coordinate enemy[MAX_ENEMIES];
unsigned long enemy_can_move = millis(); 

byte level_matrix[LEVELS_NUMBER][MATRIX_SIZE]={       //here we can change the level format very easy and fast, robust
{B11111111, B10010001, B10010101, B11010101, B10000101, B11100100, B10000101, B11111111},
{B11110111, B10010101, B10010001, B10111101, B10000001, B00111001, B10010001, B11111111},
{B11101111, B10101001, B10001101, B10111101, B10000101, B10110101, B10010001, B11110111},
{B11111111, B10000101, B10100101, B00100001, B11111101, B10000101, B10100001, B11101111},
{B11111111, B10000001, B11110101, B10100100, B00101111, B10100001, B10001001, B11111111},
{B11111111, B10000001, B10000101, B10000101, B10000100, B10100001, B10100001, B10111111},
{B10111111, B10100001, B10000001, B10000001, B10000001, B11111101, B10000001, B11011111}};

byte level_text[LEVELS_NUMBER][MATRIX_SIZE]={       //this is printable text only
{0x00,0x08,0x18,0x08,0x08,0x08,0x08,0x1C},
{0x00,0x18,0x24,0x04,0x08,0x10,0x20,0x3C},
{0x00,0x3C,0x04,0x04,0x1C,0x04,0x04,0x3C},
{0x00,0x20,0x24,0x24,0x3C,0x04,0x04,0x04},
{0x00,0x3C,0x20,0x20,0x38,0x04,0x04,0x38},
{0x00,0x18,0x24,0x20,0x38,0x24,0x24,0x18},
{0x00,0x3C,0x04,0x08,0x08,0x10,0x10,0x10}};

byte ending_text[MATRIX_SIZE] = {0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C};
byte dead_text[MATRIX_SIZE] = {0x3C,0x42,0xA5,0x81,0x99,0xA5,0x42,0x3C};

int button_up_pin_number = 5;
int button_down_pin_number = 4;
int button_left_pin_number = 3;
int button_right_pin_number = 2;
int button_up = 5;
int button_down = 4;
int button_left = 3;
int button_right = 2;


//initialize every directional button as an imput
void initialize_buttons()
{
  pinMode(button_up_pin_number, INPUT);
  pinMode(button_down_pin_number, INPUT);
  pinMode(button_left_pin_number, INPUT);
  pinMode(button_right_pin_number, INPUT);
}


//initialize the matrix for use
void initialize_matrix()
{
  //activate the LED matrix
  led_control.shutdown(0,false);
  //set the brightness -> low brightness
  led_control.setIntensity(0, 0.5);
  //clear display
  led_control.clearDisplay(0);
}


//save the binary levels in the matrix for compute verifications
void initialize_levels()
{
  int mask = 0x01;
  int value;

  for(int lvl=0; lvl<LEVELS_NUMBER; lvl++)
    for(int i=0; i<MATRIX_SIZE; i++)
    
       //go through every bit from the byte row
       for(int bt=0; bt<MATRIX_SIZE; bt++)
       { 
          if((mask << bt) & level_matrix[lvl][i]) value = 1;
          else value = 0;
          
          //save the byte as an int in the level matrix
          levels[lvl].matrix[i][MATRIX_SIZE - bt - 1] = value;
       }
}


//here we individually initialize the locations of the player, ending and starting in each matrix
void initialize_level_locations()
{
  //initialize 1st level
  levels[0].player_start.x = 1;
  levels[0].player_start.y = 1;
  levels[0].next_level.x = 5;
  levels[0].next_level.y = 7;
  levels[0].player_previous_start.x = 5;
  levels[0].player_previous_start.y = 6;
  
  //initialize 2nd level
  levels[1].player_start.x = 5;
  levels[1].player_start.y = 1;
  levels[1].next_level.x = 0;
  levels[1].next_level.y = 4;
  levels[1].previous_level.x = 5;
  levels[1].previous_level.y = 0;
  levels[1].player_previous_start.x = 1;
  levels[1].player_previous_start.y = 4;

  //initialize 3rd level
  levels[2].player_start.x = 6;
  levels[2].player_start.y = 4;
  levels[2].next_level.x = 0;
  levels[2].next_level.y = 3;
  levels[2].previous_level.x = 7;
  levels[2].previous_level.y = 4;
  levels[2].player_previous_start.x = 1;
  levels[2].player_previous_start.y = 3;

  //initialize 4th level
  levels[3].player_start.x = 6;
  levels[3].player_start.y = 3;
  levels[3].next_level.x = 3;
  levels[3].next_level.y = 0;
  levels[3].previous_level.x = 7;
  levels[3].previous_level.y = 3;
  levels[3].player_previous_start.x = 3;
  levels[3].player_previous_start.y = 1;

  //initialize 5th level
  levels[4].player_start.x = 3;
  levels[4].player_start.y = 6;
  levels[4].next_level.x = 4;
  levels[4].next_level.y = 0;
  levels[4].previous_level.x = 3;
  levels[4].previous_level.y = 7;
  levels[4].player_previous_start.x = 4;
  levels[4].player_previous_start.y = 1;

  //initialize 6th level
  levels[5].player_start.x = 4;
  levels[5].player_start.y = 6;
  levels[5].next_level.x = 7;
  levels[5].next_level.y = 1;
  levels[5].previous_level.x = 4;
  levels[5].previous_level.y = 7;
  levels[5].player_previous_start.x = 6;
  levels[5].player_previous_start.y = 1;
  levels[5].enemies_number = 2;

  //initialize 7th level
  levels[6].player_start.x = 1;
  levels[6].player_start.y = 1;
  levels[6].next_level.x = 7;
  levels[6].next_level.y = 2;
  levels[6].previous_level.x = 0;
  levels[6].previous_level.y = 1;
  levels[6].enemies_number = 1;
}


//check if the entity can spawn in that location
int can_spawn_here(int x, int y)
{
  //if it can spawn there
  if(levels[current_level].matrix[x][y] == 0)
  {
    return 0;
  }
  return -1;
}


//initialize every enemy there is on the specific map
void initialize_enemies()
{
  for(int i=0; i<levels[current_level].enemies_number; i++)
  {
    int status = 0;
    while(status == 0)
    {
      int random_x = random(2, MATRIX_SIZE-2);
      int random_y = random(2, MATRIX_SIZE-2);
      
      if(can_spawn_here(random_x, random_y) == 0)
      {
        Serial.println("An enemy appeared out of blue");

        status = 1;
        enemy[i].x = random_x;
        enemy[i].y = random_y;
      }
    }
  }
}


//initialize the level given as a parameter -> this is used only for the start and NEXT level initialization
void initialize_current_level(int current)
{
  //we start with the first level -> [0, n)
  current_level = current;
  player.x = levels[current].player_start.x;
  player.y = levels[current].player_start.y;
  
  //initialize the enemies from the current level
  initialize_enemies();

  draw_level_number(current_level);
}


//initialize the previous level
void initialize_previous_level(int current)
{
  //we start with the first level -> [0, n)
  current_level = current;
  player.x = levels[current].player_previous_start.x;
  player.y = levels[current].player_previous_start.y;

  draw_level_number(current_level);
}


//draw the specified level on the screen
void draw_level(int current)
{
  for(int i=0; i<MATRIX_SIZE; i++)
    if(player.x != i)
      led_control.setRow(0,i,level_matrix[current][i]);
    else
    {
      for(int j=0; j<MATRIX_SIZE; j++)
        if(j != player.y)
          led_control.setLed(0, i, j, levels[current_level].matrix[i][j]);
    }
}


//draw the level number on the screen
void draw_level_number(int current)
{
  for(int i=0; i<MATRIX_SIZE; i++)
    led_control.setRow(0,i,level_text[current][i]);
  delay(TEXT_DURATION);
}


//this is the final text that will be drawn when the game will end
void draw_ending_text()
{
  for(int i=0; i<MATRIX_SIZE; i++)
    led_control.setRow(0,i,ending_text[i]);
  delay(TEXT_DURATION * 2);
}


//this is drawn on the screen when you are killed by an enemy
void draw_dead_text()
{
  for(int i=0; i<MATRIX_SIZE; i++)
    led_control.setRow(0,i,dead_text[i]);
  delay(TEXT_DURATION);
}


//set the player lickering on or off
void set_lickering_status()
{
  if(millis() - player_lickering_time > LICKERING_DELAY)
  {
    player_lickering_time = millis();
    if(player_lickering_status == 0) player_lickering_status = 1;
    else player_lickering_status = 0;
  }
}


//check if the player can move again
int move_timeout()
{
  if(millis() - player_can_move > MOVING_TIME_PAUSE)
  {
    player_can_move = millis();
    return 0;
  }
  return -1;
}


//draw the position of the player
void draw_player_position()
{ 
  set_lickering_status();
  led_control.setLed(0, player.x, player.y, player_lickering_status);
}


//draw every enemy on the screen
void draw_enemies()
{
  for(int i=0; i<levels[current_level].enemies_number; i++)
  {
    led_control.setLed(0, enemy[i].x, enemy[i].y, HIGH);
  }
}


//check if the player can move on the specified position
int can_move(int x, int y)
{
  if(x < 0 || y < 0 || x >= MATRIX_SIZE || y >= MATRIX_SIZE) return -1;
  if(levels[current_level].matrix[x][y] == 1) return -1;
  return 0;
}


//this function will move the player in the specified direction
void move_player()
{
  button_up = digitalRead(button_up_pin_number);
  if(button_up == HIGH)
  {
    if(move_timeout() == 0 && can_move(player.x-1, player.y) == 0)
    {
      Serial.println("move up");
      player.x--;
    }
    return;
  }

  button_down = digitalRead(button_down_pin_number);
  if(button_down == HIGH)
  {
    if(move_timeout() == 0 && can_move(player.x+1, player.y) == 0)
    {
      Serial.println("move down");
      player.x++;
    }
    return;
  }

  button_left = digitalRead(button_left_pin_number);
  if(button_left == HIGH)
  {
    if(move_timeout() == 0 && can_move(player.x, player.y-1) == 0)
    {
      Serial.println("move left");
      player.y--;
    }
    return;
  }

  button_right = digitalRead(button_right_pin_number);
  if(button_right == HIGH)
  {
    if(move_timeout() == 0 && can_move(player.x, player.y+1) == 0)
    {
      Serial.println("move right");
      player.y++;
    }
    return;
  }
}


//this closes all the leds
void close_all_leds()
{
  for(int i=0; i<MATRIX_SIZE; i++)
    led_control.setRow(0, i, 0);
}



//check if the enemy will move again after it's time passed
int enemy_move_timeout()
{
  if(millis() - enemy_can_move > ENEMY_SPEED)
  {
    enemy_can_move = millis();
    return 0;
  }
  return -1;
}


//move every enemy on the screen
void move_enemies()
{
  if(enemy_move_timeout() == 0)
  {
    for(int i=0; i<levels[current_level].enemies_number; i++)
    { 
      int status = 0;
      while(status == 0)
      {
        int enemy_direction = random(4);

        //move UP
        if(enemy_direction == 0 && can_move(enemy[i].x-1, enemy[i].y) == 0)
        {
          status = 1;
          enemy[i].x --;
        }
        //move DOWN
        else if(enemy_direction == 1 && can_move(enemy[i].x+1, enemy[i].y) == 0)
        {
          status = 1;
          enemy[i].x ++;
        }
        //move LEFT
        else if(enemy_direction == 2 && can_move(enemy[i].x, enemy[i].y-1) == 0)
        {
          status = 1;
          enemy[i].y --;
        }
        //move RIGHT
        else if(enemy_direction == 3 && can_move(enemy[i].x, enemy[i].y+1) == 0)
        {
          status = 1;
          enemy[i].y ++;
        } 
      }
    }
  }
}


//check if you moved from one level to another
void check_level_state()
{
  //checking for the next level
  if(player.x == levels[current_level].next_level.x && player.y == levels[current_level].next_level.y)
  {
    //check if it is the ending of the final level
    if(current_level >= LEVELS_NUMBER-1)
    {
      Serial.println("You won ! Heh.. Good for you :)");
      running = 0;
      draw_ending_text();
      close_all_leds();
    }
    else
    {
      //go to the next level
      initialize_current_level(current_level+1);
      Serial.print("reached next Level: ");
      Serial.println(current_level);
    }
  }

  //checking for the previous level
  else if(player.x == levels[current_level].previous_level.x && player.y == levels[current_level].previous_level.y)
  {
    initialize_previous_level(current_level-1);
    Serial.print("reached previous Level: ");
    Serial.println(current_level);
  }
}


//check if the player collided with an enemy, in which case, the game will be reset
void check_player_dead()
{
  for(int i=0; i<levels[current_level].enemies_number; i++)
  {
    //if the player collided with an enemy
    if(enemy[i].x == player.x && enemy[i].y == player.y)
    {
      Serial.println("Oops..looks like you died. Oh well, better luck next time");
      draw_dead_text();
      initialize_current_level(0);
    }
  }
}


//this functions manages every computational required per frame, before drawing the frame
void update_frame()
{
  check_level_state();
  move_player();
  move_enemies();
  check_player_dead();
  if(running == 0) exit(0);
}


//this functions manages every drawings on the screen
void render()
{
  //draw the current level format
  draw_level(current_level);
  draw_player_position();
  draw_enemies();
}


//this code will be ran only once at the beginning
void setup() 
{
  //we enable this baud value so we can see values in the serial monitor / testing purposes only
  Serial.begin(9600);

  //initialize everything we need just once
  initialize_matrix();
  initialize_levels();
  initialize_level_locations();
  initialize_buttons();
  initialize_current_level(0);
}


//main, loop for the whole length of the application
void loop() 
{
  update_frame();
  render();
}
