/* Broker Congig
 *  This header file defines a series of configuration for connecting to WiFI,
 *  an MQTT broker, and setting MQTT topics
 */
//Libraries
#include <WiFiNINA.h>           //Wifi Library

//Set up the WiFi connection
#define VLAN_SSID "CONTRCTMFG"                     //VLAN SSID
#define VLAN_PASS "Tuthm0d!fy$s"                   //VLAN Password
IPAddress ip(192, 168, 133, 20);                   //Device IP
IPAddress dns(192, 168, 133, 20);                  //Device DNS
IPAddress subnet(255, 255, 255, 128);              //Device Subnet
IPAddress gateway(192, 168, 133, 1);               //Device Gateway
byte mac[6];

//Set up the broker connection
#define BROKER_IP "192.168.190.52"
#define BROKER_PORT 1883
#define BROKER_USER NULL //"amtadmin"
#define BROKER_PASS NULL //"Mab3l2020@"
#define BROKER_DEVICE "MaximBioCounter1"

//Root topic path
#define TOPIC_ROOT      "FlexibleAutomation/MaximBio/Line"

//Specific topics under the root topic path
#define TOPIC_TIMER (TOPIC_ROOT "/t")             //(Publish) Regularly changing variable for connection timeouts
#define TOPIC_BOX_COUNT (TOPIC_ROOT "/BoxCount")  //(Publish) number of sensor triggers
#define TOPIC_SET_COUNT (TOPIC_ROOT "/SetCount")  //(Subscribe) change the internal count variable
