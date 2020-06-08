#include <Arduino.h>

//program ini berisi 4 requst data
//2 reply digabungkan jadi satu string lalu di kirim
//jadi cuma yg dikirim incoming dan outgoing
//tambahan watchdog timer
//tambahan SHUT berkala per 5 menit
//penambahan feature sms
//deleted sms berkala per 1 menit
//get sender phone di taro setelah sotir kode SMS
//marger if SNI dan SNO ,emjadi (SNI || SNO) di send message
//penempatan fungsi disesuaikan dengan kondisi loop
//perbaikan setup read EEPROM (ID IN DAN ID OUT) tadinya UNKNOWN menjadi 000000000
//perbaikan get data modbus adress WS tadinya NaN, sekarang udah ada, salah substringnya saja. 
//perbaikan jika length ID = 10 maka ID=000000000, hal ini terjadi biasanya jika isi EEPROM ÿÿÿÿÿÿÿÿÿÿ
//penamabahan string free memory. jadi ketika LIST kita tau free memorinya berapa

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
#define PinResetSIM900A 9     //reset bukan restart
#define PIN_CS 53
#define PIN_INT 3

#define WBP_IN    20
#define LWBP_IN   22
#define WBP_OUT   24
#define LWBP_OUT  26
#define EP_MODE 30

#define EID_IN    1
#define EID_OUT   11

#define EP_PORT 50

#define EP_IP 60
#define EP_APN 80


SoftwareSerial ModIN(ModIN_rxPin, ModIN_txPin);

unsigned long   currentMillis                  =0;
unsigned long   currentMillisTimeout           =0;
unsigned long   currentMillisSHUT              =0;
long            previousMillis                 =0;
long            previousMillisTimeout          =0;
long            previousMillisSHUT             =0;

// byte    data_IN1[8]       ={0x01, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x04};//[0-22 (0x01, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x04)]
// byte    data_IN2[8]       ={0x01, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xc0};//[25-42(0x01, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xc0)]

// byte    data_OUT1[8]      ={0x02, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x37};//[0-22(0x02, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x37)]
// byte    data_OUT2[8]      ={0x02, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xf3};//[25-42(0x02, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xf3)]

String  modbuskwh_OUT     ="";
String  Datakwh_OUT1      ="";
String  Datakwh_OUT2      ="";
String  modbuskwh_IN      ="";
String  Datakwh_IN1       ="";
String  Datakwh_IN2       ="";

//nc 183.91.67.211 15055
String  IPADDRESS         ="";//"183.91.67.211";
String  PORT              ="";//"15055";
String  APN               ="";//"internet";
// String  cpin              ="";
// String  csq               ="";
// String  cipstatus         ="";
// String  respondsend       ="";

String  sender_phone      ="";
String  Reply             ="";
// String  Buffer            ="";
// String  BufferData        ="";
String  ID_IN             ="";
String  ID_OUT            ="";
String  KodeSMS           ="";
String  ResponeSMS        ="";
// String  MOBI[3]           ={};
// String  MOBO[3]           ={};
String  FV                ="ATP 20/07/17";
String  FM                ="";

char    charBuf[10];
char    kar;

uint8_t sumcpin           =0;          
uint8_t FI                =0;
uint8_t LI                =0; 
uint8_t state             =0;
uint8_t i                 =0;

boolean Resetsim          =false;
boolean TTS               =false;
boolean sinc              =false;

boolean getmodbus         =false;
boolean SendDataKWH_IN    =false;
boolean SendDataKWH_OUT   =false;
boolean sender            =false;
#define S1debug 1


bool RecieveMessage();
void TimetoSend();
void SHUTBERKALA();
void GETmodbus();
// void sendData2IN1();
// void responeKWHIN1 ();
// void sendData2IN2();
// void responeKWHIN2();
// void sendData2OUT1();
// void responeKWHOUT1();
// void sendData2OUT2();
// void responeKWHOUT2();
void fikturSMS();
void send2server();
void send2serverEbet();
void send2serverEbet328();
void SendCommand(String command, const int timeout, boolean debug);
void SendCipSend(int timeout);
void Timeout();
void READYSEND();
void SendMessage();
void ResetSim900A();
void kirimPesan();

//fungsi tambahan
// #include "RTClib.h"
// RTC_DS3231 rtc;

#include "SD.h"
File myFile;

#include "proses.hpp"
#include "input.hpp"

//ED160dc4e8bc26f2ddc736c4a2faf804afc0f00479aa2f44a2a480046787c0046a63800461a147b42489d804982d3e04b20800043c0400043c1c00043c317804a4288804a44458049e400000000000000009d80498274404a577c004a9247004a06ED261284e8bc2732ddc000043fa0000459c0000434800004396a9fc41158b444154db2340b90a3d424880004420900045c6733343c0000043c1c00043c3000044ea400044fec0004487000000000000000080004420c0004501c000453a400044a1

//ED160dc4e8bc26f2ddc736c4a2faf804afc0f00479aa2f44a2a480046787c0046a63800461a147b42489d804982d3e04b20800043c0400043c1c00043c317804a4288804a44458049e400000000000000009d80498274404a577c004a9247004a06
//ED261284e8bc2732ddc000043fa0000459c0000434800004396a9fc41158b444154db2340b90a3d424880004420900045c6733343c0000043c1c00043c3000044ea400044fec0004487000000000000000080004420c0004501c000453a400044a1


void setup() {

  Serial.begin(9600);   //debug
  Serial1.begin(9600);  //modbus KWH OUTGOING
  Serial2.begin(9600);  //SIM900
  ModIN.begin(9600);    //modbus KWH INCOMING

  delay(1000);
  pinMode(ModIN_RS485_pin, OUTPUT);
  pinMode(ModOUT_RS485_pin, OUTPUT);
  pinMode(PinResetSIM900A, OUTPUT);
  delay(100);

  //warming up  sim90A
  digitalWrite(PinResetSIM900A, LOW);
  delay(1000);
  digitalWrite(PinResetSIM900A, HIGH);
  delay(5000);

  // if(!DEBUG) wdt_enable(WDTO_8S);  //timeout watchdog timer 8 second
  
  // proses_api.tulisEprom(EP_MODE,"P164");  
  // proses_api.tulisEprom(EP_MODE,"G328");
  // proses_tulisEprom(EP_IP,"183.91.67.211");    //pasing riset text
  // proses_api.tulisEprom(EP_PORT,"15055");

  // proses_api.tulisEprom(EP_MODE,"G328");
  // proses_tulisEprom(EP_IP,"110.50.86.220");    //parsing gabung dev
  // proses_api.tulisEprom(EP_PORT,"9017");

  // proses_api.tulisEprom(EP_MODE,"P164"); 
  // proses_tulisEprom(EP_IP,"183.91.67.214");    //parsing java prod
  // proses_api.tulisEprom(EP_PORT,"9003");

  Serial.println("Modem Prima Saver v.07");
  Serial.print("FIRMWARE VERSION:");Serial.println(FV);

  if (SD.begin(PIN_CS)) {
    Serial.println("ini bisa");
  }else{
    Serial.println("ini tidak bisa");  
  }

  proses_api.tulisEprom(EID_IN,"ED1");
  proses_api.tulisEprom(EID_OUT,"ED2");
  
  ID_IN     = proses_api.bacaDataEprom(EID_IN);  
  ID_OUT    = proses_api.bacaDataEprom(EID_OUT);
  
  IPADDRESS = proses_bacaDataEprom(EP_IP);
  PORT      = proses_api.bacaDataEprom(EP_PORT);
  APN       = proses_bacaDataEprom(EP_APN);

  tools::alaramBegin();
  pinMode(PIN_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_INT), tools::alarmFunction, FALLING);

  ResetSim900A();
  Serial2.write(0x1A);
  SendCommand("AT\r\n",500,S1debug);                // Inisial Mode SMS
  SendCommand("AT+CMGF=1\r\n",500,S1debug);         // Mode TEXT
  SendCommand("AT+CNMI=3,3,0,0,0\r\n",500,S1debug); //
  SendCommand("AT+CMGD=1,4\r\n",500,S1debug);       //
  // SendCommand("AT+CMGDA=\"DEL ALL\"\r\n",500,S1debug);       //
  if(DEBUG) debug();

  tools::printData2();
}//setup

void loop() {
  wdt_reset();
  // Serial.println(clock.readRegister8Ebet(0x0e),BIN);
  // tools::printData2();

  rutin::tarikDataLoger();
  rutin::resetMeteranDanEprom();

  if (clock.isAlarm1())
  {
    Serial.println("ALARM 1 TRIGGERED!");
    rutin::reset();
    delay(1000);
  }

  if(tools::alaramIsOn()){
    Serial.println("hallo alaram 1");
    rutin::reset();
    delay(1000);
  }
  
  if(RecieveMessage()) {
    fikturSMS();
    kirimPesan();
  }

  TimetoSend();         //  Timer untuk ambil hapus sms dan flag ambil data modbus
  // SHUTBERKALA();        //  Timer state 8
  if (getmodbus){       //  youNowLah
      GETmodbus();      //
      getmodbus=false;  // 
      if (proses_api.bacaDataEprom(EP_MODE)=="G328") send2serverEbet328();// 
      else send2serverEbet();
  }                     //
  FM="";
  FM.concat(freeMemory());
  ResetSim900A();       //  Untuk reset modul gsm
  Serial.print("freeMemory()="); Serial.println(FM);
  Serial.println();
}//loop 

void TimetoSend(){
  wdt_reset();  
  currentMillis = millis();
  if (currentMillis - previousMillis > 60000) {//1 menit
    Serial.println("time to send");
    getmodbus=true;
    TTS=true;
    sinc=true;
    previousMillis = currentMillis;
  }
}//TimetoSend()

void GETmodbus(){
  wdt_reset();
   modbuskwh_IN  ="";
   modbuskwh_OUT ="";
   
   modbuskwh_IN.concat(input_getDataSensor());
   modbuskwh_OUT.concat(input_getDataSensorOut());
   rutin::logerData(modbuskwh_IN+modbuskwh_OUT);
   Serial.print("modbuskwh_IN:");Serial.println(modbuskwh_IN );
   Serial.print("modbuskwh_OT:");Serial.println(modbuskwh_OUT);
   SendDataKWH_IN         =true;
   SendDataKWH_OUT        =true;
}//GETmodbus()

// Modif Dari ATP
void send2serverEbet(){
    //Sent incoming
    SendCommand("AT\r\n",250,S1debug);
    SendCommand("AT+CPIN?\r\n",250,S1debug);
    //SendCommand("AT+CSQ",500,S1debug);
    SendCommand("AT+CREG?\r\n",500,S1debug);
    SendCommand("AT+CGATT=1\r\n",1000,S1debug);
    SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",2000,S1debug);
    SendCommand("AT+CIICR\r\n",2000,S1debug);
    SendCommand("AT+CIFSR\r\n",2000,S1debug);
    SendCommand("AT+CIPSTART=\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",2000,S1debug);
    SendCommand("AT+CIPSTATUS\r\n",2000,S1debug);
    SendCommand("AT+CIPSEND\r\n",1000,S1debug);
    SendCommand(modbuskwh_IN+"\r\n",1000,S1debug);
    SendCipSend(6000);
    SendCommand("AT+CIPSHUT\r\n",2000,S1debug);

    //Sent OutGing
    SendCommand("AT\r\n",250,S1debug);
    SendCommand("AT+CPIN?\r\n",250,S1debug);
    //SendCommand("AT+CSQ",500,S1debug);
    SendCommand("AT+CREG?\r\n",500,S1debug);
    SendCommand("AT+CGATT=1\r\n",1000,S1debug);
    SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",2000,S1debug);
    SendCommand("AT+CIICR\r\n",2000,S1debug);
    SendCommand("AT+CIFSR\r\n",2000,S1debug);
    SendCommand("AT+CIPSTART=\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",2000,S1debug);
    SendCommand("AT+CIPSTATUS\r\n",2000,S1debug);
    SendCommand("AT+CIPSEND\r\n",1000,S1debug);
    SendCommand(modbuskwh_OUT+"\r\n",1000,S1debug);
    SendCipSend(6000);
    SendCommand("AT+CIPSHUT\r\n",2000,S1debug);

    SendCommand("AT+CSQ\r\n",1000,S1debug);
}//send2serverEbet()

void send2serverEbet328(){
    //Sent incoming
    SendCommand("AT\r\n",250,S1debug);
    SendCommand("AT+CPIN?\r\n",250,S1debug);
    SendCommand("AT+CREG?\r\n",500,S1debug);
    SendCommand("AT+CGATT=1\r\n",1000,S1debug);
    SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",2000,S1debug);
    SendCommand("AT+CIICR\r\n",2000,S1debug);
    SendCommand("AT+CIFSR\r\n",2000,S1debug);
    SendCommand("AT+CIPSTART=\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",2000,S1debug);
    SendCommand("AT+CIPSTATUS\r\n",2000,S1debug);
    SendCommand("AT+CIPSEND\r\n",1000,S1debug);
    SendCommand(modbuskwh_IN,1000,S1debug);
    SendCommand(modbuskwh_OUT,1000,S1debug);
    SendCipSend(6000);
    SendCommand("AT+CIPSHUT\r\n",2000,S1debug);
    SendCommand("AT+CSQ\r\n",1000,S1debug);
}//send2serverEbet()

void SendCipSend(int timeout){
  wdt_reset();
  Reply = "";
  Serial2.write(0x1A);
  delay(timeout);
  if(Serial2.available()) {Reply = Serial2.readString();};   
  Serial.print("response cipsend: ");
  Serial.print(Reply);
}//SendCipSend()

void Timeout(){
  wdt_reset();
  
  if (sinc){
    previousMillisTimeout=currentMillis;
    sinc=false;
  }
  currentMillisTimeout = millis();
  if (currentMillisTimeout - previousMillisTimeout > 30000) {//0.5 menit
    Serial.println("time out send");
    TTS=false;
    getmodbus=false;
    previousMillisTimeout = currentMillisTimeout;
  }
}//Timeout()

void ResetSim900A(){ 
  wdt_reset();
  
  if (Resetsim){
  Serial.println("Reset SIM900A");
  digitalWrite(PinResetSIM900A, LOW);
  delay(3000);
  digitalWrite(PinResetSIM900A, HIGH);
  delay(4000);
  Resetsim=false;
  }
}//ResetSim900A()

/*//PunyaATP
void RecieveMessage(){
  wdt_reset();
  
  String      Key         ="";
              KodeSMS     ="";
  
  SendCommand("AT+CMGR=1\r",500,S1debug);
 
  FI=Reply.indexOf("1M4N1");
  ////Serial.print("FI:");Serial.println(FI);

  Key=Reply.substring(FI,FI+5);
  Key.trim();
  
  Serial.print("Key:");Serial.println(Key);
  
  if (Key=="1M4N1"){
    //Serial.println("Di Dalam");

    //dapetin kode SMS
    LI=Reply.indexOf('#',FI);
    //Serial.print("LI:");Serial.println(LI);
    LI++;
    FI=Reply.indexOf('#',LI);
    //Serial.print("FI:");Serial.println(FI);
    KodeSMS=Reply.substring(LI,FI);
    Serial.print("KodeSMS:");Serial.println(KodeSMS);

    if(KodeSMS=="LIST"){
      getmodbus=true;
    }
    
    //Serial Number INCOMING
    if(KodeSMS=="SNI"){
      Buffer="";
      LI=Reply.indexOf('#');
      //Serial.print("LI:");Serial.println(LI);
      LI++;
      FI=Reply.indexOf('#',LI);
      FI++;
      LI=Reply.indexOf('*',FI);
      Buffer=Reply.substring(FI,LI);
      //Serial.print("ID KWH INCOMING:");Serial.println(Buffer);
      
      
      //tulis ke EEPROM adres 0-9
      //Serial.print("Buffer.length():");Serial.println(Buffer.length());
      proses_api.tulisEprom(1,Buffer);
      
      //baca
      ID_IN = proses_api.bacaDataEprom(1); 
      Serial.print("ID KWH INCOMING from SMS:");Serial.println(ID_IN);//Serial.println("<=");
    }//Serial Number INCOMING
    
    //Serial Number OUTGOING
    else if (KodeSMS=="SNO"){
      Buffer="";
      LI=Reply.indexOf('#');
      //Serial.print("LI:");Serial.println(LI);
      LI++;
      FI=Reply.indexOf('#',LI);
      FI++;
      LI=Reply.indexOf('*',FI);
      Buffer=Reply.substring(FI,LI);
      //Serial.print("ID KWH OUTGOING:");Serial.println(Buffer);    


      //tulis ke EEPROM adres 10-19
      proses_api.tulisEprom(11,Buffer);
      
      //baca
      ID_OUT = proses_api.bacaDataEprom(11); 
      Serial.print("ID KWH OUTGOING from SMS:");Serial.println(ID_OUT);//Serial.println("<=");
    }//Serial Number OUTGOING

    //dapetin nomor hp pengirim
    FI=Reply.indexOf("+CMGR:");
    FI++;
    LI=Reply.indexOf('+',FI);
    FI=Reply.indexOf(',',LI);
    sender_phone=Reply.substring(LI,FI-1);
    sender_phone.trim();
    Serial.print("sender_phone:");Serial.println(sender_phone);
    
    if (sender_phone.length()>5){
      sender=true;
      Serial.println("GET PHONE SEND NUMBER");
      SendCommand("AT+CMGD=1,4\r",500,S1debug);
    }
    else{
      sender=false;
    }
    Key="";
  }

}//RecieveMessage()
*/

/*
void SendMessage(){
  wdt_reset();

  ResponeSMS="";
  
      if (KodeSMS=="LIST"){
        ResponeSMS+="SNI:";   ResponeSMS.concat(ID_IN);
        ResponeSMS+="\nSNO:"; ResponeSMS.concat(ID_OUT);
        ResponeSMS+="\nSIM:"; ResponeSMS.concat(cpin);
        ResponeSMS+="\nSQ :"; ResponeSMS.concat(csq);
        ResponeSMS+="\nSTA:"; ResponeSMS.concat(cipstatus);
        ResponeSMS+="\nMBI:"; ResponeSMS.concat(MOBI[0]);ResponeSMS+=" ";ResponeSMS.concat(MOBI[1]);
        ResponeSMS+="\nMBO:"; ResponeSMS.concat(MOBO[0]);ResponeSMS+=" ";ResponeSMS.concat(MOBO[1]);
        ResponeSMS+="\nFM :"; ResponeSMS.concat(FM);
        ResponeSMS+="\nFV :"; ResponeSMS.concat(FV);
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
      }
      else if (KodeSMS=="SNI" || KodeSMS=="SNO"){
        //tulis dulu SNI (Serial Number Incoming) pada EEPROM
    
        //kemudian bales smsnya
        ResponeSMS+="SNI:";   ResponeSMS.concat(ID_IN+konversi::toIEEE(posix_get_2_Register(0x01,0x04,0x0578,0x0002)));
        ResponeSMS+="\nSNO:"; ResponeSMS.concat(ID_OUT+konversi::toIEEE(posix_get_2_RegisterOut(0x02,0x04,0x0578,0x0002)));
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
      }
      else if(KodeSMS=="IPRESET"){
        //balas dulu sms baru reset
        ResponeSMS+="SNI:"; ResponeSMS.concat(ID_IN+konversi::toIEEE(posix_get_2_Register(0x01,0x04,0x0578,0x0002)));
        ResponeSMS+="\nSNO:";ResponeSMS.concat(ID_OUT+konversi::toIEEE(posix_get_2_RegisterOut(0x02,0x04,0x0578,0x0002)));
        ResponeSMS+="\nRESET SISTEM";
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
        delay (20000);
      }
}//SendMessage()
*/