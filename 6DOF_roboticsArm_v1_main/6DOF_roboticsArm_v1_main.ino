#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


SoftwareSerial ssc32u(12, 13);  // Assuming RX(D7) and TX(D6) pins
#define ARM_PULSE_STEP 8
#define ARM_PULSE_MAX 2000
#define ARM_PULSE_MIN 1000

uint16_t pulse = 1500;
uint8_t direction = 1;
uint16_t count = 0;
unsigned long previousMillis = 0;
const long interval = 1;  // Interval between movements in milliseconds

#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Define Emergency STOP button pins
#define EMERGENCY_STOP_PIN 16  //  connected to D8

// Pin definitions for common cathode RGB LED
#define RED_PIN 0                                                       // Red LED connected to D3
#define GREEN_PIN 14                                                    // Green LED connected to D5
#define BLUE_PIN 2                                                      // Blue LED connected to D4
uint16_t currentPositions[6] = { 1500, 1500, 1500, 1500, 1500, 1500 };  // Default positions


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to display one or two lines of text on the OLED display
void displayInfo(const String &line1, const String &line2 = "") {
    display.clearDisplay();
    display.setCursor(0, 1);
    display.setTextSize(2); // You can adjust the text size as needed
    display.setTextColor(WHITE); // Set text color
    
    display.println(line1);
    if (line2.length() > 0) { // Check if the second line of text is provided
        display.println(line2);
    }
    display.display(); // Make sure to update the display to show the new text
    delay(1000); // Optional: Delay to keep the message on screen before any further update
}

void setup() {

  Serial.begin(9600);
  Serial.println("<==== System Initializing ====>");


  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);  // Setup the emergency stop button

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);


  Serial.println("\nSoftware serial interface :: ssc32u @9600...");
  ssc32u.begin(9600);
  Serial.println("Completed!");
  glowYellowLED();
}

void loop() {
  // Check the emergency stop button state

  /*  if (digitalRead(EMERGENCY_STOP_PIN) == LOW) { // Assuming the button is active LOW
    emergencyStop();
  }
*/


  DisplayINTRO();
  LoadingBar();
  glowBlueLED();  // Turn on the Blue LED as a (In Progress indicator)

  moveToHome();      // Move motors to centered position
  displayInfo("Picking   Testube:", ">> 1");
  pickUpTestTube1();
  glowGreenLED();   // Turn on the Blue LED as a (Completed indicator)
  delay(1000);

  glowBlueLED();  // Turn on the Blue LED as a (In Progress indicator)
  displayInfo("Shaking   Testube:", ">> 1");
  shakeTestTube();
  displayInfo("Placing   Testube:", ">> 1");
  placeTestTube();
  glowGreenLED();   // Turn on the Blue LED as a (Completed indicator)
  delay(1000);

  glowBlueLED();  // Turn on the Blue LED as a (In Progress indicator)
  displayInfo("Picking   Testube:", ">> 2");
  pickUpTestTube2();
  glowGreenLED();   // Turn on the Blue LED as a (Completed indicator)
  delay(1000);
  

  glowBlueLED();  // Turn on the Blue LED as a (In Progress indicator)
  displayInfo("Shaking   Testube:", ">> 2");
  //shakeTestTube();
  displayInfo("Placing   Testube:", ">> 2");
  placeTestTube();        
  glowGreenLED();   // Turn on the Blue LED as a (Completed indicator)
  delay(1000);
  
  

  glowGreenLED();   // Turn on the Blue LED as a (Completed indicator)
  displayInfo("Completing Job");
  moveToHome();      // Move motors to centered position

  

}
//////////////////////////////////////////-Robotic ARM-//////////////////////////////////////////////////////////

void moveMotors(uint16_t p0, uint16_t p1, uint16_t p2, uint16_t p3, uint16_t p4, uint16_t p5) {
  char output[90];
  sprintf(output, "#0P%u #1P%u #2P%u #3P%u #4P%u #5P%u\r", p0, p1, p2, p3, p4, p5);
  ssc32u.write(output);
}

void smoothlyMoveServos(const uint16_t targetPositions[6]) {
  const int steps = 150;  // Increase steps for smoother transitions
  float progress, easedProgress;

  // Calculate and apply easing for each step
  for (int step = 0; step <= steps; step++) {
    uint16_t intermediatePositions[6];
    progress = (float)step / (float)steps;

    // Apply easing - Sigmoid-like function for smooth start and end
    easedProgress = progress * progress * (3 - 2 * progress);

    for (int i = 0; i < 6; i++) {
      // Calculate intermediate positions with easing applied
      intermediatePositions[i] = currentPositions[i] + (uint16_t)((targetPositions[i] - currentPositions[i]) * easedProgress);

      // Optional: Add safety checks for servo limits
    }

    // Command servos to move to the calculated intermediate positions
    moveMotors(intermediatePositions[0], intermediatePositions[1], intermediatePositions[2],
               intermediatePositions[3], intermediatePositions[4], intermediatePositions[5]);

    // Small delay to allow movement to occur, adjust based on your servo speed and responsiveness
    delay(20);  // Shorter delay for smoother transitions
  }

  // Update current positions to target after movement completion
  for (int i = 0; i < 6; i++) {
    currentPositions[i] = targetPositions[i];
  }
}

//-----------------------------------------------------------------------------------------------------------///

void moveToHome() {
  // display Picking up TestTube1
  uint16_t newPositions[6] = { 1500, 1510, 1550, 600, 1500, 0 };  // Example new positions
  smoothlyMoveServos(newPositions);
}

void pickUpTestTube1() {
  uint16_t newPositions[6] = { 2500, 800, 700, 1650, 1500, 0 };  // Example new positions
  smoothlyMoveServos(newPositions);
  uint16_t newPositions2[6] = { 2500, 740, 700, 1650, 1500, 100 };  // gripper closing
  smoothlyMoveServos(newPositions2);
  uint16_t newPositions3[6] = { 2500, 740, 700, 1650, 1500, 2000 };  // gripper closing
  smoothlyMoveServos(newPositions3);
  uint16_t newPositions4[6] = { 2500, 920, 700, 1650, 1500, 2000 };  // lifting the testtube
  smoothlyMoveServos(newPositions4);
}

void pickUpTestTube2() {
  uint16_t newPositions[6] = { 2390, 800, 700, 1650, 1500, 0 };  // Example new positions
  smoothlyMoveServos(newPositions);
  uint16_t newPositions2[6] = { 2390, 740, 700, 1650, 1500, 100 };  // gripper open
  smoothlyMoveServos(newPositions2);
  uint16_t newPositions3[6] = { 2390, 740, 700, 1650, 1500, 2000 };  // gripper closing
  smoothlyMoveServos(newPositions3);
  uint16_t newPositions4[6] = { 2390, 920, 700, 1650, 1500, 2000 };  // lifting the testtube
  smoothlyMoveServos(newPositions4);
}

void placeTestTube() {
  uint16_t newPositions[6] = { 600, 1500, 1700, 1900, 1500, 2000 };  // Example new positions pos2= 1400 or 1550
  smoothlyMoveServos(newPositions);
  uint16_t newPositions2[6] = { 600, 1290, 1700, 2000, 1500, 2000 };  // Example new positions
  smoothlyMoveServos(newPositions2);
  uint16_t newPositions3[6] = { 600, 1290, 1700, 2000, 1500, 0 };  // Example new positions
  smoothlyMoveServos(newPositions3);
  uint16_t newPositions4[6] = { 600, 1500, 1700, 2000, 1500, 2000 };  // Example new positions
  smoothlyMoveServos(newPositions4);
}

void shakeTestTube() {
  uint16_t newPositions[6] = { 2500, 900, 700, 1650, 1500, 2000 };  // Example new positions
  smoothlyMoveServos(newPositions);
  moveMotors(2500, 900, 700, 1650, 2000, 2000);
  moveMotors(2500, 900, 700, 1650, 0, 2000);
  moveMotors(2500, 900, 700, 1650, 1500, 2000);
}



//-----------------------------------------------------------------------------------------------------------///


///////////////////////////////////////////-Emergency STOP-////////////////////////////////////////////////////////////

void emergencyStop() {
  // Halt all motor movements or other activities
  // For SSC-32U servo controller, sending a position command with a speed of 0 can halt the movement.
  ssc32u.write("#0P1500 #1P1500 #2P1500 #3P1500 #4P1500 #5P1500 T0\r");

  // Turn on the red LED to indicate emergency stop
  glowRedLED();

  // Optionally, add a loop here to prevent further code execution
  // This depends on how you wish to handle the system's state post-emergency stop.
  while (true) {
    delay(100);  // Keep the program here to stop further execution.
  }
}


///////////////////////////////////////////-RGB LED-////////////////////////////////////////////////////////////
void glowRedLED() {
  Serial.println("Red LED Constant Glow");
  digitalWrite(RED_PIN, HIGH);  // Turn on the Red LED (Common Cathode: HIGH = ON)
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}

void glowGreenLED() {
  Serial.println("Green LED Constant Glow");
  digitalWrite(GREEN_PIN, HIGH);  // Turn on the Green LED (Common Cathode: HIGH = ON)
  digitalWrite(RED_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}

void glowBlueLED() {
  Serial.println("Blue LED Constant Glow");
  digitalWrite(BLUE_PIN, HIGH);  // Turn on the Green LED (Common Cathode: HIGH = ON)
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
}

void glowYellowLED() {
  Serial.println("Yellow LED Glow");
  digitalWrite(RED_PIN, HIGH);    // Turn on the Red LED
  digitalWrite(GREEN_PIN, HIGH);  // Turn on the Green LED
  digitalWrite(BLUE_PIN, LOW);    // Ensure the Blue LED is off
  // This combination of Red and Green on will create Yellow
}
/////////////////////////////////////-0.96 OLED Screen-////////////////////////////////////////////////////////

// Function to update the loading bar
void updateLoadingBar(int progress) {
  const int barWidth = 100;                          // Width of the loading bar
  const int barHeight = 10;                          // Height of the loading bar
  const int x = (display.width() - barWidth) / 2;    // Center the bar horizontally
  const int y = (display.height() - barHeight) / 2;  // Center the bar vertically

  display.clearDisplay();
  display.drawRect(x, y, barWidth, barHeight, WHITE);  // Draw the bar outline

  // Calculate width of the filled portion
  int fillWidth = (progress * barWidth) / 100;
  display.fillRect(x, y, fillWidth, barHeight, WHITE);  // Draw the filled portion

  display.display();
}

//-------------------------------------------------------------------------------------------------------------/

void LoadingBar() {
  // Example loading animation
  for (int i = 0; i <= 100; i++) {
    updateLoadingBar(i);  // Update the bar with the current progress
    delay(50);            // Adjust delay for speed of loading bar
  }

  // After loading is complete, you can display other information
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Starting Arm");
  display.display();
}


//-------------------------------------------------------------------------------------------------------------/

void DisplayINTRO() {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("-Group 4H-");
  display.println("Testtube");
  display.println("Handling");
  display.display();
  delay(2000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
