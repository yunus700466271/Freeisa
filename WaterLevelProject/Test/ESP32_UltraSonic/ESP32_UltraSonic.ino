#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "MyESP32AP"; // change to your desired access point name
const char* password = "password"; // change to your desired access point password

const int trigPin = 22; // change to the actual pin number that the JSN-SR04T trigger pin is connected to
const int echoPin = 23; // change to the actual pin number that the JSN-SR04T echo pin is connected to

WebServer server(80);

void setup() {
  Serial.begin(115200);

  // initialize the WiFi access point
  WiFi.softAP(ssid, password);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // start the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  String html = "<html><head><style>body{font-size: 20px; margin: 50px; text-align: center;} h1{font-size: 36px;} p{font-size: 24px;}</style></head>";
  html += "<body><h1>Distance Measured: " + String(distance) + " cm</h1><p>Refresh the page to measure again.</p></body></html>";

  server.send(200, "text/html", html);
}
