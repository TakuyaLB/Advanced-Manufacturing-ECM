#include <AccelStepper.h>

// Define stepper motor connections
#define dirPin 2
#define stepPin 3

// Stepper (driver mode: STEP, DIR)
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

// ----- Motor setup -----
const float STEPS_PER_REV     = 200.0;  // 1.8° stepper
const int   MICROSTEP_SETTING = 1;      // 1,2,4,8,16,32 (match your driver jumpers)
const float RPM               = 100.0;   // <<< change this for speed

// end position and speed for drilling
int targetPos = -500;
int topSpeed = (STEPS_PER_REV * MICROSTEP_SETTING) * (RPM / 60.0);

// KY-040 Rotary Encoder connections
#define CLKpin 6 // Connected to CLK on KY-040
#define DTpin 7 // Connected to DT on KY-040

int encoderPosCount = 0; // Counter for encoder position
int CLKpinLast;            // Stores the previous state of CLK pin
int CLKval;                // Stores the current state of CLK pin
boolean bCW;             // Flag to indicate direction (true = Clockwise)

// Start button connection
#define buttonPin 4  // the number of the pushbutton pin
int buttonState = 0;  // variable for reading the pushbutton status

// Up (red) button connection
#define upButtonPin 12  // the number of the up pushbutton pin
int upButtonState = 0;  // variable for reading the up pushbutton status

// Down (black) button connection
#define downButtonPin 11  // the number of the downpushbutton pin
int downButtonState = 0;  // variable for reading the down pushbutton status

// make sure buttons can't be used when drilling has already started
boolean drilling = false;

void setup() {
  // Declare motor pins as output
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Ensure we never limit ourselves below our constant speed
  stepper.setMaxSpeed(fabs(topSpeed));   // not used by runSpeed, but safe to set 

  // Declare encoder pins as input
  pinMode (CLKpin, INPUT);
  pinMode (DTpin, INPUT);

  // Read the initial state of CLK
  CLKpinLast = digitalRead(CLKpin); 

  // Declare pushbutton pin as input
  pinMode(buttonPin, INPUT);
  
  // Begin serial connection
  Serial.begin(9600);
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // read the state of the up pushbutton value:
  upButtonState = digitalRead(upButtonPin);
  // read the state of the down pushbutton value:
  downButtonState = digitalRead(downButtonPin);

  if ((buttonState == LOW) && (drilling == false)) {
    // Treat current location of stepper motor as the CENTER (logical zero)
    stepper.setCurrentPosition(0);
    drilling = true;
    drillingRoutine();
    homingRoutine();
    drilling = false;
  }

  if ((upButtonState == LOW) && (drilling == false)) {
    drilling = true;
    stepper.setSpeed(topSpeed);
    stepper.runSpeed();
    drilling = false;
    checkEncoder();
  }

  if ((downButtonState == LOW) && (drilling == false)) {
    drilling = true;
    stepper.setSpeed(-topSpeed);
    stepper.runSpeed();
    drilling = false;
    checkEncoder();
  }
}

void drillingRoutine() {
  Serial.println("drilling");
  // speed of stepper motor and max speed for safety
  stepper.setSpeed(-topSpeed);
  while (stepper.currentPosition() != targetPos) {
    stepper.runSpeed();
    checkEncoder();
  }
}

void homingRoutine() {
  Serial.println("homing");
  stepper.setSpeed(topSpeed);
  while (stepper.currentPosition() != 0) {
    stepper.runSpeed();
    checkEncoder();
  }
}

void checkEncoder() {
  CLKval = digitalRead(CLKpin); // Read the current state of CLK pin

  if (CLKval != CLKpinLast) { // If the state has changed, the knob was rotated
    // Check the state of the DT pin to determine direction
    if (digitalRead(DTpin) != CLKval) { 
      // DT state is different from CLK state: Clockwise rotation
      encoderPosCount++;
      bCW = true;
    } else {
      // DT state is the same as CLK state: Counter-Clockwise rotation
      encoderPosCount--;
      bCW = false;
    }

    // Print the direction and current position count
    Serial.print("Rotated: ");
    if (bCW) {
      Serial.println("Clockwise");
    } else {
      Serial.println("Counterclockwise");
    }
    Serial.print("Encoder Position: ");
    Serial.println(encoderPosCount);
  }

  CLKpinLast = CLKval; // Save the current state as the last state for the next loop iteration
}
