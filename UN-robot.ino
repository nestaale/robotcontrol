#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid     = "UN-Robot";
const char* password = "123456789";

// Set web server port number to 80
WebServer server(80);

// Motor pins
const int motor1Pin1 = 27; 
const int motor1Pin2 = 26; 
const int enable1Pin = 14;

const int motor2Pin1 = 33; 
const int motor2Pin2 = 25; 
const int enable2Pin = 32;

// PWM properties
const int freq = 30000;
const int resolution = 8;
int dutyCycle = 0;

String valueString = String(0);

// HTML content
const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }
    .button { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50; border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer; }
    .button2 {background-color: #555555;}
  </style>
  <script>
    function moveForward() { fetch('/forward'); }
    function moveLeft() { fetch('/left'); }
    function stopRobot() { fetch('/stop'); }
    function moveRight() { fetch('/right'); }
    function moveReverse() { fetch('/reverse'); }

    function updateMotorSpeed(pos) {
      document.getElementById('motorSpeed').innerHTML = pos;
      fetch(`/speed?value=${pos}`);
    }
  </script>
</head>
<body>
  <h1>UN Robot</h1>
  <p><button class="button" onclick="moveForward()">Fwd</button></p>
  <div style="clear: both;">
    <p>
      <button class="button" onclick="moveLeft()">L</button>
      <button class="button button2" onclick="stopRobot()">Stop</button>
      <button class="button" onclick="moveRight()">R</button>
    </p>
  </div>
  <p><button class="button" onclick="moveReverse()">Back</button></p>
  <p>Motor Speed: <span id="motorSpeed">0</span></p>
  <input type="range" min="0" max="100" step="25" id="motorSlider" oninput="updateMotorSpeed(this.value)" value="0"/>
</body>
</html>)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void setMotorPins(int m1p1, int m1p2, int m2p1, int m2p2) {
  digitalWrite(motor1Pin1, m1p1);
  digitalWrite(motor1Pin2, m1p2);
  digitalWrite(motor2Pin1, m2p1);
  digitalWrite(motor2Pin2, m2p2);
}

void handleForward() {
  Serial.println("Forward");
  setMotorPins(LOW, HIGH, LOW, HIGH);
  server.send(200);
}

void handleLeft() {
  Serial.println("Left");
  setMotorPins(LOW, LOW, LOW, HIGH);
  server.send(200);
}

void handleStop() {
  Serial.println("Stop");
  setMotorPins(LOW, LOW, LOW, LOW);
  server.send(200);
}

void handleRight() {
  Serial.println("Right");
  setMotorPins(LOW, HIGH, LOW, LOW);
  server.send(200);
}

void handleReverse() {
  Serial.println("Reverse");
  setMotorPins(HIGH, LOW, HIGH, LOW);
  server.send(200);
}

void handleSpeed() {
  if (server.hasArg("value")) {
    valueString = server.arg("value");
    int value = valueString.toInt();
    dutyCycle = map(value, 25, 100, 200, 255);
    ledcWrite(enable1Pin, dutyCycle);
    ledcWrite(enable2Pin, dutyCycle);
    Serial.printf("Motor speed set to %d\n", value);
  }
  server.send(200);
}

void setup() {
  Serial.begin(115200);

  // Set the Motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Configure PWM Pins
  ledcAttach(enable1Pin, freq, resolution);
  ledcAttach(enable2Pin, freq, resolution);
    
  // Initialize PWM with 0 duty cycle
  ledcWrite(enable1Pin, 0);
  ledcWrite(enable2Pin, 0);

  // Setup Access Point
  Serial.println("Setting AP (Access Point)...");
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Failed to setup AP");
    while (1);
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.printf("AP IP address: %s\n", IP.toString().c_str());

  // Define routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.on("/right", handleRight);
  server.on("/reverse", handleReverse);
  server.on("/speed", handleSpeed);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
