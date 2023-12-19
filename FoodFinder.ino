#include "LedControl.h"// Include LedControl library for 
controlling the LED matrix
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "pitches.h"  // Include the pitches library for note frequencies

#define NOTE_C4  261.63
#define NOTE_E4  329.63
#define NOTE_G4  392.00
#define NOTE_A4  440.00
#define NOTE_B4  493.88
#define NOTE_D4  293.66
#define NOTE_F4  349.23
#define NOTE_REST 0

#define SOUND_DURATION 200  // Adjust the duration as needed

const int minValue = 0;
const int maxValue = matrixSize - 1;

// Pin configuration for the MAX7219 LED matrix display
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int SWPin = 2;  // Joystick button pin
const int xPin = A0;
const int yPin = A1;

const int buzzerPin = 11;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

const int brightnessPin = 10;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;

bool sound = true; // Sound state, true for ON, false for OFF
const int soundAddress = 0; // EEPROM address to store sound state
bool achievedHighScore = false;
// Create an LedControl object to manage the LED matrix
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

// Global variables for matrix brightness and player position
byte matrixBrightness = 2;
 
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
 
// Threshold values for joystick input
const int minThreshold = 200;
const int maxThreshold = 800;

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

// Define global variables for time limits
int timeLimitLevel1 = 15000;  // 15 seconds for level 1
int timeLimitLevel2 = 10000;  // 10 seconds for level 2
int timeLimitLevel3 = 5000;   // 5 seconds for level 3
unsigned long startTime;      // Variable to store the start time



// Constants representing menu options
const int MENU_OPTION_START_GAME = 0;
const int MENU_OPTION_INFO = 1;
const int MENU_OPTION_SETTINGS = 2;
const int MENU_OPTION_HIGH_SCORE = 3;
const int MENU_OPTION_ABOUT = 4;

// Constants representing settings menu options
const int SETTINGS_OPTION_LCD_BRIGHTNESS = 0;
const int SETTINGS_OPTION_MATRIX_LIGHT = 1;
const int SETTINGS_OPTION_SOUND = 2;
const int SETTINGS_OPTION_RESET_HIGHSCORE = 3;
const int SETTINGS_OPTION_EXIT = 4;

// Flag variables with more descriptive names
bool isInMainMenu = true;
bool isGameInProgress = false;
bool isInSettings = false;
bool isInAbout = false;
bool isInInfo = false;
bool isGameLost = false;
bool isWinner = false;
bool isLcdSettings = false;
bool isMatrixSettings = false;



// Size of the LED matrix
const byte matrixSize = 8;
 
// Flag to track if the matrix has changed and needs to be updated
bool matrixChanged = true;


int lcdBrightnessLevels[] = { 0, 51, 102, 153, 204, 255 };  //lcd brightness levels
int matrixBrightnessLevels[] = { 1, 3, 6, 9, 12, 15 };      //matrix brightness levels

// Constants for brightness levels
const int BRIGHTNESS_MIN_LEVEL = 0;
const int BRIGHTNESS_MAX_LEVEL = 5;


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

byte smileyFace[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte grumpyFace[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};
 
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};


// Variables for random food position
int randomX = 0;
int randomY = 0;
const int minValue = 0;
const int maxValue = 8;
 
// Player score
int destroyedWalls = 0;
struct PlayerScore {
  char playerName[4];  // Max 3 letters for the player's name
  int score;
};

PlayerScore topScores[3];



void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  lcd.begin(16, 2);

  randomSeed(analogRead(0));

  // Initialize the matrix and set initial player position
  resetMatrix();
  xPos = random(minValue, maxValue);
  yPos = random(minValue, maxValue);

  // Load sound state from EEPROM
  sound = EEPROM.read(soundAddress);

  // Print initial state on LCD
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print(F("SOUND"));

  lcd.setCursor(4, 1);
  lcd.print(sound ? F("ON") : F("OFF"));

  lcd.setCursor(14, 1);
  lcd.print(F("<"));
  lcd.print(F(">"));

  // Load top scores from EEPROM
  for (int i = 0; i < 3; i++) {
    EEPROM.get(i * sizeof(HighScore), topScores[i]);
  }

  loadTopScoresFromEEPROM();
  // Initialize the timer
  startTime = millis();
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
    score = score+5;
    Serial.println(score);
    generateFood();
  }

  // Check for time limit based on the current level
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;

  if (level == 1 && elapsedTime > timeLimitLevel1) {
    gameOver();
  } else if (level == 2 && elapsedTime > timeLimitLevel2) {
    gameOver();
  } else if (level == 3 && elapsedTime > timeLimitLevel3) {
    gameOver();
  }
}




// Function to generate a new random position for food
void generateFood() {
  randomX = random(minValue, maxValue);
  randomY = random(minValue, maxValue);

  while (randomX == xPos && randomY == yPos || matrix[randomX][randomY] != 0) {
    randomX = random(minValue, maxValue);
    randomY = random(minValue, maxValue);
  }

  matrix[randomX][randomY] = 1;  // Set the new food position
  matrixChanged = true;

  // Check if the player achieved the maximum level
  if (currentLevel < MAX_LEVELS) {
    // Generate additional walls and food for the current level
    generateWallsAndFood();
  }

  lcd.setCursor(0, 1);
  lcd.print(F("Score: "));
  lcd.print(score);

  playSound(NOTE_E4);
}

void generateWallsAndFood() {
  for (int level = 1; level <= currentLevel; level++) {
    int wallCount = random(1, 4);  // Generate 1 to 3 walls
    for (int i = 0; i < wallCount; i++) {
      int wallX = random(minValue, maxValue);
      int wallY = random(minValue, maxValue);

      // Check if the randomly generated position is valid and not already occupied
      if (matrix[wallX][wallY] == 0) {
        matrix[wallX][wallY] = 1;  // Set the wall
        matrixChanged = true;
      }
    }

    // Generate an additional food item
    int foodX, foodY;
    do {
      foodX = random(minValue, maxValue);
      foodY = random(minValue, maxValue);
    } while (matrix[foodX][foodY] != 0);

    matrix[foodX][foodY] = 1;  // Set the additional food
    matrixChanged = true;
  }
}

// Function to blink the food (toggle its state)
void blinkFood() {
  matrix[randomX][randomY] = !matrix[randomX][randomY];
  foodBlinkState = !foodBlinkState;
  matrixChanged = true;
}

// Function to blink the player (toggle its state)
void blinkPlayer() {
  playerBlinkState = !playerBlinkState;
  matrixChanged = true;
}

void playWallDestructionSound() {
  playSound(NOTE_D4);
  // Adjust the delay as needed
  delay(SOUND_DURATION);
  noTone(buzzerPin);
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
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX + 1][bombY] != 1) {
      bombX++;
      matrixChanged = true;
      lc.setLed(0, bombX - 1, bombY, 0);
      lc.setLed(0, bombX, bombY, 1);
      matrix[bombX - 1][bombY] = 0;
      if (bombX < matrixSize) {
        matrix[bombX][bombY] = 2;  // 2 represents the bomb in the matrix
      }
      delay(100);  // Delay to slow down the bomb movement
    }
  } else if (xValue > maxThreshold) {
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX - 1][bombY] != 1) {
      bombX--;
      matrixChanged = true;
      lc.setLed(0, bombX + 1, bombY, 0);
      lc.setLed(0, bombX, bombY, 1);
      matrix[bombX + 1][bombY] = 0;
      if (bombX >= 0) {
        matrix[bombX][bombY] = 2;  // 2 represents the bomb in the matrix
      }
      delay(100);  // Delay to slow down the bomb movement
    }
  // Detonate the bomb and remove walls
for (int i = -1; i <= 1; i++) {
  for (int j = -1; j <= 1; j++) {
    if (bombX + i >= 0 && bombX + i < matrixSize && bombY + j >= 0 && bombY + j < matrixSize) {
      if (matrix[bombX + i][bombY + j] == 1) {
        destroyedWalls++;
        playWallDestructionSound(); // Play sound when a wall is destroyed
      }
      matrix[bombX + i][bombY + j] = 0;
    }
  }
}
  // Update the score based on the number of destroyed walls
  score += destroyedWalls;
  destroyedWalls = 0;  // Reset the count

  matrix[xPos][yPos] = 1;

  }

  if (yValue > maxThreshold) {
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX][bombY + 1] != 1) {
      bombY++;
      matrixChanged = true;
      lc.setLed(0, bombX, bombY - 1, 0);
      lc.setLed(0, bombX, bombY, 1);
      matrix[bombX][bombY - 1] = 0;
      if (bombY < matrixSize) {
        matrix[bombX][bombY] = 2;  // 2 represents the bomb in the matrix
      }
      delay(100);  // Delay to slow down the bomb movement
    }
  } else if (yValue < minThreshold) {
    while (bombX >= 0 && bombX < matrixSize && bombY >= 0 && bombY < matrixSize && matrix[bombX][bombY - 1] != 1) {
      bombY--;
      matrixChanged = true;
      lc.setLed(0, bombX, bombY + 1, 0);
      lc.setLed(0, bombX, bombY, 1);
      matrix[bombX][bombY + 1] = 0;
      if (bombY >= 0) {
        matrix[bombX][bombY] = 2;  // 2 represents the bomb in the matrix
      }
      delay(100);  // Delay to slow down the bomb movement
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








void navigateMainMenu() {
  if (selectedOption == MENU_OPTION_START_GAME) {
    startGame();
  } else if (selectedOption == MENU_OPTION_INFO) {
    enterInfoSection();
  } else if (selectedOption == MENU_OPTION_SETTINGS) {
    enterSettingsSection();
    settings();
    settingsMenu();
  } else if (selectedOption == MENU_OPTION_HIGH_SCORE) {
    enterHighScoreSection();
    displayHighScore();
  } else if (selectedOption == MENU_OPTION_ABOUT) {
    enterAboutSection();
  }
}

void startGame() {
  gameStartTime = millis();
  Serial.print(gameStartTime);
  isInMainMenu = false;
  isGameInProgress = true;
  isInSettings = false;
  isInAbout = false;
  isInInfo = false;
  isGameLost = false;
  isWinner = false;
  remainingLives = INITIAL_LIVES;
  currentRoom = ROOM_1;
  clearMatrix();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Lives:"));
  for (int i = 0; i < remainingLives; i++) {
    lcd.write(LIFE_ICON);
  }
  lcd.setCursor(0, 1);
  lcd.print(F("Score: "));
  displayTopTimes(elapsedGameTime);

  randomSeed(analogRead(RANDOM_SEED_PIN));
  generateRoom();
  playerXPos = INITIAL_PLAYER_X_POS;
  playerYPos = INITIAL_PLAYER_Y_POS;
  matrixChanged == true;

  // Display the "GOOD LUCK" message
  displayGoodLuckMessage();
  // Play the start game sound
  playStartGameSound();
}

void enterInfoSection() {
  isInMainMenu = false;
  isGameInProgress = false;
  isInSettings = false;
  isInAbout = false;
  isInInfo = true;
  isWinner = false;
  winner = false;
}

void enterSettingsSection() {
  isInMainMenu = false;
  isGameInProgress = false;
  isInSettings = true;
  isInAbout = false;
  isInInfo = false;
  isGameLost = false;
  isWinner = false;
  lcd.clear();
  clearMatrix();
}

void enterHighScoreSection() {
  isInMainMenu = false;
  isGameInProgress = false;
  isInSettings = false;
  isInAbout = false;
  isInInfo = false;
  isGameLost = false;
  isWinner = false;
  lcd.clear();
  clearMatrix();
}

void enterAboutSection() {
  isInMainMenu= false;
  isGameInProgress = false;
  isInSettings = false;
  isInAbout = true;
  isInInfo = false;
  isGameLost = false;
  isWinner = false;
}

void settings() {
  isInSettings = true;
  settingsAnimation();
  lcd.setCursor(1, 0);  // title
  lcd.print(F("SETTINGS:"));

  lcd.setCursor(15, 0);  // directions
  lcd.write(4);

  lcd.setCursor(1, 1);  // current option
  lcd.print(F(">"));
  lcd.print(settingsOptions[setting]);
}

void settingsMenu() {
  if (setting == SETTINGS_OPTION_LCD_BRIGHTNESS) {
    isInMainMenu = false;
    isGameInProgress = false;
    isInSettings = false;
    isInAbout = false;
    isInInfo = false;
    isGameLost = false;
    isWinner = false;
    isLcdSettings = true;
    isMatrixSettings = false;
    lcdBrightnessFunc();
  } else if (setting == SETTINGS_OPTION_MATRIX_LIGHT) {
    clearMatrix();
    isInMainMenu = false;
    isGameInProgress = false;
    isInSettings = false;
    isInAbout = false;
    isInInfo = false;
    isGameLost = false;
    isWinner = false;
    isLcdSettings = false;
    isMatrixSettings = true;
    matrixBrightnessFunc();
  } else if (setting == SETTINGS_OPTION_SOUND) {
    clearMatrix();
    isInMainMenu = false;
    isGameInProgress = false;
    isInSettings = false;
    isInAbout = false;
    isInInfo = false;
    isGameLost = false;
    isWinner = false;
    isLcdSettings = false;
    isMatrixSettings = false;
    soundSettings = true;
    soundFunc();
  } else if (setting == SETTINGS_OPTION_RESET_HIGHSCORE) {
    clearMatrix();
    isInMainMenu = false;
    isGameInProgress = false;
    isInSettings = false;
    isInAbout = false;
    isInInfo = false;
    isGameLost = false;
    isWinner = false;
    isLcdSettings = false;
    isMatrixSettings = false;
    soundSettings = false;
    inReset = true;
    resetHSFunc();
  } else if (setting == SETTINGS_OPTION_EXIT) {
    isInMainMenu = true;
    isGameInProgress = false;
    isInSettings = false;
    isInAbout = false;
    isInInfo = false;
    isGameLost = false;
    isWinner = false;
    clearMatrix();
  }
}






void adjustBrightness(int& brightness, int* brightnessLevels, int brightnessAddress, LiquidCrystal& lcd, int cursorX, int cursorY, const char* title) {
  int level = brightness;
  
  // Display the title on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F(title));

  // Display brightness adjustment indicators on the LCD
  lcd.setCursor(cursorX, cursorY);
  lcd.print(F("-"));  //stored in Flash memory
  for (int lvl = 0; lvl < BRIGHTNESS_MAX_LEVEL + 1; lvl++) {
    // Display a visual indicator for each brightness level
    if (level >= 0) {
      lcd.write(2);  // Brightness indicator
    } else {
      lcd.write(1);  // Dimmed indicator
    }
    level--;
  }
  lcd.print(F("+"));

  // Display navigation indicators on the LCD
  lcd.setCursor(cursorX + 10, cursorY);
  lcd.print(F("<"));
  lcd.print(F(">"));

  // Adjust the brightness based on the selected level
  analogWrite(brightnessPin, brightnessLevels[brightness]);

  // Update the brightness level in EEPROM
  EEPROM.update(brightnessAddress, brightness);
  brightness = EEPROM.read(brightnessAddress);
}

void lcdBrightnessFunc() {
  adjustBrightness(lcdBrightness, lcdBrightnessLevels, lcdBrightnessAddress, lcd, 4, 1, "LCD BRIGHTNESS");
}

void matrixBrightnessFunc() {
  adjustBrightness(matrixBrightness, matrixBrightnessLevels, matrixBrightnessAddress, lcd, 4, 1, "MATRIX LIGHT");
  lc.setIntensity(0, matrixBrightnessLevels[matrixBrightness]);
}




void playSound(int note) {
  tone(buzzerPin, note, SOUND_DURATION);
  delay(SOUND_DURATION + 50);  // Adjust the delay as needed
  noTone(buzzerPin);
}

void playStartGameSound() {
  // Play a sound to indicate the start of the game
  playSound(NOTE_C4);
  delay(200);  // Adjust the delay as needed
  noTone(buzzerPin);
}


void updateTopScores() {
  // Check if the current score is greater than any of the top scores
  for (int i = 0; i < 3; i++) {
    if (score > topScores[i].score) {
      // Shift scores down to make room for the new score
      for (int j = 2; j > i; j--) {
        strcpy(topScores[j].playerName, topScores[j - 1].playerName);
        topScores[j].score = topScores[j - 1].score;
      }

      // Get the player's name (assumes it's stored in the global variable playerName)
      strncpy(topScores[i].playerName, playerName, 3);
      topScores[i].playerName[3] = '\0';  // Null-terminate the string
      topScores[i].score = score;

      // Save the top scores to EEPROM
      saveTopScoresToEEPROM();

      break;  // Exit the loop since the score is placed in the top scores
    }
  }
}

void saveTopScoresToEEPROM() {
  int address = 0;

  for (int i = 0; i < 3; i++) {
    EEPROM.put(address, topScores[i]);
    address += sizeof(PlayerScore);
  }
}

void loadTopScoresFromEEPROM() {
  int address = 0;

  for (int i = 0; i < 3; i++) {
    EEPROM.get(address, topScores[i]);
    address += sizeof(PlayerScore);
  }
}



void gameOver() {
  // Play a special sound on the buzzer (you need to define the sound)
  playSpecialBuzzerSound();

  // Display sad face on the LED matrix
  displayMatrix(grumpyFace);

  // Display "GAME OVER" on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("GAME OVER"));

  // Update top scores
  updateTopScores();

  // Display top scores on the LCD (adjust positions and formatting)
  lcd.setCursor(0, 1);
  lcd.print(F("TOP SCORES"));

  for (int i = 0; i < 3; i++) {
    lcd.setCursor(0, i + 2);
    lcd.print(topScores[i].playerName);
    lcd.print(F(": "));
    lcd.print(topScores[i].score);
  }

  // Check if the time limit was exceeded
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;

  if (level == 1 && elapsedTime > timeLimitLevel1) {
    lcd.setCursor(0, 5);
    lcd.print(F("Time Limit Exceeded!"));
  } else if (level == 2 && elapsedTime > timeLimitLevel2) {
    lcd.setCursor(0, 5);
    lcd.print(F("Time Limit Exceeded!"));
  } else if (level == 3 && elapsedTime > timeLimitLevel3) {
    lcd.setCursor(0, 5);
    lcd.print(F("Time Limit Exceeded!"));
  }

  // Display achievement message if the player achieved a high score
  if (achievedHighScore) {
    lcd.setCursor(0, 6);
    lcd.print(F("YOU ACHIEVED HIGHSCORE!"));

    // Display a heart on the LED matrix
    displayHeartOnMatrix();
  }

}

void winGame() {
  // Play a victory sound on the buzzer (you need to define the sound)
  playVictoryBuzzerSound();

  // Display happy face on the LED matrix
  displayMatrix(smileyFace);

  // Display "YOU WIN!" on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("YOU WIN!"));

  // Update top scores
  updateTopScores();

  // Display top scores on the LCD (adjust positions and formatting)
  lcd.setCursor(0, 1);
  lcd.print(F("TOP SCORES"));

  for (int i = 0; i < 3; i++) {
    lcd.setCursor(0, i + 2);
    lcd.print(topScores[i].playerName);
    lcd.print(F(": "));
    lcd.print(topScores[i].score);
  }

  // Display achievement message if the player achieved a high score
  if (achievedHighScore) {
    lcd.setCursor(0, 5);
    lcd.print(F("YOU ACHIEVED HIGHSCORE!"));

    // Display a heart on the LED matrix
    displayHeartOnMatrix();
  }

  
}

void displayHeartOnMatrix() {
  // Assuming matrixSize is 8x8
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, heart[i] & (1 << (matrixSize - 1 - j)));
      //The 1 << (matrixSize - 1 - j) is used to correctly map the bits in the heart pattern to the LEDs in the matrix
    }
  }
  delay(3000);  // Display the heart for 3 seconds (adjust as needed)
  lc.clearDisplay(0);
}

void displayGoodLuckMessage() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(F("GOOD LUCK!"));
  delay(2000);  // Display the message for 2 seconds (adjust as needed)
  lcd.clear();
}

