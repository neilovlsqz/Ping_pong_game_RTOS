/*
// This code was designed, written, and tested by Neil Velásquez.
// It works for ESP32 microcontrollers.
// This version considers its operation with:
// - 16x15 LED matrix, meaning 465 WS2812b Neopixel LEDs.
// - 2 potentiometers of 10 kOhms.
// - A 3A power supply.
// - Of course, an ESP32 Wroom 32 development board.
// Other functions will be added over the coming days.
*/

//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>
#include <Adafruit_NeoPixel.h>

#define PIN 16  // data pin for neopixel handling
#define WIDTH 15
#define HEIGHT 31

#define PADDLE_HEIGHT 3 //Width of each paddle
#define PADDLE1Y 0 //Y position, always the same for each paddle.
#define PADDLE2Y 30

#define POT1_PIN 34  // Potentiometer to control paddle 1 in GPIO34
#define POT2_PIN 35  // Potentiometer to control paddle 2 en GPIO35

Adafruit_NeoPixel matrixx = Adafruit_NeoPixel(WIDTH * HEIGHT, PIN, NEO_GRB + NEO_KHZ800); //Neopixel leds declaration

SemaphoreHandle_t xAnalogSemaphore; //semaphore to handle analogRead

// Those are the task declarations that control every action in the game

void Taskpaddle1( void *pvParameters );
void Taskpaddle2( void *pvParameters );
void Taskball( void *pvParameters );
void Taskrender( void *pvParameters );

//Global variables (In RTOS use this type of variables carefully because it might cause issues and result in unexpected behavior).

int matrix[15][31];
/*
  [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14],
  [29,28,27,26,25,24,23,22,21,20,19,18,17,16,15],
  [30,31,32,33,34,35,36,37,38,39,40,41,42,43,44],
  [59,58,57,56,55,54,53,52,51,50,49,48,47,46,45],
  ...
  [319,318,317,316,315,314,313,312,311,310,309,308,307,306,305].  // I wrote this to try to explain how the LED matrix is structured, in zig zag pattern. 
  ...465]
*/
int counter = 0; // Aux counter

//Ball values
int ballX = WIDTH / 2; // Variable for the initial position of the ball on the X axis
int ballY = HEIGHT / 2; // Variable for the initial position of the ball on the Y axis
int ballDirX = 1; // Variable indicating the ball's direction on the X axis
int ballDirY = 1; // Variable indicating the ball's direction on the Y axis
// Position values for the paddles
int paddle1X;
int paddle2X;

int speedball = 70; //With this value you can control the speed of the ball.

// Initialize the LED matrix mapping.

void initMatrixMapping() {
    for (int i = 0; i < 31; i++) {
        if (i % 2 == 0) {  // Pair column from left to right
            for (int j = 0; j < 15; j++) {
                matrix[j][i] = counter++;
            }
        } else {  // unpair column right to left
            for (int j = 14; j >= 0; j--) {
                matrix[j][i] = counter++;
            }
        }
    }
}

void setup()
 {

  
  Serial.begin(115200);
  //analogSetWidth(5);
  initMatrixMapping(); //Call this that we defined above to map the LED matrix into a two-dimensional array.

  xAnalogSemaphore = xSemaphoreCreateMutex(); //Semaphore creation for analogReads

  xTaskCreatePinnedToCore(Taskpaddle1, "PaddleONE", 2048 , NULL, 3, NULL, 0 ); //Task creation with some parameters, set to work with Core 0

  xTaskCreatePinnedToCore(Taskpaddle2, "PaddleTWO", 2048 , NULL, 3, NULL, 0 ); // set to work with Core 0 too.

  xTaskCreatePinnedToCore(Taskball, "Ball", 2048, NULL, 2, NULL, 1); // set to work with Core 1.

  xTaskCreatePinnedToCore(Taskrender, "Render", 2048, NULL, 2, NULL, 1); // Set to work with Core 1 too.
  
  //vTaskStartScheduler(); <- Don't uncomment this because it might result in program crashes due to the watchdog.


 }

void loop()
 {
    // This thing is empty, all the fun is in the tasks.
 }


/////////////////////////////////// The tasks ///////////////////////////////////////////////////////////////

void Taskpaddle1( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.

  for(;;)
  {
    //int SensorVal1 = analogRead(POT1_PIN); 
    //Serial.println(SensorVal1);
    if (xSemaphoreTake(xAnalogSemaphore, portMAX_DELAY)) // Here we call the semaphore that we defined above to share ADC resources and execute related core inside.
    {
    int SensorVal1 = analogRead(POT1_PIN); // the method you already know for performing ADC readings.
    Serial.println(SensorVal1);
    paddle1X = map(SensorVal1, 0, 4095, 13, 1); //to map values from ADC readings.
      
    xSemaphoreGive(xAnalogSemaphore); // This thing is for release ADC, now is ready to be used with other tasks, if it's needed.
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Taskpaddle2( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.

  for(;;)
  {

    if (xSemaphoreTake(xAnalogSemaphore, portMAX_DELAY)) //same thing than taskpaddle1
    {
    int SensorVal2 = analogRead(POT2_PIN);
    Serial.println(SensorVal2);
    paddle2X = map(SensorVal2, 0, 4095, 1, 13); //this thing maps values from ADC assigning backwards than Taskpaddle1 because the paddle 2 position.
    xSemaphoreGive(xAnalogSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Taskball( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.

  for(;;)
  {
    //Ball movement
    ballX += ballDirX;
    ballY += ballDirY;
    //Bounce on horizontal
    if (ballX <= 0 || ballX >= WIDTH - 1) 
        {
            ballDirX *= -1;  // Inverts the X direction
        }
    //Bounce on vertical edges
    if (ballY <= 0 || ballY >= HEIGHT - 1) 
        {
            
            if(ballY == 0)
            {
              for(int i=0; i<15 ; i++)
              {
              matrixx.setPixelColor((matrix[i][0]), matrixx.Color(255,0,0)); // Visual indicator for paddle 2's goal
              }
              matrixx.show();
              vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(ballY == 30)
            {
              for(int i=0; i<15 ; i++)
              {
              matrixx.setPixelColor((matrix[i][30]), matrixx.Color(255,0,0)); // Visual indicator for paddle 1's goal
              }
              matrixx.show();
              vTaskDelay(pdMS_TO_TICKS(200));
            }
            ballDirY *= -1;  // Inverts the Y direction
        }
    //Collisions with con PADDLE 1
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X-2)
        {
          if(ballDirX==1) //To protect PADDLE 1 borders from "ghost" hit.
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
          }
          else{}
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X-1)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X+1)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X+2)
        {
          if(ballDirX==-1) //To protect PADDLE 2 borders from "ghost" hit.
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
          }
          else{}
        }        
    //Colisiones con PADDLE 2
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X-2)
        {
          if(ballDirX==1)
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
          }
          else
          {

          }
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X-1)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X+1)
        {
            ballDirY *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X+2)
        {
          if(ballDirX==-1)
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--; //each time the ball has a collision with a paddle, the speed increase.
          }
          else
          {

          }
        }
    vTaskDelay(pdMS_TO_TICKS(speedball)); //This corresponds to ball's speed
  }
}

void Taskrender( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.
  matrixx.begin();
  matrixx.setBrightness(200);

  for(;;)
  {
    //Serial.println("Executing taskrender");
    matrixx.clear(); //We turn off the entire matrix

    //Draw the ball
    matrixx.setPixelColor(matrix[ballX][ballY], matrixx.Color(255,255,255));
    //Draw paddle1
    for(int i=0;i<PADDLE_HEIGHT;i++)
    {
    matrixx.setPixelColor((matrix[paddle1X-1][PADDLE1Y])+i, matrixx.Color(0,255,0));
    }
    //Draw paddle2
    for(int i=0;i<PADDLE_HEIGHT;i++)
    {
    matrixx.setPixelColor((matrix[paddle2X-1][PADDLE2Y])+i, matrixx.Color(0,0,255));
    }

    matrixx.show();
    vTaskDelay(pdMS_TO_TICKS(20)); //Rendering update frequency.

  }
}
