#include "painlessMesh.h"
#include "ArduinoJson.h"
#include <BH1750.h>
#include <Wire.h>
#include <Arduino.h>
#include "DHT_Async.h"
#include <HCSR04.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   DHT_SENSOR_TYPE DHT_TYPE_11

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
BH1750 lightMeter;
UltraSonicDistanceSensor distanceSensor(5, 18);  // Initialize sensor that uses digital pins 13 and 12.

static const int DHT_SENSOR_PIN = 32;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

static int lux = 0;
static int distance = 0;
static float temperature = 0;
static float humidity = 0;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

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

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {

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

  // Serial.println(msg);
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {

  Serial.begin(115200);
  Wire.begin(21,22);
  lightMeter.begin();

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.onReceive(&receivedCallback);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  
}

void loop() {
  mesh.update();
}
