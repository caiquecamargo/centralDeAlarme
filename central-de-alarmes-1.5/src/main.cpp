#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "screen.h"

// PORTS
//...Alarm inputs
static uint8_t TEMPERATURA_EBILOCK = A0;
static uint8_t DISTRIBUICAO        = A2;
static uint8_t PORTA_BARRA_FUNDA   = A1;
static uint8_t FIBRA_LINHA07       = A5;
static uint8_t FIBRA_LINHA10_TORRE = A4;
static uint8_t FIBRA_LINHA10_CCO   = A3;

//...Buttons
static uint8_t UP_BUTTON            = 7;
static uint8_t DOWN_BUTTON          = 5;
static uint8_t INHIBIT_ALARM_BUTTON = 3;
static uint8_t CONFIG_BUTTON        = 6;

//...Outputs
static uint8_t BUZZER    = 12;
static uint8_t LED_RED   = 2;
static uint8_t LED_GREEN = 4;

//Input states values
static uint8_t STATE_TEMPERATURA_EBILOCK = 0;
static uint8_t STATE_DISTRIBUICAO        = 1;
static uint8_t STATE_PORTA_BARRA_FUNDA   = 2;
static uint8_t STATE_FIBRA_LINHA07       = 3;
static uint8_t STATE_FIBRA_LINHA10_TORRE = 4;
static uint8_t STATE_FIBRA_LINHA10_CCO   = 5;

//Debounce to control 800Hz fiber input
boolean confirmStateL7   = false;
uint16_t debounceL7On    = 0;
uint16_t debounceL7Off   = 0;

boolean confirmStateL10c = false;
uint16_t debounceL10cOn  = 0;
uint16_t debounceL10cOff = 0;

boolean confirmStateL10t = false;
uint16_t debounceL10tOn  = 0;
uint16_t debounceL10tOff = 0;

static uint8_t  ANALOG_READ_LIMIAR       = 30;
static uint16_t DEBOUNCE_SENSIBILITY_ON  = 150;
static uint16_t DEBOUNCE_SENSIBILITY_OFF = 750;

//Variables to watch state changes
int8_t state[6];
uint8_t stateValue[] = {3, 5, 7, 11, 13, 17}; 
uint8_t currentState = 0;
int16_t previousState = -1;

//Variable susceptible to change by interruption
volatile boolean isAlarmInhibitButtonPressed  = false;

//Screen selection
static uint8_t MAIN_SCREEN = 0;
static uint8_t CONFIG_SCREEN = 1;
uint8_t selectedScreen = MAIN_SCREEN;

//Função acionada quando o botão de inibição do buzzer é pressionado.
void verifyInhibitAlarmButton(){
  isAlarmInhibitButtonPressed = true;
}

//Função que define as saídas para o estado de alarme, ativando o buzzer.
void triggerAlarm(){
  digitalWrite(BUZZER,    HIGH);
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, LOW);
}

//Função que coloca o programa em estado normal.
void turnOffAlarm(){
  digitalWrite(BUZZER,    LOW);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, HIGH);
}

//Função que mantém o programa em estado de alarme mas inibe o buzzer.
void inhibitAlarm(){
  digitalWrite(BUZZER, LOW);
}

void restoreAllAlarm(){
  for(int i = 0; i < 3; i++){
    digitalWrite(BUZZER, LOW);
    delay(300);
    digitalWrite(BUZZER, HIGH);
    delay(300);
  }
  digitalWrite(BUZZER, LOW);
  delay(300);
}

void restorePartialAlarm(){
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
void watchInputs(){
  if(state[STATE_TEMPERATURA_EBILOCK] >= 0)
    state[STATE_TEMPERATURA_EBILOCK] = (digitalRead(TEMPERATURA_EBILOCK))?1:0;

  if (state[STATE_DISTRIBUICAO] >= 0)
    state[STATE_DISTRIBUICAO] = (digitalRead(DISTRIBUICAO))?0:1;

  if (state[STATE_PORTA_BARRA_FUNDA] >= 0)
    state[STATE_PORTA_BARRA_FUNDA] = (digitalRead(PORTA_BARRA_FUNDA))?1:0;

  if (state[STATE_FIBRA_LINHA07] >= 0){
    uint16_t l7Value = analogRead(FIBRA_LINHA07);
    if (l7Value > ANALOG_READ_LIMIAR) {
      debounceL7On += 1;
    }
    if (l7Value < ANALOG_READ_LIMIAR) {
      debounceL7Off += 1;
    }
    if (debounceL7On > DEBOUNCE_SENSIBILITY_ON) {
      debounceL7On = 0;
      debounceL7Off = 0;
      if (confirmStateL7) state[STATE_FIBRA_LINHA07] = 1;
      confirmStateL7 = true;
    }
    if (debounceL7Off > DEBOUNCE_SENSIBILITY_OFF) {
      debounceL7On = 0;
      debounceL7Off = 0;
      if (!confirmStateL7) state[STATE_FIBRA_LINHA07] = 0;
      confirmStateL7 = false;
    }
  }

  if (state[STATE_FIBRA_LINHA10_CCO] >= 0){
    uint16_t l10cValue = analogRead(FIBRA_LINHA10_CCO);
    if (l10cValue > ANALOG_READ_LIMIAR) {
      debounceL10cOn += 1;
    }
    if (l10cValue < ANALOG_READ_LIMIAR) {
      debounceL10cOff += 1;
    }
    if (debounceL10cOn > DEBOUNCE_SENSIBILITY_ON) {
      debounceL10cOn = 0;
      debounceL10cOff = 0;
      if (confirmStateL10c) state[STATE_FIBRA_LINHA10_CCO] = 1;
      confirmStateL10c = true;
    }
    if (debounceL10cOff > DEBOUNCE_SENSIBILITY_OFF) {
      debounceL10cOn = 0;
      debounceL10cOff = 0;
      if (!confirmStateL10c) state[STATE_FIBRA_LINHA10_CCO] = 0;
      confirmStateL10c = false;
    }
  }

  if (state[STATE_FIBRA_LINHA10_TORRE] >= 0){
    uint16_t l10tValue = analogRead(FIBRA_LINHA10_TORRE);
    if (l10tValue > ANALOG_READ_LIMIAR) {
      debounceL10tOn += 1;
    }
    if (l10tValue < ANALOG_READ_LIMIAR) {
      debounceL10tOff += 1;
    }
    if (debounceL10tOn > DEBOUNCE_SENSIBILITY_ON) {
      debounceL10tOn = 0;
      debounceL10tOff = 0;
      if (confirmStateL10t) state[STATE_FIBRA_LINHA10_TORRE] = 1;
      confirmStateL10t = true;
    }
    if (debounceL10tOff > DEBOUNCE_SENSIBILITY_OFF) {
      debounceL10tOn = 0;
      debounceL10tOff = 0;
      if (!confirmStateL10t) state[STATE_FIBRA_LINHA10_TORRE] = 0;
      confirmStateL10t = false;
    }
  }
}

//Verifica as mudanças de estado das entradas.
void watchState(){
  currentState = 0;

  //Cada estado da entrada tem um valor único, devido a soma de números primos.
  for (int i = 0; i < 6; i++){
    if (state[i] == 0) {
      currentState += stateValue[i];
    }
  }
}

//Verifica se o programa está em estado alarmado e aciona as saídas.
void watchAlarms(){
  if (previousState > currentState){
    if (currentState == 0) restoreAllAlarm();
    else restorePartialAlarm();
  }
  else if (currentState != 0 && !isAlarmInhibitButtonPressed)
    triggerAlarm();
  else if (isAlarmInhibitButtonPressed) inhibitAlarm();
  else turnOffAlarm();
}

//Funação que controla as diferentes funções da tela de configurações do programa.
void configScreen(){
  uint16_t cursorPosition = 0;
  boolean hasToRefreshScreen = true;

  //Enquanto não apertado o botão de saída da tela de configurações o programa continua nela.
  while(selectedScreen == CONFIG_SCREEN){
    //Verifica se o botão de subida do menu foi pressionado.
    if (!digitalRead(UP_BUTTON) && cursorPosition > 0){
      cursorPosition--;
      hasToRefreshScreen = true;
      delay(50);
    }

    //Verifica se o botão de descida do menu foi pressionado.
    if (!digitalRead(DOWN_BUTTON) && cursorPosition < 5){
      cursorPosition++;
      hasToRefreshScreen = true;
      delay(50);
    }

    //Verifica se o botão de configurações foi pressionado. Ao clicar a entrada selecionada fica desabilitada, não alarmando em caso de mudança de estado.
    if (!digitalRead(CONFIG_BUTTON)){
      state[cursorPosition] = (state[cursorPosition] + 1) * -1;
      hasToRefreshScreen = true;
      delay(50);
    }

    //Se houve alguma mudança de estado atualiza a tela.
    if (hasToRefreshScreen){
      Screen::drawConfigScreen(
        (state[STATE_TEMPERATURA_EBILOCK] < 0)?RED:WHITE,
        (state[STATE_DISTRIBUICAO] < 0)?RED:WHITE,
        (state[STATE_PORTA_BARRA_FUNDA] < 0)?RED:WHITE,
        (state[STATE_FIBRA_LINHA07] < 0)?RED:WHITE,
        (state[STATE_FIBRA_LINHA10_TORRE] < 0)?RED:WHITE,
        (state[STATE_FIBRA_LINHA10_CCO] < 0)?RED:WHITE
      );

      hasToRefreshScreen = false;
      delay(200);

      Screen::drawArrow(cursorPosition);
    }

    //Se o botão de inibir o buzzer for pressionado o programa volta para a tela inicial.
    if (isAlarmInhibitButtonPressed){
       selectedScreen = MAIN_SCREEN;
    }
  }

  previousState = -1;
}

//Desenha a tela inicial do programa.
void mainScreen() {
  watchInputs();
  watchState();
  watchAlarms();
  if (previousState != currentState) {
    Screen::drawMainScreen(state);

    if(previousState < currentState || currentState == 0) isAlarmInhibitButtonPressed = false;
    previousState = currentState;
    
    delay(200);
  }
}

void setup() {
  //Init LCD display
  Screen::init();

  //Init digital ports
  pinMode(TEMPERATURA_EBILOCK,INPUT);
  pinMode(DISTRIBUICAO,       INPUT);
  pinMode(PORTA_BARRA_FUNDA,  INPUT);
  pinMode(FIBRA_LINHA07,      INPUT);
  pinMode(FIBRA_LINHA10_TORRE,INPUT);
  pinMode(FIBRA_LINHA10_CCO,  INPUT);

  //Init buttons
  pinMode(UP_BUTTON,     INPUT_PULLUP);
  pinMode(DOWN_BUTTON,   INPUT_PULLUP);
  pinMode(INHIBIT_ALARM_BUTTON,  INPUT_PULLUP);
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);

  //Init outputs
  pinMode(BUZZER,    OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  //Leds test
  digitalWrite(BUZZER,    HIGH);
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(BUZZER,    LOW);
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, LOW);

  //Define STATE_TEMPERATURA_EBILOCK as disabled
  state[STATE_TEMPERATURA_EBILOCK] = -1;

  //Define the interruption to INHIBIT_ALARM_BUTTON
  attachInterrupt(digitalPinToInterrupt(INHIBIT_ALARM_BUTTON),  verifyInhibitAlarmButton,  LOW);

  //Boot animation
  Screen::drawBootScreen();
}

void loop() {
  if (!digitalRead(CONFIG_BUTTON)){
    selectedScreen = CONFIG_SCREEN;
    delay(200);
  }
  
  selectedScreen == MAIN_SCREEN ? mainScreen() : configScreen();
  
  delay(50);
}