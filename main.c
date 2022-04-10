/*****************************************************************
* Name        : <Zach Wilson>
* Program     : Program <10>
* Class       : ENGE 320
* Date        : <12/6/16>
* Description : <Atari Breakout>
* =============================================================
* Intro screen has name/options: (10) ____
* Game screen prints lives, high score, score: (20) ____
* High score is maintained between games: (10) ____
* Ball bounces properly off of wall/bricks: (30) ____
* Ball bounces variably off of paddle: (20) ____
* Ball only erases one brick for each hit: (10) ____
* Paddle moves correctly in horizontal: (10) ____
* Paddle moves correctly in vertical: (10) ____
* Sounds are emitted at the appropriate time and don’t affect game play: (20) ____
* Sounds are varied for different elements: (10) ____
* Game continues play faster when bricks are clear: (10) ____
* Game plays demo mode after delay: (30) ____
* Game lives operate correctly: (10) ____
* Total = ______ / 200
*****************************************************************/

//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "video.h"
#include "font.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "adc.h"
#include "button.h"
#include "pwm.h"

#define BTN_ACTIVE (PORT_PA08)
#define BTN_ACTIVE_GROUP (0)
#define BTN_ACTIVE_PIN (PIN_PA08%32)

// In Reg
#define BTN_ACTIVE_VAL ((PORT->Group[BTN_ACTIVE_GROUP].IN.reg) & BTN_ACTIVE)

/* some RGB color definitions                                                 */
#define BLACK           (0x0000)      /*   0,   0,   0 */
#define NAVY            (0x000F)      /*   0,   0, 128 */
#define DARK_GREEN      (0x03E0)      /*   0, 128,   0 */
#define DARK_CYAN       (0x03EF)      /*   0, 128, 128 */
#define MAROON          (0x7800)      /* 128,   0,   0 */
#define PURPLE          (0x780F)      /* 128,   0, 128 */
#define OLIVE           (0x7BE0)      /* 128, 128,   0 */
#define LIGHT_GREY      (0xC618)      /* 192, 192, 192 */
#define DARK_GREY       (0x7BEF)      /* 128, 128, 128 */
#define BLUE            (0x001F)      /*   0,   0, 255 */
#define GREEN           (0x07E0)      /*   0, 255,   0 */
#define CYAN            (0x07FF)      /*   0, 255, 255 */
#define RED             (0xF800)      /* 255,   0,   0 */
#define MAGENTA         (0xF81F)      /* 255,   0, 255 */
#define YELLOW          (0xFFE0)      /* 255, 255,   0 */
#define WHITE           (0xFFFF)      /* 255, 255, 255 */
#define ORANGE          (0xFD20)      /* 255, 165,   0 */
#define GREEN_YELLOW    (0xAFE5)      /* 173, 255,  47 */
#define PINK            (0xF81F)
#define BRIGHT_GREEN    (0x37E0)

// Note Frequencies:
#define C  (261.16)
#define Cs (277.18)
#define D  (293.66)
#define Ds (311.13)
#define E  (329.63)
#define F  (349.23)
#define Fs (369.99)
#define G  (392.00)
#define Gs (415.30)
#define A  (440.00)
#define As (466.16)
#define B  (493.88)
#define B3 (252)
#define Gs3 (209)

// These border definitions:
#define LEFT_BORDER (10)
#define RIGHT_BORDER (166)
#define BOTTOM_BORDER (220)
#define TOP_BORDER (50)
#define PADDLE_UPWARD_BOUND (110)
#define PADDLE_LOWER_BOUND (215)

// Ball Dimensions
#define BALL_DIAMETER (8)

// Ball Init
#define BALL_X (125)
#define BALL_Y (125)
#define BALL_X_SPEED (2)
#define BALL_Y_SPEED (2)
#define BALL_MAX (5)

// Paddle Dimensions
#define PADDLE_LENGTH (30)
#define PADDLE_WIDTH (5)
#define PADDLE_SPEED (2)

// Brick Dimensions
#define BRICK_LENGTH (24)
#define BRICK_WIDTH (5)
#define BRICK_X (11)
#define  BRICK_Y (61)

// Game Play Defaults
#define LIVES (5)
#define LEVEL_1_POINTS (1)
#define LEVEL_2_POINTS (4)
#define LEVEL_3_POINTS (7)

//-----------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//-----------------------------------------------------------------------------

static volatile uint32_t millis;

// Screen Adjustment Flags
uint8_t title = 0;
uint8_t normal = 0;
uint8_t paint_normal = 0;
uint8_t advanced = 0;
uint8_t paint_advanced = 0;
uint8_t demo = 0;
uint8_t paint_demo = 0;
uint8_t game_over = 0;
uint8_t paint_game_over = 0;
static volatile uint32_t demo_time = 0;

// Ball Characteristics
uint8_t ball_dead = 0;
uint8_t x = 0;
uint8_t y = 0;
int8_t x_speed = 0;
int8_t y_speed = 0;
uint8_t x_neg = 0;
uint8_t y_neg = 0;
uint8_t demo_x_pos = 0;
uint8_t demo_y_pos = 0;
uint8_t demo_dead = 0;
uint8_t ball_max_speed = 0;

// Paddle
uint8_t paddle_x = 75;
uint8_t paddle_y = 210;
uint16_t paddle_color = WHITE;
uint8_t joystick_right = 0;
uint8_t joystick_left = 0;
uint8_t joystick_up = 0;
uint8_t joystick_down = 0;

// ADC Values
uint8_t x_left = 0;
uint8_t x_right = 0;
uint8_t y_up = 0;
uint8_t y_down = 0;
uint16_t adc_x_val = 0;
uint16_t adc_y_val = 0;
uint8_t adc_case = 0;

// Score Tickers
uint8_t update_score = 0; // prevents severe lag
uint8_t dec_lives = 0;
uint16_t total_score = 0;

static volatile uint8_t score_1 = 0;
static volatile uint8_t score_2 = 0;
static volatile uint8_t score_3 = 0;
uint16_t total_high_score = 0;
uint8_t high_score_1 = 0;
uint8_t high_score_2 = 0;
uint8_t high_score_3 = 0;
uint8_t lives = LIVES;
uint8_t level = 0;

// Bricks
uint8_t brick_make_x = 0;
uint8_t brick_make_y = 0;
uint16_t brick_make_color = 0;
uint8_t brick_i = 0;
uint8_t brick_e = 0;
uint8_t brick_struck = 0;
uint8_t all_bricks_gone = 0; // flag that won't be reset to 0 if all bricks are dead
uint8_t bricks_alive[36] = {
  1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1
};   
uint8_t bricks_x[6] = {11, 37, 63, 89, 115, 141};      
uint8_t bricks_y[6] = {61, 68, 75, 82, 89, 96};        
uint16_t bricks_color[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE};

// Buttons
uint8_t button_val = 0;

// Sound defaults
uint8_t paddle_sound = 0;
uint8_t wall_sound = 0;
uint8_t row1_sound = 0;
uint8_t row2_sound = 0;
uint8_t row3_sound = 0;
uint8_t row4_sound = 0;
uint8_t row5_sound = 0;
uint8_t row6_sound = 0;
static volatile uint32_t sound_time = 0;


//-----------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//-----------------------------------------------------------------------------

void joystick();
void adc_check();
void paddle_move();
void paint_score();
void paint_high_score();
void paint_lives();
void decrement_lives();
void increment_score();
void increment_high_score();
void paint_bricks();
void kill_brick();
void check_bricks();
void kill_score();
void kill_high_score();
void kill_lives();
void sounds();

//-----------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//-----------------------------------------------------------------------------

//=============================================================================

int main(void)
{  
  // uint8_t i = 0;
  uint16_t color = 0x0100;
  
  /* Initialize the SAM system */
  SystemInit();

  // Initialize the board
  video_init();
  adc_init();
  buttons_init();
  pwm_init();


  SysTick_Config(48000); //  Configure the SysTick timer for a ms
  // Configure the Arduino LED
  REG_PORT_DIR0 |= PORT_PA17;  
  REG_PORT_OUTSET0 = PORT_PA17;
  
  // These are the default fonts
  font_t*font1, *font2, *font3, *font4;
  font1 = font_get(FONT_6x8);
  font2 = font_get(FONT_8x8);
  font3 = font_get(FONT_8x12);
  font4 = font_get(FONT_12x16); 

  update_score = 1;
  
  uint8_t step = 0;
  uint64_t prev_millis;
  
  // Initial Ball Conditions
  x_speed = BALL_X_SPEED;
  y_speed = BALL_Y_SPEED;
  x = BALL_X;
  y = BALL_Y;
  
  // Initial Paddle Conditions
  paddle_x = 75;
  paddle_y = 210;
  paddle_color = WHITE;  
  
  // Make ADC Value Accurate
  update_adc();
  for (int r = 0; r < 100; r ++)
  {
    ;
  }
  
  update_adc();
  for (int r = 0; r < 100; r ++)
  {
    ;
  }
  
  update_adc();
  for (int r = 0; r < 100; r ++)
  {
    ;
  }  
    
  score_3 = 0;
  score_2 = 0;
  score_1 = 0;
  
  high_score_3 = 0;
  high_score_2 = 0;
  high_score_1 = 0;
  
  lives = (LIVES + 1);
  title = 1;
  
  while (1) 
  { 
    // Check for shift to Title Screen
    if (title)
    {
      video_paint_rect(0, 0, 176, 220, BLACK);
      video_paint_rect(0, 0, 176, 220, WHITE);
      video_paint_rect(10, 10, 156, 220, BLACK);
      video_paint_rect(0, 40, 176, 10, WHITE);
      video_paint_rect(0, 0, 176, 40, BLACK);      
      video_print_string("BREAK OUT", font4, 35, 10, RED, BLACK);
      paint_bricks();
      video_print_sprite(70,110);
      video_print_string("Hit Left", font1, 22, 175, RED, BLACK);
      video_print_string("Button", font1, 31, 185, RED, BLACK);
      video_print_string("For Normal", font1, 19, 195, RED, BLACK);
      video_print_string("Hit Right", font1, 101,175, RED, BLACK);
      video_print_string("Button", font1, 109,185, RED, BLACK);
      video_print_string("For Advanced", font1, 92, 195, RED, BLACK);
      video_paint_rect(75, 210, 30, 5, WHITE);
      lives = (LIVES);
      score_1 = 0;
      score_2 = 0;
      score_3 = 0;
      total_score = 0;
      level = 0;
       x = BALL_X;
       y = BALL_Y;
       x_speed = BALL_X_SPEED;
       y_speed = BALL_Y_SPEED;
       paddle_x = 75;
       paddle_y = 210;
       demo_time = 0;
       sound_time = 0;
      pwm_disable();
      row1_sound = 0;
      row2_sound = 0;
      row3_sound = 0;
      row4_sound = 0;
      row5_sound = 0;
      row6_sound = 0;
      paddle_sound = 0;
      wall_sound = 0;        
      
      demo_time = millis;
      while(!normal && !demo && !advanced && !game_over)
      {
        if ((millis - demo_time) > 5500)
        {
          demo = 1;
          paint_demo = 1;
          title = 0;
        }
        else if (BTN_ACTIVE_VAL)
        {
          button_val = button_read();
          if (button_val == 1)
          {
            advanced = 1;
            paint_advanced = 1;
          }
          if (button_val == 3)
          {
            normal = 1;
            paint_normal = 1;
        }
        title = 0;
        }
      }
    }  
    /////////////////////////////////////////////
    // Normal game play
    if (paint_normal)
    {
      paint_normal = 0;
      normal = 1;
      
      // Paint the background and borders
      video_paint_rect(0, 0, 176, 220, WHITE);
      video_paint_rect(10, 10, 156, 220, BLACK);
      video_paint_rect(0, 40, 176, 10, WHITE);
      video_paint_rect(0, 0, 176, 40, BLACK);
      paint_bricks();
  
      // Paint the display
      video_print_string("High Score", font1, 0, 0, BLUE, BLACK);
      video_print_string("Lives", font1, 75, 0, BLUE, BLACK);
      video_print_string("Score", font1, 135, 0, BLUE, BLACK);
      increment_score();
      increment_high_score();
      paint_lives();
      paint_score();
      paint_high_score();
      x_speed = (BALL_X_SPEED + level);
      y_speed = (BALL_Y_SPEED + level);
      x = BALL_X;
      y = BALL_Y;
      paddle_x = 75;
      paddle_y = 210;
      // revive all bricks
      for (int h = 0; h < 36; h++)
      {
        bricks_alive[h] = 1;
      }
   }
   
    /////////////////////////////////////////////
    // demo game play
    if (paint_demo)
    {
      paint_demo = 0;
      demo = 1;
      
      // Paint the background and borders
      video_paint_rect(0, 0, 176, 220, WHITE);
      video_paint_rect(10, 10, 156, 220, BLACK);
      video_paint_rect(0, 40, 176, 10, WHITE);
      video_paint_rect(0, 0, 176, 40, BLACK);
      paint_bricks();
  
      // Paint the display
      video_print_string("High Score", font1, 0, 0, BLUE, BLACK);
      video_print_string("Lives", font1, 75, 0, BLUE, BLACK);
      video_print_string("Score", font1, 135, 0, BLUE, BLACK);
      increment_score();
      score_1 = 0;
      score_2 = 0;
      score_3 = 0;
      x = BALL_X;
      y = BALL_Y;
      total_score = 0;
      increment_high_score();
      paint_lives();
      paint_score();
      paint_high_score();
      // revive all bricks
      for (int h = 0; h < 36; h++)
      {
        bricks_alive[h] = 1;
      }
   }   
   
    /////////////////////////////////////////////
    // Normal game play
    if (paint_advanced)
    {
      paint_advanced = 0;
      advanced = 1;
      
      // Paint the background and borders
      video_paint_rect(0, 0, 176, 220, WHITE);
      video_paint_rect(10, 10, 156, 220, BLACK);
      video_paint_rect(0, 40, 176, 10, WHITE);
      video_paint_rect(0, 0, 176, 40, BLACK);
      paint_bricks();
  
      // Paint the display
      video_print_string("High Score", font1, 0, 0, BLUE, BLACK);
      video_print_string("Lives", font1, 75, 0, BLUE, BLACK);
      video_print_string("Score", font1, 135, 0, BLUE, BLACK);
      increment_score();
      increment_high_score();
      paint_lives();
      paint_score();
      paint_high_score();
      x_speed = (BALL_X_SPEED + level);
      y_speed = (BALL_Y_SPEED + level);
      x = BALL_X;
      y = BALL_Y;
      paddle_x = 75;
      paddle_y = 210;
      // revive all bricks
      for (int h = 0; h < 36; h++)
      {
        bricks_alive[h] = 1;
      }
    }   
   
 
    /////////////////////////////////////////////
    // Game Over
    if (game_over)
    {
      total_score = 0;
      demo = 0;
      paint_demo = 0;
      normal = 0;
      paint_normal = 0;
      advanced = 0;
      brick_struck = 0;
      all_bricks_gone = 0;
      paint_advanced = 0;
      score_1 = 0;
      score_2 = 0;
      score_3 = 0;
      lives = (LIVES + 1); // because it's decremented
      level = 0; //. this increases ball speed
       
       if (paint_game_over)
       {
         // Paint the background and borders
         video_paint_rect(0, 0, 176, 220, BLACK);
         video_print_sprite(70,110);
      
         // Paint the display
         video_print_string("My Foot", font4, 40, 70, RED, BLACK);
         video_print_string("Is Shaking...", font4, 20, 90, RED, BLACK); 
         video_print_string("Press any button to continue", font1, 4, 210, GREEN, BLACK);        
       }
       demo_time = millis;
      while(game_over)
      {
        if ((millis - demo_time) > 5500)
        {
          title = 1;
          game_over = 0;
          paint_game_over = 0;
        }
        if (BTN_ACTIVE_VAL)
        {
         game_over = 0;
         paint_game_over = 0;
         
         title = 1;
        }
      }
       
    }    
    
    
    // Check ADC Value
    adc_check();    
    ball_move();
    joystick();
    paddle_move();
    if ((millis - sound_time) > 50)
    {
      pwm_disable();
      row1_sound = 0;
      row2_sound = 0;
      row3_sound = 0;
      row4_sound = 0;
      row5_sound = 0;
      row6_sound = 0;
      paddle_sound = 0;
      wall_sound = 0;  
    }
    sounds();
    if (demo)
    {
      if (BTN_ACTIVE_VAL)
      {
        title = 1;
        demo = 0;
        advanced = 0;
        normal = 0;
        score_1 = 0;
        score_2 = 0;
        score_3 = 0;
        total_score = 0;
        update_score = 1;
        demo_time = millis;
        while ((millis - demo_time) > 100);
      }
    }
    
    // Only updates score if needed, prevents lag
    if (update_score)
    {
      increment_score();
      if ((!demo && normal) || (!demo && advanced) || (!demo && title))
      increment_high_score();      
      paint_score();
      paint_high_score();
      paint_lives();
      update_score = 0;
    }
    
    // Makes the screen update at roughly 60 Hz
    prev_millis = millis;
    while (17 > (millis - prev_millis));
  }
}

//-----------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//-----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////

void ball_move()
{  
  // Erase old ball image
  video_paint_rect(x, y, 8, 8, BLACK);
  
  
  // This might be causing to go into wall.....
  
  // Reset the ball parameters to initial conditions
  if (ball_dead)
  {
    decrement_lives();
    update_score = 1;
    x = BALL_X;
    y = BALL_Y;
    x_speed = (BALL_X_SPEED + level);
    y_speed = (BALL_Y_SPEED + level);
    ball_dead = 0;
  }
  
  
  // Check for wall
  ///////////////////////////////////////////////
  if((x + x_speed) <= LEFT_BORDER)
  {
    x_speed = (-x_speed);
    if (LEFT_BORDER == (x - abs(x_speed)))
    {
      x = LEFT_BORDER;
    }
    wall_sound = 1;
    sound_time = millis;
  }
  if((x + BALL_DIAMETER + abs(x_speed)) >= RIGHT_BORDER)
  {
    x_speed = (-x_speed);
    wall_sound = 1;
    sound_time = millis;
  }
  if((y + BALL_DIAMETER + abs(y_speed)) >= BOTTOM_BORDER)
  {
    // y_speed = (-y_speed);
    ball_dead = 1;
    wall_sound = 1;
    sound_time = millis;
  }
  if((y + y_speed) <= TOP_BORDER)
  {
    y_speed = (-y_speed);
    wall_sound = 1;
    sound_time = millis;
    if (TOP_BORDER == (y - abs(y_speed)))
    {
      y = TOP_BORDER;
    }
  }
  ///////////////////////////////////////
  
  if ((abs(x_speed) - x_speed) == 0)
  {
    demo_x_pos = 1;
  }
  else
  {
    demo_x_pos = 0;
  }
  if ((abs(y_speed) - y_speed) == 0)
  {
    demo_y_pos = 1;
  }
  else 
  {
    demo_y_pos = 0;
  }
  
  
  
  // Check for Paddle:
  ///////////////////////////////////////
  
  // Top of the Paddle
  if(( (y + y_speed + BALL_DIAMETER) >= paddle_y) && ((y) <= (paddle_y + PADDLE_WIDTH)) )
  {
    // Makes sure that the ball is within the x bounds of the paddle
    if (((x + BALL_DIAMETER + (abs(x_speed))) >= paddle_x) && ((x + (abs(x_speed))) <= (paddle_x + PADDLE_LENGTH)))
    {
      paddle_sound = 1;
      sound_time = millis;
      y_speed = (-y_speed);
    }    
  }     
  
  // Right side of the Paddle
   if ( (x <= (paddle_x + PADDLE_LENGTH)) && ((x + abs(x_speed) + BALL_DIAMETER) > (paddle_x + PADDLE_LENGTH )) )
   {
     if (((y + abs(y_speed) + BALL_DIAMETER) >= (paddle_y + 3)) && ((y + abs(y_speed)) <= (paddle_y + PADDLE_WIDTH + 3)))
     {
       x_speed = (-x_speed);
       // y_speed = (-y_speed);
       paddle_sound = 1;
       sound_time = millis;
     }
   }
  
  // Left side of the Paddle
  if (((x + x_speed + BALL_DIAMETER) > (paddle_x)) && (x < paddle_x))
  {
    if (((y + y_speed + BALL_DIAMETER) > paddle_y) && ((y + y_speed) < (paddle_y + PADDLE_WIDTH)))
    {
      x_speed = (-x_speed);
      // y_speed = (-y_speed);
      paddle_sound = 1;
      sound_time = millis;
    }
  }  
  ///////////////////////////////////////
  check_bricks();
  
  x += x_speed;
  y += y_speed;
  
  // Print New Ball
  video_paint_rect(x, y, 8, 8, WHITE); 
}

void paddle_move()
{ 
  
  // If not Demo Mode
  if (!demo)
  {  
    // If paddle moves right, paint previous left piece
    if (joystick_right)
    {
      if ((paddle_x >= (LEFT_BORDER + 2)))
      video_paint_rect((paddle_x - PADDLE_SPEED), paddle_y, PADDLE_SPEED, PADDLE_WIDTH, BLACK);  
    
      // Check for wall
      if ((paddle_x + PADDLE_SPEED + PADDLE_LENGTH) < (RIGHT_BORDER))
      {
        paddle_x = (paddle_x + PADDLE_SPEED);
      }
    }
    // If paddle moves left, paint previous right piece
    if (joystick_left)
    {
      if ((paddle_x + PADDLE_LENGTH) <= (RIGHT_BORDER - 2))
      video_paint_rect((paddle_x + PADDLE_LENGTH), paddle_y, PADDLE_SPEED, PADDLE_WIDTH, BLACK);
      // Check for wall
      if ((paddle_x - PADDLE_SPEED) > LEFT_BORDER)
      {
        paddle_x = (paddle_x - PADDLE_SPEED);
      }
    }  
  
    ////////////////////////
    // If advanced mode, allow up and down
    if (advanced)
    {
        // If paddle moves up, paint previous down piece
        if (joystick_up)
        {
          video_paint_rect((paddle_x), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          if (joystick_left)
          {
            video_paint_rect((paddle_x + PADDLE_SPEED), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          }        
          if (joystick_right)
          {
            video_paint_rect((paddle_x - PADDLE_SPEED), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          }              
                
          // Check for wall
          if ((paddle_y - PADDLE_SPEED) > PADDLE_UPWARD_BOUND)
          {
            paddle_y = (paddle_y - PADDLE_SPEED);
          }
        }
      
        // If paddle moves down, paint previous up piece
        if (joystick_down)
        {
          video_paint_rect((paddle_x), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          if (joystick_left)
          {
            video_paint_rect((paddle_x + PADDLE_SPEED), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          }
          if (joystick_right)
          {
            video_paint_rect((paddle_x - PADDLE_SPEED), (paddle_y), (PADDLE_LENGTH), (PADDLE_SPEED + PADDLE_WIDTH), BLACK);
          }
        
          // Check for wall
          if ((paddle_y + PADDLE_SPEED + PADDLE_WIDTH) < PADDLE_LOWER_BOUND)
          {
            paddle_y = (paddle_y + PADDLE_SPEED);
          }
        }      
    }
    //////End Advance Mode
  }
  /////////////////End Normal & Advanced
  
  // Demo Mode
  else if (demo)
  {    
    video_paint_rect(paddle_x, paddle_y, PADDLE_LENGTH, PADDLE_WIDTH, BLACK);    
    if ((((paddle_x + (PADDLE_LENGTH / 2)) < x)) && ( (paddle_x + PADDLE_LENGTH + PADDLE_SPEED) <= RIGHT_BORDER) )
    {
      paddle_x = (paddle_x + PADDLE_SPEED);
    }
    else if ( (((paddle_x + (PADDLE_LENGTH / 2)) > x)) && ((paddle_x - PADDLE_SPEED) >= (LEFT_BORDER)) )
    {
      paddle_x = (paddle_x - PADDLE_SPEED);
    }
  }    
  // Paint new Paddle
  video_paint_rect(paddle_x, paddle_y, PADDLE_LENGTH, PADDLE_WIDTH, paddle_color);
}

void joystick ()
{
  joystick_left = 0;
  joystick_right = 0;
  joystick_down = 0;
  joystick_up = 0;
  
  if (160 <= adc_x_val)
  {
    joystick_left = 1;
  }
  
  if (125 >= adc_x_val)
  {
    joystick_right = 1;
  }  
  
  if (125 >= adc_y_val)
  {
    joystick_up = 1;
  }
  
  if (160 <= adc_y_val)
  {
    joystick_down = 1;
  }  
}

void adc_check()
{
  // Align adc_counter value to timer_val
  //This updates the ADC values
  if (0 == adc_case)
  {
    adc_y_val = adc_get_y();
    update_adc();
    adc_case ++;
    for (int r = 0; r < 50; r ++)
    {
      ;
    }
  }
  if (1 == adc_case)
  {
    update_adc();
    adc_case ++;
    for (int r = 0; r < 50; r ++)
    {
      ;
    }
  }
  if (2 == adc_case)
  {
    adc_x_val = adc_get_x();
    update_adc();
    adc_case = 0;
    for (int r = 0; r < 50; r ++)
    {
      ;
    }
  }
}

void paint_score()
{
  kill_score();
  switch (score_1)
  {
    case 0:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
  
    // Middles
    // video_paint_rect(163, 20, 8, 2, WHITE);
  
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
  
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
  
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
  
    // Bottom Lefts
    video_paint_rect(162, 23, 2, 6, WHITE);
  
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);  
    break;  
    
    case 1:    
    // Top Rights
    video_paint_rect(170, 11, 2, 9, WHITE);

    // Bottom Rights
    video_paint_rect(170, 22, 2, 9, WHITE);
    break;    

    case 2:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    // video_paint_rect(170, 23, 2, 6, WHITE);
    break;   
    
    case 3:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;     
    
    case 4:
    // Tops
    // video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    // video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;    
    
    case 5:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    // video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;    
    
    case 6:
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    // video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;    
    
    case 7:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    // video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    // video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;    
    
    case 8:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 6, WHITE);
    break;    
    
    case 9:
    // Tops
    video_paint_rect(163, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(163, 20, 8, 2, WHITE);
    
    // Bottoms
    // video_paint_rect(163, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(162, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(170, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(162, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(170, 23, 2, 8, WHITE);
    break;    
  }  
  
  // Score 2 -------------------------
  switch (score_2)
  {
    case 0:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    // video_paint_rect(146, 20, 8, 2, WHITE);    
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
  
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
  
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
  
    // Bottom Lefts
    video_paint_rect(145, 23, 2, 6, WHITE);
  
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;
    
    case 1:    
    // Top Rights
    video_paint_rect(153, 11, 2, 9, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 22, 2, 9, WHITE);
    break;    
    
    case 2:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(145, 23, 2, 6, WHITE);
    break;
    
    case 3:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break; 
   
    case 4:
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;       

    case 5:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;
    
    case 6:
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(145, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;    
        
    case 7:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;
         
    case 8:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(146, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(145, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 6, WHITE);
    break;

    case 9:
    // Tops
    video_paint_rect(146, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(146, 20, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(145, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(153, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(153, 23, 2, 8, WHITE);
    break;            
  }
  
  // Score 3 -------------------------
  switch(score_3)
  {
    case 0:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
  
    // Middles
    // video_paint_rect(129, 20, 8, 2, WHITE);
  
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);
  
    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);
  
    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);
  
    // Bottom Lefts
    video_paint_rect(128, 23, 2, 6, WHITE);
  
    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE); 
    break; 

    case 1:
    // Top Rights
    video_paint_rect(136, 11, 2, 9, WHITE);
    
    // Bottom Rights
    video_paint_rect(136, 22, 2, 9, WHITE);
    break;    

    case 2:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);
    
    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(128, 23, 2, 6, WHITE);
    break;
    
    case 3:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;    

    case 4:
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;

    case 5:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;

    case 6:
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(128, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;

    case 7:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;

    case 8:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(129, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(128, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(136, 23, 2, 6, WHITE);
    break;

    case 9:
    // Tops
    video_paint_rect(129, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(129, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(128, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(136, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(136, 23, 2, 8, WHITE);
    break;

  }      
}

void paint_high_score()
{
  kill_high_score();
  switch (high_score_1)
  {
    case 0:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
  
    // Middles
    // video_paint_rect(42, 20, 8, 2, WHITE);
  
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);
  
    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);
  
    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);
  
    // Bottom Lefts
    video_paint_rect(41, 23, 2, 6, WHITE);
  
    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);    
    break;
    
    case 1:
    // Top Rights
    video_paint_rect(49, 11, 2, 9, WHITE);

    // Bottom Rights
    video_paint_rect(49, 22, 2, 9, WHITE);
    break;    

    case 2:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(41, 23, 2, 6, WHITE);
    break;

    case 3:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;
    
    case 4:
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;
    
    case 5:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);
 
    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;

    case 6:
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(41, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;

    case 7:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;

    case 8:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(42, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(41, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(49, 23, 2, 6, WHITE);
    break;

    case 9:
    // Tops
    video_paint_rect(42, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(42, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(41, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(49, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(49, 23, 2, 8, WHITE);
    break;       
  }
  
  // high_score 2 -------------------------
  switch (high_score_2)
  {
    case 0:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 1:
    // Top Rights
    video_paint_rect(32, 11, 2, 9, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 22, 2, 9, WHITE);
    break;
    
    case 2:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    // video_paint_rect(32, 23, 2, 6, WHITE);
    break;

    case 3:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;

    case 4:
    // Tops
    // video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    // video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 5:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 6:
    // Tops
    // video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    // video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 7:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    // video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    // video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    // video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    // video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 8:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(25, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(24, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(32, 23, 2, 6, WHITE);
    break;
    
    case 9:
    // Tops
    video_paint_rect(25, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(25, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(24, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(32, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(32, 23, 2, 8, WHITE);
    break;
  }
  
  switch (high_score_3)
  {
    case 0:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(7, 13, 2, 6, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(7, 23, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break;
    
    case 1:
    // Top Rights
    video_paint_rect(15, 11, 2, 9, WHITE);

    // Bottom Rights
    video_paint_rect(15, 22, 2, 9, WHITE);
    break;    

    case 2:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(7, 23, 2, 6, WHITE);
    break;

    case 3:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break;

     case 4:
     // Middles
     video_paint_rect(8, 20, 8, 2, WHITE);

     // Top Lefts
     video_paint_rect(7, 13, 2, 6, WHITE);

     // Top Rights
     video_paint_rect(15, 13, 2, 6, WHITE);

     // Bottom Rights
     video_paint_rect(15, 23, 2, 6, WHITE);
     break;
 
    case 5:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(7, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break; 

    case 6:
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(7, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(7, 23, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break;

    case 7:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break;

    case 8:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Bottoms
    video_paint_rect(8, 30, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(7, 13, 2, 6, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(7, 23, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 6, WHITE);
    break;

    case 9:
    // Tops
    video_paint_rect(8, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(8, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(7, 13, 2, 6, WHITE);

    // Top Rights
    video_paint_rect(15, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(15, 23, 2, 8, WHITE);
    break;    
  }
}

void paint_lives()
{
  kill_lives();
  switch (lives)
  {
    case 0:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(84, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;
    
    case 1:
    // Top Rights
    video_paint_rect(92, 11, 2, 9, WHITE);

    // Bottom Rights
    video_paint_rect(92, 22, 2, 9, WHITE);
    break;    
    
    case 2:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
     
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);
     
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);
     
    // Bottom Lefts
    video_paint_rect(84, 23, 2, 6, WHITE);
    break;   

    case 3:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;

    
    case 4:
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;    
    
    case 5:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;

    case 6:
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);

    // Bottom Lefts
    video_paint_rect(84, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;

    case 7:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);

    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;

    case 8:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);
    
    // Bottoms
    video_paint_rect(85, 30, 8, 2, WHITE);
    
    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);
    
    // Bottom Lefts
    video_paint_rect(84, 23, 2, 6, WHITE);
    
    // Bottom Rights
    video_paint_rect(92, 23, 2, 6, WHITE);
    break;

    case 9:
    // Tops
    video_paint_rect(85, 10, 8, 2, WHITE);
    
    // Middles
    video_paint_rect(85, 20, 8, 2, WHITE);

    // Top Lefts
    video_paint_rect(84, 13, 2, 6, WHITE);
    
    // Top Rights
    video_paint_rect(92, 13, 2, 6, WHITE);

    // Bottom Rights
    video_paint_rect(92, 23, 2, 8, WHITE);
    break;    
  }
}

void increment_score()
{
  uint16_t temp_total_score = total_score;
  score_1 = (temp_total_score % 10);
  temp_total_score = (temp_total_score / 10);
  score_2 = (temp_total_score % 10);
  temp_total_score = (temp_total_score / 10);
  score_3 = (temp_total_score % 10);
}

void increment_high_score()
{
  if (score_3 > high_score_3)
  {
    high_score_3 = score_3;
    high_score_2 = score_2;
    high_score_1 = score_1;
  }
  else if (score_2 > high_score_2)
  {
    high_score_2 = score_2;
    high_score_1 = score_1;
  }
  else if (score_1 > high_score_1)
  {
    high_score_1 = score_1;
  }  
}

void decrement_lives()
{
  lives --;
  if (lives == 0)
  {
    if (!demo)
    {
      // Game Over
      game_over = 1;
      paint_game_over = 1;
      total_score = 0;
    }
    else 
    {
      title = 1;
    }    
  }
}

void paint_bricks()
{
  uint8_t i = 0;
  uint8_t e = 0;
  
  for (i = 0; i < 6; i++)
  {
    brick_make_y = bricks_y[i];
    brick_make_color = bricks_color[i];
    
    for (e = 0; e < 6; e++)
    {
      brick_make_x = bricks_x[e];
      video_paint_rect(brick_make_x, brick_make_y, BRICK_LENGTH, BRICK_WIDTH, brick_make_color);
    }     
  }
}

void check_bricks()
{
  // Check for Bricks
  uint8_t alive_count = 0;
  brick_i = 0;
  brick_e = 0;
  brick_struck = 0;
  all_bricks_gone = 1;
  for (brick_i = 0; brick_i < 6; brick_i++)
  {
    brick_make_y = bricks_y[brick_i];
    brick_make_color = bricks_y[brick_i];
    for (brick_e = 0; brick_e < 6; brick_e++)
    {
      
      brick_make_x = bricks_x[brick_e];
      if (bricks_alive[alive_count] && !brick_struck)
      {
        all_bricks_gone = 0;
        // Check underside of brick for contact
        if( ((y + y_speed) <= ( brick_make_y + BRICK_WIDTH)) && ((y + y_speed) >= ( brick_make_y)) && ((x + x_speed) >= brick_make_x) && ((x + x_speed) <= (brick_make_x + BRICK_LENGTH)) )
        {
          y_speed = (-y_speed);
          bricks_alive[alive_count] = 0;
        }
        
        // Check top side of brick for contact
        else if( ((y + y_speed + BALL_DIAMETER) <= ( brick_make_y + BRICK_WIDTH)) && ((y + y_speed + BALL_DIAMETER) >= ( brick_make_y)) && ((x + x_speed) >= brick_make_x) && ((x + x_speed) <= (brick_make_x + BRICK_LENGTH)) )
        {
          y_speed = (-y_speed);
          bricks_alive[alive_count] = 0;
        }      
        
        // Check left side of brick
        else if ( ((x + x_speed + BALL_DIAMETER) >= (brick_make_x)) && ((x + x_speed + BALL_DIAMETER) <= (brick_make_x + BRICK_LENGTH)) && ((y + y_speed + BALL_DIAMETER) >= (brick_make_y)) && ((y + y_speed) <= (brick_make_y + BRICK_WIDTH)) )
        {
          x_speed = (-x_speed);
          bricks_alive[alive_count] = 0;
        }        
        // Check right side of brick  
        else if ( ((x + x_speed) <= (brick_make_x + BRICK_LENGTH)) && ((x + x_speed + BALL_DIAMETER) >= (brick_make_x + BRICK_LENGTH)) && ((y + y_speed + BALL_DIAMETER) >= (brick_make_y)) && ((y + y_speed) <= (brick_make_y + BRICK_WIDTH)) )
        {
          x_speed = (-x_speed);
          bricks_alive[alive_count] = 0;
        }        
        
        if (!bricks_alive[alive_count])
        {
          kill_brick();
          brick_struck = 1;
          if ((brick_i >= 0) && (brick_i < 2))
          {
            total_score = (total_score + LEVEL_3_POINTS);
          } 
          else if ((brick_i >= 2) && (brick_i < 4))
          {
            total_score = (total_score + LEVEL_2_POINTS);
          }
          else if ((brick_i >= 4) && (brick_i < 6))
          {
            total_score = (total_score + LEVEL_1_POINTS);
          } 
          // Make sound
          if (brick_i == 0)
          {
            row1_sound = 1;
            sound_time = millis;
          }
          else if (brick_i ==1)
          {
            row2_sound = 1;
            sound_time = millis;
          }
          else if (brick_i == 2)
          {
            row3_sound = 1;
            sound_time = millis;
          }
          else if (brick_i == 3)
          {
            row4_sound = 1;
            sound_time = millis;
          }
          else if (brick_i == 4)
          {
            row5_sound = 1;
            sound_time = millis;
          }             
          else if (brick_i == 5)
          {
            row6_sound = 1;
            sound_time = millis;
          }        
                    
          update_score = 1;                            
        }
      }
      alive_count ++;
    }
  }
  if (all_bricks_gone)
  {
    if (BALL_MAX >= level)
    {
      level ++;
    }    
    if (advanced)
    {
      paint_advanced = 1;
    }      
    else if (normal)
    {
      paint_normal = 1;
    }      
  } 
  if (all_bricks_gone && demo)
  {
    game_over = 1;
    paint_game_over = 1;
  } 
}

void kill_brick()
{
  video_paint_rect(brick_make_x, brick_make_y, BRICK_LENGTH, BRICK_WIDTH, BLACK);
}

void kill_score()
{
  video_paint_rect(120, 10, 56, 30, BLACK);
}

void kill_high_score()
{
  video_paint_rect(5, 10, 50, 30, BLACK);
}

void kill_lives()
{
  video_paint_rect(80, 10, 15, 30, BLACK);
}

void sounds()
{
  if (row1_sound)
  {
    pwm_update_dc(pwm_adjust_freq(A));
    pwm_enable();
  }    
  
  else if (row2_sound)
  {
    pwm_update_dc(pwm_adjust_freq(B));
    pwm_enable();
  }  
  
  else if (row3_sound)
  {
    pwm_update_dc(pwm_adjust_freq(C));
    pwm_enable();
  }  
  
  else if (row4_sound)
  {
    pwm_update_dc(pwm_adjust_freq(Cs));
    pwm_enable();
  }  
  
  else if (row5_sound)
  {
    pwm_update_dc(pwm_adjust_freq(D));
    pwm_enable();
  }  
  
  else if (row6_sound)
  {
    pwm_update_dc(pwm_adjust_freq(E));
    pwm_enable();
  }  
  
  else if (paddle_sound)
  {
    pwm_update_dc(pwm_adjust_freq(F));
    pwm_enable();
  }  
  
  else if (wall_sound)
  {
    pwm_update_dc(pwm_adjust_freq(G));
    pwm_enable();
  }  
  
}
//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------

void SysTick_Handler ()
{
	millis++;
}