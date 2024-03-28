/* Beam Break Counter
 *  A program for sensors that counts the number of times a set of
 *  sensors has triggered.  This program can handle up to four sensors.
 *  The data is transmitted via an MQTT topic, and the internal counting
 *  variable can be set over an MQTT topic.  See the BrokerConfig.h tab
 *  for wifi, broker, and topic settings.
 *  
 *  This program does not continue counting when not connected to the
 *  broker.  This would require timer interrupts or pin interrupts, as
 *  the sensor pins must be checked periodically, which the WiFi and MQTT
 *  connection timeout periods can't accomodate in the main loop.
 *  
 *  MQTT Topics (See broker settings):
 *  TOPIC_BOX_COUNT   The numaric count of sensor triggers.
 *  TOPIC_SET_COUNT   The count requested by a seperate client.  Currently
 *                    only a reset function.
 *  
 *  Version: 0.0.4
 *  Author: R. D. MacBride
 *  
 *  For questions, call (804) 426-3681
 */

/* External libraries and drivers
 *  -Arduino SAMD Boards
 *  -WifiNINA 1.8.13
 *  -PubSubClient 2.8.0
 */

/* Hardware
 *  -Arduino Nano 33 IoT
 *    -Powered by 12V
 *  -Normally closed beam-break sensors connected to pins 2 through 5.
 *    -Powered by 12V
 *    -Outputs stepped down from 12V to 3.3V with a resistor divider as follows:
 *        ┌───██───┬───██───┬───██───┐
 *       12V     10k   Sensor   5.6k    D2     5.6k    0V
 */

//Definitions
#define SENSE1 2              //Sensor 1 pin
#define SENSE2 3              //Sensor 2 pin
#define SENSE3 4              //Sensor 3 pin
#define SENSE4 5              //Sensor 4 pin
#define DEBOUNCE_DELAY 100    //Debouncing delay
#define REPORT_DELAY 5000     //Serial monitor report delay
#define PUBLISH_DELAY 1000    //MQTT publish delay


//Libraries
#include <WiFiNINA.h>           //Wifi Library
#include <PubSubClient.h>       //MQTT Library
#include "BrokerConfig.h"       //VLAN and MQTT broker connection details
#include "Debounce.h"           //Debouncer Utility


//Debounced sensor inputs
Debounce Sensor1(SENSE1, DEBOUNCE_DELAY);   //Debounced input 1 class
Debounce Sensor2(SENSE2, DEBOUNCE_DELAY);   //Debounced input 2 class
Debounce Sensor3(SENSE3, DEBOUNCE_DELAY);   //Debounced input 3 class
Debounce Sensor4(SENSE4, DEBOUNCE_DELAY);   //Debounced input 4 class


//Event timers
unsigned long publishTimer = 0;   //Last publish to the MQTT broker
unsigned long reportTimer = 0;    //Last report to the serial monitor


//Box counting variables
int count = 0;                    //Box count since last reset
int pubCount = 0;                 //Last published box count

//MQTT Callback Definition
void mqttCallback(char* topic, byte* payload, unsigned int length);


//WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(BROKER_IP, BROKER_PORT, mqttCallback, wifiClient);


void setup() {
  //Start serial communication to monitor
  Serial.begin(9600);

  //Wait for connection, but timeout after half a second and continue
  //Serial communication is good for debugging, but this device normally
  //runs without it.  The delay allows for cleaner debug monitoring.
  while (!Serial && millis() < 500);
  
  Serial.println("Counter Started!");

  //Establish an initial connection to the broker.
  //The same function runs later if the connection gets interrupted.
  connectToBroker();
}


void loop() {
  //Maintain a connection to the broker.
  mqttClient.loop();
  
  //Print module status on delay timeout
  if(millis() - reportTimer > REPORT_DELAY){
    reportTimer = millis();
    
    //Print sensor states
    Serial.print("Sensors: ");
    Serial.print(String(Sensor1.state()) + ",");
    Serial.print(String(Sensor2.state()) + ",");
    Serial.print(String(Sensor3.state()) + ",");
    Serial.print(String(Sensor4.state()) + "\t");

    //Print current count
    Serial.println("Count: " + String(count));
  }

  //Publish the count on timer
  if(millis() - publishTimer > PUBLISH_DELAY){
    publishTimer = millis();
    pubCount = count;

    char buffer[33];
    if(mqttClient.publish(TOPIC_BOX_COUNT, itoa(pubCount, buffer, 10))){
      Serial.print(TOPIC_BOX_COUNT);
      Serial.print(": ");
      Serial.println(itoa(pubCount, buffer, 10));
    }else{
      Serial.println("Warning: Publish Failed!");
      connectToBroker();
    }
  }
  
  //Perform an input update and check for a state change from low to high.  Increment the counter on box detection.
  if(Sensor1.heartbeat() && Sensor1.state() == HIGH) count = count + 16;
  if(Sensor2.heartbeat() && Sensor2.state() == HIGH) count++;
  if(Sensor3.heartbeat() && Sensor3.state() == HIGH) count++;
  if(Sensor4.heartbeat() && Sensor4.state() == HIGH) count++;
}

//Process incomming contents to a subscribed topic
//There is only one, otherwise we would need to figure out the topic.
void mqttCallback(char* topic, byte* payload, unsigned int length){
  
  //String topicRootStr = String(TOPIC_ROOT);
  //if(topic == topicRootStr + "/SetCount")
  //{
  //  Serial.println("*****TRUE*****");
  //  Serial.write
    //Serial.println(payload);
  //}
  //else
  //{
    count -= pubCount;
    Serial.println("Count Reset!");
  //}
}

//Connect to WiFi and the broker.  Set up MQTT settings.
void connectToBroker(){
  //Disconnect from the broker if connected.
  //After an initial connection, PubSubClient always thinks its connected, and won't attemt
  //a reconnect.  If the program is here, the connection is no good.
  mqttClient.disconnect();

  //Attempt to connect to the network if not connected.
  if(WiFi.status() != WL_CONNECTED){
    // attempt to connect to WiFi network:
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(VLAN_SSID);

    //Set a static IP - There is a known issue assigning a static IPs on Nina.
    //Currently using DHCP reservation
    //WiFi.config(ip, dns, subnet, gateway);

    //Attempt to connect to WiFi 5 times.
    int i = 0;
    while (!WiFi.begin(VLAN_SSID, VLAN_PASS)) {
      Serial.print(".");
      delay(1000);
      i++;
      if(i >= 5) break;
    }

    //Provide WiFi post-setup information
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to Network!");
      Serial.println();

      //Check Firmware of the wireless module.  
      String fv = WiFi.firmwareVersion();
      Serial.print("Firmware: ");
      Serial.println(fv);
      if (fv != "1.0.0") {
        Serial.println("Please upgrade the firmware");
      }
    
      Serial.print("Local IP: ");
      Serial.println(WiFi.localIP());
    
      Serial.print("Subnet Mask: ");
      Serial.println(WiFi.subnetMask());

      Serial.print("Gateway IP: ");
      Serial.println(WiFi.gatewayIP());

      printCurrentNet();
      printWiFiData();
    }else{
      Serial.println("Connection to Network Failed!");
    }
    Serial.println();
  }


  //Connect to the broker
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.print(BROKER_IP);
  Serial.print(":");
  Serial.println(BROKER_PORT);

  //Attempt to connect to the broker 5 times.
  int i = 0;
  while (!mqttClient.connect(BROKER_DEVICE, BROKER_USER, BROKER_PASS)) {
    Serial.print(".");
    delay(1000);
    i++;
    if(i > 5) break;
  }

  //Provide broker post-setup information
  if(mqttClient.connected()){
    
    //Subscribe to set topic
    mqttClient.subscribe(TOPIC_SET_COUNT);
  
    Serial.println("Connected to Broker!");
    Serial.println();
  }else{
    Serial.println("Connection to Broker Failed!");
    Serial.println();
  }
}

//Convert MAC array to string
//Code from getmicros.net
void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void printWiFiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address : ");
  Serial.println(ip);

  Serial.print("Subnet mask: ");
  Serial.println((IPAddress)WiFi.subnetMask());

  Serial.print("Gateway IP : ");
  Serial.println((IPAddress)WiFi.gatewayIP());

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}
