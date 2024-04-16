#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
DisplaySH1107_128x64_I2C display(-1);

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hello from node 3";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

void receivedCallback(uint32_t from, String &msg)
{
  char buffer[50];

  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

  String json;
  DynamicJsonDocument doc(3072);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);

  if(error)
  {
    Serial.printf("deserializeJson failed: ");
    Serial.println(error.c_str());
  }

  String Temp1 = doc["T1"];
  String Hum1 = doc["H1"];
  String Lux1 = doc["L1"];
  String Motion1 = doc["M1"];

  String Temp2 = doc["T2"];
  String Hum2 = doc["H2"];
  String Lux2 = doc["L2"];
  String Motion2 = doc["M2"];

  String Temp3 = doc["T3"];
  String Hum3 = doc["H3"];
  String Lux3 = doc["L3"];

  if( (Temp1 != "null") || (Hum1 != "null") || (Lux1 != "null") || (Motion1 != "null") )
  {
    sprintf(buffer, " Temp1:%s", Temp1);
    display.printFixed(0,  0, buffer, STYLE_NORMAL);

    sprintf(buffer, " Humi1:%s", Hum1);
    display.printFixed(0,  8, buffer, STYLE_NORMAL);

    sprintf(buffer, " Lux1 :%s", Lux1);
    display.printFixed(0,  16, buffer, STYLE_NORMAL);

    sprintf(buffer, " Mot1 :%s", Motion1);
    display.printFixed(0,  24, buffer, STYLE_NORMAL);
  }

  if( (Temp2 != "null") || (Hum2 != "null") || (Lux2 != "null") || (Motion2 != "null") )
  {
    sprintf(buffer, " Temp2:%s", Temp2);
    display.printFixed(0,  32, buffer, STYLE_NORMAL);

    sprintf(buffer, " Humi2:%s", Hum2);
    display.printFixed(0,  40, buffer, STYLE_NORMAL);

    sprintf(buffer, "Lux2 :%s", Lux2);
    display.printFixed(65,  0, buffer, STYLE_NORMAL);

    sprintf(buffer, "Mot2 :%s", Motion2);
    display.printFixed(65,  8, buffer, STYLE_NORMAL);
  }
  
  if( (Temp3 != "null") || (Hum3 != "null") || (Lux3 != "null") )
  {
    sprintf(buffer, "Temp3:%s", Temp3);
    display.printFixed(65,  16, buffer, STYLE_NORMAL);

    sprintf(buffer, "Humi3:%s", Hum3);
    display.printFixed(65,  24, buffer, STYLE_NORMAL);

    sprintf(buffer, "Lux3 :%s", Lux3);
    display.printFixed(65,  32, buffer, STYLE_NORMAL);
  }
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

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.onReceive(&receivedCallback);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  /* Select the font to use with menu and all font functions */
  display.begin();

  display.setFixedFont(ssd1306xled_font6x8);
  display.clear();
  display.printFixed(0,  0, " Temp1=0", STYLE_NORMAL);
  display.printFixed(0,  8, " Humi1=0", STYLE_NORMAL);
  display.printFixed(0,  16, " Lux1 =0", STYLE_NORMAL);
  display.printFixed(0,  24, " Mot1 =0", STYLE_NORMAL);
  display.printFixed(0,  32, " Temp2=0", STYLE_NORMAL);
  display.printFixed(0,  40, " Humi2=0", STYLE_NORMAL);
  display.printFixed(65,  0, "Lux2 =0", STYLE_NORMAL);
  display.printFixed(65,  8, "Mot2 =0", STYLE_NORMAL);
  display.printFixed(65,  16, "Temp3=0", STYLE_NORMAL);
  display.printFixed(65,  24, "Humi3=0", STYLE_NORMAL);
  display.printFixed(65,  32, "Lux3 =0", STYLE_NORMAL);
}

void loop() {
  mesh.update();
}

