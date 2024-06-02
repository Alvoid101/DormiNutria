#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Servo.h>

//Pulsadores
const int s1 = 12;
const int s2 = 8;
const int s3 = 7;
const int s4 = 4;
const int s5 = 2;

//Estado de las entradas
int state1 = 0;
int last1 = 1;

int state2 = 0;
int last2 = 1;

int state3 = 0;
int last3 = 1;

int state4 = 0;
int last4 = 1;

int state5 = 0;
int last5 = 1;

int state6 = 0;
int last6 = 1;

//Control canciones
byte iCancion = 1;
byte volumen = 10;

//Led RGB
#define LEDROJO  3    // pin 3 a anodo LED rojo
#define LEDVERDE  6    // pin 10 a anodo LED verde
#define LEDAZUL 5   // pin 11 a anodo LED azul

byte r = 255;
byte g = 228;
byte b = 1;

byte iColorLED = 0;


bool maximoBrillo = false;
byte intensidadLED = 0;

//Variables de tiempo
unsigned long tiempo;
unsigned long tiempoAcumulado = 0;

//Variables servo motor
byte giros = 180;
bool activarServo = false;

//Variables respiracion
bool respira = false;
#define servoPin 9
Servo servoMotor;

//Objetos repoductor
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
  //Comunicacion serie
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  //Comprobar si hay una SD conectada
  if (!myDFPlayer.begin(mySoftwareSerial)) {  
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }

  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.setTimeOut(500);

  //----Establecer olumen inicial----
  myDFPlayer.volume(volumen);  //Set volume value (0~30).

  //----Establecer ecualizacion----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);

  //----Establecer tipo de almacenamiento----
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  //Pines pulsadores
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);
  pinMode(s3, INPUT_PULLUP);
  pinMode(s4, INPUT_PULLUP);
  pinMode(s5, INPUT_PULLUP);

  //Pines LED RGB
  pinMode(LEDROJO, OUTPUT); // todos los pines como salida
  pinMode(LEDVERDE, OUTPUT);
  pinMode(LEDAZUL, OUTPUT);

  //Pines Servo motor
  servoMotor.attach(servoPin);
  Serial.println(servoMotor.read());
}
void loop()
{

  tiempo = millis();

  state1 = digitalRead(s1);
  state2 = digitalRead(s2);
  state3 = digitalRead(s3);
  state4 = digitalRead(s4);
  state5 = digitalRead(s5);

  respirar();

  if (state1 != last1) 
  { 
    
    if (state1 == LOW) 
    {

      if(volumen <= 26)
      {
        volumen += 2;
      }
      else
      {
        volumen = 0;
      }
      myDFPlayer.volume(volumen);
      
    }
  }
  
  last1  = state1;
  
  if (state2 != last2 ) 
  {
    if (state2 == LOW) 
    {
      reiniciarServo();
      activarServo = false;
      reproducirCanciones();
    }
  }
  
  last2  = state2;

  if (state3 != last3 ) 
  {
    if (state3 == LOW) 
    {
      respira = false;
      activarRespiracion();
    }
  }
  
  last3  = state3;

  if (state4 != last4 ) 
  {
    if (state4 == LOW) 
    {
      Serial.println("Pulsando 3");
      cambiarColorLED();
    }
  }
  
  last4  = state4;

  if (state5 != last5 ) 
  {
    
    if (state5 == LOW) 
    {
      activarDesactivarServo();
    }
  }
  
  last5  = state5;

}

//Metodos pulsadores
void reproducirCanciones()
{
  if (iCancion > 6)
  {
    iCancion = 1;
  }
  
  myDFPlayer.play(iCancion);
  myDFPlayer.enableLoop();

  iCancion ++;
}

void activarRespiracion() 
{
  activarServo = true;
  reiniciarServo();
  myDFPlayer.play(7);
  myDFPlayer.enableLoop();
  giros = 180;
  maximoBrillo = false;
  intensidadLED = 0;
  respira = true;
}

//Metodos accion actual del peluche
void respirar()
{
    aumentarIntensidadLED(11);//El tiempo para que se sincronice debe ser de 11ms
    disminuirIntensidadLED(11);
}

//Metodos LED
void aumentarIntensidadLED(int tiempo_aumento)
{

  if(tiempo - tiempoAcumulado>=tiempo_aumento && giros >= 0 && !maximoBrillo)
  {
    
    tiempoAcumulado = tiempo;
    mostrar_Color_Especifico(intensidadLED*(r/180), intensidadLED*(g/180), intensidadLED*(b/180));
    if(activarServo)
    {
    servoMotor.write(giros);
    }
    giros --;
    intensidadLED ++;
    //Serial.println(giros);
    maximoBrillo = giros == 0;
  }

}

void disminuirIntensidadLED(int tiempo_disminucion)
{
  if(tiempo - tiempoAcumulado>=tiempo_disminucion && giros <= 180 && maximoBrillo)
  {
    tiempoAcumulado = tiempo;
    mostrar_Color_Especifico(intensidadLED*(r/180), intensidadLED*(g/180), intensidadLED*(b/180));
    if(activarServo)
    {
    servoMotor.write(giros);
    }
    giros ++;
    intensidadLED --;
    //Serial.println(giros);
    maximoBrillo = !(giros == 180);
  }
}

void mostrar_Color_Especifico(int r, int g, int b) {
  analogWrite(LEDROJO, r);  
  analogWrite(LEDVERDE, g);
  analogWrite(LEDAZUL, b);

}


void cambiarColorLED()
{
    if(iColorLED == 0)
    {
      r = 255;
      g = 1;
      b = 251;
    }

    else if(iColorLED == 1)
    {
      r = 0;
      g = 255;
      b = 0;
    }

    else if(iColorLED == 2)
    {
      r = 1;
      g = 217;
      b = 255;
    }

    else if(iColorLED == 3)
    {
      r = 255;
      g = 228;
      b = 1;
    }

    else if(iColorLED == 4)
    {
      r = 255;
      g = 255;
      b = 255;
    }

    else
    {
      iColorLED = -1;
      
      r = 0;
      g = 0;
      b = 0;
    }

  iColorLED ++;
}

//Metodos servo motor
void reiniciarServo()
{
  byte anguloActual = servoMotor.read();
  while (anguloActual < 180){
    servoMotor.write(anguloActual);
    anguloActual = anguloActual + 1;
    delay(20);
  }
  giros = 180;
  maximoBrillo = false;
  intensidadLED = 0;
  respira = false;
}

void activarDesactivarServo()
{
  if(activarServo)
  {
    activarServo = false;
  }
  else if (!activarServo) 
  {
    activarServo = true;
    reiniciarServo();
  }
}
