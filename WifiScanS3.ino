#include "WiFi.h"
#include "TFT_eSPI.h"

int APP_FONTSIZE = 1;  // uses either 1 or 2 to display 10 or 5 APs per page

int APP_BORDER = APP_FONTSIZE * 4;
int APP_LINE = APP_FONTSIZE * 8;
int APP_TITELSIZE = APP_LINE + APP_BORDER;
int APP_BOX = APP_LINE * 3;  // app uses 3 lines per AP element rectangle
int MAX_ELEM = 10;

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);

// what view to display and if update is needed (==button pressed)
int view = 0;
bool update_view = false;

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

//variables to keep track of the timing of recent scan run
unsigned long scan_time = 0;
unsigned long last_scan_time = 0;

// callback for right button press
void IRAM_ATTR isr() {
  button_time = millis();
  if (button_time - last_button_time > 250) {
    view++;
    if (view > 3) view = 0;
    update_view = true;
    last_button_time = button_time;
  }
}

void setup() {
  // Use right button to iterate through pages
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(14, isr, FALLING);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // initialize screen
  lcd.init();
  lcd.setRotation(0);
  lcd.fillScreen(TFT_BLACK);

  sprite.createSprite(TFT_WIDTH, TFT_HEIGHT);
  sprite.setTextDatum(TL_DATUM);

  sprite.setTextColor(TFT_BLACK, TFT_GREEN);
  sprite.fillRoundRect(0, 0, TFT_WIDTH, APP_LINE, 3, TFT_GREEN);
  sprite.drawString("Wifi Scanner", 48, 0, APP_FONTSIZE);
  sprite.fillRoundRect(0, TFT_HEIGHT - APP_LINE, TFT_WIDTH, APP_LINE, 3, TFT_GREEN);

  sprite.pushSprite(0, 0);
  sprite.setTextColor(TFT_BLACK, TFT_WHITE);
}

void loop() {
  if (update_view) {
    if (view == 0) {
      APP_FONTSIZE = 1;
      APP_BORDER = APP_FONTSIZE * 4;
      APP_LINE = APP_FONTSIZE * 8;
      APP_TITELSIZE = APP_LINE + APP_BORDER;
      APP_BOX = APP_LINE * 3;  // app uses 3 lines per AP element rectangle
      MAX_ELEM = 10;
    }
    if (view == 1) {
      APP_FONTSIZE = 2;
      APP_BORDER = APP_FONTSIZE * 4;
      APP_LINE = APP_FONTSIZE * 8;
      APP_TITELSIZE = APP_LINE + APP_BORDER;
      APP_BOX = APP_LINE * 3;  // app uses 3 lines per AP element rectangle
      MAX_ELEM = 5;
    }
    if (view == 2) {
      APP_FONTSIZE = 1;
      APP_BORDER = APP_FONTSIZE * 4;
      APP_LINE = APP_FONTSIZE * 8;
      APP_TITELSIZE = APP_LINE + APP_BORDER;
      APP_BOX = APP_LINE;  // app uses 1 line per AP element rectangle
      MAX_ELEM = 30;
    }

    if (view == 3) {
      APP_FONTSIZE = 2;
      APP_BORDER = APP_FONTSIZE * 4;
      APP_LINE = APP_FONTSIZE * 8;
      APP_TITELSIZE = APP_LINE + APP_BORDER;
      APP_BOX = APP_LINE;  // app uses 1 line per AP element rectangle
      MAX_ELEM = 15;
    }
    //re-draw green header and footer
    sprite.fillRoundRect(0, 0, TFT_WIDTH, TFT_HEIGHT, 3, TFT_BLACK);
    sprite.setTextColor(TFT_BLACK, TFT_GREEN);
    sprite.fillRoundRect(0, 0, TFT_WIDTH, APP_LINE, 3, TFT_GREEN);
    sprite.drawString("Wifi Scanner", 48, 0, APP_FONTSIZE);
    sprite.fillRoundRect(0, TFT_HEIGHT - APP_LINE, TFT_WIDTH, APP_LINE, 3, TFT_GREEN);
    sprite.drawString("Net: " + String(WiFi.scanComplete()) + "  ", 4, TFT_HEIGHT - APP_LINE, APP_FONTSIZE);  // shows counter of all discovered networks
    sprite.drawString("View [ " + String(view) + " ]", TFT_WIDTH - 4 - 32 - 32, TFT_HEIGHT - APP_LINE, APP_FONTSIZE);
    sprite.setTextColor(TFT_BLACK, TFT_WHITE);
    sprite.pushSprite(0, 0);
    update_view = false;
  }

  // this initiates a new scan every 5s, if there is no scan running (it should not)
  scan_time = millis();
  if (scan_time - last_scan_time > 5000) {
    if (WiFi.scanComplete() == -2) {
      WiFi.scanNetworks(true, true);
    }
    last_scan_time = scan_time;
  }
  int n = WiFi.scanComplete();

  if (n >= 0) {
    sprite.setTextColor(TFT_BLACK, TFT_GREEN);
    sprite.drawString("Net: " + String(n) + "  ", 4, TFT_HEIGHT - APP_LINE, APP_FONTSIZE);  // shows counter of all discovered networks
    //sprite.drawString("View [ " + String(view) + " ]", TFT_WIDTH - 4 - 32 - 32, TFT_HEIGHT - APP_LINE, APP_FONTSIZE);

    sprite.setTextColor(TFT_BLACK, TFT_WHITE);

    for (int i = 0; i < n || i < MAX_ELEM; ++i) {
      sprite.fillRoundRect(0, APP_TITELSIZE + i * (APP_BORDER + APP_BOX), TFT_WIDTH, APP_BOX, 3, TFT_WHITE);                                                                                                                                             // draw white box
      sprite.drawString(WiFi.SSID(i).c_str(), 4, APP_TITELSIZE + i * (APP_BORDER + APP_BOX), APP_FONTSIZE);                                                                                                                                              // first line is SSID
      if (view < 2) sprite.drawString(WiFi.BSSIDstr(i), 4, APP_TITELSIZE + i * (APP_BORDER + APP_BOX) + APP_LINE, APP_FONTSIZE);                                                                                                                         // second line is BSSID and needs to have a y-axis offset of APP_LINE
      if (view < 2) sprite.drawString("RSSI: " + String(WiFi.RSSI(i)) + " db | Ch: " + String(WiFi.channel(i)) + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : " *"), 4, APP_TITELSIZE + i * (APP_BORDER + APP_BOX) + APP_LINE * 2, APP_FONTSIZE);  //third line is RSSI and Channel. Has a y-axis offset of 2*APP_LINE
      delay(10);
    }

    sprite.fillRect(0, APP_LINE + n * (APP_BORDER + APP_BOX), TFT_WIDTH, TFT_HEIGHT - n * (APP_BORDER + APP_BOX) - APP_LINE * 2, TFT_BLACK);
    sprite.pushSprite(0, 0);
    WiFi.scanDelete();
  }
}
