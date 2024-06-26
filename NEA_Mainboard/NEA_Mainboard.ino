#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
const int relay1pin = 33;  // the number of the LED pin
const int relay2pin = 32;  // the number of the LED pin

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
DisplaySH1107_128x64_I2C display(-1);

static int PersonCount = 0;
static int detetion1_set = 0;
static int detection2_set = 0;
static int decrementlogic = 0;
static int incrementlogic = 0;
static int detection1_pervious = 0;
static int detection2_pervious = 0;
static int motion1 =0;
static int motion2 = 0;

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

  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

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
    sprintf(buffer, " Temp1:%03d", Temp1.toInt());
    display.printFixed(0,  0, buffer, STYLE_NORMAL);

    sprintf(buffer, " Humi1:%03d", Hum1.toInt());
    display.printFixed(0,  8, buffer, STYLE_NORMAL);

    sprintf(buffer, " Lux1 :%03d", Lux1.toInt());
    display.printFixed(0,  16, buffer, STYLE_NORMAL);
    
    motion1 = Motion1.toInt();
    if(Motion1.toInt() < 10)
    {
      display.printFixed(0,  24, " Mot1 : 1", STYLE_NORMAL);
      detetion1_set = 1;
    }
    else
    {
      display.printFixed(0,  24, " Mot1 : 0", STYLE_NORMAL);
    }
    sprintf(buffer, " USD1 :%03d", Motion1.toInt());
    display.printFixed(0,  32, buffer, STYLE_NORMAL);
    // Serial.printf("Distance1: %s",Motion1);
    // Serial.println(Motion1);
  }

  if( (Temp2 != "null") || (Hum2 != "null") || (Lux2 != "null") || (Motion2 != "null") )
  {
    sprintf(buffer, " Temp2:%03d", Temp2.toInt());
    display.printFixed(0,  40, buffer, STYLE_NORMAL);

    sprintf(buffer, "Humi2:%03d", Hum2.toInt());
    display.printFixed(65,  0, buffer, STYLE_NORMAL);

    sprintf(buffer, "Lux2 :%03d", Lux2.toInt());
    display.printFixed(65,  8, buffer, STYLE_NORMAL);
    
    motion2 = Motion2.toInt();
    if(Motion2.toInt() < 10)
    {
      display.printFixed(65,  16, "Mot2 : 1", STYLE_NORMAL);
      detection2_set = 1;
    }
    else
    {
      display.printFixed(65,  16, "Mot2 : 0", STYLE_NORMAL);
    }

    sprintf(buffer, "USD2 :%03d", Motion2.toInt());
    display.printFixed(65,  24, buffer, STYLE_NORMAL);
    // Serial.printf("  Distance2: ");
    // Serial.println(Motion2);
  }
  
  if( (Temp3 != "null") || (Hum3 != "null") || (Lux3 != "null") )
  {
    sprintf(buffer, "Temp3:%03d", Temp3.toInt());
    display.printFixed(65,  32, buffer, STYLE_NORMAL);

    sprintf(buffer, "Humi3:%03d", Hum3.toInt());
    display.printFixed(65,  40, buffer, STYLE_NORMAL);

    sprintf(buffer, "Lux3 :%03d", Lux3.toInt());
    display.printFixed(65,  48, buffer, STYLE_NORMAL);
    // Serial.printf("Lux3: ");
    // Serial.println(Lux3);
  }

  // Serial.printf("d1:%d d2: %d \n",detetion1_set,detection2_set);

  if((detetion1_set == 1) && (detection2_set == 0))
  {
    incrementlogic = 1;
  }


  if(incrementlogic == 1)
  {
    if((detection2_set == 1)&&(detection2_pervious == 0))
    {
      PersonCount ++;
    }
  }

  if((detection2_set == 1) && (detetion1_set ==0))
  {
    decrementlogic = 1;
  }

  if(decrementlogic == 1)
  {
    if((detetion1_set == 1)&&(detection1_pervious == 0))
    {
      if(PersonCount > 0)
      PersonCount --;
    }
  }

  if(detetion1_set==1 && detection2_set==1)
  {
    if(motion1 > 10 && motion2 > 10)
    {
      detetion1_set = 0;
      detection2_set = 0;
      incrementlogic = 0;
      decrementlogic = 0;
    }
  }

  sprintf(buffer, " PerCo:%03d", PersonCount);
  display.printFixed(0,  48, buffer, STYLE_NORMAL);
  // Serial.printf("pc:%d dec:%d inc:%d\n", PersonCount,decrementlogic,incrementlogic);
  detection1_pervious = detetion1_set;
  detection2_pervious = detection2_set;
  if(PersonCount > 0)
  {
    digitalWrite(relay1pin, LOW);
  }
  else
  {
    digitalWrite(relay1pin, HIGH);
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
  display.printFixed(0,  0, " Temp1:0", STYLE_NORMAL);
  display.printFixed(0,  8, " Humi1:0", STYLE_NORMAL);
  display.printFixed(0,  16, " Lux1 :0", STYLE_NORMAL);
  display.printFixed(0,  24, " Mot1 :0", STYLE_NORMAL);
  display.printFixed(0,  32, " USD1 :0", STYLE_NORMAL);
  display.printFixed(0,  40, " Temp2:0", STYLE_NORMAL);
  display.printFixed(0,  48, " PerCo:0", STYLE_NORMAL);
  display.printFixed(65,  0, "Humi2:0", STYLE_NORMAL);
  display.printFixed(65,  8, "Lux2 :0", STYLE_NORMAL);
  display.printFixed(65,  16, "Mot2 :0", STYLE_NORMAL);
  display.printFixed(65,  24, "USD2 :0", STYLE_NORMAL);
  display.printFixed(65,  32, "Temp3:0", STYLE_NORMAL);
  display.printFixed(65,  40, "Humi3:0", STYLE_NORMAL);
  display.printFixed(65,  48, "Lux3 :0", STYLE_NORMAL);
  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);
  digitalWrite(relay1pin, HIGH);
}

void loop() {
  mesh.update();
}

