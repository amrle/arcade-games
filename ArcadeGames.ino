/*
  ArcadeGames program.
  Play 2 games (Snake and Atari Breakout) on a 8x8 LED matrix.
  Your score is displayed on a INEX-GLCD5110 display.
  When you win a game, 3 LEDs light up and fade.
*/

// Libraries needed to control a INEX-GLCD5110 Display.
// More information about the libraries can be found here:
// https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library
// https://lastminuteengineers.com/nokia-5110-lcd-arduino-tutorial/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Libraries needed to control a 8x8 LED dot matrix with a MAX7219 breakout board.
// More information about the libraries can be found here:
// http://wayoda.github.io/LedControl/pages/software.html
#include "LedControl.h"

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 2, 4); // Initialize LCD pins (CLK,DIN,D/C,CE,RST).

LedControl matrix = LedControl(10, 8, 9, 1); // Initialize LED matrix pins (DATA, CLK, CS/LOAD).

int leds[] = {11, 3}; // PWM pins for LEDs/

int joystickX = A1; // Joystick analog input for the x-direction.
int joystickY = A0;// Joystick analog input for the y-direction.
int joysticButtonkPin = 13; // Set pin for joystick button to 13.
int joystickButtonState = 0; // JoystickButtonState stores the digital state of the button (HIGH/LOW),

int mapX = 0; // Stores mapped x-value.
int mapY = 0; // Stores mapped y-value.

int selection = 0; // Stores game selection (1 or 2).

void setup()
{
  display.begin(); // Start the LCD display. Adafruit splashscreen will appear.
  for (int i = 0; i < 2; i++) // Loop through entire array (from 0 to 2)
  {
    pinMode(leds[i], OUTPUT); // Set pins to output mode
  }
  Serial.begin(9600); // Set the data rate to 9600 bits per second.

  display.clearDisplay(); // Clear LCD display.
  display.setContrast(30); // Set the LCD's contrast.
  display.setTextColor(BLACK, WHITE); // Set the LCD text to black and the background to "white".

  matrix.shutdown(0, false); // Turn off power saving, enables display.
  matrix.setIntensity(0, 8); // Set brightness to 50% (ranges from 0 - 15).
  matrix.clearDisplay(0);// Clears screen.

  pinMode(joystickX, INPUT); // Set joystick X pin to input.
  pinMode(joystickY, INPUT); // Set joystick X pin to input.
  pinMode(joysticButtonkPin, INPUT_PULLUP); // Set joystick button pin to INPUT_PULLUP (internal resistor).
  // Pressing the joystick gives a LOW signal.
}

void loop()
{
  matrix.clearDisplay(0);// Clear LED matrix.
  display.clearDisplay(); // Clear LCD display.

  joystickButtonState = digitalRead(joysticButtonkPin); // Read the state of the joystick button.
  mapX = map(analogRead(joystickX), 0, 1023, 512, -512); // Map the x-value from 0 - 1023 to -512 to 512.
  mapY = map(analogRead(joystickY), 0, 1023, 512, -512); // Map the y-value from 0 - 1023 to -512 to 512.

  // Menu screen if the user selects the first game (snake).
  if (mapY > 500 || selection == 0)
  {
    selection = 1;
    display.clearDisplay();
    display.print("");
    display.setTextSize(2); // Set font size to 2.
    display.println("GAMES"); // Print GAMES header to the LCD.
    display.setTextSize(1); // Set font size to 1.
    display.println(">  SNAKE"); // Print SNAKE option to the LCD.
    display.println("  BREAKOUT"); // Print BREAKOUT option to the LCD.
    display.display(); // Update the LCD display.
  }
  // Menu screen if the user selects the second game (Atari Breakout).
  else if (mapY < -500)
  {
    selection = 2;
    display.clearDisplay();
    display.print("");
    display.setTextSize(2); // Set font size to 2.
    display.println("GAMES"); // Print GAMES header to the LCD.
    display.setTextSize(1); // Set font size to 1.
    display.println("  SNAKE"); // Print SNAKE option to the LCD.
    display.println(">  BREAKOUT"); // Print BREAKOUT option to the LCD.
    display.display(); // Update the LCD display.
  }

  if (joystickButtonState == LOW) // Run the following if the joystick is pressed.
  {
    switch (selection)
    {
      case 1:
        Serial.println("Snake selected.");
        snakeGame(); // Call the snake game function if the selection is 1.
        break;
      case 2:
        Serial.println("Breakout selected.");
        breakoutGame();  // Call the (atari) breakout game function if the selection is 2.
        break;
    }
    selection = 0; // Reset selection value after the game is finished.
    Serial.println("Returning to main menu...");
  }
}

void snakeGame()
{
  int snakeXVel = 0; // Snake's velocity in the x direction.
  int snakeYVel = 0; // Snake's velocity in the y direction.
  int snakeLength = 8; // Snake's maximum length.

  int snake[snakeLength][2]; // Array holding the x and y coordinates of the snake.
  int currentLength = 1; // Current length of the snake.

  int food[2] = {}; // Array holding the x and y coordinates of the food.
  int seed = 0; // Seed for random generation.
  bool eaten = false; // Stores whether the game has finished or not.

  bool gameOver = false; // Stores whether the game has finished or not.

  for (int i = 0; i < snakeLength; i++)
  {
    snake[i][0] = 7;
    snake[i][1] = 4;
  }

  do
  {
    randomSeed(seed); // Use a seed to generate random numbers.
    // This ensures the location of the food is random everytime.

    // Display the current score.
    display.clearDisplay();
    display.setTextSize(1);
    display.println("SNAKE");
    display.setTextSize(2);
    display.print("Score:");
    display.println(currentLength);
    display.display();

    joystickButtonState = digitalRead(joysticButtonkPin); // Read the state of the joystick button.
    mapX = map(analogRead(joystickX), 0, 1023, 512, -512); // Map the x-value from 0 - 1023 to -512 to 512.
    mapY = map(analogRead(joystickY), 0, 1023, 512, -512); // Map the y-value from 0 - 1023 to -512 to 512.

    // The following if statements ensure that the position only changes when the joystick pulled almost all the way.
    if (mapX > 500 &&  snakeXVel == 0 && !(mapY > 500 || mapY < -500))
    {
      snakeXVel = -1; // Move the snake to the right.
      snakeYVel = 0; // Stop y-movement.
    }
    else if (mapX < -500 && snakeXVel == 0 &&  !(mapY > 500 || mapY < -500))
    {
      snakeXVel = 1; // Move the snake to the left.
      snakeYVel = 0; // Stop y-movement.
    }
    if (mapY > 500 && snakeYVel == 0 &&  !(mapX > 500 || mapX < -500))
    {
      snakeYVel = -1; // Move the snake down.
      snakeXVel = 0; // Stop x-movement.
    }
    else if (mapY < -500 && snakeYVel == 0 &&  !(mapX > 500 || mapX < -500))
    {
      snakeYVel = 1; // Move the snake up.
      snakeXVel = 0; // Stop x-movement.
    }

    // Turn off the snake's LEDs so its position can be updated.
    for (int i = 0; i < snakeLength; i++)
    {
      matrix.setLed(0,  snake[i][0], snake[i][1], false);
    }

    // Move the snake's body forward one LED at a time.
    for (int i = snakeLength - 1; i > 0; i--)
    {
      snake[i][0] = snake[i - 1][0];
      snake[i][1] = snake[i - 1][1];
    }

    // Update the x-position of the snake's head. Must be from 0 - 7.
    snake[0][0] += snakeXVel;
    if (snake[0][0] < 0) snake[0][0] = 7;
    if (snake[0][0] > 7) snake[0][0] = 0;

    // Update the y-position of the snake's head. Must be from 0 - 7.
    snake[0][1] += snakeYVel;
    if (snake[0][1] < 0) snake[0][1] = 7;
    if (snake[0][1] > 7) snake[0][1] = 0;

    // Turn the snake's LEDs back on because its position is updated.
    for (int i = 0; i < currentLength; i++)
    {
      matrix.setLed(0,  snake[i][0], snake[i][1], true);
    }

    // Generate a new location of the food if the current one has already been eaten.
    if (eaten)
    {
      bool repeat;
      do
      {
        repeat = false;
        food[0] = (int)random(8); // Generate new x-position for food.
        food[1] = (int)random(8); // Generate new y-position for food.

        // Ensure the food does not generate on top of the snake body.
        for (int i = 0; i < snakeLength; i++)
        {
          if (snake[i][0] == food[0] && snake[i][1] == food[1])
          {
            repeat = true;
            break;
          }
        }
      } while (repeat);
      eaten = false;
    }

    matrix.setLed(0,  food[0], food[1], true); // Turn on food LED.

    if (snake[0][0] == food[0] && snake[0][1] == food[1]) // Check if snake head collides with food.
    {
      eaten = true;
      matrix.setLed(0,  food[0], food[1], false); // Turn off food LED.
      currentLength++;
      Serial.print("Score has increased. It is now "); // Display score to serial monitor.
      Serial.println(currentLength);
    }
    if (currentLength == snakeLength) // If the snake reaches its maximum length, the game is over and the user won.
    {
      gameOver  = true;
      winnerScreen(); // Display the winner screen and light up the red, yellow and blue LEDs.
    }

    for (int i = 1; i < currentLength; i++) // If the snake runs into itself, the game is over and the user lost.
    {
      if (snake[0][0] == snake[i][0] && snake[0][1] == snake[i][1]) // Check if the snake head collides with its body.
      {
        gameOver  = true;
        gameOverScreen(); // Display the game over screen.
      }
    }
    delay(300); // Pause the game for 300 milliseconds.
    seed++; // Update seed.
  } while (gameOver == false);
}

void breakoutGame()
{
  int ballX = 0; // Ball's x position.
  int ballY = 3; // Ball's y position.
  int prevBallX = 0; // Stores the ball's previous x position.
  int prevBallY = 0; // Stores the ball's previous y position.

  int ballVelX = 1; // Ball's velocity in the x direction.
  int ballVelY = 1; // Ball's velocity in the y direction.

  int bar[2] = {3, 4}; // Stores the x coordinates of the 2x1 paddle (bar).
  int barVelX = 1; // Bar's velocity in the x direction.

  // Stores the location of the bricks. (Brick = 1, Empty = 0).
  int bricks[3][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
  };

  int score = 0;// Store game score.

  bool gameOver = false; // Stores whether the game has finished or not.

  do
  {
    // Display current game score on the LCD.
    display.clearDisplay(); // Clear LCD display.
    display.setTextSize(1); // Set font size to 1.
    display.println("BREAKOUT"); // Display BREAKOUT header.
    display.setTextSize(2); // Set font size to 2.
    display.print("Score:"); // Display Score: header.
    display.println(score); // Display score.
    display.display(); // Update LCD display.

    joystickButtonState = digitalRead(joysticButtonkPin); // Read the state of the joystick button.
    mapX = map(analogRead(joystickX), 0, 1023, 512, -512); // Map the x-value from 0 - 1023 to -512 to 512.
    mapY = map(analogRead(joystickY), 0, 1023, 512, -512); // Map the y-value from 0 - 1023 to -512 to 512.

    // Loop through entire bricks 2D array.
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        if (bricks[i][j] == 1)
        {
          matrix.setLed(0, j, i, true); // Turn on LED on at (j, i).
        }
        else if (bricks[i][j] == 0)
        {
          matrix.setLed(0, j, i, false); // Turn on LED off at (j, i).
        }
      }
    }

    if (mapX > 500) // Only change bar position when the joystick is pushed all the way to the right (500 = threshold value).
    {
      matrix.setLed(0,   bar[0], 7, false); // Turn off bar LED so position can be updated.
      matrix.setLed(0,    bar[1], 7, false); // Turn off bar LED so position can be updated.
      bar[0] --; // Update bar's x-position. Bar moves to the right.
      bar[1] --; // Update bar's x-position. Bar moves to the right.
    }
    else if (mapX < - 500) // Only change bar position when the joystick is pushed all the way to the left.
    {
      matrix.setLed(0,   bar[0], 7, false);
      matrix.setLed(0,    bar[1], 7, false);
      bar[0] ++; // Update bar's x-position. Bar moves to the left.
      bar[1] ++; // Update bar's x-position. Bar moves to the left.
    }

    if (ballX == 0 ) // When ball hits the left border.
    {
      ballVelX = 1; // Set ball's x-velocity to 1 so the ball moves rightwards.
    }
    else if (ballX == 7 && ballY < 7 ) // When ball hits the right border.
    {
      ballVelX = -1; // Set ball's x-velocity to -1 so the ball moves leftwards.
    }

    if (ballY == 0 ) // When ball hits the top border.
    {
      ballVelY = 1; // Set ball's y-velocity to 1 so the ball moves downwards.
    }

    if (ballY == 6 && (ballX  == bar[0] || ballX == bar[1]) ) // When ball hits the top of the bar.
    {
      ballVelY = -1; // Set ball's y-velocity to -1 so the ball moves upwards.
    }
    else if (ballY == 6 && (ballX + ballVelX  == bar[0] ) ) // When ball hits the right corner of the bar.
    {
      // Move the ball right and up.
      ballVelY = -1;
      ballVelX = -1;
    }
    else if (ballY == 6 && (ballX + ballVelX == bar[1])  )  // When ball hits the left corner of the bar.
    {
      // Move the ball left and up.
      ballVelY = -1;
      ballVelX = 1;
    }

    if (bar[1] > 7) // Stop the bar from leaving through the left side of the matrix.
    {
      bar[1] = 7;
      bar[0] = 6;
    }
    else if (bar[0] < 0) // Stop the bar from leaving through the right side of the matrix.
    {
      bar[1] = 1;
      bar[0] = 0;
    }

    prevBallX = ballX; // Store ball's previous x position.
    prevBallY = ballY; // Store ball's previous y position.

    ballX += ballVelX; // Update ball's x position.
    ballY += ballVelY; // Update ball's y position.

    if (prevBallX != ballX && prevBallY != ballVelY ) // Only update when the ball's position has changed.
    {
      matrix.setLed(0,  prevBallX, prevBallY, false);
    }

    if (ballX > 7) { // Cap the ball's x position to 7.
      ballX = 7;
    }
    else if (ballX < 0) { // Cap the ball's x position to 0.
      ballX = 0;
    }

    if (ballY > 7) { // If the ball hits the bottom border, announce that the game is over.
      gameOver = true;
      gameOverScreen(); // Display gameover screen on the LCD display.
    }

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        if (ballX == j && ballY == i && bricks[i][j] == 1) // Check if the ball has hit the brick at (j, i).
        {
          bricks[i][j] = 0;
          score++;
          Serial.print("Score has increased. It is now "); // Display score to serial monitor.
          Serial.println(score);
        }
      }
    }

    matrix.setLed(0, ballX, ballY, true); // Turn on ball LED.
    matrix.setLed(0, bar[0], 7, true);  // Turn on bar LED.
    matrix.setLed(0,  bar[1], 7, true);  // Turn on bar LED.

    // Check if all bricks have been destroyed. If so the game is over.
    if (gameOver == false)
    {
      gameOver = true;
      // Loop through entire brick 2D array.
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 8; j++)
        {
          if (bricks[i][j] == 1) // If there is a brick that is not destoryed, then the game is not over.
          {
            gameOver = false;
          }
        }
      }

      if (gameOver)  {
        matrix.clearDisplay(0); // Clear LED matrix.
        winnerScreen(); // Display the winnerScreen and light up the red, yellow and blue LEDs.
      }
    }

    delay(200);
  } while (gameOver == false);
}

void winnerScreen()
{
  // Tell the user they have won.
  display.clearDisplay();
  display.setTextSize(2);
  display.println("WINNER!");
  display.setTextSize(1);
  display.println("Amazing job!");
  display.display();

  Serial.println("Fading LEDs...");

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 256; j++) // Fade side LEDs on.
    {
      analogWrite(leds[0], j);
      delay(3);
    }
    for (int j = 0; j < 256; j++) // Fade middle LED on.
    {
      analogWrite(leds[1], j);
      delay(3);

    }

    for (int j = 256; j > 0; j -= 2)
    {
      analogWrite(leds[0], j); // Fade side LEDs off.
      delay(3);

    }
    for (int j = 256; j > 0; j -= 2)
    {
      analogWrite(leds[1], j); // Fade middle LED off.
      delay(3);
    }

    for (int j = 0; j < 10; j++) // Repeat the following 10 times.
    {
      analogWrite(leds[0], 255); // Turn the side LEDs on.
      delay(100);
      analogWrite(leds[1], 255); // Turn the middle LED on.
      delay(100);
      analogWrite(leds[0], 0); // Turn the side LEDs off.
      delay(100);
      analogWrite(leds[1], 0); // Turn the middle LED off.
      delay(100);
    }

    for (int k = 0; k < 3; k++) // Repeat the following 3 times.
    {
      for (int j = 0; j < 256; j++ ) // Fade all LEDs on.
      {
        analogWrite(leds[0], j);
        analogWrite(leds[1], j);

        delay(3);
      }
      for (int j = 256; j > 0; j -= 2) // Fade all LEDs off.
      {
        analogWrite(leds[0], j);
        analogWrite(leds[1], j);
        delay(3);
      }
    }
  }

  for (int j = 256; j >= 0; j--) // Fade all LEDs off slowly.
  {
    analogWrite(leds[0], j);
    analogWrite(leds[1], j);
    delay(3);
  }

  Serial.println("User has won the game.");

  delay(1000); // Wait 1000 milliseconds.
}

void gameOverScreen()
{
  // Tell the user they have lost.
  display.clearDisplay();
  display.setTextSize(2);
  display.println("GAME");
  display.println("OVER :(");
  display.display();

  Serial.println("User has lost the game.");

  delay(5000); // Wait 5000 milliseconds.
}