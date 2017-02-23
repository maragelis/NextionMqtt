#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Nextion.h>
#include <SoftwareSerial.h>


const char* ssid = "SSID";
const char* password = "*********";
const char* mqtt_server = "192.168.2.231";
const char* root_topicOut = "Nextion/Out";
const char* root_topicIn = "Nextion/In";

WiFiClient espClient;
PubSubClient client(espClient);


SoftwareSerial nextion(13, 15);
Nextion myNextion(nextion, 9600);

struct JsonPayload{
  String Topic;
  String Payload;
} ;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Program Start");
setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
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


void DoHisterisi(String Component)
{
   if(Component=="65230")
    {
      myNextion.setComponentText("Histerisi.t0","2,2");
      
    }else
    {
      myNextion.setComponentText("Histerisi.t0","1,1");
      
    }
   
   
   //Component=65220)
}

String GetComponent(String NextionMessage)
{

  NextionMessage.replace(" ","");
  NextionMessage.replace("ffff","");
  NextionMessage.trim();
    return NextionMessage;

}


void setup_wifi(){

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_AP);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
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


