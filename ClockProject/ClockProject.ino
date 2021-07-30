#include <NTPClient.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS     -1
#define TFT_RST    12  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_DC     5  // define data/command pin

// !!! DANGER !!!
// Reading this code may cause headaches, depression, 
// PTSD and possibly other undocumented illness.
// CONTINUE WITH CAUTION!


// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
float p = 3.1415926;

const char *ssid     = "SSID";
const char *password = "WIFIPASSWORD";

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset 25200
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 25200, 60000);

// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

String weeks[7] = {"  Minggu  ", "  Senin  ", "  Selasa  ", "  Rabu  ", "  Kamis  ", "  Jumat  ", "  Sabtu  "};
String months[12] = {" Januari ", " Februari ", "   Maret   ", "  April  ", "  Mei  ", " Juni ", " Juli ", " Agustus ", " September ", " Oktober ", " November ", " Desember "};
void centerText(String text, int fSize, int y, int offset = 0) {
  int x = (tft.width() - ((text.length() * 6 - 1) * fSize)) / 2;
  tft.setCursor(x + offset, y);
  tft.setTextSize(fSize);
  tft.print(text);
}

void setup() {
  Serial.begin(115200);
  tft.init(240, 240, SPI_MODE2);
  tft.setRotation(2);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  tft.setTextWrap(false);


  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, 240, 45, ST77XX_YELLOW);


  timeClient.begin();
}

void loop() {
  if ( WiFi.status() != WL_CONNECTED ) {
    WiFi.reconnect();
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( ":" );
    }
  }
  timeClient.update();
  time_t t = timeClient.getEpochTime();
  
  
  String AMPM = "";
  if (hour(t) >= 12) {
    AMPM.concat("PM");
  } else {
    AMPM.concat("AM");
  }
  Serial.println(isPM());
  Serial.println(isAM());

  char buff[32];
  sprintf(buff, "%02d:%02d:%02d", hourFormat12(t), minute(t), second(t));
  Serial.println(buff);



  tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW); // draws the clock
  centerText(buff, 4, 9, -12);
  tft.setTextSize(2);
  tft.print(AMPM);


  if ( (weekday(t) == 7) || (weekday(t) == 1) ) { //Chooses the color depending on the day
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  } else if (weekday(t) == 6) {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  } else {
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  }
  centerText(weeks[weekday(t) - 1], 3, 54);


  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  centerText(String(day(t)), 9, 91);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  centerText(months[month(t) - 1], 3, 168);
  tft.setTextColor(0xF81F, ST77XX_BLACK);
  centerText(String(year(t)), 4, 201);


  delay(100);
}
