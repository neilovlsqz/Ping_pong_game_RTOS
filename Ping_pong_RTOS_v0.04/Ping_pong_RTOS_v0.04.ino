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

int matrix[15][31];
/*
  [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14],
  [29,28,27,26,25,24,23,22,21,20,19,18,17,16,15],
  [30,31,32,33,34,35,36,37,38,39,40,41,42,43,44],
  [59,58,57,56,55,54,53,52,51,50,49,48,47,46,45],
  ...
  [319,318,317,316,315,314,313,312,311,310,309,308,307,306,305].  // I wrote this to try to explain how the led matrix was made, in zig zag. 
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

int speedball = 70; //ball speed

// Initialize the led matrix mapping.

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
  initMatrixMapping();

  xAnalogSemaphore = xSemaphoreCreateMutex(); //Semaphore creation for analogReads

  xTaskCreatePinnedToCore(Taskpaddle1, "PaddleONE", 2048 , NULL, 3, NULL, 0 ); //Core 0

  xTaskCreatePinnedToCore(Taskpaddle2, "PaddleTWO", 2048 , NULL, 3, NULL, 0 );

  xTaskCreatePinnedToCore(Taskball, "Ball", 2048, NULL, 2, NULL, 1); //Core 1

  xTaskCreatePinnedToCore(Taskrender, "Render", 2048, NULL, 2, NULL, 1);
  
  //vTaskStartScheduler();


 }

void loop()
 {
    // This thing is empty, all the fun is in the tasks.
 }


////////////////////////////////////////////////////////////////////////////////////////////////// Los task
void Taskpaddle1( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.

  for(;;)
  {
    int SensorVal1 = analogRead(POT1_PIN);
    //Serial.println(SensorVal1);
    if (xSemaphoreTake(xAnalogSemaphore, portMAX_DELAY)) 
    {
    int SensorVal1 = analogRead(POT1_PIN);
    Serial.println(SensorVal1);
    
    // This range is removed to control only the center of each paddle; 
    // the left and right edges are controlled by the definition of PADDLE_HEIGHT 
    // to prevent the paddle from leaving the matrix.
      /*if(SensorVal1 >=0 && SensorVal1 <=2)
      {
        paddle1X = 0;       
      }*/
      if(SensorVal1 >=0 && SensorVal1 <=315)
      {
        paddle1X = 13;
      }
      if(SensorVal1 >=316 && SensorVal1 <=630)
      {
        paddle1X = 12;
      }
      if(SensorVal1 >=631 && SensorVal1 <=945)
      {
        paddle1X = 11;
      }
      if(SensorVal1 >=946 && SensorVal1 <=1260)
      {
        paddle1X = 10;
      }
      if(SensorVal1 >=1261 && SensorVal1 <=1575)
      {
        paddle1X = 9;
      }
      if(SensorVal1 >=1576 && SensorVal1 <=1890)
      {
        paddle1X = 8;
      }
      if(SensorVal1 >=1891 && SensorVal1 <=2205)
      {
        paddle1X = 7;
      }
      if(SensorVal1 >=2206 && SensorVal1 <=2520)
      {
        paddle1X = 6;
      }
      if(SensorVal1 >=2521 && SensorVal1 <=2835)
      {
        paddle1X = 5;
      }
      if(SensorVal1 >=2836 && SensorVal1 <=3150)
      {
        paddle1X = 4;
      }
      if(SensorVal1 >=3151 && SensorVal1 <=3465)
      {
        paddle1X = 3;
      }
      if(SensorVal1 >=3466 && SensorVal1 <=3780)
      {
        paddle1X = 2;
      }
      if(SensorVal1 >=3781 && SensorVal1 <=4095)
      {
        paddle1X = 1;
      }
      /*if(SensorVal1 >=3550 && SensorVal1 <=3822)
      {
        paddle1X = 14;
      }*/
      xSemaphoreGive(xAnalogSemaphore);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Taskpaddle2( void *pvParameters )
{

  (void) pvParameters; //Just to avoid warnings about no used variables.

  for(;;)
  {

    if (xSemaphoreTake(xAnalogSemaphore, portMAX_DELAY)) 
    {
    int SensorVal2 = analogRead(POT2_PIN);
    Serial.println(SensorVal2);
    
    // This range is removed to control only the center of each paddle; 
    // the left and right edges are controlled by the definition of PADDLE_HEIGHT 
    // to prevent the paddle from leaving the matrix.
      /*if(SensorVal2 >=0 && SensorVal2 <=2)
      {
        paddle2X = 0;       
      }*/
      if(SensorVal2 >=0 && SensorVal2 <=315)
      {
        paddle2X = 1;
      }
      if(SensorVal2 >=316 && SensorVal2 <=630)
      {
        paddle2X = 2;
      }
      if(SensorVal2 >=631 && SensorVal2 <=945)
      {
        paddle2X = 3;
      }
      if(SensorVal2 >=946 && SensorVal2 <=1260)
      {
        paddle2X = 4;
      }
      if(SensorVal2 >=1261 && SensorVal2 <=1575)
      {
        paddle2X = 5;
      }
      if(SensorVal2 >=1576 && SensorVal2 <=1890)
      {
        paddle2X = 6;
      }
      if(SensorVal2 >=1891 && SensorVal2 <=2205)
      {
        paddle2X = 7;
      }
      if(SensorVal2 >=2206 && SensorVal2 <=2520)
      {
        paddle2X = 8;
      }
      if(SensorVal2 >=2521 && SensorVal2 <=2835)
      {
        paddle2X = 9;
      }
      if(SensorVal2 >=2836 && SensorVal2 <=3150)
      {
        paddle2X = 10;
      }
      if(SensorVal2 >=3151 && SensorVal2 <=3465)
      {
        paddle2X = 11;
      }
      if(SensorVal2 >=3466 && SensorVal2 <=3780)
      {
        paddle2X = 12;
      }
      if(SensorVal2 >=3781 && SensorVal2 <=4095)
      {
        paddle2X = 13;
      }
      /*if(SensorVal2 >=3550 && SensorVal2 <=3822)
      {
        paddle2X = 14;
      }*/
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
            speedball--;
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X-2)
        {
          if(ballDirX==1) //To protect PADDLE 1 borders from "ghost" hit.
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--;
          }
          else{}
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X-1)
        {
            ballDirY *= -1;
            speedball--;
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X+1)
        {
            ballDirY *= -1;
            speedball--;
        }
    else if(ballY == PADDLE1Y+1 && ballX == paddle1X+2)
        {
          if(ballDirX==-1) //To protect PADDLE 2 borders from "ghost" hit.
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--;
          }
          else{}
        }        
    //Colisiones con PADDLE 2
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X)
        {
            ballDirY *= -1;
            speedball--;
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X-2)
        {
          if(ballDirX==1)
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--;
          }
          else
          {

          }
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X-1)
        {
            ballDirY *= -1;
            speedball--;
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X+1)
        {
            ballDirY *= -1;
            speedball--;
        }
    else if(ballY == PADDLE2Y-1 && ballX == paddle2X+2)
        {
          if(ballDirX==-1)
          {
            ballDirY *= -1;
            ballDirX *= -1;
            speedball--;
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