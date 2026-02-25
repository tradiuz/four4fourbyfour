/*******************************************************************************
 * Start of Arduino_GFX setting
 *
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6, SCK: 13, MOSI: 11, MISO: 12
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22, SCK: 18, MOSI: 23, MISO: nil
 * ESP32-C2/3 various dev board: CS:  7, DC:  2, RST:  1, BL:  3, SCK:  4, MOSI:  6, MISO: nil
 * ESP32-C5 various dev board  : CS: 23, DC: 24, RST: 25, BL: 26, SCK: 10, MOSI:  8, MISO: nil
 * ESP32-C6 various dev board  : CS: 18, DC: 22, RST: 23, BL: 15, SCK: 21, MOSI: 19, MISO: nil
 * ESP32-H2 various dev board  : CS:  0, DC: 12, RST:  8, BL: 22, SCK: 10, MOSI: 25, MISO: nil
 * ESP32-P4 various dev board  : CS: 26, DC: 27, RST: 25, BL: 24, SCK: 36, MOSI: 32, MISO: nil
 * ESP32-S2 various dev board  : CS: 34, DC: 38, RST: 33, BL: 21, SCK: 36, MOSI: 35, MISO: nil
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48, SCK: 36, MOSI: 35, MISO: nil
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5, SCK: 14, MOSI: 13, MISO: 12
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28, SCK: 18, MOSI: 19, MISO: 16
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23, SCK: 19, MOSI: 21, MISO: 20
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3, SCK: 10, MOSI: 12, MISO: 11
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23, SCK: 13, MOSI: 11, MISO: 12
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13, SCK: 11, MOSI:  9, MISO: 10
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0, SCK:  8, MOSI: 10, MISO:  9
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22, SCK: 13, MOSI: 11, MISO: 12
 ******************************************************************************/
#include <Arduino_GFX_Library.h>
#include <string>

/*******************************************************************************
 *  Pin definitions
 ******************************************************************************/


#define GFX_BL 15  // PIN_LCD_BL

#define PIN_LCD_DC 16
#define PIN_LCD_CS 17
#define PIN_LCD_RST 20  // Also resets the TP

#define PIN_ADC_CS 5

#define PIN_CLK0 18
#define PIN_MOSI 19
#define PIN_MISO 4

#define PIN_SDA 12
#define PIN_SCL 13

#define PIN_TP_INT 29
#define PIN_IMU_INT 14
#define PIN_ENC_INT 2

#define PIN_OUT1 7
#define PIN_OUT2 9
#define PIN_OUT3 8
#define PIN_OUT4 22

#define CHAR_M 2
#define CHAR_H 12
#define CHAR_W 16

#define BOX_H 16

Arduino_DataBus *bus = new Arduino_RPiPicoSPI(PIN_LCD_DC /* DC */, PIN_LCD_CS /* CS */, PIN_CLK0 /* SCK */, PIN_MOSI /* MOSI */, -1 /* MISO */, spi0 /* spi */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 1 /* rotation */, true /* IPS */);

std::string box1_text = "0.1h";
std::string box2_text = "100h";
std::string box3_text = "1000h";
std::string box4_text = "10.0kh";

void renderbox(int box, char *content) {
  int row = 0;
  char buf[7];
  if (box > 3) { row = gfx->height() - BOX_H; };
  int currentCursorX = gfx->getCursorX();
  int currentCursorY = gfx->getCursorY();
  gfx->setCursor(int(gfx->width() * box / 4), row);
  sprintf(buf, "%6s", content);
  gfx->printf(buf);
  gfx->setCursor(currentCursorX, currentCursorY);
};

void setup(void) {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->setTextSize(CHAR_M);

  gfx->setTextColor(RGB565_ORANGE);
  gfx->setCursor(0, 0);

  gfx->startWrite();
  gfx->writeFastHLine(0, BOX_H, gfx->width(), random(0xffff));
  gfx->writeFastHLine(0, gfx->height() - BOX_H, gfx->width(), random(0xffff));
  gfx->writeFastVLine(int(gfx->width() / 4), 0, BOX_H, random(0xffff));
  gfx->writeFastVLine(int(gfx->width() / 2), 0, BOX_H, random(0xffff));
  gfx->writeFastVLine(int(gfx->width() * 3 / 4), 0, BOX_H, random(0xffff));
  gfx->endWrite();


  renderbox(0, &box1_text[0]);
  renderbox(1, &box2_text[0]);
  renderbox(2, &box3_text[0]);
  renderbox(3, &box4_text[0]);

  delay(1000);
}

int i = 1;
void loop() {

  delay(1000);
}
