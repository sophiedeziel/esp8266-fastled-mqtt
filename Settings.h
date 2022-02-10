const bool apMode = false;

// AP mode password
const char WiFiAPPSK[] = "";

// Wi-Fi network to connect to (if not in AP mode)
const char* ssid = "";
const char* password = "";

#define DATA_PIN      D5
#define CLK_PIN       D4   
#define LED_TYPE      SK9822
#define COLOR_ORDER   BGR
#define NUM_LEDS      20

#define MILLI_AMPS         500     // IMPORTANT: set here the maxmilli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

// MQTT Broker settings
const char* mqtt_server = "10.0.1.7";
const int mqtt_port = 1883;
const char* mqtt_user = "technopolisha";
const char* mqtt_password = "unchatdanslo";
const char* mqtt_set_topic = "lampe_led/5/set";
const char* mqtt_state_topic = "lampe_led/5/state";
const char* mqtt_clientid = "lampe_led/5";
const char* mqtt_name = "Lampe LED géante";


// openssl x509 -fingerprint -in  mqttserver.crt - Only if you must verify your certs for connection issues with MQTT
//const char* fingerprint = "AF FD A4 F6 3B 74 FE EE A5 71 3A 08 7F 30 F8 CF DE 8D E6 7F";
