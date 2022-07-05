// this constant won't change:
const int  buttonPin = 4;    // the pin that the pushbutton is attached to
const int ledPin = 5;       // the pin that the LED is attached to

const int BUTTON_PRESSED = HIGH;

// Variables will change:
int buttonPushCounter = 0;
int buttonState = 0;     
int lastButtonState = 0;
int milliSecondsSinceBoot = 0;

const int capacity = 5;
const int maxIndex = capacity - 1;

struct Press {
   unsigned int deviceId;
   unsigned int millisecondsSinceBoot;
};

Press presses[capacity];
int nextPressesIndex = 0;
bool full = false;

void addPress(unsigned int deviceId) {
  int milliSecondsSinceBoot = esp_timer_get_time()/1000;
  Serial.println(milliSecondsSinceBoot);
  presses[nextPressesIndex] = (Press){deviceId, (unsigned int) milliSecondsSinceBoot};
  if(!full && nextPressesIndex == maxIndex) {
    full = true;
  }
  nextPressesIndex = (nextPressesIndex + 1) % capacity;
}


void printPress(Press press) {
  Serial.print(press.deviceId);
  int seconds = press.millisecondsSinceBoot / 1000; 
  int deci = (press.millisecondsSinceBoot % 1000) / 10;
  Serial.print(":");
  Serial.print(seconds);
  Serial.print(".");
  Serial.print(deci);
}

void printPresses() {
  Serial.print("[");

  

  for(int i=nextPressesIndex-1; i>=0; i--) {
    printPress(presses[i]);
    Serial.print(", ");
  }

  if(full) { // full
     for(int i=capacity-1; i>=nextPressesIndex; i--) {
        printPress(presses[i]);
      Serial.print(", ");
     }
  }

  Serial.print("]");
  Serial.println();
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == BUTTON_PRESSED) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      int milliSecondsSinceBoot = esp_timer_get_time()/1000;
      addPress(0);
      printPresses();
      
      // Serial.print("number of button pushes: ");
      // Serial.println(buttonPushCounter);
    
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
    }

    delay(50); // Delay a little bit to avoid bouncing
  }
  lastButtonState = buttonState;



  if (buttonPushCounter % 4 == 0) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

}
