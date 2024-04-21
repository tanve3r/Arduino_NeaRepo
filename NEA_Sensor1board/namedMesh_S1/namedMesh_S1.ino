//************************************************************
// this is a Assignment code for sensor board 1
// Here we send the sensor board data to main board
//
//************************************************************
#include "namedMesh.h"
#include "ArduinoJson.h"
#include <BH1750.h>
#include <Wire.h>
#include <Arduino.h>
#include "DHT_Async.h"
#include <HCSR04.h>


#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   DHT_SENSOR_TYPE DHT_TYPE_11

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;
BH1750 lightMeter;
UltraSonicDistanceSensor distanceSensor(5, 18);  // Initialize sensor that uses digital pins 13 and 12.

static const int DHT_SENSOR_PIN = 32;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

static int lux = 0;
static int distance = 0;
static float temperature = 0;
static float humidity = 0;

String nodeName = "S1board"; // Name needs to be unique

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 4000ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}

Task taskSendMessage( TASK_MILLISECOND*200, TASK_FOREVER, []() {
    // String msg = String("This is a message from: ") + nodeName + String(" for logNode");
    String to = "MainBoard";

    lux = (int)lightMeter.readLightLevel();
    distance = distanceSensor.measureDistanceCm();

    if (measure_environment(&temperature, &humidity)) {
      Serial.print("T = ");
      Serial.print(temperature, 1);
      Serial.print(" deg. C, H = ");
      Serial.print(humidity, 1);
      Serial.println("%");
      }

    DynamicJsonDocument doc(1024);
    doc["L1"] = lux;
    doc["T1"] = (int)temperature;
    doc["H1"] = (int)humidity;
    doc["M1"] = distance;

    String msg;
    serializeJson(doc,msg);

    mesh.sendSingle(to, msg); 
    taskSendMessage.setInterval( TASK_MILLISECOND * 200);
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
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
