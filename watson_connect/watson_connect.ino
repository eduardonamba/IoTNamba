#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>
#include <PubSubClient.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

/* ------------------ */
/* SKETCH CREDENTIALS */
/* ------------------ */

char* deviceId     = "nambaproject001"; // * set your device id (will be the MQTT client username)
char* deviceSecret = "TnosSCBq3rUBu0/RRUB3BXKZ3qZCLN5g"; // * set your device secret (will be the MQTT client password)
char* outTopic     = "devices/nambaproject001/set"; // * MQTT channel where physical updates are published
char* inTopic      = "devices/nambaproject001/get"; // * MQTT channel where lelylan updates are received
char* clientId     = "KY-001"; // * set a random string (max 23 chars, will be the MQTT client id)


/* ------------ */
/* SKETCH LOGIC */
/* ------------ */

/* Server settings */
byte server[] = { 96, 126, 109, 170 }; // MQTT server address

/* Sample payload published to lelylan */
/* The id is the status property id of the basic light /*
/* http://lelylan.github.io/types-dashboard-ng/#/types/518be107ef539711af000001/ */
char* payloadOn  = "{\"properties\":[{\"id\":\"54033694bdd5392354000006\",\"value\":\"10\"}]}";
char* payloadOff = "{\"properties\":[{\"id\":\"54033694bdd5392354000006\",\"value\":\"20\"}]}";

/* ---------------- */
/* WIFI CREDENTIALS */
/* ---------------- */
char ap_ssid[]     = "nWifi"; //"AndroidAP";         // SSID of network
char ap_password[] = "N@mb@007";  //"PASSWORD";          // Password of network

/* ------------ */
/* WIFI SETTINGS */
/* ------------ */

// Pins
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// Connection info data lengths
#define IP_ADDR_LEN     4   // Length of IP address in bytes

// Constants

const unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
const unsigned int timeout = 30000;             // Milliseconds

/* Wifi configuration */
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client wifiClient = SFE_CC3000_Client(wifi);

/* MQTT communication */
void callback(char* topic, byte* payload, unsigned int length); // subscription callback
PubSubClient client(server, 1883, callback, wifiClient);         // mqtt client

/* ------------    */
/* SENSOR SETTINGS */
/* ------------    */

// Data wire is plugged into port 2 on the Arduino
//#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
//OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
//DallasTemperature sensors(&oneWire);

long time = 0;        // the last time the output pin was toggled
long debounce = 30000;  // the debounce time, increase if the output flickers

char message_buff[100];

void setup(void)
{
  ConnectionInfo connection_info;
  int i;
  
  // start serial port
  Serial.begin(9600);
  
  // Initialize CC3000 (configure SPI communications)
  if ( wifi.init() )
  {
    Serial.println("CC3000 initialization complete");
  }
  else
  {
    Serial.println("Something went wrong during CC3000 init!");
  }
  

  // Connect using DHCP
  Serial.print("Connecting to SSID: ");
  Serial.println(ap_ssid);
  if(!wifi.connect(ap_ssid, ap_security, ap_password, timeout))
  {
    Serial.println("Error: Could not connect to AP");
  }

  // Gather connection details and print IP address
  if ( !wifi.getConnectionInfo(connection_info) )
  {
    Serial.println("Error: Could not obtain connection details");
  }
  else
  {
    Serial.print("IP Address: ");
    for (i = 0; i < IP_ADDR_LEN; i++)
    {
      Serial.print(connection_info.ip_address[i]);
      if ( i < IP_ADDR_LEN - 1 )
      {
        Serial.print(".");
      }
    }
    Serial.println();
  }
  
  lelylanConnection(); // MQTT server connection
  
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  //sensors.begin();
}

void loop(void)
{
  lelylanConnection();
  
  if (millis() - time > debounce) {
    // call sensors.requestTemperatures() to issue a global temperature
    // request to all devices on the bus
    Serial.print("Requesting temperatures...");
    //sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
  
    Serial.print("Temperature for the device 1 (index 0) is: ");
    float temp = 345.55;//sensors.getTempCByIndex(0);
    Serial.println(temp);
    
    lelylanPublish(temp);
    
    time = millis();
  }
}

/* MQTT server connection */
void lelylanConnection()
{
  // add reconnection logics
  if (!client.connected())
  {
    // connection to MQTT server
    if (client.connect(clientId, deviceId, deviceSecret))
    {
      Serial.println("[PHYSICAL] Successfully connected with MQTT");
      //lelylanSubscribe(); // topic subscription
    }
  }
  client.loop();
}

/* MQTT publish */
void lelylanPublish(float value)
{
  // "{\"properties\":[{\"id\":\"54033694bdd5392354000006\",\"value\":\"10\"}]}"
  char charVal[10];
  dtostrf(value, 4, 3, charVal);
  
  String pubString = "{\"properties\":[{\"id\":\"54033694bdd5392354000006\",\"" + String(charVal) + "\"}]}";
    Serial.println(pubString);
    pubString.toCharArray(message_buff, pubString.length()+1);
    
    client.publish(outTopic, message_buff);
    
  //if (value == "on")
  //  client.publish(outTopic, payloadOn); // light on
  //else
  //  client.publish(outTopic, payloadOff); // light off
}

/* Receive Lelylan message and confirm the physical change */
void callback(char* topic, byte* payload, unsigned int length)
{
}

