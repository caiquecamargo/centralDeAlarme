#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_DC            9     // Sainsmart RS/DC
#define TFT_RST           8     // Sainsmart RES
#define TFT_CS           10     // Sainsmart CS
static Adafruit_ST7735 TFT = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
#define TFTW            128     // screen width
#define TFTH            160     // screen height
#define TFTW2            64     // half screen width
#define TFTH2            80     // half screen height

//Facilita na definição das cores.
#define WHITE   ST7735_WHITE 
#define BLUE    ST7735_BLUE
#define RED     ST7735_RED
#define YELLOW  ST7735_YELLOW
#define GREEN   ST7735_GREEN
#define BLACK   ST7735_BLACK

//DIGITAL PORTS
//...Entradas dos alarmes
static int TEMPERATURA_EBILOCK = A0;
static int DISTRIBUICAO        = A2;
static int PORTA_BARRA_FUNDA   = A1;
static int FIBRA_LINHA07       = A5;
static int FIBRA_LINHA10_TORRE = A4;
static int FIBRA_LINHA10_CCO   = A3;

//...Botões
static int UP_BUTTON     = 7;
static int DOWN_BUTTON   = 5;
static int INIBE_BUTTON  = 3;
static int CONFIG_BUTTON = 6;

//..Saídas
static int BUZZER    = 12;
static int LED_RED   = 2;
static int LED_GREEN = 4;

//Variáveis de definição de posição para a interface gráfica.
static int POS_INICIAL_MAIN_Y   = 30;
static int POS_INICIAL_SETA_Y   = 28;
static int DIST                 = 22;
static int DIST_CONFIG          = 22;
static int POS_INICIAL_MAIN_X   = 10;
static int POS_INICIAL_CONFIG_X = 40;

//Variáveis de definição de entrada.
static int STATE_TEMPERATURA_EBILOCK = 0;
static int STATE_DISTRIBUICAO        = 1;
static int STATE_PORTA_BARRA_FUNDA   = 2;
static int STATE_FIBRA_LINHA07       = 3;
static int STATE_FIBRA_LINHA10_TORRE = 4;
static int STATE_FIBRA_LINHA10_CCO   = 5;

//Debounce para controle dos 800Hz das Fibras
int debounce_linha07     = 0;
int debounce_linha10c    = 0;
int debounce_linha10T    = 0;
int DEBOUNCE_SENSIBILITY = 2000;

//Array de estado das estradas e variáveis de controle de mudança dos estados.
int state[6];
int stateValor[] = {1, 3, 5, 7, 11, 13}; 
int stateChange;
int atualizaTela = -1;

//Variável suscetível a mudança por interrupção.
volatile boolean stateInibeButton  = false;

//Variáveis de mudança de estado do programa.
boolean stateConfig       = false;
boolean stateConfigChange = true;

void setup() {
  //Inicia o display LCD.
  TFT.initR(INITR_BLACKTAB);

  //Define os pinos de entrada dos alarmes
  pinMode(TEMPERATURA_EBILOCK,INPUT);
  pinMode(DISTRIBUICAO,       INPUT);
  pinMode(PORTA_BARRA_FUNDA,  INPUT);
  pinMode(FIBRA_LINHA07,      INPUT_PULLUP);
  pinMode(FIBRA_LINHA10_TORRE,INPUT_PULLUP);
  pinMode(FIBRA_LINHA10_CCO,  INPUT_PULLUP);

  //Define os pinos de entrada dos botões.
  pinMode(UP_BUTTON,     INPUT_PULLUP);
  pinMode(DOWN_BUTTON,   INPUT_PULLUP);
  pinMode(INIBE_BUTTON,  INPUT_PULLUP);
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);

  //Define as saídas dos leds de indicação e do buzzer.
  pinMode(BUZZER,    OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  //Teste de leds
  digitalWrite(BUZZER,    HIGH);
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(BUZZER,    LOW);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, LOW);

  state[STATE_TEMPERATURA_EBILOCK] = -1;

  //Define a interrupção para o botão de inibição do buzzer.
  attachInterrupt(digitalPinToInterrupt(INIBE_BUTTON),  verificaInibeButton,  LOW);

  //Função de tela de boot.
  bootScreen();
}

//Função que define as saídas para o estado de alarme, ativando o buzzer.
void soaAlarme(){
  digitalWrite(BUZZER,    HIGH);
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, LOW);
}

//Função que coloca o programa em estado normal.
void desligaAlarme(){
  digitalWrite(BUZZER,    LOW);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, HIGH);
}

//Função que mantém o programa em estado de alarme mas inibe o buzzer.
void inibeAlarme(){
  digitalWrite(BUZZER, LOW);
}

void restabeleceTotal(){
  for(int i = 0; i < 3; i++){
    digitalWrite(BUZZER, LOW);
    delay(300);
    digitalWrite(BUZZER, HIGH);
    delay(300);
  }
  digitalWrite(BUZZER, LOW);
  delay(300);
}

void restabeleceParcial(){
  for(int i = 0; i < 2; i++){
    digitalWrite(BUZZER, LOW);
    delay(300);
    digitalWrite(BUZZER, HIGH);
    delay(300);
  }
  digitalWrite(BUZZER, LOW);
  delay(300);
}

//Função que verifica cada uma das entradas e define o estado em que se encontram (1 - para normal, 0 - para alarmado).
void verificaPortas(){
  if(state[STATE_TEMPERATURA_EBILOCK] >= 0)
    state[STATE_TEMPERATURA_EBILOCK] = (digitalRead(TEMPERATURA_EBILOCK))?1:0;

  if (state[STATE_DISTRIBUICAO] >= 0)
    state[STATE_DISTRIBUICAO] = (digitalRead(DISTRIBUICAO))?0:1;

  if (state[STATE_PORTA_BARRA_FUNDA] >= 0)
    state[STATE_PORTA_BARRA_FUNDA] = (digitalRead(PORTA_BARRA_FUNDA))?1:0;

  if (state[STATE_FIBRA_LINHA07] >= 0){
    if (!digitalRead(FIBRA_LINHA07)) debounce_linha07 = 0;
    else debounce_linha07 += (debounce_linha07 > DEBOUNCE_SENSIBILITY)?0:1;
    state[STATE_FIBRA_LINHA07] = (debounce_linha07 > DEBOUNCE_SENSIBILITY)?0:1;
  }

  if (state[STATE_FIBRA_LINHA10_CCO] >= 0){
    if (!digitalRead(FIBRA_LINHA10_CCO)) debounce_linha10c = 0;
    else debounce_linha10c += (debounce_linha10c > DEBOUNCE_SENSIBILITY)?0:1;
    state[STATE_FIBRA_LINHA10_CCO] = (debounce_linha10c > DEBOUNCE_SENSIBILITY)?0:1;
  }

  if (state[STATE_FIBRA_LINHA10_TORRE] >= 0){
    if (!digitalRead(FIBRA_LINHA10_TORRE)) debounce_linha10T = 0;
    else debounce_linha10T += (debounce_linha10T > DEBOUNCE_SENSIBILITY)?0:1;
    state[STATE_FIBRA_LINHA10_TORRE] = (debounce_linha10T > DEBOUNCE_SENSIBILITY)?0:1;
  }
}

//Função acionada quando o botão de inibição do buzzer é pressionado.
void verificaInibeButton(){
  stateInibeButton = true;
}

//Verifica as mudanças de estado das entradas.
void verificaStateChange(){
  stateChange = 0;

  //Cada estado da entrada tem um valor único, devido a soma de números primos.
  for (int i = 0; i < 6; i++){
    if (state[i] == 0)
      stateChange += stateValor[i];
  }
}

//Verifica se o programa está em estado alarmado e aciona as saídas.
void verificaAlarme(){
  if (atualizaTela > stateChange){
    if (stateChange == 0) restabeleceTotal();
    else restabeleceParcial();
  }
  else if (stateChange != 0 && stateInibeButton == false)
    soaAlarme();
  else if (stateInibeButton == true) inibeAlarme();
  else desligaAlarme();
}

//Funação que controla as diferentes funções da tela de configurações do programa.
void controlConfigScreen(){
  int posConfig = 0;
  stateConfigChange = true;

  //Enquanto não apertado o botão de saída da tela de configurações o programa continua nela.
  while(stateConfig){
    //Verifica se o botão de subida do menu foi pressionado.
    if (!digitalRead(UP_BUTTON)){
      if (posConfig > 0){
        posConfig--;
        stateConfigChange = true;
      }
      delay(50);
    }

    //Verifica se o botão de descida do menu foi pressionado.
    if (!digitalRead(DOWN_BUTTON)){
      if (posConfig < 5){
        posConfig++;
        stateConfigChange = true;
      }
      delay(50);
    }

    //Verifica se o botão de configurações foi pressionado. Ao clicar a entrada selecionada fica desabilitada, não alarmando em caso de mudança de estado.
    if (!digitalRead(CONFIG_BUTTON)){
      state[posConfig] += 1;
      state[posConfig] *= -1;
      stateConfigChange = true;
      delay(50);
    }

    //Se houve alguma mudança de estado atualiza a tela.
    if (stateConfigChange){
      configScreen();
      drawSeta(posConfig);
    }

    //Se o botão de inibir o buzzer for pressionado o programa volta para a tela inicial.
    if (stateInibeButton){
       stateConfig = false;
    }
  }

  atualizaTela = -1;
}

//Desenha a seta na tela de configurações.
void drawSeta(int posConfig){
  TFT.fillRect(POS_INICIAL_MAIN_X, POS_INICIAL_SETA_Y + posConfig * DIST_CONFIG, 10, 10, WHITE);
  TFT.fillTriangle(POS_INICIAL_MAIN_X + 10,
                   POS_INICIAL_SETA_Y -  3 + posConfig * DIST_CONFIG,
                   POS_INICIAL_MAIN_X + 10,
                   POS_INICIAL_SETA_Y + 13 + posConfig * DIST_CONFIG,
                   POS_INICIAL_MAIN_X + 20,
                   POS_INICIAL_SETA_Y +  4 + posConfig * DIST_CONFIG,
                   WHITE);
}

//Desenha a tela de configurações.
void configScreen(){
  TFT.fillScreen(BLACK);

  TFT.drawRect(0, 0, 128, 160, WHITE);
  
  TFT.setTextColor(WHITE);
  TFT.setCursor(10, 7);
  TFT.setTextSize(1);
  TFT.println("CONFIGURACOES");
  TFT.drawFastHLine(8, 15, 113, WHITE);

  TFT.setTextColor((state[STATE_TEMPERATURA_EBILOCK] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y);
  TFT.setTextSize(1);
  TFT.println("Temp. EBILOCK");

  TFT.setTextColor((state[STATE_DISTRIBUICAO] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y + DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Distribuicao");

  TFT.setTextColor((state[STATE_PORTA_BARRA_FUNDA] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y + 2 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Porta BFU");

  TFT.setTextColor((state[STATE_FIBRA_LINHA07] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y + 3 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L7");

  TFT.setTextColor((state[STATE_FIBRA_LINHA10_TORRE] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y + 4 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 T");

  TFT.setTextColor((state[STATE_FIBRA_LINHA10_CCO] < 0)?RED:WHITE);
  TFT.setCursor(POS_INICIAL_CONFIG_X, POS_INICIAL_MAIN_Y + 5 * DIST_CONFIG);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 C");

  stateConfigChange = false;
  
  delay(200);
}

/*
 * Faz o desenho que designa o estado da entrada monitorada.
 * -> Desenha um "tick" VERDE se a entrada estiver OK. 
 * -> Desenha um X VERMELHO circunscrito se a entrada estiver alarmada.
 * -> Desenha um TRIÂNGULO COM UMA EXCLAMAÇÃO se a entrada estiver desativada.
 */
void drawContato(int posx, int posy, int estado){
  if(estado > 0){
    TFT.drawLine(113, posy + 2, 115, posy + 4, GREEN);
    TFT.drawLine(115, posy + 4, 120, posy - 1, GREEN);
  }
  else if (estado < 0){
    TFT.drawTriangle(111, posy + 7, 121, posy + 7, 116, posy - 2, YELLOW);
    TFT.drawFastVLine(116, posy, 4, YELLOW);
    TFT.drawPixel(116, posy + 5, YELLOW);
  }
  else{
    TFT.drawCircle(115, posy + 2, 5, RED);
    TFT.drawLine(111, posy - 2, 119, posy + 5, RED);
    TFT.drawLine(111, posy + 5, 119, posy - 2, RED);
  }

  int i = 0;
  while(posx + i * 2 < 110){
    TFT.drawPixel(posx + i * 2, posy + 7, WHITE);
    i++;
  }
}

//Desenha a tela inicial do programa.
void mainScreen() {
  TFT.fillScreen(BLACK);

  TFT.drawRect(0, 0, 128, 160, WHITE);
  
  TFT.setTextColor(WHITE);
  TFT.setCursor(10, 7);
  TFT.setTextSize(1);
  TFT.println("CENTRAL DE ALARMES");
  TFT.drawFastHLine(8, 15, 113, WHITE);
  
  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y);
  TFT.setTextSize(1);
  TFT.println("Temp. EBILOCK:");
  drawContato(94, POS_INICIAL_MAIN_Y, state[STATE_TEMPERATURA_EBILOCK]);
  
  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y + DIST);
  TFT.setTextSize(1);
  TFT.println("Distribuicao:");
  drawContato(88, POS_INICIAL_MAIN_Y + DIST, state[STATE_DISTRIBUICAO]);

  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y + 2 * DIST);
  TFT.setTextSize(1);
  TFT.println("Porta BFU:");
  drawContato(70, POS_INICIAL_MAIN_Y + 2 * DIST, state[STATE_PORTA_BARRA_FUNDA]);

  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y + 3 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L7:");
  drawContato(65, POS_INICIAL_MAIN_Y + 3 * DIST, state[STATE_FIBRA_LINHA07]);

  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y + 4 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 Torre:");
  drawContato(106, POS_INICIAL_MAIN_Y + 4 * DIST, state[STATE_FIBRA_LINHA10_TORRE]);

  TFT.setCursor(POS_INICIAL_MAIN_X, POS_INICIAL_MAIN_Y + 5 * DIST);
  TFT.setTextSize(1);
  TFT.println("Fibra L10 CCO:");
  drawContato(110, POS_INICIAL_MAIN_Y + 5 * DIST, state[STATE_FIBRA_LINHA10_CCO]);

  if(atualizaTela < stateChange || stateChange == 0) stateInibeButton = false;
  atualizaTela = stateChange;
  
  delay(200);
}

//Desenha a tela de boot do sistema.
void bootScreen(){
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

void loop() {
  if (!digitalRead(CONFIG_BUTTON)){
    stateConfig = true;
    delay(200);
  }
  
  if (!stateConfig){
    verificaPortas();
    verificaStateChange();
    verificaAlarme();
    if (atualizaTela != stateChange)
      mainScreen();
  }
  else{
    controlConfigScreen();
  }
  
  delay(50);
}
