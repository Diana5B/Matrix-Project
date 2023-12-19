#include "LedControl.h"
 
// Pin configuration for the MAX7219 LED matrix display
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int SWPin = 2;  // Joystick button pin
 
// Create an instance of the LedControl class for interfacing with the LED matrix
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER
 
// Global variables for matrix brightness and player position
byte matrixBrightness = 2;
 
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
 
// Threshold values for joystick input
const int minThreshold = 200;
const int maxThreshold = 600;
 
// Time intervals for game logic and bomb shooting
const byte moveInterval = 300;
const byte bombInterval = 500;  // Interval between shooting bombs
 
// Last time the player moved and last time a bomb was shot
unsigned long long lastMoved = 0;
unsigned long long lastBombTime = 0;
 
// Blinking time intervals for player, food, and bombs
const int playerBlinkTime = 500;  // Adjust this for player blinking speed
const int foodBlinkTime = 300;    // Adjust this for food blinking speed
const int bombBlinkTime = 100;    // Adjust this for bomb blinking speed
 
// Last time each element blinked
unsigned long long lastPlayerBlinkTime = 0;
unsigned long long lastFoodBlinkTime = 0;
unsigned long long lastBombBlinkTime = 0;
 
// Blinking state flags for player, food, and bombs
bool playerBlinkState = false;
bool foodBlinkState = false;
bool bombBlinkState = false;
 
// Size of the LED matrix
const byte matrixSize = 8;
 
// Flag to track if the matrix has changed and needs to be updated
bool matrixChanged = true;
 
// 2D array to represent the game matrix
byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },  //0
  { 0, 1, 1, 1, 1, 1, 1, 0 },  //1
  { 0, 1, 0, 0, 0, 0, 1, 0 },  //2
  { 0, 1, 0, 1, 1, 0, 1, 0 },  //3
  { 0, 1, 0, 0, 0, 0, 1, 0 },  //4
  { 0, 1, 1, 1, 1, 1, 1, 0 },  //5
  { 0, 0, 0, 0, 0, 0, 0, 0 },  //6
  { 0, 0, 0, 0, 0, 0, 0, 0 }   //7
};
 
// Byte array to represent a pattern for a bomb
byte matrixByte[matrixSize] = {
  B00000000,
  B01000100,
  B00101000,
  B00010000,
  B00010000,
  B00010000,
  B01010100,
  B00000000
};
 
// Variables for random food position
int randomX = 0;
int randomY = 0;
const int minValue = 0;
const int maxValue = 8;
 
// Player score
int score = 0;
 
// Setup function - runs once at the beginning
void setup() {
  Serial.begin(9600);
 
  // Initialize the LED matrix display
  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
 
  // Generate initial food and set player position
  generateFood();
  matrix[xPos][yPos] = 1;
 
  // Set joystick button pin as input with an internal pull-up resistor
  pinMode(SWPin, INPUT_PULLUP);
}
 
// Main game loop function
void loop() {
  // Check if it's time to update player position
  if (millis() - lastMoved > moveInterval) {
    updatePositions();
    lastMoved = millis();
  }
 
  // Check if it's time to blink player and food
  if (millis() - lastPlayerBlinkTime > playerBlinkTime) {
    blinkPlayer();
    lastPlayerBlinkTime = millis();
  }
  if (millis() - lastFoodBlinkTime > foodBlinkTime) {
    blinkFood();
    lastFoodBlinkTime = millis();
  }
 
  // Check if it's time to shoot a bomb
  if (millis() - lastBombTime > bombInterval) {
    if (digitalRead(SWPin) == LOW)  // Check if the joystick button is pressed
    {
      shootBomb();
    }
    lastBombTime = millis();
  }
 
  // Check if it's time to blink bombs
  if (millis() - lastBombBlinkTime > bombBlinkTime) {
    bombBlink();
    lastBombBlinkTime = millis();
  }
 
  // Check if the matrix has changed and update display
  if (matrixChanged == true) {
    updateMatrix();
    matrixChanged = false;
  }
 
  // Check if the player has eaten the food and generate a new one
  if (randomX == xPos && randomY == yPos) {
    score++;
    Serial.println(score);
    generateFood();
  }
}
 
// Function to generate a new random position for food
void generateFood() {
  randomX = random(minValue, maxValue);
  randomY = random(minValue, maxValue);
 
  // Avoid generating food on the player's position or on existing food
  while (randomX == xPos && randomY == yPos && matrix[randomX][randomY] == 1) {
    randomX = random(minValue, maxValue);
    randomY = random(minValue, maxValue);
    
  }
  Serial.print("test");
  Serial.println(randomX);
   Serial.println(randomY);
  // Set the new food position
  matrix[randomX][randomY] = 1;
  matrixChanged = true;
}
 
// Function to blink the food (toggle its state)
void blinkFood() {
  matrix[randomX][randomY] = !matrix[randomX][randomY];
  foodBlinkState = !foodBlinkState;
  matrixChanged = true;
}
 
// Function to blink the player (toggle its state)
void blinkPlayer() {
  //matrix[xPos][yPos] = !matrix[xPos][yPos];
  playerBlinkState = !playerBlinkState;
  matrixChanged = true;
}
 
// Function to blink bombs (toggle their state)
void bombBlink() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == 2)  // Check for bombs
      {
        lc.setLed(0, row, col, bombBlinkState);
      }
    }
  }
  bombBlinkState = !bombBlinkState;
}
 
// Function to shoot a bomb from the player's position
void shootBomb() {
  int bombX = xPos;
  int bombY = yPos;
 
  // Determine the direction based on joystick input
  int xValue = analogRead(A0);
  int yValue = analogRead(A1);
 
  // Adjust bomb position based on joystick input
  if (xValue < minThreshold) {
    /*if (bombX < matrixSize - 1 && matrix[bombX + 1][bombY] != 1) {
      bombX++;
    }*/
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX + 1][bombY] != 1) {
      Serial.println(bombX);
      bombX++;
      matrixChanged = true;
      lc.setLed(0, bombX - 1, bombY, 0);
      lc.setLed(0, bombX, bombY, 1);
      matrix[bombX - 1][bombY] = 0;
      if(bombX<matrixSize){
        matrix[bombX][bombY] = 2;  // 2 represents the bomb in the matrix
      }
      delay(100);                // Delay to slow down the bomb movement
    }
  } else if (xValue > maxThreshold) {
    /*
    if (bombX > 0 && matrix[bombX - 1][bombY] != 1) {
      bombX--;
    }*/
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX-1][bombY] != 1)
    {
      Serial.println(bombX);
        bombX--;
        matrixChanged = true;
        lc.setLed(0, bombX+1, bombY, 0);
        lc.setLed(0, bombX, bombY, 1);
        matrix[bombX + 1][bombY] = 0;
        if(bombX>=0){
          matrix[bombX][bombY] = 2; // 2 represents the bomb in the matrix
        }
        delay(100); // Delay to slow down the bomb movement
    }
  }
 
  if (yValue > maxThreshold) {
    /*
    if (bombY < matrixSize - 1 && matrix[bombX][bombY + 1] != 1) {
      bombY++;
    }*/
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX][bombY+1] != 1)
    {
      Serial.println(bombX);
        bombY++;
        matrixChanged = true;
        lc.setLed(0, bombX, bombY-1, 0);
        lc.setLed(0, bombX, bombY, 1);
        matrix[bombX][bombY-1] = 0;
        if(bombY<matrixSize){
          matrix[bombX][bombY] = 2; // 2 represents the bomb in the matrix
        }
        delay(100); // Delay to slow down the bomb movement
    }
  } else if (yValue < minThreshold) {
    /*
    if (bombY > 0 && matrix[bombX][bombY - 1] != 1) {
      bombY--;
    }*/
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX][bombY-1] != 1)
    {
      Serial.println(bombX);
        bombY--;
        matrixChanged = true;
        lc.setLed(0, bombX, bombY+1, 0);
        lc.setLed(0, bombX, bombY, 1);
        matrix[bombX][bombY+1] = 0;
        if(bombY>=0){
          matrix[bombX][bombY] = 2; // 2 represents the bomb in the matrix
        }
        delay(100); // Delay to slow down the bomb movement
    }
  }
 
 
 
  // Detonate the bomb and remove walls
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (bombX + i >= 0 && bombX + i < matrixSize && bombY + j >= 0 && bombY + j < matrixSize) {
        matrix[bombX + i][bombY + j] = 0;
      }
    }
  }
  matrix[xPos][yPos] = 1;
}
 
// Function to update the byte matrix for the LED display
void updateByteMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, matrixByte[row]);
  }
}
 
// Function to update the entire LED matrix display based on the game matrix
void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      // Check if it's the player and in the blinking state
      if (row == xPos && col == yPos && playerBlinkState) {
        lc.setLed(0, row, col, 0);
      }
      // Check if it's the food and in the blinking state
      else if (row == randomX && col == randomY && foodBlinkState) {
        lc.setLed(0, row, col, 0);
      }
      // Check if it's a bomb and in the blinking state
      else if (matrix[row][col] == 2) {
        lc.setLed(0, row, col, bombBlinkState);
      }
      // Otherwise, set the LED based on the game matrix
      else {
        lc.setLed(0, row, col, matrix[row][col]);
      }
    }
  }
}
 
// Function to update the player's position based on joystick input
void updatePositions() {
  int xValue = analogRead(A0);
  int yValue = analogRead(A1);
 
  xLastPos = xPos;
  yLastPos = yPos;
 
  // Update player position based on joystick input
  if (xValue < minThreshold) {
    if (xPos < matrixSize - 1 && matrix[xPos + 1][yPos] != 1) {
      xPos++;
    }
  }
  if (xValue > maxThreshold) {
    if (xPos > 0 && matrix[xPos - 1][yPos] != 1) {
      xPos--;
    }
  }
 
  if (yValue > maxThreshold) {
    if (yPos < matrixSize - 1 && matrix[xPos][yPos + 1] != 1) {
      yPos++;
    }
  }
 
  if (yValue < minThreshold) {
    if (yPos > 0 && matrix[xPos][yPos - 1] != 1) {
      yPos--;
    }
  }
 
  // Check if the player position has changed
  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
 
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
  }
}
