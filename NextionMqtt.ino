#include <FS.h>   
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Nextion.h>
#include <SoftwareSerial.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>  

char mqtt_server[40];
char mqtt_port[6] = "1883";
bool InputSwitch1state=false;
bool InputSwitch2state=false;


const char* root_topicOut = "Nextion/Out";
const char* root_topicIn = "Nextion/In";

const int NextionRX = 13;
const int NextionTX = 15;
const int InputSwitch1 = 5;
const int InputSwitch2 = 4;
const int RelaySwitch1 = 14;
const int RelaySwitch2 = 12;
const int I2busSDA  = 16;
const int I2cbusCLA = 2;
const char* Switch1Nextion="LocalSw1";
const char* Switch2Nextion="LocalSw2";



WiFiClient espClient;
PubSubClient client(espClient);


SoftwareSerial nextion(NextionRX, NextionTX);
Nextion myNextion(nextion, 9600);

struct JsonPayload{
  String Topic;
  String Payload;
} ;

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}



void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial.println("Program Start");
setup_wifi();

pinMode(InputSwitch1,INPUT);
pinMode(InputSwitch2,INPUT);
pinMode(RelaySwitch1,OUTPUT);
pinMode(RelaySwitch2,OUTPUT);
  
myNextion.init();


  
}


void loop() {

if (!client.connected()) {
    reconnect();
  }
  client.loop();  

String message = myNextion.listen(); //check for message
  if(message != ""){ // if a message is received...
    String Component = GetComponent(message);
    SendMessage((char*) Component.c_str());
    
    Serial.println(Component); //...print it out
  }
  else 
  {
    // Serial.println("No message"); //...print it out
    }

}


void mountfs()
{
   if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}






void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String Payload;
  for (int i = 0; i < length; i++) {
    Payload += (char)payload[i];
    
  }

  Serial.println(Payload);
  JsonPayload data = Decodejson((char*)Payload.c_str());
    Serial.println("JSON Returned!");
    RunCommandonNextion(data);
    
  
}


bool RunCommandonNextion(struct JsonPayload data){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data.Payload);

if (!root.success()) {
         Serial.println("RunCommandonNextion JSON parsing failed!");
    return false;
}
  
  if (data.Topic == "setComponentText")
    {
      Serial.println("setComponentText init");
       Serial.println((const char *)root["component"]);
        Serial.println((const char *)root["text"]);
      myNextion.setComponentText(root["component"],root["text"]);
      return true;
    }

    if (data.Topic == "sendCommand")
    {
      Serial.println("sendCommand init");
      Serial.print("With Command =");
      Serial.println((const char *) root["command"] );
        myNextion.sendCommand( root["command"] );
          return true;
    }
    
}


struct JsonPayload Decodejson(char* Payload) 
{
 JsonPayload data;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(Payload);
  if (!root.success()) {
     data = {"",""};
    Serial.println("JSON parsing failed!");
    return data;
  } else
 {
   String topic = root["topic"];
   Serial.println(topic);
   String payload = root["payload"];
   Serial.println(payload);
    JsonPayload data1 = {topic,payload};
    return data1;
   
    
 }
    
  
  return data;
}


void SendMessage(char* Message)
{
  
  client.publish(root_topicOut, Message);
}




String GetComponent(String NextionMessage)
{

  NextionMessage.replace(" ","");
  NextionMessage.replace("ffff","");
  NextionMessage.trim();
    return NextionMessage;

}


void setup_wifi(){


  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  
  wifiManager.addParameter(&custom_mqtt_server);
  
  wifiManager.autoConnect("NextionAP");


strcpy(mqtt_server, custom_mqtt_server.getValue());
  


  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  
  Serial.println(custom_mqtt_server.getValue());
  
  client.setServer( mqtt_server, 1883);
  
  client.setCallback(callback);
  reconnect();

  
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

  void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(root_topicOut, "Connected!");
      // ... and resubscribe
      client.subscribe(root_topicIn);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

