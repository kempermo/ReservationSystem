#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "HTTPSRedirect.h"

const char ssid[] = "PHOENIX_Guest"; // put your wifi ssid here
const char pass[] = "phoenix123"; // put your wifi password here

Servo myservo;  // create servo object to control a servo
#define SERVO_PIN 2 // pin where the servo is attached
int servoPosition = 0;    // variable to store the servo position

const char* host = "script.google.com"; // link to the google script
const int httpsPort = 443; // port for google
const char *GScriptId = "AKfycbxYfv03sykkpL6LAX6PurHAejRYcd0uK9o3ZQX4HY3yzkPm-Z0"; // id of the google script
String url = String("/macros/s/") + GScriptId + "/exec"; // final url for google script

HTTPSRedirect* client = nullptr; // the object that will communicate with google

//String title;
bool roomAvailable = false; // holding the information if the room is stil free
long durationTime = 0; // holding the information how long the room is stil free or how long it will be occupied (in seconds) 

unsigned long lastHttpsTime = 0; // how often the data will be polled (in seconds)
int pollingInterval = 20000;

void setup()
{
  // start the serial connection
  Serial.begin(9600);

  // start the wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); 

  // wait for the wifi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // print some debug information about the wifi connection
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // start the servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
  myservo.write(servoPosition); //
}

void loop()
{
  // wait for a predefined period
  if (millis() - lastHttpsTime > pollingInterval)
  {
    String response = httpsGet(); // actually get the data

    // parse the data from the json to make use of it
    if(response.length() > 0)
    {
      StaticJsonBuffer<400> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(response);

      JsonObject& data = root["data"];

      String title = data["title"];
      roomAvailable = data["roomAvailable"];
      durationTime = int(data["durationTime"])/60;

      Serial.print("Title: ");
      Serial.println(title);
      Serial.print("roomAvailable: ");
      Serial.println(roomAvailable);
      Serial.print("durationTime: ");
      Serial.println(durationTime);

      myservo.write(map(durationTime,0,3000,0,180));
    }
  }
}

String httpsGet()
{
  static bool flag = false;
  String payload = "";

  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr) {
    if (!client->connected()) {
      client->connect(host, httpsPort);
    }
  }

  Serial.println("GET Data from calendar:");
  if (client->GET(url, host)) {
    payload = client->getResponseBody();
    Serial.println(payload);
  }

  return payload;
}
