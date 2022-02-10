


void initFastLED(void) {
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(100);
  //TODO on 160Hz the LED's are full white! Why?
//  if(power == 1 && currentPatternIndex == patternCount - 1) {
//    fill_solid(leds, NUM_LEDS, solidColor);  
//  }else {
//    fill_solid(leds, NUM_LEDS, CRGB::Black);  
//  }
  
  FastLED.show();
}

//void loadSettings()
//{
//  brightness = 100;
//
//  currentPatternIndex = EEPROM.read(1);
//  if (currentPatternIndex < 0)
//    currentPatternIndex = 0;
//  else if (currentPatternIndex >= patternCount)
//    currentPatternIndex = patternCount - 1;
//
//  byte r = EEPROM.read(2);
//  byte g = EEPROM.read(3);
//  byte b = EEPROM.read(4);
//
//  if (r == 0 && g == 0 && b == 0)
//  {
//  }
//  else
//  {
//    solidColor = CRGB(r, g, b);
//  }
////  power = EEPROM.read(5);
//}

void logSys() {
  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();
}



// Only to test if SSL cert is matching if u have connection problems ...
//void verifytls() {
//  // Use WiFiClientSecure class to create TLS connection
//  Serial.print(" verifytls ... connecting to ");
//  Serial.println(mqtt_server);
//  if (!espClient.connect(mqtt_server, mqtt_port)) {
//    Serial.println("connection failed");
//    return;
//  }
//
//  Serial.println(" checking fingerprint ... ");
//  if (espClient.verify(fingerprint, mqtt_server)) {
//    Serial.println("certificate matches");
//  } else {
//    Serial.println("certificate doesn't match");
//  }
//}
