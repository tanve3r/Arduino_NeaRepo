//************************************************************
// this is a Assignment code for sensor board 1
// Here we send the sensor board data to main board
//
//************************************************************
#include "namedMesh.h"
#include "ArduinoJson.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"


#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

const int relay1pin = 33;  // the number of the LED pin
const int relay2pin = 32;  // the number of the LED pin

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;
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
static int transInt = 500;
static int lastdet = 0;


String nodeName = "MainBoard"; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, []() {
    String msg = String("This is a message from: ") + nodeName + String(" for logNode");
    String to = "S3board";
    mesh.sendSingle(to, msg); 
}); // start with a one second interval

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

  String Lux3 = doc["L3"];
  String PIR3 = doc["M3"];

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
  
  if( (PIR3 != "null") || (Lux3 != "null") )
  {
    sprintf(buffer, "PIR3:%03d", PIR3.toInt());
    display.printFixed(65,  32, buffer, STYLE_NORMAL);

    sprintf(buffer, "Lux3 :%03d", Lux3.toInt());
    display.printFixed(65,  40, buffer, STYLE_NORMAL);
    // Serial.printf("Lux3: ");
    // Serial.println(Lux3);

    if((PIR3.toInt() == 1) && (Lux3.toInt() < 100))
    {
      digitalWrite(relay1pin, LOW);
    }
    else
    {
      // switch on buzzer ??
    }
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
    if(millis() - lastdet > transInt)
    {
        if(motion1 > 10 && motion2 > 10)
        {
          detetion1_set = 0;
          detection2_set = 0;
          incrementlogic = 0;
          decrementlogic = 0;
        }
    }
    lastdet = millis();


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

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 
  mesh.onReceive(&receivedCallback);

  // mesh.onReceive([](uint32_t from, String &msg) {
  //   Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  // });

  // mesh.onReceive([](String &from, String &msg) {
  //   Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  // });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
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
  display.printFixed(65,  32, "PIR3:0", STYLE_NORMAL);
  display.printFixed(65,  40, "Lux3 :0", STYLE_NORMAL);
  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
