#include <Arduino_GFX_Library.h>
#include <string>
#include <RP2040_Slow_PWM.h>


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

/*******************************************************************************
 *  PWM Settings
 ******************************************************************************/

#define HW_TIMER_INTERVAL_US 20L
#define USING_PWM_FREQUENCY  true
#define NUMBER_ISR_PWMS 4

/*******************************************************************************
 *  Display Helpers
 ******************************************************************************/

#define CHAR_M 2
#define CHAR_H 12  // CHAR_M*6
#define CHAR_W 16  // CHAR_M*8

#define BOX_H 20

/*******************************************************************************
 *  Slow PWM Config for LFOs
 ******************************************************************************/

// Init RPI_PICO_Timer
RP2040_Timer ITimer(0);

// Init RP2040_Slow_PWM, each can service 16 different ISR-based PWM channels
RP2040_Slow_PWM ISR_PWM;

bool TimerHandler(struct repeating_timer *t) {
  (void)t;

  ISR_PWM.run();

  return true;
}

#define CLOCK_PPQ 24

uint16_t theme_bg = RGB565_BLACK,
         theme_line = RGB565_BLUE,
         theme_main = RGB565_BLUE,
         theme_alt = RGB565_LIME,
         theme_text = RGB565_ORANGE;
volatile uint16_t theme_beat = RGB565_BLACK;

volatile uint8_t tick=0;
volatile bool beat;
void tick_beat(){
  tick = (tick + 1) % CLOCK_PPQ;
  if (tick == 0) {
    beat = true;
  }
};

typedef struct {
  uint8_t pin;
  float frequency;
  bool clock;
  void (*callback)();
} Oscillator;

Oscillator osc[]={
  { PIN_OUT1, 1.0f, false},
  { PIN_OUT2, 10.0f, false},
  { PIN_OUT3, 100.0f, false},
  { PIN_OUT4, float(60.0f * CLOCK_PPQ) / 240.0f , true, tick_beat}
};


/*******************************************************************************
 *  Display Setup
 ******************************************************************************/


Arduino_DataBus *bus = new Arduino_RPiPicoSPI(PIN_LCD_DC /* DC */, PIN_LCD_CS /* CS */, PIN_CLK0 /* SCK */, PIN_MOSI /* MOSI */, -1 /* MISO */, spi0 /* spi */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 1 /* rotation */, true /* IPS */);



void renderbox(uint8_t col, int8_t row, float value, std::string unit) {
  uint16_t col_x, row_y;
  char buf[6];
  if (unit == "hz") {
    if (value > 1000) {
      value = value / 1000;
      unit = "khz";
    } else {
      unit = " hz";
    }
    snprintf(buf, 6, " %*f", 4, value);
  } else {
    snprintf(buf, 6, " %*.0f", 4, unit == "bpm" ? (value * 240.0f) / float(CLOCK_PPQ) : value);
  }
  col_x = gfx->width() * col / 4;
  if (row >= 0) {
    row_y = 2 + (BOX_H * row);
  } else {
    row_y = gfx->height() + (BOX_H * row) - 2;
  }
  uint16_t currentCursorX = gfx->getCursorX();
  uint16_t currentCursorY = gfx->getCursorY();
  gfx->setCursor(col_x, row_y);
  gfx->printf(buf);
  gfx->setCursor(gfx->getCursorX(), gfx->getCursorY() + (CHAR_H / CHAR_M));
  gfx->setTextSize(1);
  gfx->printf(&unit[0]);
  gfx->setTextSize(CHAR_M);
  gfx->setCursor(currentCursorX, currentCursorY);
};

#include "math.h"

void writePolygon(int16_t x, int16_t y, int16_t r, uint8_t f, uint16_t d, uint16_t color)
{ 
  int px[f],py[f];
  for (int i = 0; i < f; i++) {
        double a = (2 * PI / f) * i + (d * PI / 180);
        Serial.printf("Angle: %f [%d degrees]\n",a, int(a*180/PI));
        px[i] = x + r * cos(a);
        py[i] = y + r * sin(a);
    }
    for (int i = 0; i< f - 1; i++){
      gfx->writeSlashLine(px[i], py[i], px[i+1], py[i+1], color);
    }
      gfx->writeSlashLine(px[f-1], py[f-1], px[0], py[0], color);
}


void drawPolygon(int16_t x, int16_t y, int16_t r, uint8_t f, uint16_t d, uint16_t color)
{ 
    gfx->startWrite();
    writePolygon(x, y, r, f, d, color);
    gfx->endWrite();
}

uint8_t box_c = 0;

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
  gfx->fillScreen(theme_bg);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->setTextSize(CHAR_M);

  gfx->setTextColor(theme_text, theme_bg);
  gfx->setCursor(0, 0);

  gfx->startWrite();

  gfx->writeFastVLine(int(gfx->width() / 4) -1, 0, gfx->height(), theme_line);
  gfx->writeFastVLine(int(gfx->width() / 2) -1, 0, gfx->height(), theme_line);
  gfx->writeFastVLine(int(gfx->width() * 3 / 4) -1, 0, gfx->height(), theme_line);
  gfx->writeFastHLine(0, BOX_H, gfx->width(), theme_line);
  gfx->writeFastHLine(0, BOX_H * 2, gfx->width(), theme_line);
  gfx->writeFastHLine(0, gfx->height() - BOX_H, gfx->width(), theme_line);
  gfx->writeFillRectPreclipped(0, BOX_H * 2 + 1, gfx->width(), gfx->height() - (BOX_H * 3) - 2, theme_bg);
  gfx->endWrite();

  box_c = 0;
  for (uint8_t index = 0; index < 4; index++) {
    ISR_PWM.setPWM(osc[index].pin, osc[index].frequency , 50, osc[index].clock ? osc[index].callback : nullptr);
    renderbox(box_c % 4, int(box_c / 4), osc[index].frequency, osc[index].clock ? "bpm" : "hz");
    box_c++;
  }

  renderbox(box_c % 4, int(box_c / 4), -20, "db");
  box_c++;

  delay(1000);



  if (!ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler)) {
    Serial.println("SLOW PWM startup failed!");
  }
}

typedef struct {
    uint16_t x; // Center X
    uint16_t y; // Center Y
    uint16_t r;  // Radius
    uint8_t f;  // Faces
    uint16_t d; // Rotation (Degrees)
} Polygon;

Polygon poly[3];

void loop() {
  box_c = 0;
  gfx->startWrite();
  for (int gon = 0; gon < 3; gon++){
    writePolygon( poly[gon].x, poly[gon].y, poly[gon].r, poly[gon].f, poly[gon].d , theme_bg);
    poly[gon] = {int(gfx->width() * (gon+1)/4), int(gfx->height()/2), 30, gon+3, (poly[gon].d + 10) % 360};
    writePolygon( poly[gon].x, poly[gon].y, poly[gon].r, poly[gon].f, poly[gon].d , theme_line);
  }
  if(beat){
    theme_beat = theme_beat == theme_alt? theme_main : theme_alt;
    gfx->writeFillRect(gfx->width() - 5 , BOX_H + 2, 3 , BOX_H - 4, theme_beat);
    beat = false;
  }
  for (uint8_t index = 0; index < 4; index++) {
    ISR_PWM.setPWM(osc[index].pin, osc[index].frequency , 50, osc[index].clock ? osc[index].callback : nullptr);
    renderbox(box_c % 4, int(box_c / 4), osc[index].frequency, osc[index].clock ? "bpm" : "hz");
    box_c++;
  }
  gfx->endWrite();
  delay(20);
}

/*
void nothin(){
  box_c = 0;

  for (uint8_t index = 0; index < NUMBER_LFOS; index++) {

    LFO_Freq[index] = (float(random(2000)) / 100.0);
    Serial.printf("LFO: %d : %5.1f hz\n", index + 1, LFO_Freq[index]);
    ISR_PWM.modifyPWMChannel_Period(index, LFO_Pin[index], 1000 / LFO_Freq[index], 50);
    renderbox(box_c % 4, int(box_c / 4), LFO_Freq[index], "hz");
    box_c++;
  }
  for (uint8_t index = 0; index < NUMBER_CLOCKS; index++) {
    CLOCK_BPM[index] = 40 + random(200);
    Serial.printf("Clock: %d : %5.1f bpm\n", index + 1, CLOCK_BPM[index]);
    ISR_PWM.modifyPWMChannel_Period(index + NUMBER_LFOS -1, CLOCK_Pin[index], 60000 / CLOCK_BPM[index] / CLOCK_PPQ, 50);
    renderbox(box_c % 4, int(box_c / 4), CLOCK_BPM[index], "bpm");
    box_c++;
  }
  delay(6000);
}
*/