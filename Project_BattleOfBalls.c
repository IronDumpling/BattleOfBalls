/* ************************************************* Definition Area ************************************************** */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000  // Memory 1
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000  // Memory 2
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000  // Text Showing Base Address
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100  // Keyboard/Mouse Input Base Address
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020  // Draw Pixel Base Address
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

/* VGA Colors */
#define BLACK 0x0000
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

/* Absolute Value */
#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Number of Balls */
#define FOOD_NUM 50
#define AI_NUM 20

/* ************************************************** Global Area ***************************************************** */
#include<time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
	
/* Type Definition of Balls */
typedef struct ourBall{
    bool isEaten;    // Eaten/Dead = 1
                     // Alive = 0
    short int color;
    int radius;
    int xLocation;
    int yLocation;
    int score;
} Ball;

/* Function Prototypes */
void wait_for_vsync();

void initial_game();
void initial_memory_base();
void initial_player();
void initial_AI();
void initial_food();

void plot_game();
void plot_food();
void plot_AI();
void plot_player();
void clear_screen();
void plot_pixel(int, int, short int);
void plot_circle(Ball);
void draw_line(int, int, int, int, short int);

void video_text(int, int, char *);
void cleartext();

void update_game();
void ball_generate();
void food_generate();
void ball_update();

void game_react();
void playerEatFood();
void AIEatFood();
void playerEatAI();
void AIMove();
void AIChase();

void close_game();

float findDistance(Ball, Ball);
bool overlapPayer(Ball);
void swap(int*, int*);


/* Global Variables */
Ball player;         // Ball of Player
Ball AI[AI_NUM];     // Ball Array of AI
Ball food[FOOD_NUM]; // Ball Array of Food
short int color[9] = {RED, YELLOW, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
volatile int pixel_buffer_start; 
volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;

/* ********************************************* Main Function Area *************************************************** */

// Main Function
int main(){
    
    // Initial Games
    initial_game();
    
    // Game While Loop
    while(true){
        /* Erase any boxes and lines that were drawn in the last iteration */
        clear_screen();
        
        // code for updating the locations of boxes (not shown)
        update_game();
        
        // code for drawing the boxes and lines (not shown)
        plot_game();
                
        char text_top_row[40] = "Battle of Balls";
		char text_bottom_row[40] = "Score:\0";
        video_text(1, 1, text_top_row);
		video_text(1, 2, text_bottom_row);
		
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
    
    // Close Graphics
    close_game();
    
    return 0;
}

/* ****************************************** VGA Related Functions Area ********************************************** */

// Function 1: Wait for screen to be syncronised
void wait_for_vsync(){
    volatile int *pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE;
    register int status;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr + 3);
    
    while((status & 0x01) != 0){
        status = *(pixel_ctrl_ptr + 3);
    }
}

/* ******************************************* Initial Game Functions Area ******************************************** */

// Function 2: Initialise Game Randomly
void initial_game(){
    initial_memory_base();
    
    // Random generate seed
    srand((unsigned)time(NULL));
    
    initial_player();
    
    initial_AI();
    
    initial_food();
}

// Function 3:
void initial_memory_base(){
    /* set front pixel buffer to start of FPGA On-chip memory */
    // first store the address in the back buffer
    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE;
    
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    
    // pixel_buffer_start points to the pixel buffer
    clear_screen();
    
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    
    // we draw on the back buffer
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

// Function 4: Random Generate Player Location
void initial_player(){
    // Initialise Player's Information
    player.color = WHITE;
    player.radius = 10;
    player.isEaten = false;
      
    // Initialise Player's Location
    player.xLocation = RESOLUTION_X/2;
    player.yLocation = RESOLUTION_Y/2;
}

// Function 5: Random Generate AI Balls
void initial_AI(){
    for (int i = 0; i < AI_NUM; i++){
        AI[i].color = color[rand()%9];
        AI[i].isEaten = false;
        AI[i].radius = (int)(rand() % 10 + 10);
        
        AI[i].xLocation = rand() % (RESOLUTION_X - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
        AI[i].yLocation = rand() % (RESOLUTION_Y - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
        
        // AI Balls won't over the boarder
        while(overlapPayer(AI[i])){
            AI[i].xLocation = rand() % (RESOLUTION_X - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
            AI[i].yLocation = rand() % (RESOLUTION_Y - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
        }
    }
}

// Function 6: Random Generate Foods
void initial_food(){
    for (int i = 0; i < FOOD_NUM; i++){
        food[i].radius = 1;
        food[i].color = color[rand()%9];
        food[i].isEaten = false;
        
        food[i].xLocation = (int)(rand() % RESOLUTION_X);
        food[i].yLocation = (int)(rand() % RESOLUTION_Y);
        
        while(overlapPayer(food[i])){
            food[i].xLocation = (int)(rand() % RESOLUTION_X);
            food[i].yLocation = (int)(rand() % RESOLUTION_Y);
        }
    }
}
//void initial_score(){

//}

/* ***************************************** Keyboard Input Functions Area ******************************************** */

// Function 7: Press [Direction] Button to Move Balls

// Function 8: Press [Enter] Button to Start

// Function 9: Press [Space] Button to Pause or Resume

/* *************************************** Graphics Drawing Functions Area ******************************************** */

// Function 10: Draw Main Function
void plot_game(){
    plot_player();
    
    plot_food();
    
    plot_AI();
}

// Function 11: Plot Food
void plot_food(){
    for(int i = 0; i < FOOD_NUM; i++){
        if(!food[i].isEaten){
            plot_circle(food[i]);
        }else{
            food[i].radius = 1;
            food[i].color = color[rand()%9];
            food[i].isEaten = false;
            
            food[i].xLocation = (int)(rand() % RESOLUTION_X);
            food[i].yLocation = (int)(rand() % RESOLUTION_Y);
            
            while(overlapPayer(food[i])){
                food[i].xLocation = (int)(rand() % RESOLUTION_X);
                food[i].yLocation = (int)(rand() % RESOLUTION_Y);
            }
        }
    }
}

// Function 12: Plot AI Balls
void plot_AI(){
    for (int i = 0; i < AI_NUM; i++){
      if (!AI[i].isEaten){
          plot_circle(AI[i]);
      }else{
          AI[i].color = color[rand()%9];   //rand()%256  随机取值 0-255
          AI[i].isEaten = false;
          AI[i].radius = (int)(rand() % 10 + 10);
          
          AI[i].xLocation = rand() % (RESOLUTION_X - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
          AI[i].yLocation = rand() % (RESOLUTION_Y - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
          
          // AI Balls won't over the boarder
          while(overlapPayer(AI[i])){
              AI[i].xLocation = rand() % (RESOLUTION_X - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
              AI[i].yLocation = rand() % (RESOLUTION_Y - (int)(AI[i].radius + 0.5)) + (int)(AI[i].radius + 0.5);
          }
      }
    }
}

// Function 13: Plot Player
void plot_player(){
    plot_circle(player);
}

// Function 14: Clear Screen
void clear_screen(){
    for(int x = 0; x < RESOLUTION_X; ++x){
        for(int y = 0; y < RESOLUTION_Y; ++y){
            if(*(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) != BLACK)
                plot_pixel(x, y, BLACK);
        }
    }
}

// Function 15: Plot pixels
void plot_pixel(int x, int y, short int color){
    if(x >= 0 && x < RESOLUTION_X && y >= 0 && y < RESOLUTION_Y)
        *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
}

// Function 16: Plot Circle
void plot_circle(Ball ball){
    
    int x = ball.xLocation;
    int y = ball.yLocation;
    int r = ball.radius;
    short int color = ball.color;
    
    int count = 0;
    int d = 1-r;
    
    while(r > count){
        draw_line(-count+x, r+y, count+x, r+y, color);
        draw_line(-r+x, count+y, r+x, count+y, color);
        draw_line(-count+x, -r+y, count+x, -r+y,  color);
        draw_line(-r+x, -count+y, r+x, -count+y,  color);
        
        if(d < 0){
            d = d + 2*count + 3;
        }else{
            d = d + 2*(count - r) + 5;
            r--;
        }
        
        count++;
    }
}

// Function 17: Plot lines
void draw_line(int startX, int startY, int endX, int endY, short int color){
    bool isSteep = (ABS(endY - startY) > ABS(endX - startX));
    
    if(isSteep){
        swap(&startX, &startY);
        swap(&endX, &endY);
    }
    
    if(startX > endX){
        swap(&startX, &endX);
        swap(&startY, &endY);
    }
    
    // Bresenham's Algorithm
    int deltaX = endX - startX;
    int deltaY = ABS(endY - startY);
    int error = -(deltaX / 2);
    int stepY;
    
    // y step
    if(startY < endY){
        stepY = 1;
    }else if(startY > endY){
        stepY = -1;
    }else{
        stepY = 0;
    }
    
    // for loop
    int y = startY;

    for(int x = startX; x < endX; ++x){
        if(isSteep){
            plot_pixel(y, x, color);
        }else{
            plot_pixel(x, y, color);
        }
        
        error = error + deltaY;
        
        if(error >= 0){
            y = y + stepY;
            error = error - deltaX;
        }
    }
}

/* *************************************** Graphics Update Functions Area ********************************************* */

// Function 18: Update Main Fuction
void update_game(){
    
}

// Function 19: Balls Location Update
void ball_update(){
    
}

/* ************************************** Graphics React Functions Area *********************************************** */

// Function 20: Graphics React Main Function
void game_react(){
    
}

// Function 21: Player Eat Food
void playerEatFood(){
    
}

// Function 22: AI Eat Food
void AIEatFood(){
    
}

// Function 23: Player Eat AI or AI Eat Player
void playerEatAI(){
    
}

// Function 24: AI Movement
void AIMove(){
    
}

// Function 25: Chase Algorithm
void AIChase(){
    
}

/* ***************************************** Text Drawing Functions Area ********************************************** */
void video_text(int x, int y, char * text_ptr) {
	int offset;
	volatile char * character_buffer =(char *)FPGA_CHAR_BASE; // video character buffer

	offset = (y << 7) + x;
	while (*(text_ptr)) {
		*(character_buffer + offset) =*(text_ptr); // write to the character buffer
		++text_ptr;
		++offset;
	}
}

/* *************************************** Score Update Functions Area ************************************************ */
void cleartext(){
	for(int x=0;x<80;x++){
		for(int y=0;y<60;y++){
			video_text(x, y, " \0");
		}
	}
}

/* ******************************************* Tool Functions Area **************************************************** */

// Function: Close Game Main Function
void close_game(){
    
}

/* ******************************************* Tool Functions Area **************************************************** */

// Function: Find Distance Between Balls
float findDistance(Ball ball1, Ball ball2){
    return 1;
}

// Function: the ball is overlap with the player
bool overlapPayer(Ball ball){
    if(((ball.xLocation - ball.radius) < (player.xLocation + player.radius)) && ((ball.xLocation + ball.radius) > (player.xLocation - player.radius))){
        if(((ball.yLocation - ball.radius) < (player.yLocation + player.radius)) && ((ball.yLocation + ball.radius) > (player.yLocation - player.radius))){
            return true;
        }
    }
    
    return false;
}


// Function : Swap
void swap(int* a, int* b){
    int temp = *a;
    *a = *b;
    *b = temp;
}
