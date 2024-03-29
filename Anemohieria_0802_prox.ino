#pragma once
/*
   Anemohiereia: (Mycenaean Greek) The wind priestess
   ESP8266 Weather display based on openweathermap.org

   Curator: iteinos@github
   References: see comments below
*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
#define inactivity_delay 60000 //screen backlight auto-desactivation after certain time
#define DOP 12 //doppler radar pin
long inactivity_timer = 0;
/*
   ESP8266 Manual Wifi Configuration with EEPROM without Hard-Code
   sourced from how2electronics.com

   https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/

*/
//Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;
long offline_timer = 0;

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-open-weather-map-thingspeak-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Code compatible with ESP8266 Boards Version 3.0.0 or above
  (see in Tools > Boards > Boards Manager > ESP8266)
*/

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "USE YOURS"; //change to your api key
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd4";

// Replace with your country code and city
String city = "MANQUE"; //need not change
String countryCode = "MANQUE"; //need not change
bool rebootUpdate = 0;
// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 3600000;
String weather_actual;
String jsonBuffer;
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

byte character_antenna[8] = { B10101, B10101, B01110, B00100, B00100, B00100, B00100, B00100 };
byte character_serial[8] = { B01010, B01010, B11111, B01110, B01110, B00100, B00100, B00100 };
byte character_centigrade[8] = { B01000, B10100, B01000, B00011, B00100, B00100, B00011, B00000 };
byte character_clock[8] = { B0, B1110, B10101, B10111, B10001, B1110, B0, B0 };
byte celsius[] = {
  B11000,
  B11000,
  B00110,
  B01001,
  B01000,
  B01001,
  B00110,
  B00000
};
LiquidCrystal_I2C lcd(0x27, 8, 2);
const char* timezone;
bool miseajour = 0;
JSONVar myObject;
void getWeather() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay ) miseajour = 0;
  if (!miseajour || !rebootUpdate) {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      if (!rebootUpdate) rebootUpdate = 1;
      if (rebootUpdate && (!miseajour)) miseajour = 1;
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      Serial.println(serverPath);
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      myObject = JSON.parse(jsonBuffer);
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      if (rebootUpdate || miseajour) {
        lcd.clear();
        lcd.setCursor(0, 1);
        int tmp = myObject["main"]["temp"];
        tmp -= 273;
        lcd.print(tmp);
        lcd.print(char(2));
        lcd.print(" ");
        lcd.print(myObject["main"]["humidity"]);
        lcd.print("%");
        lcd.setCursor(0, 1);
        String wedS = JSON.stringify(myObject["weather"]);
        char wed[500];
        wedS.toCharArray(wed, 500);
        /*THIS PART CHOPS THE JSON STRING BIT BY BIT TO GET THE WEATHER DESCRIPTION STRING*/
        char* token1 = strtok(wed, ",");
        char* token2 = strtok(NULL, ",");
        char* token3 = strtok(NULL, ",");
        char* wedFull1 = strtok(token3, "\"");
        char* wedFull2 = strtok(NULL, "\"");
        char* wedFull3 = strtok(NULL, "\"");
        Serial.print("Weather: ");
        Serial.println(wedFull3); //weather full description
        weather_actual = String(wedFull3);
        char* wedescr1 = strtok(token2, "\"");
        char* wedescr2 = strtok(NULL, "\"");
        char* wedescr3 = strtok(NULL, "\"");
        Serial.print("Weather description: ");
        Serial.println(wedescr3);
        String wedshort = String(wedescr3);
        wedshort.toUpperCase();
        // lcd.print(wedshort); //weather short description////////////////////////////////////
        //lcd.setCursor(8, 0);
        // lcd.print(myObject["main"]["pressure"]);
        //lcd.print("hPa");
        Serial.print("JSON object = ");
        Serial.println(myObject);
        Serial.print("Temperature: ");
        Serial.println(myObject["main"]["temp"]); //IN KELVIN
        Serial.print("Pressure: ");
        Serial.println(myObject["main"]["pressure"]);
        Serial.print("Humidity: ");
        Serial.println(myObject["main"]["humidity"]);
        Serial.print("Wind Speed: ");
        Serial.println(myObject["wind"]["speed"]);
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("!ERROR!");
        lcd.setCursor(0, 1);
        lcd.print("!RETRY!");
      }
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    rebootUpdate = 0;
  }
  // Free resources
  http.end();

  return payload;
}
void setup()
{
  delay(700);
  pinMode(DOP, INPUT);
  lcd.init();
  lcd.createChar(0, character_antenna);
  lcd.createChar(1, character_serial);
  lcd.createChar(2, celsius);
  lcd.createChar(3, character_clock);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOAD");
  lcd.setCursor(0, 1);
  lcd.print("CONFIG");
  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOADING");
  //---------------------------------------- Read EEPROM for SSID and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  char espace = ' ';
  String ecity = "";
  for (int i = 96; i < 128; ++i)
  {
    if (char(EEPROM.read(i)) != espace)
      ecity += char(EEPROM.read(i));
    else ecity += "\0";
  }
  Serial.print("CITY: ");
  Serial.println(ecity);
  ecity.trim();
  city = ecity.c_str();
  lcd.setCursor(0, 0);
  lcd.print(city);
  lcd.print(",");
  String ecountry = "";
  for (int i = 128; i < 160; ++i)
  {
    if (char(EEPROM.read(i)) != espace)
      ecountry += char(EEPROM.read(i));
    else ecountry += "\0";
  }
  String etimezone = "";
  for (int i = 160; i < 192; ++i)
  {
    if (char(EEPROM.read(i)) != espace)
      etimezone += char(EEPROM.read(i));
    else etimezone += "\0";
  }
  Serial.print("COUNTRY: ");
  lcd.setCursor(0, 1);
  Serial.println(ecountry);
  ecountry.trim();
  countryCode = ecountry.c_str();
  timezone = etimezone.c_str();
  timeClient.begin();
  int offset = atoi(timezone) * 3600;
  Serial.println(offset);
  timeClient.setTimeOffset(offset);
  lcd.print(countryCode);
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SUCCESS");
    lcd.setCursor(0, 1);
    lcd.print(esid.c_str());
    Serial.println("Connected to Internet.");
    return;
  }
  else
  {
    lcd.clear();
    switch_LCD_content("Apo_Anemohiereia", "@192.168.4.1");
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

}

long dernierTime = 0;

void scrollTime() {
  String tempus = timeClient.getFormattedTime();
  lcd.setCursor(0, 0);
  lcd.print(tempus.substring(0, 2));
  lcd.print(":");
  lcd.print(tempus.substring(3, 5));
  lcd.print(":");
  lcd.print(tempus.substring(6, 8));
}

bool hibernated = 0;

void loop() {
  /**DOPPLER PRECSENCE SENSOR WILL DESACTIVATE BACKLIGHT TO SAVE POWER**/
  if (digitalRead(DOP) && hibernated) {
    Serial.println("DISPLAY RESUSCITATED");
    hibernated = 0;
    inactivity_timer = millis();
    lcd.setBacklight(1);
    lcd.display();
    scrollTime();
    lcd.setCursor(0, 1);
    int tmp = myObject["main"]["temp"];
    tmp -= 273;
    lcd.print(tmp);
    lcd.print(char(2));
    lcd.print(" ");
    lcd.print(myObject["main"]["humidity"]);
    lcd.print("%");
    lcd.setCursor(0, 1);
    String wedS = JSON.stringify(myObject["weather"]);
    char wed[500];
    wedS.toCharArray(wed, 500);
    /*THIS PART CHOPS THE JSON STRING BIT BY BIT TO GET THE WEATHER DESCRIPTION STRING*/
    char* token1 = strtok(wed, ",");
    char* token2 = strtok(NULL, ",");
    char* token3 = strtok(NULL, ",");
    char* wedFull1 = strtok(token3, "\"");
    char* wedFull2 = strtok(NULL, "\"");
    char* wedFull3 = strtok(NULL, "\"");
    Serial.print("Weather: ");
    Serial.println(wedFull3); //weather full description
    weather_actual = String(wedFull3);
  } else if (digitalRead(DOP) && !hibernated) {
    inactivity_timer = millis();
  }
  if (millis() - inactivity_timer > inactivity_delay && !hibernated) {
    lcd.setBacklight(0);
    lcd.clear();
    lcd.noDisplay();
    hibernated = 1;
    Serial.println("DISPLAY HIBERNATED");
  }
  if ((WiFi.status() == WL_CONNECTED)){
    offline_timer = millis();
  }
  if(millis() - offline_timer > 5*60000) ESP.restart(); //auto reboot when disconnected from wifi
  if (!hibernated) {
    scrollWeather();
    if ((WiFi.status() == WL_CONNECTED))
    {
      getWeather();
      while ((!timeClient.update() && (millis() - dernierTime) > timerDelay) || ((!timeClient.update()) && millis() < 10000)) {
        timeClient.forceUpdate();
        dernierTime = millis();
        Serial.println("Time updated");
      }
    }
    else
    { lcd.setCursor(7, 1);
      lcd.print(char(0));
    }
  }
  if(millis()<5*60000) ArduinoOTA.handle(); //OTA service
}


//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connection timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("Apo_Anemohiereia", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Anemohiereia Configuration Interface";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PASS: </label><input name='pass' length=64><label>CITY: </label><input name='city' length=32><label>COUNTRY: </label><input name='country' length=32><label>TIMEZONE: </label><input name='timezone' length=32><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qcity = server.arg("city");
      String qcountry = server.arg("country");
      String qtimezone = server.arg("timezone");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 256; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        } for (int i = 0; i < qcity.length(); ++i)
        {
          EEPROM.write(96 + i, qcity[i]);
          Serial.print("Wrote: ");
          Serial.println(qcity[i]);
        } for (int i = 0; i < qcountry.length(); ++i)
        {
          EEPROM.write(128 + i, qcountry[i]);
          Serial.print("Wrote: ");
          Serial.println(qcountry[i]);
        } for (int i = 0; i < qtimezone.length(); ++i)
        {
          EEPROM.write(160 + i, qtimezone[i]);
          Serial.print("Wrote: ");
          Serial.println(qtimezone[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      }
      else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

long scrollTimer = 0;

void scrollWeather() {
  scrollTime();
  if (millis() - scrollTimer > 15000) {
    Serial.println("SCROLLING");
    scrollTimer = millis();
    String wedbuffer = "        ";
    wedbuffer.concat(weather_actual);
    wedbuffer.concat(" ");
    wedbuffer.toUpperCase();
    for (;;) {
      scrollTime();
      lcd.setCursor(0, 1);
      lcd.print(wedbuffer);
      wedbuffer = wedbuffer.substring(1, wedbuffer.length());
      if (wedbuffer.length() < 8) {
        for (int i = 8 - wedbuffer.length(); i > 0; i--) {
          lcd.print(" ");
        }
      }
      delay(250);
      if (wedbuffer.length() == 0) {
        lcd.clear();
        lcd.setCursor(0, 1);
        int tmp = myObject["main"]["temp"];
        tmp -= 273;
        lcd.print(tmp);
        lcd.print(char(2));
        lcd.print(" ");
        lcd.print(myObject["main"]["humidity"]);
        lcd.print("%");
        lcd.setCursor(0, 1);
        String wedS = JSON.stringify(myObject["weather"]);
        char wed[500];
        wedS.toCharArray(wed, 500);
        /*THIS PART CHOPS THE JSON STRING BIT BY BIT TO GET THE WEATHER DESCRIPTION STRING*/
        char* token1 = strtok(wed, ",");
        char* token2 = strtok(NULL, ",");
        char* token3 = strtok(NULL, ",");
        char* wedFull1 = strtok(token3, "\"");
        char* wedFull2 = strtok(NULL, "\"");
        char* wedFull3 = strtok(NULL, "\"");
        Serial.print("Weather: ");
        Serial.println(wedFull3); //weather full description
        weather_actual = String(wedFull3);
        break;
      }
    }
  }
}

long switch_content_timer = 0;
bool switch_state = 1;
void switch_LCD_content(String a, String b) {
  String upperA = a.substring(0, 8);
  String lowerA = a.substring(8, 16);
  String upperB = b.substring(0, 8);
  String lowerB = b.substring(8, 16);
  String upperBuffer;
  String lowerBuffer;
  if (millis() - switch_content_timer > 1000) {
    switch_state = 1 - switch_state;
    switch_content_timer = millis();
  }
  if (switch_state) {
    upperBuffer = upperA;
    lowerBuffer = lowerA;
  } else {
    upperBuffer = upperB;
    lowerBuffer = lowerB;
  }
  lcd.setCursor(0, 0);
  lcd.print(upperBuffer);
  lcd.setCursor(0, 1);
  lcd.print(lowerBuffer);
}
