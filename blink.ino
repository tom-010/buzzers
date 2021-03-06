#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Web server 
const char* ssid = "buzzers";
const char* password = "buzzerspw";
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";




// https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
// https://randomnerdtutorials.com/esp32-access-point-ap-web-server/#:~:text=ESP32%20IP%20Address,ESP32%20point%20will%20be%20printed.


// IO
const int  buttonPin = 4;    // the pin that the pushbutton is attached to
const int ledPin = 5;       // the pin that the LED is attached to
const int BUTTON_PRESSED = HIGH;

// State
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


String pressToCsvLine(Press press) {
  return String(press.deviceId) + "," + String(press.millisecondsSinceBoot);
}

String presssesToCsv(int maxlines) {
  String res = "";

  for(int i=nextPressesIndex-1; i>=0 && maxlines>0; i--, maxlines--) {
    res += pressToCsvLine(presses[i]) + "\n";
  }

  if(full) { // full
     for(int i=capacity-1; i>=nextPressesIndex && maxlines>0; i--, maxlines--) {
        res += pressToCsvLine(presses[i]) + "\n";
     }
  }
  return res;
}


String processor(const String& var) {
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\"><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String getParam(AsyncWebServerRequest *request, String name, String orElse) {
  for(int i=0;i<request->params();i++){
     AsyncWebParameter* p = request->getParam(i);
     if(p->name() == name) {
      return p->value();
     }
  }
  return orElse;
}

int strToMaxlength(String encoded) {
  int res = encoded.toInt();
  if(res <= 0) {
    res = capacity;
  }
  return res;
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send(200, "text/csv", presssesToCsv(strToMaxlength(getParam(request, "n", "not found"))));
  });

  server.begin();

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
      Serial.println(presssesToCsv(3));
      
      // Serial.print("number of button pushes: ");
      // Serial.println(buttonPushCounter);
    
    } else {
      // if the current state is LOW then the button went from on to off:
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
