#include "screen.h"

#define TFT_DC            9     // Sainsmart RS/DC
#define TFT_RST           8     // Sainsmart RES
#define TFT_CS           10     // Sainsmart CS
static Adafruit_ST7735 TFT = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
#define TFTW            128     // screen width
#define TFTH            160     // screen height
#define TFTW2            64     // half screen width
#define TFTH2            80     // half screen height

static uint8_t INIT_POS_MAIN_Y   = 30;
static uint8_t INIT_POS_MAIN_X   = 10;
static uint8_t INIT_POS_ARROW_Y  = 28;
static uint8_t DIST              = 22;
static uint8_t DIST_CONFIG       = 22;
static uint8_t INIT_POS_CONFIG_X = 40;

static uint8_t tempEbilockState = 0;
static uint8_t distributionState = 1;
static uint8_t bfuDoorState = 2;
static uint8_t l7FiberState = 3;
static uint8_t l10tFiberState = 4;
static uint8_t l10cFiberState = 5;

void Screen::init () {
  TFT.initR(INITR_BLACKTAB);
}

void Screen::drawBootScreen () {
  TFT.fillScreen(BLACK);
  
  TFT.drawRect(0, 0, 128, 160, WHITE);
  TFT.fillRect(10, TFTH2 - 20, TFTW - 20, 1, WHITE);
  TFT.fillRect(10, TFTH2 + 32, TFTW - 20, 1, WHITE);
  TFT.setTextColor(WHITE);
  TFT.setTextSize(3);
  TFT.setCursor( TFTW2 - (6 * 9), TFTH2 - 16);
  TFT.println("  NX  ");
  TFT.setTextSize(3);
  TFT.setCursor( TFTW2 - (6 * 9) + 9, TFTH2 + 8);
  TFT.println(" LUZ");
  TFT.setTextSize(0);
  TFT.setCursor( 10, TFTH2 - 28);
  TFT.println("Eq. de Sinalizacao");
  TFT.setCursor( TFTW2 - (12 * 3) - 15, TFTH2 + 34);
  TFT.println("Central de Alarmes");
  delay(3000);
}

void Screen::drawMainScreen (int8_t* state) {
  TFT.fillScreen(BLACK);

  TFT.drawRect(0, 0, 128, 160, WHITE);
  
  TFT.setTextColor(WHITE);
  TFT.setCursor(10, 7);
  TFT.setTextSize(1);
  TFT.println("CENTRAL DE ALARMES");
  TFT.drawFastHLine(8, 15, 113, WHITE);
  
  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y);
  TFT.setTextSize(1);
  TFT.println("Temp. EBILOCK:");
  drawStateIndicator(94, INIT_POS_MAIN_Y, state[tempEbilockState]);
  
  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y + DIST);
  TFT.setTextSize(1);
  TFT.println("Distribuicao:");
  drawStateIndicator(88, INIT_POS_MAIN_Y + DIST, state[distributionState]);

  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y + 2 * DIST);
  TFT.setTextSize(1);
  TFT.println("Porta BFU:");
  drawStateIndicator(70, INIT_POS_MAIN_Y + 2 * DIST, state[bfuDoorState]);

  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y + 3 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L7:");
  drawStateIndicator(65, INIT_POS_MAIN_Y + 3 * DIST, state[l7FiberState]);

  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y + 4 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 Torre:");
  drawStateIndicator(106, INIT_POS_MAIN_Y + 4 * DIST, state[l10tFiberState]);

  TFT.setCursor(INIT_POS_MAIN_X, INIT_POS_MAIN_Y + 5 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 CCO:");
  drawStateIndicator(110, INIT_POS_MAIN_Y + 5 * DIST, state[l10cFiberState]);
}

void Screen::drawConfigScreen (
  uint16_t tempEbilockColor,
  uint16_t distributionColor,
  uint16_t bfuDoorColor,
  uint16_t l7FiberColor,
  uint16_t l10tFiberColor,
  uint16_t l10cFiberColor
) {
  TFT.fillScreen(BLACK);

  TFT.drawRect(0, 0, 128, 160, WHITE);
  
  TFT.setTextColor(WHITE);
  TFT.setCursor(10, 7);
  TFT.setTextSize(1);
  TFT.println("CONFIGURACOES");
  TFT.drawFastHLine(8, 15, 113, WHITE);

  TFT.setTextColor(tempEbilockColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y);
  TFT.setTextSize(1);
  TFT.println("Temp. EBILOCK");

  TFT.setTextColor(distributionColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y + DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Distribuicao");

  TFT.setTextColor(bfuDoorColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y + 2 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Porta BFU");

  TFT.setTextColor(l7FiberColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y + 3 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L7");

  TFT.setTextColor(l10tFiberColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y + 4 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 T");

  TFT.setTextColor(l10cFiberColor);
  TFT.setCursor(INIT_POS_CONFIG_X, INIT_POS_MAIN_Y + 5 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 C");
}

void Screen::drawArrow (uint16_t pos) {
  TFT.fillRect(INIT_POS_MAIN_X, INIT_POS_ARROW_Y + pos * DIST_CONFIG, 10, 10, WHITE);
  TFT.fillTriangle(INIT_POS_MAIN_X + 10,
                   INIT_POS_ARROW_Y -  3 + pos * DIST_CONFIG,
                   INIT_POS_MAIN_X + 10,
                   INIT_POS_ARROW_Y + 13 + pos * DIST_CONFIG,
                   INIT_POS_MAIN_X + 20,
                   INIT_POS_ARROW_Y +  4 + pos * DIST_CONFIG,
                   WHITE);
}

/*
 * Faz o desenho que designa o estado da entrada monitorada.
 * -> Desenha um "tick" VERDE se a entrada estiver OK. 
 * -> Desenha um X VERMELHO circunscrito se a entrada estiver alarmada.
 * -> Desenha um TRIÂNGULO COM UMA EXCLAMAÇÃO se a entrada estiver desativada.
 */
void Screen::drawStateIndicator (uint16_t posx, uint16_t posy, int16_t state) {
  if(state > 0){
    TFT.drawLine(113, posy + 2, 115, posy + 4, GREEN);
    TFT.drawLine(115, posy + 4, 120, posy - 1, GREEN);
  }
  else if (state < 0){
    TFT.drawTriangle(111, posy + 7, 121, posy + 7, 116, posy - 2, YELLOW);
    TFT.drawFastVLine(116, posy, 4, YELLOW);
    TFT.drawPixel(116, posy + 5, YELLOW);
  }
  else{
    TFT.drawCircle(115, posy + 2, 5, RED);
    TFT.drawLine(111, posy - 2, 119, posy + 5, RED);
    TFT.drawLine(111, posy + 5, 119, posy - 2, RED);
  }

  uint8_t i = 0;
  while(posx + i * 2 < 110){
    TFT.drawPixel(posx + i * 2, posy + 7, WHITE);
    i++;
  }
}