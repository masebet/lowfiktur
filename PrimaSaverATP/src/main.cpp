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

#define ModIN_rxPin 11
#define ModIN_txPin 12
#define ModIN_RS485_pin 13
#define ModOUT_RS485_pin 10
#define PinResetSIM900A 9
SoftwareSerial ModIN(ModIN_rxPin, ModIN_txPin);

unsigned long   currentMillis                  =0;
unsigned long   currentMillisTimeout           =0;
unsigned long   currentMillisSHUT              =0;
long            previousMillis                 =0;
long            previousMillisTimeout          =0;
long            previousMillisSHUT             =0;

byte    data_IN1[8]       ={0x01, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x04};//[0-22 (0x01, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x04)]
byte    data_IN2[8]       ={0x01, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xc0};//[25-42(0x01, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xc0)]

byte    data_OUT1[8]      ={0x02, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x37};//[0-22(0x02, 0x03, 0x00, 0x00, 0x00, 0x16, 0xc4, 0x37)]
byte    data_OUT2[8]      ={0x02, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xf3};//[25-42(0x02, 0x03, 0x00, 0x18, 0x00, 0x12, 0x45, 0xf3)]

String  modbuskwh_OUT     ="";
String  Datakwh_OUT1      ="";
String  Datakwh_OUT2      ="";
String  modbuskwh_IN      ="";
String  Datakwh_IN1       ="";
String  Datakwh_IN2       ="";

String  IPADDRESS         ="183.91.67.211";
String  PORT              ="15055";
String  APN               ="internet";
String  cpin              ="";
String  csq               ="";
String  cipstatus         ="";
String  respondsend       ="";

String  sender_phone      ="";
String  Reply             ="";
String  Buffer            ="";
String  BufferData        ="";
String  ID_IN             ="";
String  ID_OUT            ="";
String  KodeSMS           ="";
String  ResponeSMS        ="";
String  MOBI[3]           ={};
String  MOBO[3]           ={};
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
#define S1debug true


void RecieveMessage();
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
void send2server();
void SendCommand(String command, const int timeout, boolean debug);
void SendCipSend(String command, const int timeout, boolean debug);
void Timeout();
void READYSEND();
void SendMessage();
void ResetSim900A();
//fungsi tambahan
#include "RTClib.h"
RTC_DS3231 rtc;

#include "proses.hpp"
#include "input.hpp"

//setup
void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);//modbus KWH OUTGOING
  Serial2.begin(9600);//SIM900
  ModIN.begin(9600);//modbus KWH INCOMING

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

  wdt_enable(WDTO_8S);//timeout watchdog timer 8 second
  
  Serial.println("Modem Prima Saver v.07");
  Serial.print("FIRMWARE VERSION:");Serial.println(FV);
  
  SendCommand("AT+CMGF=1\r",500,S1debug);
  SendCommand("AT+CNMI=3,3,0,0,0\r",500,S1debug);
  SendCommand("AT+CMGD=1,4\r",500,S1debug);
  //===================================================================//
  // proses_api.tulisEprom(1,"CR1");
  // proses_api.tulisEprom(11,"CR2");
  
  //read adres 0-9 (ID INCOMING)
  ID_IN = proses_api.bacaDataEprom(1);
  Serial.print("ID_IN:");Serial.println(ID_IN);//Serial.println("<=");
  //read adres 0-9 (ID INCOMING)

  //read adres 10-19 (ID OUTGOING)
  ID_OUT = proses_api.bacaDataEprom(11);
  Serial.print("ID_OUT:");Serial.println(ID_OUT);//Serial.println("<=");
  //read adres 10-19 (ID OUTGOING)
  
  //===================================================================//
  // proses_api.tulisEprom(20,200);       //WBP   IN
  // proses_api.tulisEprom(22,200);       //LWBP  IN
  // proses_api.tulisEprom(24,200);       //WBP   OUT
  // proses_api.tulisEprom(26,200);       //LWBP  OUT
  // Serial.println(printHex(proses_api.bacaDataEprom(21).toInt(),8));
  

   rtc.begin();

  //rtc.adjust(DateTime(2020, 3, 4, 16, 14, 0));    //For example to set January 27 2017 at 12:56 you would call
}

//loop
void loop() {
  wdt_reset();
  
  RecieveMessage(); 
  TimetoSend();
  SHUTBERKALA();
  
  if (getmodbus){
      GETmodbus();
      getmodbus=false;
    }
    
  send2server();
  
  FM="";
  FM.concat(freeMemory());
   
  SendMessage(); 
  ResetSim900A();

  Serial.print("freeMemory()="); Serial.println(FM);
  Serial.println();
  }//loop
  

// =======================================================================================//
//RecieveMessage
void RecieveMessage(){
  wdt_reset();
  
  String      Key         ="";
              KodeSMS     ="";
  
  SendCommand("AT+CMGR=1\r",500,S1debug);
  //Serial.println("Di Luar");
  
  FI=Reply.indexOf("1M4N1");
  //Serial.print("FI:");Serial.println(FI);
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
 }//RecieveMessage

 

//TimetoSend
void TimetoSend(){//timer waktu ambil data dari kwh dan kirim ke server
  wdt_reset();
  
  currentMillis = millis();
  //Serial.print("===========waktu:");Serial.println(currentMillis-previousMillis);
  if (currentMillis - previousMillis > 60000) {//1 menit
    Serial.println("time to send");
    Serial.println("Delete SMS Berkala per 1 menit");
    SendCommand("AT+CMGD=1,4\r",500,S1debug);
    getmodbus=true;
    TTS=true;
    sinc=true;
    previousMillis = currentMillis;
  }
}//TimetoSend

//SHUTBERKALA
void SHUTBERKALA(){
  wdt_reset();

  currentMillisSHUT = millis();
  //Serial.print("^^^^TimeSHUT:");Serial.println(currentMillisSHUT - previousMillisSHUT);
  if (currentMillisSHUT - previousMillisSHUT > 300000) {//5 menit
    Serial.println("SHUT BERKALA");
    previousMillisSHUT = currentMillisSHUT;
    state=8;
  }  
}//SHUTBERKALA

//GETmodbus
void GETmodbus(){
  wdt_reset();
  
  // for(i=0; i<3; i++){
  //   wdt_reset();
    
  //   sendData2IN1();
  //   delay (500);
  //   responeKWHIN1();
  //   sendData2IN2();
  //   delay (500);
  //   responeKWHIN2();

  //   sendData2OUT1();
  //   delay (500);
  //   responeKWHOUT1();
  //   sendData2OUT2();
  //   delay (500);
  //   responeKWHOUT2();
  // }
   modbuskwh_IN  ="";
   modbuskwh_OUT ="";
   
  //  modbuskwh_IN.concat(Datakwh_IN1);
  //  modbuskwh_IN.concat(Datakwh_IN2);

  //  modbuskwh_OUT.concat(Datakwh_OUT1);
  //  modbuskwh_OUT.concat(Datakwh_OUT2);

   modbuskwh_IN.concat(input_getDataSensor());
   modbuskwh_OUT.concat(input_getDataSensorOut());

   Serial.print("modbuskwh_IN:");Serial.println(modbuskwh_IN );
   Serial.print("modbuskwh_OT:");Serial.println(modbuskwh_OUT);
   SendDataKWH_IN         =true;
   SendDataKWH_OUT        =true;
}//GETmodbus

/*
  ///////////INCOMING 
  //sendData2IN1
  void sendData2IN1() {
    wdt_reset();
    
    Serial.println("send data IN1 0-22");
    digitalWrite(ModIN_RS485_pin, HIGH);
    delay(1);
    for (char i = 0; i < 8; i++) {
      wdt_reset();
      ModIN.write(data_IN1[i]);
      delay(1);
      Serial.print(data_IN1[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    digitalWrite(ModIN_RS485_pin, LOW);
    delay(1);
    }//sendData2IN1

  //responeKWHIN1
  void responeKWHIN1 () {
    wdt_reset();
    
    Reply         ="";
    modbuskwh_IN  ="";
    Datakwh_IN1   ="";
    Buffer        ="";
    BufferData    = "";

    while (ModIN.available() > 0) {
        byte b = ModIN.read();
        Reply += String (b, HEX);
        if (Reply.length() < 2) {
          modbuskwh_IN += "0";
          modbuskwh_IN.concat(Reply);
        }
        else {
          modbuskwh_IN.concat(Reply);
        }
        Reply = "";
      }
    //    Serial.print("modbuskwh_IN:");Serial.print(modbuskwh_IN);Serial.print("<=");
    //    Serial.println();
      
    //sortir Slave ID
    Buffer = "";
    Buffer = modbuskwh_IN.substring(0,6);
    Buffer.trim();
    //Serial.print(Buffer);
    //sortir Slave ID
      
    //validasi data INCOMING (01)
    if (Buffer == "01032c") {
      //Datakwh_IN1+="@";
      Datakwh_IN1.concat (ID_IN);
      //Datakwh_IN1+="#";
        
      //sortir 9 data
      BufferData = "";
      BufferData = modbuskwh_IN.substring(6,78);
      Datakwh_IN1.concat (BufferData);

      //sortir 1 data WS
      BufferData = "";
      BufferData = modbuskwh_IN.substring(86,94);
      Datakwh_IN1.concat (BufferData);
        
      MOBI[0]="";
      MOBI[0]+="V";
    }
    else{
      //Datakwh_IN1+="@";
      Datakwh_IN1.concat (ID_IN);
      //Datakwh_IN1+="#";
      Datakwh_IN1+="EMPTY";
      MOBI[0]="";
      MOBI[0]+="NV";
      }//validasi data INCOMING (01)

    Serial.println(Datakwh_IN1);
    Serial.println();
    }//responeKWHIN1

  //sendData2IN2
  void sendData2IN2() {
    wdt_reset();
    
    Serial.println("send data IN2 25-42");
    digitalWrite(ModIN_RS485_pin, HIGH);
    delay(1);
    for (char i = 0; i < 8; i++) {
      wdt_reset();
      ModIN.write(data_IN2[i]);
      delay(1);
      Serial.print(data_IN2[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
    digitalWrite(ModIN_RS485_pin, LOW);
    delay(1);
    }//sendData2IN2

  //responeKWHIN2
  void responeKWHIN2 () {
    wdt_reset();
    
    Reply         ="";
    modbuskwh_IN  ="";
    Datakwh_IN2   ="";
    Buffer        ="";
    BufferData    ="";

    while (ModIN.available() > 0) {
        byte b = ModIN.read();
        Reply += String (b, HEX);
        if (Reply.length() < 2) {
          modbuskwh_IN += "0";
          modbuskwh_IN.concat(Reply);
        }
        else {
          modbuskwh_IN.concat(Reply);
        }
        Reply = "";
      }
      //Serial.print(modbuskwh_IN);
      //Serial.println();
      
    //sortir Slave ID
    Buffer = "";
    Buffer = modbuskwh_IN.substring(0,6);
    Buffer.trim();
    //Serial.print(Buffer);
    //sortir Slave ID
      
    //validasi data INCOMING (01)
    if (Buffer == "010324") {    
      //sortir 9 data
      BufferData = "";
      BufferData = modbuskwh_IN.substring(6,78);
      Datakwh_IN2.concat (BufferData);
      //Datakwh_IN2+="--";
      MOBI[1]="";
      MOBI[1]+="V";
    }
    else{
      Datakwh_IN2+="EMPTY";
      MOBI[1]="";
      MOBI[1]+="NV";
    }
      //validasi data INCOMING (01)

    Serial.println(Datakwh_IN2);
    Serial.println();
  }//responeKWHIN2
  ///////////INCOMING 
    
  ///////////OUTGOING 
  //sendData2OUT1
  void sendData2OUT1() {
    wdt_reset();
    
    Serial.println("send data OUT1 0-22");
    digitalWrite(ModOUT_RS485_pin, HIGH);
    delay(1);
    for (char i = 0; i < 8; i++) {
      wdt_reset();
      Serial1.write(data_OUT1[i]);
      delay(1);
      Serial.print(data_OUT1[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
    digitalWrite(ModOUT_RS485_pin, LOW);
    delay(1);
  }//sendData2OUT1
  //responeKWHOUT1
  void responeKWHOUT1 () {
    wdt_reset();
    
    Reply         ="";
    modbuskwh_OUT ="";
    Datakwh_OUT1  ="";
    Buffer        ="";
    BufferData    = "";

    while (Serial1.available() > 0) {
        byte b = Serial1.read();
        Reply += String (b, HEX);
        if (Reply.length() < 2) {
          modbuskwh_OUT += "0";
          modbuskwh_OUT.concat(Reply);
        }
        else {
          modbuskwh_OUT.concat(Reply);
        }
        Reply = "";
      }
    //    Serial.print("modbuskwh_OUT:");Serial.print(modbuskwh_OUT);Serial.print("<=");
    //    Serial.println();
          
    //sortir Slave ID
    Buffer = "";
    Buffer = modbuskwh_OUT.substring(0,6);
    Buffer.trim();
    //Serial.print(Buffer);
    //sortir Slave ID
      
    //validasi data OUTGOING (02)
    if (Buffer == "02032c") {
      //Datakwh_OUT1+="@";
      Datakwh_OUT1.concat (ID_OUT);
      //Datakwh_OUT1+="#";
        
      //sortir 9 data
      BufferData = "";
      BufferData = modbuskwh_OUT.substring(6,78);
      Datakwh_OUT1.concat (BufferData);

      //sortir 1 data WS
      BufferData = "";
      BufferData = modbuskwh_OUT.substring(86,94);
      Datakwh_OUT1.concat (BufferData);
      MOBO[0]="";
      MOBO[0]+="V";      
      }
    else{
      //Datakwh_OUT1+="@";
      Datakwh_OUT1.concat (ID_OUT);
      //Datakwh_OUT1+="#";
      Datakwh_OUT1+="EMPTY";
      MOBO[0]="";
      MOBO[0]+="NV";
      }//validasi data OUTGOING (02)

      Serial.println(Datakwh_OUT1);
      Serial.println();
    }//responeKWHOUT1

        
  //sendData2OUT2
  void sendData2OUT2() {
    wdt_reset();
    
    Serial.println("send data OUT2 25-42");
    digitalWrite(ModOUT_RS485_pin, HIGH);
    delay(1);
    for (char i = 0; i < 8; i++) {
      wdt_reset();
      Serial1.write(data_OUT2[i]);
      delay(1);
      Serial.print(data_OUT2[i], HEX);
      Serial.print(" ");
    }

    Serial.println();
    digitalWrite(ModOUT_RS485_pin, LOW);
    delay(1);
    }//sendData2OUT2
  //responeKWHOUT2
  void responeKWHOUT2 () {
    wdt_reset();
    
    Reply         ="";
    modbuskwh_OUT ="";
    Datakwh_OUT2  ="";
    Buffer        ="";
    BufferData    = "";

      while (Serial1.available() > 0) {
        byte b = Serial1.read();
        Reply += String (b, HEX);
        if (Reply.length() < 2) {
          modbuskwh_OUT += "0";
          modbuskwh_OUT.concat(Reply);
        }
        else {
          modbuskwh_OUT.concat(Reply);
        }
        Reply = "";
      }
      //Serial.print(modbuskwh_OUT);
      //Serial.println();
      
      //sortir Slave ID
      Buffer = "";
      Buffer = modbuskwh_OUT.substring(0,6);
      Buffer.trim();
      //Serial.print(Buffer);
      //sortir Slave ID
      
      //validasi data OUTGOING (02)
      if (Buffer == "020324") {   
        //sortir 9 data
        BufferData = "";
        BufferData = modbuskwh_OUT.substring(6,78);
        Datakwh_OUT2.concat (BufferData);
        //Datakwh_OUT2+="++";
        MOBO[1]="";
        MOBO[1]+="V";
      }
      else {
        Datakwh_OUT2+="EMPTY";
        MOBO[1]="";
        MOBO[1]+="NV";      
      }//validasi data OUTGOING (02)

      Serial.println(Datakwh_OUT2);
      Serial.println();
    }//responeKWHOUT2
  ///////////OUTGOING  
*/

//=========================================================================================//
//send2server
void send2server(){
  wdt_reset();
  
  switch(state){
    case 0:
    SendCommand("AT+CPIN?",250,S1debug);
    SendCommand("AT+CSQ",500,S1debug);
    SendCommand("AT+CREG?",500,S1debug);
    
    if(cpin=="READY"){
      sumcpin=0;
      }
    else{
      if (sumcpin>5){
        Resetsim=true;
        sumcpin=0;
      }
      else {
        sumcpin++;
      }
    }
    Serial.print("sumcpin=");Serial.println(sumcpin);   
    state=1;
    break;

    case 1:
    SendCommand("AT+CGATT=1",1000,S1debug);
    state=2;
    break;

    case 2:
    //SendCommand("AT+CSTT=\"internet\"",2000,S1debug);//internet kalau kartu biasa
    SendCommand("AT+CSTT=\""+APN+"\"",2000,S1debug);//internet kalau kartu biasa
    state=3;
    break;

    case 3:
    SendCommand("AT+CIICR",2000,S1debug);
    state=4;
    break;

    case 4:
    SendCommand("AT+CIFSR",2000,S1debug);
    state=5;
    break;

    case 5:
    SendCommand("AT+CIPSTART=\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"",2000,S1debug);
    state=6;
    break;

    case 6:
    SendCommand("AT+CIPSTATUS",2000,S1debug);
    if (cipstatus=="CONNECT OK"){
      state=7;
    }
    else if(cipstatus=="PDP DEACT"){
      state=8;
    }
    else{
      state=0;
    }
    break;

    case 7:
     if (TTS){
      Timeout();
      READYSEND();
    }
    state=0;
    break;

    case 8:
    SendCommand("AT+CIPSHUT",1000,S1debug);
    state=0;
    break;
  }
}

//SendCommand
void SendCommand(String command, const int timeout, boolean debug){
  wdt_reset();
    
  Reply = ""; 
      
  Serial2.println(command); 
  if (command=="AT+CPIN?")
    {
     cpin="";
     if(debug){
     long int time = millis();   
     while( (time+timeout) > millis()){
      while(Serial2.available()){       
        Reply += char(Serial2.read());
          }  
        }    
       //Serial.print("Reply:");
       //Serial.println(Reply);
       FI=0;
       LI=0;
       FI=Reply.indexOf("+CPIN:");
       LI=Reply.indexOf('Y',FI);
       cpin=Reply.substring(FI+6,LI+1);
       cpin.trim();
       if (cpin=="READY"){
        cpin="";
        cpin+="READY";
       }
       else{
        cpin="";
        cpin+="UNREADY";        
       }
       Serial.print("respon CPIN:");
       Serial.println(cpin);
     } 
    }
    
  else if(command=="AT+CSQ"){
      csq="";
      if(debug){
        long int time = millis();   
        while( (time+timeout) > millis()){
          while(Serial2.available()){       
           Reply += char(Serial2.read());
          }  
        }    
       //Serial.print("Reply:");
       //Serial.println(Reply);
       FI=0;
       LI=0;
       FI=Reply.indexOf("+CSQ:");
       LI=Reply.indexOf(',',FI);
       csq=Reply.substring(FI+6,LI+3);
       csq.trim();
       Serial.print("response CSQ:");
       Serial.println(csq);
     } 
    } 
    
  else if(command=="AT+CIPSTATUS")
    {
     cipstatus="";
     if(debug){
        long int time = millis();   
        while( (time+timeout) > millis()){
          while(Serial2.available()){       
           Reply += char(Serial2.read());
          }  
        }    
       //Serial.print("response:");
       //Serial.println(response);
       FI=0;
       LI=0;
       FI=Reply.indexOf("STATE:");
       //LI=response.indexOf('1',FI);
       cipstatus=Reply.substring(FI+7,FI+7+15);
       cipstatus.trim();
       Serial.print("response cipstatus:");
       Serial.println(cipstatus);//Serial.println("<==END");
     }
    }
    
  else{  
      if(debug){
        long int time = millis();   
        while( (time+timeout) > millis()){
          while(Serial2.available()){       
          Reply += char(Serial2.read());
         }  
       }    
       //Serial.print("response:");
       Serial.println(Reply);
      } 
     } 
}//SendCommand

//SendCipSend
void SendCipSend(String command, const int timeout, boolean debug){
  wdt_reset();
     
  Reply = "";
  FI=0;
  LI=0;    
    
  Serial2.println(command); 
  if(debug){
    respondsend="";
    long int time = millis();   
    while( (time+timeout) > millis()){
      while(Serial2.available()){       
        Reply += char(Serial2.read());
        }  
     }    
    //Serial.print("response:");
    //Serial.println(Reply);
    FI=0;
    LI=0;
    FI=Reply.indexOf("SEND");
    LI=Reply.indexOf("OK",FI);
    respondsend=Reply.substring(FI,LI+2);
    respondsend.trim();
    Serial.print("response cipsend:");
    Serial.print(respondsend);Serial.println("<==END");
  } 
}//SendCipSend

//Timeout
void Timeout(){//buat membatasi waktu pengiriman
  wdt_reset();
  
  if (sinc){
    previousMillisTimeout=currentMillis;
    sinc=false;
  }
  currentMillisTimeout = millis();
  //Serial.print("++++++TimeOut:");Serial.println(currentMillisTimeout - previousMillisTimeout);
  if (currentMillisTimeout - previousMillisTimeout > 30000) {//0.5 menit
    Serial.println("time out send");
    TTS=false;
    getmodbus=false;
    previousMillisTimeout = currentMillisTimeout;
  }
}//Timeout


//READYSEND
void READYSEND(){//ini fungsi send data ke server  
  wdt_reset();
  
  if(SendDataKWH_IN){
    SendCommand("AT+CIPSEND",1000,S1debug);
    SendCommand(modbuskwh_IN,2000,S1debug);
    SendCipSend(String ((char)26),6000,S1debug);
    Serial.println("SEND Datakwh_IN"); 
    if (respondsend=="SEND OK"){
      SendDataKWH_IN=false;
      Serial.println("succes SEND Datakwh_IN");
    }
    else{
      SendDataKWH_IN=true;
    }   
  }   
  
  if(SendDataKWH_OUT){
    SendCommand("AT+CIPSEND",1000,S1debug);
    //SendCommand(Datakwh_OUT1,2000,S1debug);
    SendCommand(modbuskwh_OUT,2000,S1debug);
    SendCipSend(String ((char)26),6000,S1debug); 
    Serial.println("SEND Datakwh_OUT"); 
    if (respondsend=="SEND OK"){
      SendDataKWH_OUT=false;
      Serial.println("succes SEND Datakwh_OUT");
    }
    else{
      SendDataKWH_OUT=true;
    } 
  }

  if (SendDataKWH_IN==false && SendDataKWH_OUT==false){
    TTS=false;
    Serial.println("ALL DATA SEND SUCCES");
  }
}//READYSEND

//SendMessage
void SendMessage(){
  wdt_reset();
  
  ResponeSMS="";
  
  if (sender){
      if (KodeSMS=="LIST"){
        ResponeSMS+="SNI:";ResponeSMS.concat(ID_IN);
        ResponeSMS+="\nSNO:";ResponeSMS.concat(ID_OUT);
        ResponeSMS+="\nSIM:";ResponeSMS.concat(cpin);
        ResponeSMS+="\nSQ :";ResponeSMS.concat(csq);
        ResponeSMS+="\nSTA:";ResponeSMS.concat(cipstatus);
        ResponeSMS+="\nMBI:";ResponeSMS.concat(MOBI[0]);ResponeSMS+=" ";ResponeSMS.concat(MOBI[1]);
        ResponeSMS+="\nMBO:";ResponeSMS.concat(MOBO[0]);ResponeSMS+=" ";ResponeSMS.concat(MOBO[1]);
        ResponeSMS+="\nFM :";ResponeSMS.concat(FM);
        ResponeSMS+="\nFV :";ResponeSMS.concat(FV);
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
      }
    
      else if (KodeSMS=="SNI" || KodeSMS=="SNO"){
        //tulis dulu SNI (Serial Number Incoming) pada EEPROM
    
        //kemudian bales smsnya
        ResponeSMS+="SNI:";ResponeSMS.concat(ID_IN);
        ResponeSMS+="\nSNO:";ResponeSMS.concat(ID_OUT);
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
      }
    
      else if(KodeSMS=="IPRESET"){
        //balas dulu sms baru reset
        ResponeSMS+="SNI:"; ResponeSMS.concat(ID_IN);
        ResponeSMS+="\nSNO:";ResponeSMS.concat(ID_OUT);
        ResponeSMS+="\nRESET SISTEM";
        
        SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
        SendCommand(ResponeSMS,500,S1debug);
        SendCommand(String ((char)26),2000,S1debug);
    
        KodeSMS="";
        sender=false;
        delay (20000);
      }
  }
}//SendMessage

//resetSIM
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
}//resetSIM