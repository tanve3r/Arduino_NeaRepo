//************************************************************
// this is a Assignment code for sensor board 3 
// Here we send the sensor board data to main board
//
//************************************************************
#include "namedMesh.h"
#include "ArduinoJson.h"
#include <BH1750.h>
#include <Wire.h>
#include <Arduino.h>
// #include "DHT_Async.h"

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// #define   DHT_SENSOR_TYPE DHT_TYPE_11

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;
BH1750 lightMeter;

#define timeSeconds 10

// Set GPIOs for LED and PIR Motion Sensor
const int led = 26;
const int motionSensor = 32;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;


// static const int DHT_SENSOR_PIN = 32;
// DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

static int lux = 0;
// static float temperature = 0;
// static float humidity = 0;

String nodeName = "S3board"; // Name needs to be unique

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
  motion = true;
  Serial.println("MOTION DETECTED!!!");
}

// /*
//  * Poll for a measurement, keeping the state machine alive.  Returns
//  * true if a measurement is available.
//  */
// static bool measure_environment(float *temperature, float *humidity) {
//     static unsigned long measurement_timestamp = millis();

//     /* Measure once every four seconds. */
//     if (millis() - measurement_timestamp > 4000ul) {
//         if (dht_sensor.measure(temperature, humidity)) {
//             measurement_timestamp = millis();
//             return (true);
//         }
//     }

//     return (false);
// }

Task taskSendMessage( TASK_MILLISECOND*100, TASK_FOREVER, []() {
    // String msg = String("This is a message from: ") + nodeName + String(" for logNode");
    String to = "MainBoard";

    lux = (int)lightMeter.readLightLevel();

    // if (measure_environment(&temperature, &humidity)) {
    //   Serial.print("T = ");
    //   Serial.print(temperature, 1);
    //   Serial.print(" deg. C, H = ");
    //   Serial.print(humidity, 1);
    //   Serial.println("%");
    //   }

    DynamicJsonDocument doc(1024);
    doc["L3"] = lux;
    doc["M3"] = (int)motion;

    String msg;
    serializeJson(doc,msg);

    mesh.sendSingle(to, msg); 
}); // start with a one second interval

void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  lightMeter.begin();

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

    // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();

  // Current time
  now = millis();
  // if((digitalRead(led) == HIGH) && (motion == false)) 
  // {
  //   Serial.println("MOTION DETECTED!!!");
  //   motion = true;
  // }
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds * 1000))) {
    Serial.println("Motion stopped...");
    // digitalWrite(led, LOW);
    startTimer = false;
    motion = false;
  }
}
