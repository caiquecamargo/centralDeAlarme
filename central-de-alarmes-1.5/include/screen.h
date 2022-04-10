#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define WHITE   ST7735_WHITE 
#define BLUE    ST7735_BLUE
#define RED     ST7735_RED
#define YELLOW  ST7735_YELLOW
#define GREEN   ST7735_GREEN
#define BLACK   ST7735_BLACK

class Screen {
  public:
    static void init();
    static void drawBootScreen();
    static void drawMainScreen(int8_t* state);
    static void drawConfigScreen(
      uint16_t tempEbilockColor,
      uint16_t distributionColor,
      uint16_t bfuDoorColor,
      uint16_t l7FiberColor,
      uint16_t l10tFiberColor,
      uint16_t l10cFiberColor
    );
    static void drawArrow(uint16_t pos);
    static void drawStateIndicator(uint16_t posx, uint16_t posy, int16_t state);
};