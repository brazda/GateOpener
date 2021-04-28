#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const boolean static_ip = STATIC_IP;
IPAddress ip(IP);
IPAddress gateway(GATEWAY);
IPAddress subnet(SUBNET);

const char* mqtt_broker = MQTT_BROKER;
const char* mqtt_clientId = MQTT_CLIENTID;
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;

const char* GATE1_alias = GATE1_ALIAS;
const char* mqtt_GATE1_action_topic = MQTT_GATE1_ACTION_TOPIC;
const char* mqtt_GATE1_status_topic = MQTT_GATE1_STATUS_TOPIC;
const int GATE1_openPin = GATE1_OPEN_PIN;
const int GATE1_closePin = GATE1_CLOSE_PIN;
const int GATE1_statusPin = GATE1_STATUS_PIN;
const char* GATE1_statusSwitchLogic = GATE1_STATUS_SWITCH_LOGIC;

const boolean GATE2_enabled = GATE2_ENABLED;
const char* GATE2_alias = GATE2_ALIAS;
const char* mqtt_GATE2_action_topic = MQTT_GATE2_ACTION_TOPIC;
const char* mqtt_GATE2_status_topic = MQTT_GATE2_STATUS_TOPIC;
const int GATE2_openPin = GATE2_OPEN_PIN;
const int GATE2_closePin = GATE2_CLOSE_PIN;
const int GATE2_statusPin = GATE2_STATUS_PIN;
const char* GATE2_statusSwitchLogic = GATE2_STATUS_SWITCH_LOGIC;
String requestedAction = "";
String requestedgate = "";

int GATE1_lastStatusValue = 2;
int GATE2_lastStatusValue = 2;
unsigned long GATE1_lastSwitchTime = 0;
unsigned long GATE2_lastSwitchTime = 0;
int debounceTime = 2000;
int count=0;
unsigned long previousTime = 0;
unsigned long currentTime = 0;
const unsigned long eventInterval = 30000;
const unsigned long openInterval = 3000;


int runstatd1 = 0;
int runstatd2 = 0;

String availabilityBase = MQTT_CLIENTID;
String availabilitySuffix = "/availability";
String availabilityTopicStr = availabilityBase + availabilitySuffix;
const char* availabilityTopic = availabilityTopicStr.c_str();
const char* birthMessage = "online";
const char* lwtMessage = "offline";

WiFiClient espClient;
PubSubClient client(espClient);

// Wifi setup function

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (static_ip) {
    WiFi.config(ip, gateway, subnet);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(" WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback when MQTT message is received; calls triggergateAction(), passing topic and payload as parameters

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char*)payload;
  triggergateAction(topicToProcess, payloadToProcess);
}

// Functions that check gate status and publish an update when called

void publish_GATE1_status() {
  if (digitalRead(GATE1_statusPin) == LOW) {
    if (GATE1_statusSwitchLogic == "NO") {
      Serial.print(GATE1_alias);
      Serial.print(" closed! Publishing to ");
      Serial.print(mqtt_GATE1_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE1_status_topic, "CLOSE", true);
    }
    else if (GATE1_statusSwitchLogic == "NC") {
      Serial.print(GATE1_alias);
      Serial.print(" open! Publishing to ");
      Serial.print(mqtt_GATE1_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE1_status_topic, "OPEN", true);      
    }
    else {
      Serial.println("Error! Specify only either NO or NC for GATE1_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
    }
  }
  else {
    if (GATE1_statusSwitchLogic == "NO") {
      Serial.print(GATE1_alias);
      Serial.print(" open! Publishing to ");
      Serial.print(mqtt_GATE1_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE1_status_topic, "OPEN", true);
    }
    else if (GATE1_statusSwitchLogic == "NC") {
      Serial.print(GATE1_alias);
      Serial.print(" closed! Publishing to ");
      Serial.print(mqtt_GATE1_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE1_status_topic, "CLOSE", true);      
    }
    else {
      Serial.println("Error! Specify only either NO or NC for GATE1_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
    }
  }
}

void publish_GATE2_status() {
  if (digitalRead(GATE2_statusPin) == LOW) {
    if (GATE2_statusSwitchLogic == "NO") {
      Serial.print(GATE2_alias);
      Serial.print(" closed! Publishing to ");
      Serial.print(mqtt_GATE2_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE2_status_topic, "CLOSE", true);
    }
    else if (GATE2_statusSwitchLogic == "NC") {
      Serial.print(GATE2_alias);
      Serial.print(" open! Publishing to ");
      Serial.print(mqtt_GATE2_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE2_status_topic, "OPEN", true);      
    }
    else {
      Serial.println("Error! Specify only either NO or NC for GATE2_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
    }
  }
  else {
    if (GATE2_statusSwitchLogic == "NO") {
      Serial.print(GATE2_alias);
      Serial.print(" open! Publishing to ");
      Serial.print(mqtt_GATE2_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE2_status_topic, "OPEN", true);
    }
    else if (GATE2_statusSwitchLogic == "NC") {
      Serial.print(GATE2_alias);
      Serial.print(" closed! Publishing to ");
      Serial.print(mqtt_GATE2_status_topic);
      Serial.println("...");
      client.publish(mqtt_GATE2_status_topic, "CLOSE", true);      
    }
    else {
      Serial.println("Error! Specify only either NO or NC for GATE2_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
    }
  }
}


// Function that publishes birthMessage

void publish_birth_message() {
  // Publish the birthMessage
  Serial.print("Publishing birth message \"");
  Serial.print(birthMessage);
  Serial.print("\" to ");
  Serial.print(availabilityTopic);
  Serial.println("...");
  client.publish(availabilityTopic, birthMessage, true);
}


// Function called by callback() when a message is received 
// Passes the message topic as the "requestedgate" parameter and the message payload as the "requestedAction" parameter

void triggergateAction(String requestedgate, String requestedAction) {
  if (requestedgate == mqtt_GATE1_action_topic && requestedAction == "OPEN") {
    Serial.print("Triggering ");
    Serial.print(GATE1_alias);
    Serial.println(" OPEN relay!");
    uint16_t i, j;
    analogWrite(motor1pin1, 0);
    analogWrite(motor1pin2, 800);    
    digitalWrite(RPWM_Output, HIGH);
    delay(1000);
    runstatd1 = 1; 
    }
  else if (requestedgate == mqtt_GATE1_action_topic && requestedAction == "CLOSE") {
    Serial.print("Triggering ");
    Serial.print(GATE1_alias);
    Serial.println(" CLOSE relay!");
    analogWrite(motor1pin1, 800);
    analogWrite(motor1pin2, 0);
    digitalWrite(RPWM_Output, HIGH);
    delay(1000);
    runstatd1 = 1; 

  }
  else if (requestedgate == mqtt_GATE1_action_topic && requestedAction == "STATE") {
    Serial.print("Publishing on-demand status update for ");
    Serial.print(GATE1_alias);
    Serial.println("!");
    publish_birth_message();
    publish_GATE1_status();
  }
  else if (requestedgate == mqtt_GATE1_action_topic && requestedAction == "STOP") {
   Serial.print("Triggering ");
    Serial.print(GATE1_alias);
    Serial.println(" STOP relay!");
    analogWrite(motor1pin1, 0);
    analogWrite(motor1pin2, 0);    
    digitalWrite(RPWM_Output, LOW);
    runstatd1 = 0; 
  }
  else if (requestedgate == mqtt_GATE2_action_topic && requestedAction == "OPEN") {
    Serial.print("Triggering ");
    Serial.print(GATE2_alias);
    Serial.println(" OPEN relay!");
    analogWrite(motor2pin1, 800);
    analogWrite(motor2pin2, 0);
    digitalWrite(LPWM_Output, HIGH);
    runstatd2 = 1; 
  }
  else if (requestedgate == mqtt_GATE2_action_topic && requestedAction == "CLOSE") {
    Serial.print("Triggering ");
    Serial.print(GATE2_alias);
    Serial.println(" CLOSE relay!");
    analogWrite(motor2pin1, 0);
    analogWrite(motor2pin2, 800);
    digitalWrite(LPWM_Output, HIGH);
    runstatd2 = 1; 
  }
  else if (requestedgate == mqtt_GATE2_action_topic && requestedAction == "STATE") {
    Serial.print("Publishing on-demand status update for ");
    Serial.print(GATE2_alias);
    Serial.println("!");
    publish_birth_message();
    publish_GATE2_status();
  }
    else if (requestedgate == mqtt_GATE2_action_topic && requestedAction == "STOP") {
    Serial.print("Triggering ");
    Serial.print(GATE2_alias);
    Serial.println(" CLOSE relay!");
    analogWrite(motor2pin1, 0);
    analogWrite(motor2pin2, 0);
    digitalWrite(LPWM_Output, LOW);
    runstatd2 = 0; 
  }  
  else { Serial.println("Unrecognized action payload... taking no action!");
  }
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current gate statuses on connect

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_clientId, mqtt_username, mqtt_password, availabilityTopic, 0, true, lwtMessage)) {
      Serial.println("Connected!");

      // Publish the birth message on connect/reconnect
      publish_birth_message();

      // Subscribe to the action topics to listen for action messages
      Serial.print("Subscribing to ");
      Serial.print(mqtt_GATE1_action_topic);
      Serial.println("...");
      client.subscribe(mqtt_GATE1_action_topic);
      
      if (GATE2_enabled) {
        Serial.print("Subscribing to ");
        Serial.print(mqtt_GATE2_action_topic);
        Serial.println("...");
        client.subscribe(mqtt_GATE2_action_topic);
      }

      // Publish the current gate status on connect/reconnect to ensure status is synced with whatever happened while disconnected
      publish_GATE1_status();
      if (GATE2_enabled) { publish_GATE2_status();
      }

    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Setup the output and input pins used in the sketch
  // Set the lastStatusValue variables to the state of the status pins at setup time
  // Setup gate 1 pins
  pinMode(GATE1_openPin, OUTPUT);
  pinMode(GATE1_closePin, OUTPUT);
  
  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);
  

  // Set input pin to use internal pullup resistor
  pinMode(GATE1_statusPin, INPUT_PULLUP);
  // Update variable with current gate state
  GATE1_lastStatusValue = digitalRead(GATE1_statusPin);

  // Setup gate 2 pins
  if (GATE2_enabled) {
    pinMode(GATE2_openPin, OUTPUT);
    pinMode(GATE2_closePin, OUTPUT);


    // Set input pin to use internal pullup resistor
    pinMode(GATE2_statusPin, INPUT_PULLUP);
    // Update variable with current gate state
    GATE2_lastStatusValue = digitalRead(GATE2_statusPin);
  }

  // Setup serial output, connect to wifi, connect to MQTT broker, set MQTT message callback
  Serial.begin(115200);

  Serial.println("Starting GateOpener...");

  setup_wifi();
  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);

    // Port defaults to 8266
   ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("GateController");

  // No authentication by default
   ArduinoOTA.setPassword((const char *)"321");
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
//digitalWrite(GATE1_statusPin, HIGH);

    /* Updates frequently */
  unsigned long currentTime = millis();

  /* This is the event */
  // Connect/reconnect to the MQTT broker and listen for messages
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

 if ((currentTime - previousTime >= eventInterval) && (runstatd1 = 1)) {
    analogWrite(motor1pin1, 0);
    analogWrite(motor1pin2, 0);
    digitalWrite(RPWM_Output, LOW);
    runstatd1 = 0;      
    previousTime = currentTime;
    }
 if ((currentTime - previousTime >= eventInterval) && (runstatd2 = 1)) {
    analogWrite(motor2pin1, 0);
    analogWrite(motor2pin2, 0);
    digitalWrite(LPWM_Output, LOW);
    runstatd2 = 0;      
    previousTime = currentTime;
    }
  
if ((digitalRead(GATE1_statusPin) == LOW) && (runstatd1 = 1 ))  {
    analogWrite(motor1pin1, 0);
    analogWrite(motor1pin2, 0);
    digitalWrite(RPWM_Output, LOW);
    runstatd1 = 0;      
    }
if ((digitalRead(GATE1_statusPin) == LOW) && (runstatd2 = 1 ))  {
    analogWrite(motor2pin1, 0);
    analogWrite(motor2pin2, 0);
    digitalWrite(LPWM_Output, LOW);
    runstatd2 = 0;      
    }
    

}
