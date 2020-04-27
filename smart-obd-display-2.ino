//Für das Display
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

//PSRAM muss enabled sein!
//User-Setup geklärt?

//WIFI
#include <WiFi.h>
#include "ELMduino.h"

const char* ssid = "WiFi_OBDII";

//IP Adress of your ELM327 Dongle
IPAddress server(192, 168, 0, 10);
WiFiClient client;
ELM327 myELM327;

uint32_t rpm = 0;

//Generell
#include <elapsedMillis.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define TFT_GREY 0x5AEB

#define LOOP_PERIOD 100 // Display updates every 35 ms

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = 120, osy = 120; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update

int old_analog =  -999; // Value last displayed
int old_digital = -999; // Value last displayed

int value[3] = {0, 0, 0};
int old_value[3] = { -1, -1, -1};

void setup(void) {
  tft.init();
  tft.setRotation(0);
  Serial.begin(115200); // For debug
  tft.fillScreen(TFT_BLACK);
  startScreen();
  delay(10);
  pinMode(4,OUTPUT); digitalWrite(4,true);

  tft.drawString("Verbinde mit WLAN...",0,300,2);
  updateTime = millis(); // Next update time

  // Connecting to ELM327 WiFi
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  tft.drawString("IP:",0,300,2);
  tft.drawString(String(WiFi.localIP()),40,300,2);

  if (client.connect(server, 35000))
    tft.drawString("Mit Telnet verbunden.",0,300,2);
  else
  {
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("Keine Telnet-Verbindung!",0,300,2);
    while(1);
  }
  tft.drawString("Verbinde mit ELM...",0,300,2);
  myELM327.begin(client);
  
  tft.fillScreen(TFT_BLACK);
  analogMeter(); // Draw analogue meter
  // Draw 3 linear meters
  
  plotLinear("bar", 0, 160);
  plotLinear("A1", 80, 160);
  plotLinear("A2", 160, 160);
  
}


void loop() {
  
  if (updateTime <= millis()) {
    updateTime = millis() + LOOP_PERIOD;

    float tempRPM = myELM327.rpm();

    if (myELM327.status == ELM_SUCCESS)
    {
      rpm = (uint32_t)tempRPM;
      Serial.print("RPM: "); Serial.println(rpm);
      plotNeedle2(int(rpm));
    } else {
      printError();
    }     
    
  }
}


// #########################################################################
// Update needle position
// #########################################################################
void plotNeedle2(int value)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8]; dtostrf(value, 5, 0, buf);
  tft.drawRightString(buf, 50, 119 - 20, 2);  

  // Move the needle util new value reached
  
    old_analog = value; // Update immediately id delay is 0

    float sdeg = map(old_analog, 0, 6000, -150, -30); // Map value to angle
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(120 + 20 * ltx - 1, 140 - 20, osx - 1, osy, TFT_WHITE);
    tft.drawLine(120 + 20 * ltx, 140 - 20, osx, osy, TFT_WHITE);
    tft.drawLine(120 + 20 * ltx + 1, 140 - 20, osx + 1, osy, TFT_WHITE);

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("RPM", 120, 70, 4); // // Comment out to avoid font 4

    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * 98 + 120;
    osy = sy * 98 + 140;

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 + 20 * ltx - 1, 140 - 20, osx - 1, osy, TFT_RED);
    tft.drawLine(120 + 20 * ltx, 140 - 20, osx, osy, TFT_MAGENTA);
    tft.drawLine(120 + 20 * ltx + 1, 140 - 20, osx + 1, osy, TFT_RED);
  
}


// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter()
{
  // Meter outline
  tft.fillRect(0, 0, 239, 126, TFT_GREY);
  tft.fillRect(5, 3, 230, 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -60; i <= 60; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 140;
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 140;
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 140;

    // Yellow zone limits
    if (i >= 20 && i < 40) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    }

    // Green zone limits
    //if (i >= 0 && i < 25) {
    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    //}

    // Orange zone limits
    if (i >= 40 && i < 60) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Short scale tick length
    if (i % 20 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 140;
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 20 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 140;
      switch (i / 20) {
        case -3: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -2: tft.drawCentreString("1", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("2", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("3", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("4", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("5", x0, y0 - 12, 2); break;
        case 3: tft.drawCentreString("6", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 60) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.drawString("x1000", 5 + 230 - 40, 119 - 20, 2); // Units at bottom right
  tft.drawCentreString("RPM", 120, 70, 4); // Comment out to avoid font 4
  tft.drawRect(5, 3, 230, 119, TFT_BLACK); // Draw bezel line

  plotNeedle2(0); // Put meter needle at 0
}

// #########################################################################
//  Draw a linear meter on the screen
// #########################################################################
void plotLinear(char *label, int x, int y)
{
  int w = 36;
  tft.drawRect(x, y, w, 155, TFT_GREY);
  tft.fillRect(x + 2, y + 19, w - 3, 155 - 38, TFT_WHITE);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString(label, x + w / 2, y + 2, 2);

  for (int i = 0; i < 110; i += 10)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 6, TFT_BLACK);
  }

  for (int i = 0; i < 110; i += 50)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 9, TFT_BLACK);
  }

  tft.fillTriangle(x + 3, y + 127, x + 3 + 16, y + 127, x + 3, y + 127 - 5, TFT_RED);
  tft.fillTriangle(x + 3, y + 127, x + 3 + 16, y + 127, x + 3, y + 127 + 5, TFT_RED);

  tft.drawCentreString("---", x + w / 2, y + 155 - 18, 2);
}

// #########################################################################
//  Adjust 6 linear meter pointer positions
// #########################################################################
void plotPointer(void)
{
  int dy = 187;
  byte pw = 16;

  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  // Move the 3 pointers one pixel towards new value
  for (int i = 0; i < 3; i++)
  {
    char buf[8]; dtostrf(value[i], 4, 0, buf);
    tft.drawRightString(buf, i * 40 + 36 - 5, 187 - 27 + 155 - 18, 2);

    int dx = 3 + 40 * i;
    if (value[i] < 0) value[i] = 0; // Limit value to emulate needle end stops
    if (value[i] > 100) value[i] = 100;

    while (!(value[i] == old_value[i])) {
      dy = 187 + 100 - old_value[i];
      if (old_value[i] > value[i])
      {
        tft.drawLine(dx, dy - 5, dx + pw, dy, TFT_WHITE);
        old_value[i]--;
        tft.drawLine(dx, dy + 6, dx + pw, dy + 1, TFT_RED);
      }
      else
      {
        tft.drawLine(dx, dy + 5, dx + pw, dy, TFT_WHITE);
        old_value[i]++;
        tft.drawLine(dx, dy - 6, dx + pw, dy - 1, TFT_RED);
      }
    }
  }
}

void printError()
{
  Serial.print("Received: ");
  for (byte i = 0; i < PAYLOAD_LEN; i++)
    Serial.write(myELM327.payload[i]);
  Serial.println();
  
  if (myELM327.status == ELM_SUCCESS)
    Serial.println(F("\tELM_SUCCESS"));
  else if (myELM327.status == ELM_NO_RESPONSE)
    Serial.println(F("\tERROR: ELM_NO_RESPONSE"));
  else if (myELM327.status == ELM_BUFFER_OVERFLOW)
    Serial.println(F("\tERROR: ELM_BUFFER_OVERFLOW"));
  else if (myELM327.status == ELM_UNABLE_TO_CONNECT)
    Serial.println(F("\tERROR: ELM_UNABLE_TO_CONNECT"));
  else if (myELM327.status == ELM_NO_DATA)
    Serial.println(F("\tERROR: ELM_NO_DATA"));
  else if (myELM327.status == ELM_STOPPED)
    Serial.println(F("\tERROR: ELM_STOPPED"));
  else if (myELM327.status == ELM_TIMEOUT)
    Serial.println(F("\tERROR: ELM_TIMEOUT"));
  else if (myELM327.status == ELM_TIMEOUT)
    Serial.println(F("\tERROR: ELM_GENERAL_ERROR"));

  delay(100);
}

void startScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("smart", 120, 140, 4);
  tft.setTextSize(1);
  tft.drawCentreString("OBD Display Version 2",120,180,2);

}
