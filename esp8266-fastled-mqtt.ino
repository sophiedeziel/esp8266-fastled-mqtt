/*
   ESP8266 + FastLED + MQTT: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2015 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Pour mon setup avec un wemos mini:
 * Type de carte: LOLIN(WEMOS) D1 R2 & Mini
 * Upload speed: 921600
*/


// TODO Flickering LED's ... https://github.com/FastLED/FastLED/issues/394 or https://github.com/FastLED/FastLED/issues/306
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
//#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include "GradientPalettes.h"
#include "Settings.h"



CRGB leds[NUM_LEDS];

const uint8_t brightnessCount = 5;
uint8_t brightnessMap[brightnessCount] = { 16, 32, 64, 128, 255 };
int brightnessIndex = 0;
uint8_t brightness = brightnessMap[brightnessIndex];

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 10

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

uint8_t currentPatternIndex = 10; // Index number of which pattern is current
bool autoplayEnabled = false;

uint8_t autoPlayDurationSeconds = 10;
unsigned int autoPlayTimeout = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
//
CRGB solidColor = CRGB::White;
//
uint8_t power = 1;

// Mqtt Vars
WiFiClient espClient;
PubSubClient client(espClient);


typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];
// List of patterns to cycle through.  Each is defined as a separate function below.

#include "PatternLogics.h"

PatternAndNameList patterns = {
  { colorwaves, "Color Waves" },
  { palettetest, "Palette Test" },
  { pride, "Pride" },
  { rainbow, "Rainbow" },
  { rainbowWithGlitter, "Rainbow With Glitter" },
  { confetti, "Confetti" },
  { sinelon, "Sinelon" },
  { juggle, "Juggle" },
  { bpm, "BPM" },
  { fire, "Fire" },
  { showSolidColor, "Solid Color" },
};
const uint8_t patternCount = ARRAY_SIZE(patterns);

#include "Inits.h"


void setup(void) {
  Serial.begin(115200);
  delay(100);
  //Serial.setDebugOutput(true);
  EEPROM.begin(512);
//  loadSettings();
  initFastLED();
//
  logSys();


  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected! With IP ");
  Serial.print(WiFi.localIP());
  Serial.println(" have FUN :) ");

  //  Mqtt Init
  client.setBufferSize(1024);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  //autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);
}

// Format is: command:value
// value has to be a number, except rgb commands
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  char tmp[length + 1];
  strncpy(tmp, (char*)payload, length);
  tmp[length] = '\0';
  String data(tmp);


  Serial.printf("Received Data from Topic: %s", data.c_str());
  Serial.println();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, tmp);
  
  if ( data.length() > 0) {
    const char* state = doc["state"];
    int brightness = doc["brightness"] | -1;
    const char* effect = doc["effect"] | "";
    JsonObject color = doc["color"];
    Serial.println(state);
  
    if (String(state) == "OFF") {
      setPower(0);
    } else if (String(state) == "ON") {
      setPower(1);
    } else {
      Serial.println("Invalid state");
    }
  
    if (brightness >= 0) {
      setBrightness(brightness);
    }
  
    if (effect != "") {
      Serial.println(effect);
      for (int i = 0; i < patternCount; i++) {
        if ( patterns[i].name.equals(effect)) {
          setPattern(i);
          break;
        }
      }
    } else {
      Serial.println("No effect");
    }
  
    if (!color.isNull()) {
      int r = color["r"];
      int g = color["g"];
      int b = color["b"];
  
      setSolidColor(r,g,b);
    }
  }
  Serial.println("Finished Topic Data ...");
}

void sendDiscoveryTopic(){
  Serial.println("Sending config");
  char configMessage[500];
  char effect_list[100];
  char configTopic[50];
  DynamicJsonDocument doc(500);
  
  doc["name"] = mqtt_name;
  doc["unique_id"] = mqtt_clientid;
  doc["cmd_t"] = mqtt_set_topic;
  doc["schema"] = "json";
  doc["brightness"] = true;
  doc["effect"] = true;
  doc["color_mode"] = true;
  doc["supported_color_modes"][0] = "rgb";
  doc["icon"] = "mdi:led-variant-on";

  for (int i = 0; i < patternCount; i++) {
    doc["effect_list"][i] = patterns[i].name;
  }
  
  serializeJson(doc, configMessage);
  
  sprintf(configTopic, "homeassistant/light/%s/config", mqtt_clientid);
  Serial.println(configTopic);
  Serial.println(configMessage);
  client.publish(configTopic, configMessage);
}



void loop(void) {
  yield(); // Avoid crashes on ESP8266
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));
  if (!client.connected()) {
    reconnectMqtt();
  }
  client.loop();

  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {

    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
 
    // change to a new cpt-city gradient palette
    EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {
      gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
      gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
    }
  
    // slowly blend the current cpt-city gradient palette to the next
    EVERY_N_MILLISECONDS(40) {
      nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 16);
    }
//  
//    if (autoplayEnabled && millis() > autoPlayTimeout) {
//      adjustPattern(true);
//      autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);
//    }
//  
    // Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternIndex].pattern();

    
  }

  FastLED.show();

  // insert a delay to keep the framerate modest
  delay(1000 / FRAMES_PER_SECOND);
}


void reconnectMqtt() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(mqtt_clientid, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_set_topic);
      sendDiscoveryTopic();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;
  EEPROM.write(5, power);
  EEPROM.commit();
  
}

void setSolidColor(CRGB color)
{
  setSolidColor(color.r, color.g, color.b);
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  solidColor = CRGB(r, g, b);

  EEPROM.write(2, r);
  EEPROM.write(3, g);
  EEPROM.write(4, b);

  setPattern(patternCount - 1);
}

// increase or decrease the current pattern number, and wrap around at theends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the ends
  if (currentPatternIndex < 0)
    currentPatternIndex = patternCount - 1;
  if (currentPatternIndex >= patternCount)
    currentPatternIndex = 0;

  EEPROM.write(1, currentPatternIndex);
  EEPROM.commit();
}

void setPattern(int value)
{
  // don't wrap around at the ends
  if (value < 0)
    value = 0;
  else if (value >= patternCount)
    value = patternCount - 1;

  currentPatternIndex = value;

  EEPROM.write(1, currentPatternIndex);
  EEPROM.commit();
}

// adjust the brightness, and wrap around at the ends
void adjustBrightness(bool up)
{
  if (up)
    brightnessIndex++;
  else
    brightnessIndex--;

  // wrap around at the ends
  if (brightnessIndex < 0)
    brightnessIndex = brightnessCount - 1;
  else if (brightnessIndex >= brightnessCount)
    brightnessIndex = 0;

  brightness = brightnessMap[brightnessIndex];

  FastLED.setBrightness(brightness);

  EEPROM.write(0, brightness);
  EEPROM.commit();
}

void setBrightness(int value)
{
  // don't wrap around at the ends
  if (value > 255)
    value = 255;
  else if (value < 0) value = 0;

  brightness = value;

  FastLED.setBrightness(brightness);

  EEPROM.write(0, brightness);
  EEPROM.commit();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


boolean isValidNumber(String str) {
  // TODO replace with regex check
  bool result = false;
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i))) {
      result = true;
    } else {
      result = false;
      break;
    }
  }
  return result;
}
