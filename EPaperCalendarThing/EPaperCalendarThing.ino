#include <ArduinoJson.h>

#include <GxEPD.h>
#include "SPI.h"
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel
#include <WiFi.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include "icons.h"


#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10800        /* Time ESP32 will go to sleep (in seconds) */

// !!! DANGER !!!
// Reading this code may cause headaches, depression, 
// PTSD and possibly other undocumented illness.
// CONTINUE WITH CAUTION!

// Load Picopixel Font
#include <Fonts/Picopixel.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>




// Configure SPI EPaper Pins
#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17


// Manual wake up button
#define BUTTON_PIN 39



const char* ssid     = "SSID";
const char* password = "WIFIPASSWORD";



// Strings for months and weeks, using indonesian because its shorter and easier to fit 
String weeks[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String months[12] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};

// create the display object
GxIO_Class io(SPI, /*CS=5*/ ELINK_SS, /*DC=*/ ELINK_DC, /*RST=*/ ELINK_RESET);
GxEPD_Class display(io, /*RST=*/ ELINK_RESET, /*BUSY=*/ ELINK_BUSY);

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset 25200
int timezone = 7 * 3600;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", timezone);



// Adds a 10px offset to the y coordinates for setCursor
void setCursor(int x, int y, int ofst = 10) {
  display.setCursor(x, y + ofst);
}

// Create centered text (Only for default font) its kinda trash tbh
void centerText(String text, int fSize, int y, int offset = 0) {
  int x = (display.width() - ((text.length() * 6 - 1) * fSize)) / 2;
  display.setCursor(x + offset, y);
  display.setTextSize(fSize);
  display.print(text);
}

// Remove last character from a string
String popChar(String stringPop) {
  int lastIndex = stringPop.length() - 1;
  stringPop.remove(lastIndex);
  return stringPop;
}

// Rounds a Float to last digit
float roundone(float value) {
  return round(value*10)/10;
}

// This is basically like curl
String httpGETRequest(const String serverName) {
  HTTPClient http;

  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}"; 

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println("Retrying...");
    delay(500);
    // constantly retry until it gets it
    payload = httpGETRequest(serverName);
  }
  // Free resources
  http.end();

  return payload;
}
String jsonBuffer;
String sendGet;
String celcius = "ºC";
const size_t capacity = 57*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(8) + JSON_ARRAY_SIZE(48) + 11*JSON_OBJECT_SIZE(1) + 65*JSON_OBJECT_SIZE(4) + 8*JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(7) + 37*JSON_OBJECT_SIZE(12) + 11*JSON_OBJECT_SIZE(13) + JSON_OBJECT_SIZE(14) + 8*JSON_OBJECT_SIZE(15) + 1790;
DynamicJsonDocument doc(capacity);
//int retriesBeforeSleep = 0;





void setup() {
  // put your setup code here, to run once:

    Serial.begin(115200);
    delay(10);
    
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
//        retriesBeforeSleep += 1;
//        if (retriesBeforeSleep = 120) {
//          Serial.println("Failed to connect after like 120 seconds, so why even bother. retrying in 1200seconds");
//          esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1200);
//          esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
//          Serial.println("START SLEEPING!");
//          esp_deep_sleep_start();
//        }
        Serial.print(".");
        
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    while (timeClient.getEpochTime() < (timezone + 1000)) {
      timeClient.update();
      delay(100);
    }

//    delay(100);
    int epoc = timeClient.getEpochTime();
    delay(100);
    time_t t = epoc;
    Serial.println(epoc);
    Serial.println(timeClient.getFormattedTime());
    Serial.println(weeks[weekday(t) - 1]);

    jsonBuffer = httpGETRequest("https://api.openweathermap.org/data/2.5/onecall?lat=-7.104347&lon=111.626445&exclude=minutely&units=metric&appid=xxxxxxxxxxxxxxxxxxxxxxx");
    String debugURL = "https://xxxxxxxxxxxx.m.pipedream.net/batterywithLED?time=" + String(epoc);
    sendGet = httpGETRequest(debugURL);
    Serial.println(sendGet);
//    Serial.println(jsonBuffer);
    deserializeJson(doc, jsonBuffer);

    float cTemp = doc["current"]["temp"];
    float cFeelsLike = doc["current"]["feels_like"];
    String cWeather = doc["current"]["weather"][0]["main"];
    String cWeatherDesc = doc["current"]["weather"][0]["description"];
    String cIcon = doc["current"]["weather"][0]["icon"];
    float nhTemp = doc["hourly"][4]["temp"];
    String nhWeather = doc["hourly"][4]["weather"][0]["main"];
    float ndTempDay = doc["daily"][1]["temp"]["day"];
    float ndTempNight = doc["daily"][1]["temp"]["night"];
    float ndTempEve = doc["daily"][1]["temp"]["eve"];
    float ndTempMorn = doc["daily"][1]["temp"]["morn"];
    String ndWeather = doc["daily"][1]["weather"][0]["main"];
    float ndTempAvg = (ndTempDay + ndTempNight + ndTempEve + ndTempMorn) / 4;
    Serial.println(cTemp);
    Serial.println(cFeelsLike);
    Serial.println(cWeather);
    Serial.println(cIcon);
    Serial.println(nhTemp);
    Serial.println(nhWeather);
    Serial.println(ndTempAvg);
    Serial.println(ndWeather);


  




    
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
    display.init(); // enable diagnostic output on Serial
    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);



    // Draws Icons based on API output
    if (cIcon == "01n" || cIcon == "01d") {
      display.drawBitmap(112, 5, weather1, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "02n" || cIcon == "02d") {
      display.drawBitmap(112, 5, weather2, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "03n" || cIcon == "03d") {
      display.drawBitmap(112, 5, weather3, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "04n" || cIcon == "04d") {
      display.drawBitmap(112, 5, weather4, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "09n" || cIcon == "09d") {
      display.drawBitmap(112, 5, weather5, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "10n" || cIcon == "10d") {
      display.drawBitmap(112, 5, weather6, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "11n" || cIcon == "11d") {
      display.drawBitmap(112, 5, weather7, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "13n" || cIcon == "13d") {
      display.drawBitmap(112, 5, weather8, 50, 50, GxEPD_BLACK);
    } else if (cIcon == "50n" || cIcon == "50d") {
      display.drawBitmap(112, 5, weather9, 50, 50, GxEPD_BLACK);
    } else {
      display.drawBitmap(112, 5, weather1, 50, 50, GxEPD_BLACK);
    }


    

    int weatherOffset = 69;

    // Draws the main temp and weather data
    centerText(popChar(String(roundone(cTemp))),2 , 5, weatherOffset - 22 + 22);
    display.print((char)247);
    display.print("C");
    centerText(popChar(String(roundone(cFeelsLike))),1 , 26, weatherOffset - 20 + 22);
    display.print((char)247);
    display.print("C");
    centerText(cWeather, 2, 35, weatherOffset - 16 + 22);
//    centerText(String(year(t)),1 , 111, weatherOffset);



    int futureOffset = 0;

    // Draws the weather prediction data
    centerText("In 3 Hours",1 , 64, weatherOffset - 50 + futureOffset);
    centerText("Tomorrow",1 , 64, weatherOffset + 20 + futureOffset);
    
    centerText(String(int(round(nhTemp))),2 , 76, weatherOffset - 50 + futureOffset - 12);
    display.print((char)247);
    display.print("C");
    centerText(String(int(round(ndTempAvg))),2 , 76, weatherOffset + 20 + futureOffset - 12);
    display.print((char)247);
    display.print("C");

    centerText(nhWeather,1 , 95, weatherOffset - 50 + futureOffset);
    centerText(ndWeather,1 , 95, weatherOffset + 20 + futureOffset);
    

    
    // draws the main date and time
    int dateOffset = -69;
    centerText(weeks[weekday(t) - 1],3 , 3, dateOffset);
    centerText(months[month(t) - 1],2 , 93, dateOffset);
    centerText(String(year(t)),1 , 111, dateOffset);
    centerText(String(day(t)), 8 , 31, dateOffset);


    // Draws some small details with the smaller font
    display.setFont(&Picopixel);
    centerText("FeelsLike: ", 1, 24, weatherOffset + 25);
    centerText(cWeatherDesc, 1, 55, weatherOffset + 22);
    centerText("Last updated at " + timeClient.getFormattedTime(), 1, 118, 75);
    
    display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);

    display.update();

    Serial.println("SLEEPING!");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
    Serial.println("START SLEEPING!");
    esp_deep_sleep_start();
}

void loop() {
  // put your main code here, to run repeatedly:

}
