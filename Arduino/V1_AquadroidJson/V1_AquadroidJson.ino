/*
Sergio Mokshin
Automação Live - Jan /2015
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "Wire.h"
#include <OneWire.h> // Importar biblioteca
#include <Servo.h>

OneWire ds(2);  // pin 10 (with 4.7K resistor)
#define DS1307_I2C_ADDRESS 0x68

Servo myservo;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xA6, 0x09 };
byte ip[] = { 192, 168, 0, 201 };
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 };
EthernetServer server(80);

#define PIN_RED 3
#define PIN_GREEN 6
#define PIN_BLUE 5
#define PIN_FEEDER 9
#define PIN_SAIDA_BUZZ 8


int MemSaida1 = 1; //Endereço de memoria com conteudo ultimo comando enviado S1
int MemSaida2 = 2; //Endereço de memoria com conteudo ultimo comando enviado S2
int MemSaida3 = 3; //Endereço de memoria com conteudo ultimo comando enviado S3
int MemSaida4 = 4; //Endereço de memoria com conteudo ultimo comando enviado S4
int MemAuto   = 5; //Endereço de memoria com conteudo ultimo comando enviado Módulo Automatico

//Saidas 1 e 2 não permitem agendamento de horario (Bomba e termostato)
int MemSaida3HrI = 6; //Endereço de memoria com conteudo inicio horario Saida 3
int MemSaida3HrF = 7; //Endereço de memoria com conteudo fim horario Saida 3
int MemSaida4HrI = 8; //Endereço de memoria com conteudo inicio horario Saida 4
int MemSaida4HrF = 9; //Endereço de memoria com conteudo fim horario Saida 4
int MemRGBWHITEHrI = 10; //Endereço de memoria com conteudo inicio horario RGB com cor Branca
int MemRGBWHITEHrF = 11; //Endereço de memoria com conteudo fim horario RGB com cor Branca
int MemRGBBLUEHrI = 12; //Endereço de memoria com conteudo inicio horario RGB com cor Azul
int MemRGBBLUEHrF = 13; //Endereço de memoria com conteudo fim horario RGB com cor Azul
int MemFEEDHr1 = 14; //Endereço de memoria com conteudo horario primeira alimentação
int MemFEEDHr2 = 15; //Endereço de memoria com conteudo horario segunda alimentação
int MemLastFeed = 16; //Endereço de memoria com  horario da ultima alimentação


int ValueSaveSaida1 = 0; //Conteudo da memoria com status Saida 1
int ValueSaveSaida2 = 0; //Conteudo da memoria com status Saida 2
int ValueSaveSaida3 = 0; //Conteudo da memoria com status Saida 3
int ValueSaveSaida4 = 0; //Conteudo da memoria com status Saida 4
int ValueSaveAuto   = 0; //Conteudo da memoria com status Automatico ou  Manual
int ValueSaida3HrI = 0; //Conteudo da memoria com inicio horario Saida 3
int ValueSaida3HrF = 0; //Conteudo da memoria fim horario Saida 3
int ValueSaida4HrI = 0; //Conteudo da memoria inicio horario Saida 4
int ValueSaida4HrF = 0; //Conteudo da memoria fim horario Saida 4
int ValueRGBWHITEHrI = 0; //Conteudo de memoria inicio horario RGB com cor Branca
int ValueRGBWHITEHrF = 0; //Conteudo de memoria fim horario RGB com cor Branca
int ValueRGBBLUEHrI = 0; //Conteudo de memoria inicio horario RGB com cor Azul
int ValueRGBBLUEHrF = 0; //Conteudo de memoria  fim horario RGB com cor Azul
int ValueFEEDHr1 = 0; //Conteudo de memoria  horario primeira alimentação
int ValueFEEDHr2 = 0; //Conteudo de memoria  horario segunda alimentação
int ValueLastFeed = 0; //Conteudo de memoria  horario ultima alimentação

int buzzer = 0;

String readString;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
float celsius;

void setup() {

  wdt_enable(WDTO_8S); //Habilita Watchdog em 8 Segundos

  Wire.begin();

  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.begin(9600);

  //Setup Inicial-> descomentar/ atribuir valores iniciais / build / upload / comentar novamente/ upload
  /*
  EEPROM.write(MemSaida1, 0);
  EEPROM.write(MemSaida2, 0);
  EEPROM.write(MemSaida3, 0);
  EEPROM.write(MemSaida4, 0);
  EEPROM.write(MemAuto, 1);
  EEPROM.write(MemSaida3HrI, 9); //-> Saida 3 (Luz Fluorecente) liga 9:00
  EEPROM.write(MemSaida3HrF, 16); //-> Saida 3 (Luz Fluorecente) desliga 16:00
  EEPROM.write(MemSaida4HrI, 0);
  EEPROM.write(MemSaida4HrF, 0);
  EEPROM.write(MemRGBWHITEHrI, 17); //-> RGB Branco liga 17:00
  EEPROM.write(MemRGBWHITEHrF, 18); //-> RGB Branco desliga 18:00
  EEPROM.write(MemRGBBLUEHrI, 19); //-> RGB Azul liga 19:00
  EEPROM.write(MemRGBBLUEHrF, 22);//-> RGB Azul desliga 22:00
  EEPROM.write(MemFEEDHr1, 10);//-> Primeira alimentação 10:00
  EEPROM.write(MemFEEDHr2, 19);//-> Segunda alimentação 19:00
*/

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(PIN_SAIDA_BUZZ, OUTPUT);  
  
  //Inicializando placa com valores armazenados na FLASH Memory
  digitalWrite(A0, EEPROM.read(MemSaida1));
  digitalWrite(A1, EEPROM.read(MemSaida2));
  digitalWrite(A2, EEPROM.read(MemSaida3));
  digitalWrite(A3, EEPROM.read(MemSaida4));
  ValueSaveAuto = EEPROM.read(MemAuto);
  ValueSaida3HrI = EEPROM.read(MemSaida3HrI);
  ValueSaida3HrF = EEPROM.read(MemSaida3HrF);
  ValueSaida4HrI = EEPROM.read(MemSaida4HrI);
  ValueSaida4HrF = EEPROM.read(MemSaida4HrF);
  ValueRGBWHITEHrI = EEPROM.read(MemRGBWHITEHrI);
  ValueRGBWHITEHrF = EEPROM.read(MemRGBWHITEHrF);
  ValueRGBBLUEHrI = EEPROM.read(MemRGBBLUEHrI);
  ValueRGBBLUEHrF = EEPROM.read(MemRGBBLUEHrF);
  ValueFEEDHr1 = EEPROM.read(MemFEEDHr1);
  ValueFEEDHr2 = EEPROM.read(MemFEEDHr2);
  ValueLastFeed = EEPROM.read(MemLastFeed);

  buzzer = 0;

  //Setup inicial do horario do DS1307
  second = 00;
  minute = 8;
  hour = 22;
  dayOfWeek = 1;
  dayOfMonth = 1;
  month = 2;
  year = 15;
  // setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

 BuzzerLiga();
}

void loop() {
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  GetTemp();
  WebServer();
  ModoAuto();
  Buzzer();
  wdt_reset(); //Reset do WatchDog
}

void WebServer() {

  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {
          Serial.println(readString);
          if (readString.indexOf("?AUTOL") > 0) {
            ValueSaveAuto = 1;
            EEPROM.write(MemAuto, 1);
          }
          if (readString.indexOf("?AUTOD") > 0) {
            ValueSaveAuto = 0;
            EEPROM.write(MemAuto, 0);
          }

          if (readString.indexOf("?S1L") > 0) {
            digitalWrite(A0, HIGH);
            EEPROM.write(MemSaida1, 1);
            buzzer = 1;
          }
          if (readString.indexOf("?S1D") > 0) {
            digitalWrite(A0, LOW);
            EEPROM.write(MemSaida1, 0);
            buzzer = 2;
          }

          if (readString.indexOf("?S2L") > 0) {
            digitalWrite(A1, HIGH);
            EEPROM.write(MemSaida2, 1);
            buzzer = 1;
          }
          if (readString.indexOf("?S2D") > 0) {
            digitalWrite(A1, LOW);
            EEPROM.write(MemSaida2, 0);
            buzzer = 2;
          }

          if (readString.indexOf("?S3L") > 0) {
            digitalWrite(A2, HIGH);
            EEPROM.write(MemSaida3, 1);
            buzzer = 1;
          }
          if (readString.indexOf("?S3D") > 0) {
            digitalWrite(A2, LOW);
            EEPROM.write(MemSaida3, 0);
            buzzer =2;
          }

          if (readString.indexOf("?S4L") > 0) {
            digitalWrite(A3, HIGH);
            EEPROM.write(MemSaida4, 1);
            buzzer = 1;
          }
          if (readString.indexOf("?S4D") > 0) {
            digitalWrite(A3, LOW);
            EEPROM.write(MemSaida4, 0);
            buzzer = 2;
          }

          if (readString.indexOf("?RED") > 0) {
            analogWrite(PIN_RED, 255);
            analogWrite(PIN_GREEN, 0);
            analogWrite(PIN_BLUE, 0);
            buzzer = 1;
          }

          if (readString.indexOf("GRE") > 0) {
            analogWrite(PIN_RED, 0);
            analogWrite(PIN_GREEN, 255);
            analogWrite(PIN_BLUE, 0);
            buzzer = 1;
          }

          if (readString.indexOf("?BLU") > 0) {
            analogWrite(PIN_RED, 0);
            analogWrite(PIN_GREEN, 0);
            analogWrite(PIN_BLUE, 255);
            buzzer = 1;
          }

          if (readString.indexOf("?WHI") > 0) {
            analogWrite(PIN_RED, 255);
            analogWrite(PIN_GREEN, 255);
            analogWrite(PIN_BLUE, 255);
            buzzer = 1;
          }

          if (readString.indexOf("?RGBOFF") > 0) {
            analogWrite(PIN_RED, 0);
            analogWrite(PIN_GREEN, 0);
            analogWrite(PIN_BLUE, 0);
            buzzer = 2;
          }

          if (readString.indexOf("?FEE") > 0) {
            Alimenta();
            buzzer = 1;
          }

          if (readString.indexOf("?AgeS3HrI") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemSaida3HrI, cmd);
            ValueSaida3HrI = cmd;
            buzzer = 3;
          }          
          if (readString.indexOf("?AgeS3HrF") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemSaida3HrF, cmd);
            ValueSaida3HrF = cmd;
            buzzer = 3;
          }

          if (readString.indexOf("?AgeS4HrI") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemSaida4HrI, cmd);
            ValueSaida4HrI = cmd;
            buzzer = 3;
          }          
          if (readString.indexOf("?AgeS4HrF") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemSaida4HrF, cmd);
            ValueSaida4HrF = cmd;
            buzzer = 3;
          }

          if (readString.indexOf("?AgeRGBWHITEHrI") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemRGBWHITEHrI, cmd);
            ValueRGBWHITEHrI = cmd;
            buzzer = 3;
          }
          if (readString.indexOf("?AgeRGBWHITEHrF") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemRGBWHITEHrF, cmd);
            ValueRGBWHITEHrF = cmd;
            buzzer = 3;
          }
          
          if (readString.indexOf("?AgeRGBBLUEHrI") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemRGBBLUEHrI, cmd);
            ValueRGBBLUEHrI = cmd;
            buzzer = 3;
          }
          if (readString.indexOf("?AgeRGBBLUEHrF") > 0) {
            int cmd = readString.substring(readString.indexOf("|") +1, readString.lastIndexOf("|")).toInt();;
            EEPROM.write(MemRGBBLUEHrF, cmd);
            ValueRGBBLUEHrF = cmd;
            buzzer = 3;
          }

          SendResponse(client);
          delay(1);
          client.stop();
          readString = "";

        }
      }
    }
  }

}


void SendResponse(EthernetClient client) {

  int S1 = digitalRead(A0);
  int S2 = digitalRead(A1);
  int S3 = digitalRead(A2);
  int S4 = digitalRead(A3);

  int LedR = analogRead(6);
  int LedG = analogRead(5);
  int LedB = analogRead(3);

  client.println(F("HTTP/1.1 200 OK")); //send new page
  client.println(F("Content-Type: application/json"));
  client.println();
  client.print("dataCB");
  client.println(F("({"));

  client.print("\"Auto\":\"");
  client.print(ValueSaveAuto);
  client.println("\"");

  client.print(",\"Temp\":\"");
  client.print(celsius, DEC);
  client.println("\"");

  client.print(",\"Data\":\"");
  client.print(dayOfMonth, DEC);
  client.print("/");
  client.print(month, DEC);
  client.print("/");
  client.print(year, DEC);
  client.println("\"");
  client.print(",\"Hora\":\"");
  client.print(hour, DEC);
  client.print(":");
  client.print(minute, DEC);
  client.print(":");
  client.print(second, DEC);
  client.println("\"");

  client.print(",\"S1\":");
  client.println(S1);
  client.print(",\"S2\":");
  client.println(S2);
  client.print(",\"S3\":");
  client.println(S3);
  client.print(",\"S4\":");
  client.println(S4);

  client.print(",\"AgeS3HrI\":");
  client.println(ValueSaida3HrI);
  client.print(",\"AgeS3HrF\":");
  client.println(ValueSaida3HrF);
  client.print(",\"AgeS4HrI\":");
  client.println(ValueSaida4HrI);
  client.print(",\"AgeS4HrF\":");
  client.println(ValueSaida4HrF);

  client.print(",\"AgeRGBWHITEHrI\":");
  client.println(ValueRGBWHITEHrI);
  client.print(",\"AgeRGBWHITEHrF\":");
  client.println(ValueRGBWHITEHrF);
  client.print(",\"AgeRGBBLUEHrI\":");
  client.println(ValueRGBBLUEHrI);
  client.print(",\"AgeRGBBLUEHrF\":");
  client.println(ValueRGBBLUEHrF);

  client.print(",\"AgeFeed1\":");
  client.println(ValueFEEDHr1);
  client.print(",\"AgeFeed2\":");
  client.println(ValueFEEDHr2);

  client.println(F("})"));
  client.println();

}

void ModoAuto() {

  //Saida 1 e Saida 2 utilizadas na Bomba e termostato não possuem agendamento de horarios
  if (ValueSaveAuto == 1)
  {
    //Saida 3
    if (ValueSaida3HrI >= hour && ValueSaida3HrF <= hour)
    {
      digitalWrite(A2, HIGH);
      EEPROM.write(MemSaida3, 1);
    }
    else
    {
      digitalWrite(A2, LOW);
      EEPROM.write(MemSaida3, 0);
    }

    //Saida 4
    if (ValueSaida4HrI >= hour && ValueSaida4HrF <= hour)
    {
      digitalWrite(A3, HIGH);
      EEPROM.write(MemSaida4, 1);
    }
    else
    {
      digitalWrite(A3, LOW);
      EEPROM.write(MemSaida4, 0);
    }

    //RGB
    if (ValueRGBWHITEHrI >= hour && ValueRGBWHITEHrF <= hour)
    {
      analogWrite(PIN_RED, 255);
      analogWrite(PIN_GREEN, 255);
      analogWrite(PIN_BLUE, 255);
    }
    else if (ValueRGBWHITEHrI >= hour && ValueRGBWHITEHrF <= hour)
    {
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      if (ValueRGBWHITEHrF == hour) // Mais escuro na ultima hora do agendamento azul
      {
        analogWrite(PIN_BLUE, 80);
      }
      else
      {
        analogWrite(PIN_BLUE, 255);
      }
    }
    else
    {
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
    }

    ValueLastFeed = EEPROM.read(MemLastFeed);
    //Alimentação Agendada 1
    if (ValueFEEDHr1 == hour && ValueLastFeed != hour)
    {
      Alimenta();
      EEPROM.write(MemLastFeed, hour);
    }
    //Alimentação Agendada 2
    if (ValueFEEDHr2 == hour && ValueLastFeed != hour)
    {
      Alimenta();
      EEPROM.write(MemLastFeed, hour);
    }
  }
}

void GetTemp() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }

  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44);

  delay(1000);     // maybe 750ms is enough, maybe not  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3;
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1;
  }
  celsius = (float)raw / 16.0;
}


void Alimenta()
{

  myservo.attach(PIN_FEEDER);
  int angle = 0;
  //  for(angle = 90; angle < 110; angle += 1)
  for (angle = 90; angle < 150; angle += 1)
  {
    myservo.write(angle);
    delay(20);
  }
  myservo.detach();
  delay(300);
  //  BuzzerConfirma();
  Serial.print("Alimentacao OK");

}

void getDateDs1307(byte * second,
                   byte * minute,
                   byte * hour,
                   byte * dayOfWeek,
                   byte * dayOfMonth,
                   byte * month,
                   byte * year)
{

  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}

byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void setDateDs1307(byte second,        // 0-59
                   byte minute,        // 0-59
                   byte hour,          // 1-23
                   byte dayOfWeek,     // 1-7
                   byte dayOfMonth,    // 1-28/29/30/31
                   byte month,         // 1-12
                   byte year)          // 0-99
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));      // If you want 12 hour am/pm you need to set
  // bit 6 (also need to change readDateDs1307)
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}


void Buzzer()
{
  if(buzzer ==1)
  {
    BuzzerLiga();
  }  
  else if(buzzer ==2)
  {
    BuzzerDesliga();
  }  
  else if(buzzer ==3)
  {
    BuzzerAgenda();
  }  
  buzzer = 0;
}

void BuzzerLiga()
{
  buzz(PIN_SAIDA_BUZZ, 3000, 50);        
  buzz(PIN_SAIDA_BUZZ, 2500, 30);        
  buzz(PIN_SAIDA_BUZZ, 3500, 250);        
}

void BuzzerAgenda()
{
  buzz(PIN_SAIDA_BUZZ, 3500, 100);    
}

void BuzzerDesliga()
{
  buzz(PIN_SAIDA_BUZZ, 3500, 500);        
}

void buzz(int targetPin, long frequency, long length) {
  long delayValue = 1000000/frequency/2;
  long numCycles = frequency * length/ 1000;
 for (long i=0; i < numCycles; i++){
    digitalWrite(targetPin,HIGH);
    delayMicroseconds(delayValue);
    digitalWrite(targetPin,LOW);
    delayMicroseconds(delayValue);
  }
}


