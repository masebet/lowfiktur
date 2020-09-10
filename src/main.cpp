#include <Arduino.h>
#include <avr/wdt.h>
#include "MemoryFree.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>

#define ModIN_rxPin 11
#define ModIN_txPin 12
#define ModIN_RS485_pin 13
#define ModOUT_RS485_pin 10
#define PinResetSIM900A 9
#define PIN_CS 53
#define PIN_INT 3


#define WBP_BIF   16
#define LWBP_BIF  18
#define WBP_IN    20
#define LWBP_IN   22
#define WBP_OUT   24
#define LWBP_OUT  26

#define WBP_BIF16   100
#define LWBP_BIF16  104
#define WBP_IN16    108
#define LWBP_IN16   112
#define WBP_OUT16   114
#define LWBP_OUT16  118

#define EP_MODE   30
#define EID_IN    1
#define EID_OUT   3
#define EID_BIF   5

#define EP_PORT 50

#define EP_IP 60
#define EP_APN 80

SoftwareSerial ModIN(ModIN_rxPin, ModIN_txPin);

String  modbuskwh_OUT     ="";
String  modbuskwh_IN      ="";
String  modbuskwh_BIF     ="";


//nc 183.91.67.211 15055
String  IPADDRESS         ="";//"183.91.67.211";
String  PORT              ="";//"15055";
String  APN               ="";//"internet";

String  csq               ="";
String  respondsend       ="";
String  sdCard            ="";


String  sender_phone      ="";
String  Reply             ="";
String  ID_IN             ="";
String  ID_OUT            ="";
String  ID_BIF            ="";
String  ResponeSMS        ="";
String  MOBI              ="";
String  MOBO              ="";
String  MOBF              ="";
String  FV                ="CC_2R 05/05/2020";
String  FM                ="";
String tahun              ="";
String bulan              ="";
String hari               ="";

boolean Resetsim          =true;
boolean getmodbus         =false;
#define S1debug 0

bool RecieveMessage();
bool TimetoSend();
void GETmodbus();
void fikturSMS();
bool SendCommand(String command, unsigned int timeout, boolean debug);
void SendCipSend(unsigned int timeout);
void Timeout();
void READYSEND();
void SendMessage();
void ResetSim900A();
void kirimPesan();
void cekSignal();
void cekModbus();
void sdLoger();
void GETmodbusTigaData();
void send2serverEbetTiga();
void(* reset) (void) = 0;

#include "SD.h"
File myFile;

#include "proses.hpp"
#include "input.hpp"
// #include "input1.hpp"
#include "sms.h"


void waitrespon(unsigned long timeOut){ 
  wdt_reset();
  String waitr="";
  unsigned long time = millis();
  while ( (millis()-time) < timeOut ){
      if(Serial2.available()) waitr += (char) Serial2.read();
  }
  Serial.print("\r\nlistener sim900 : ");Serial.println(waitr);
  if(waitr.indexOf("+")>=0)Reply=waitr;
}


void setup() {

  Serial.begin(9600);         //debug
  Serial2.begin(9600);        //simcom
  ModIN.begin(9600);          //Modbus1
  // Serial1.begin(9600);     //Modbus2
  wdt_enable(WDTO_8S);

  pinMode(ModIN_RS485_pin, OUTPUT);
  pinMode(ModOUT_RS485_pin, OUTPUT);
  pinMode(PinResetSIM900A, OUTPUT);

  ResetSim900A();

  proses_tulisEprom(EP_IP,"183.91.67.214");    //parsing java prod
  proses_api.tulisEprom(EP_PORT,"9000");


  // proses_tulisEprom(EP_APN,"internet");
  // proses_tulisEprom(EP_APN,"m2minternet");


  Serial.println("Modem Prima Saver v.07");
  Serial.print("FIRMWARE VERSION:"); Serial.println(FV);

  ID_BIF    = "ED0";   
  ID_IN     = "ED1";  
  ID_OUT    = "ED2";
  
  IPADDRESS = proses_bacaDataEprom(EP_IP);
  PORT      = proses_api.bacaDataEprom(EP_PORT);
  APN       = proses_bacaDataEprom(EP_APN);

  waitrespon(3000);
  Serial2.print("AT\r\n");
  waitrespon(3000);
  Serial2.print("AT+CMGF=1\r\n");
  waitrespon(3000);
  Serial2.print("AT+CREG=1\r\n");
  waitrespon(3000);
  // Serial2.print("AT+CNMI=1,2,0,0,0\r\n");
  // waitrespon(3000);
  Serial2.print("AT+CNMI=3,3,0,0,0\r\n");
  waitrespon(3000);
  Serial2.print("AT+CMGD=1,4\r\n");
  waitrespon(3000);
  Serial2.print("AT+CLIP=1\r\n");
  waitrespon(3000);
  Serial2.print("AT+CLTS=1\r\n");
  waitrespon(3000);
  // Serial2.print("AT&W\r\n");
  // waitrespon(3000);
  waitrespon(5000);

  Serial2.print("AT+CCLK?\r\n");
  waitrespon(3000);
  if(Reply.substring(20,22).toInt()>10){
    tahun = Reply.substring(20,22).toInt()+2000;
    bulan = Reply.substring(23,25).toInt();
    hari  = Reply.substring(26,28).toInt();
    // Reply.substring(29,31).toInt();
    // Reply.substring(32,34).toInt();
    // Reply.substring(35,37).toInt();
  }

  cekSignal();
  sdLoger();
  cekModbus();
  return;
}//setup()

void loop() {
  wdt_reset();
  waitrespon(3000);

  if(Reply.indexOf("RING")>0){
    Serial2.print("ATH0\r\n");
    waitrespon(3000);
    sms::getNomor(Reply);
    Reply = " LIST";
    sms::fikturSMS();
    sms::kirimPesan();
  }

  TimetoSend();
  if (getmodbus){
    getmodbus=false;
    GETmodbusTigaData();
    send2serverEbetTiga();
  }

  FM="";
  FM.concat(freeMemory());
  ResetSim900A();
}//loop()

#define DELAYTCP    120000
#define DELAYRESET  6000000

unsigned long timeFlag,previousMillis;
bool TimetoSend(){
  wdt_reset();  
  if (millis() - previousMillis > DELAYTCP ) {
      previousMillis = millis();
      if(S1debug){}else{Serial.println("time to send");}
      getmodbus = true;
  }

  if(millis()-timeFlag > DELAYRESET){
    timeFlag = millis();
    if(S1debug){}else{Serial.println("time to Reset");}
    Resetsim = true;
    reset();
  }
  return 0;
}//TimetoSend()


void GETmodbusTigaData(){
  wdt_reset();
   modbuskwh_BIF ="";
   modbuskwh_IN  ="";
   modbuskwh_OUT ="";
   modbuskwh_BIF.concat(input_getDataSensor0());
   modbuskwh_IN.concat(input_getDataSensor1());
   modbuskwh_OUT.concat(input_getDataSensor2());
   
  Serial2.print("AT+CCLK?\r\n");
  waitrespon(3000);
  if(Reply.substring(20,22).toInt()>10){
    tahun = Reply.substring(20,22).toInt()+2000;
    bulan = Reply.substring(23,25).toInt();
    hari  = Reply.substring(26,28).toInt();
    // Reply.substring(29,31).toInt();
    // Reply.substring(32,34).toInt();
    // Reply.substring(35,37).toInt();
  }
  
   rutin::logerData(modbuskwh_IN+modbuskwh_OUT);
  //  if(S1debug){}else{Serial.print("modbuskwh_BI:");Serial.println(modbuskwh_BIF);}
  //  if(S1debug){}else{Serial.print("modbuskwh_IN:");Serial.println(modbuskwh_IN );}
  //  if(S1debug){}else{Serial.print("modbuskwh_OT:");Serial.println(modbuskwh_OUT);}
}//GETmodbus()

void send2serverEbetTiga(){
  wdt_reset();
  SendCommand("AT+CIPCLOSE\r\n",500,S1debug);
  SendCommand("AT+CIPSHUT\r\n",500,S1debug);
  SendCommand("AT+CIPMUX=0\r\n",500,S1debug);
  SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
  SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",500,S1debug);
  for(int i=0;i<5;i++){
    SendCommand("AT+CIICR\r\n",2000,S1debug);
    SendCommand("AT+CIFSR\r\n",500,S1debug);
    back : SendCommand("AT+CIPSTART=\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",1000,S1debug);
    if(i>5) return; else i++;
    if(Reply.indexOf("CONNECT")>0) break;
    else if(Reply.indexOf("RING")>0) return;
    else goto back;
  }

  SendCommand("AT+CIPSEND\r\n",1000,S1debug);
  Serial2.flush();
  SendCommand(modbuskwh_IN,1000,S1debug);
  SendCommand(modbuskwh_OUT,1000,S1debug);
  SendCommand(modbuskwh_BIF,1000,S1debug); 
  SendCipSend(1500);
  SendCommand("AT+CIPCLOSE\r\n",500,S1debug);
  SendCommand("AT+CIPSHUT\r\n",500,S1debug);
}//send2serverEbetTiga()

// void send2serverEbetTiga(){
//   wdt_reset();
//   SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
//   SendCommand("AT+CIPSHUT\r\n",500,S1debug);
//   SendCommand("AT+CIPMUX=1\r\n",500,S1debug);
//   SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
//   SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",500,S1debug);
//   for(int i=0;i<5;i++){
//     SendCommand("AT+CIICR\r\n",2000,S1debug);
//     SendCommand("AT+CIFSR\r\n",500,S1debug);
//     SendCommand("AT+CIPSTART=0,\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",1000,S1debug);
//     back : SendCommand("AT+CIPSTATUS=0\r\n",600,S1debug);
//     if(i>5) return; else i++;
//     if(Reply.indexOf("CONNECTED")>0) break;
//     else if(Reply.indexOf("RING")>0) return;
//     else if(Reply.indexOf("CONNECTING")>0) goto back;
//   }
//   SendCommand("AT+CIPSEND=0\r\n",1000,S1debug);
//   Serial2.flush();
//   SendCommand(modbuskwh_IN,1000,S1debug);
//   SendCommand(modbuskwh_OUT,1000,S1debug);
//   SendCommand(modbuskwh_BIF,1000,S1debug); 
//   SendCipSend(1500);
//   SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
//   SendCommand("AT+CIPSHUT\r\n",500,S1debug);
// }//send2serverEbetTiga()

bool SendCommand(String command, unsigned int timeout, boolean debug){
  wdt_reset();
  Serial2.print(command); 

  Reply = "";
  unsigned long time = millis();
  while ((millis()-time) < timeout ){
      if(Serial2.available()) Reply += (char) Serial2.read();//Reply = Serial2.readString();
  }
  // Serial.println(command);
  Serial.println(Reply);
  return 0;
}//SendCommand()

void SendCipSend(unsigned int timeout){
  wdt_reset();
  respondsend = "";
  Serial2.write(0x1A);
  waitrespon(timeout);
  if(respondsend.indexOf("SEND OK")>0)respondsend="SEND OK";
  if(respondsend.indexOf("SEND FAIL")>0)respondsend="SEND FAIL";
  else respondsend="";
}//SendCipSend()

void cekSignal(){
  wdt_reset();
  SendCommand("AT+CSQ\r\n",500,S1debug);
  int f,l;
  f=Reply.indexOf("+CSQ:"); 
  l=Reply.indexOf("\n",f);
  csq = Reply.substring(f+6,l);
  return;
}//cekSignal()

void cekModbus(){
  wdt_reset();
  MOBF = ID_BIF+printHexDate(posix_get_2_Register(0x08,0x04,0x0578,0x0002),8);
  MOBI = ID_IN+printHexDate(posix_get_2_Register(0x01,0x04,0x0578,0x0002),8);
  MOBO = ID_OUT+printHexDate(posix_get_2_Register(0x02,0x04,0x0578,0x0002),8);
  return;
}//cekModbus()

void sdLoger(){
  wdt_reset();
  if (SD.begin(PIN_CS)) {
    sdCard = "SDCARD ON";
  }else{
    sdCard = "SDCARD OFF";  
  }
  return;
}//sdLoger()

void ResetSim900A(){ 
  wdt_reset();
  if (Resetsim){
    Resetsim=false;
    digitalWrite(PinResetSIM900A, LOW);
    delay(1000);
    digitalWrite(PinResetSIM900A, HIGH);
    delay(1000);
  }
}//ResetSim900A()