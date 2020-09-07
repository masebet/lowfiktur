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

#define EP_DEV0 130
#define EP_DEV1 132
#define EP_DEV2 133

#define EP_IDDEV0 "12345671"
#define EP_IDDEV1 "12345672"
#define EP_IDDEV2 "12345673"


SoftwareSerial ModIN(ModIN_rxPin, ModIN_txPin);

unsigned long   currentMillisTimeout           =0;
unsigned long   currentMillisSHUT              =0;
long            previousMillis                 =0;
long            previousMillisTimeout          =0;
long            previousMillisSHUT             =0;

String  modbuskwh_OUT     ="";
String  modbuskwh_IN      ="";
String  modbuskwh_BIF      ="";


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
String  ID_BIF            ="ED0";
String  KodeSMS           ="";
String  ResponeSMS        ="";
String  MOBI              ="";
String  MOBO              ="";
String  MOBF              ="";
String  FV                ="CC_2R 05/05/2020";
String  FM                ="";

char    charBuf[10];
char    kar;

uint8_t sumcpin           =0;          
uint8_t state             =0;
uint8_t i                 =0;

boolean Resetsim          =true;
boolean TTS               =false;
boolean sinc              =false;

boolean getmodbus         =false;
boolean sender            =false;
#define S1debug 0

bool RecieveMessage();
bool TimetoSend();
void SHUTBERKALA();
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
void GETmodbusTigaData();
void send2serverEbetTiga();
void GETmodbusSatu();
void send2serverEbetSatu();
void pub(String topik, String pesan, int panjang);

// void(* reset) (void) = 0;

#include "SD.h"
File myFile;

#include "proses.hpp"
#include "input.hpp"
#include "input1.hpp"
#include "sms.h"


void waitrespon(unsigned long timeOut){                       //listener sim900
  String waitr="";
  unsigned long time = millis();
  while ( (millis()-time) < timeOut ){
      if(Serial2.available()) waitr += (char) Serial2.read();
  }
  if(S1debug){}else{Serial.print("\r\nlistener sim900 : ");Serial.println(waitr);}
  if(waitr.indexOf("+")>=0)Reply=waitr;
}


void setup() {

  Serial.begin(9600);         //debug
  // Serial.begin(115200);
  Serial2.begin(9600);        //simcom
  // Serial2.begin(115200);   //simcom
  
  ModIN.begin(9600);          //Modbus1
  // Serial1.begin(9600);     //Modbus2

  clock.begin();

  pinMode(ModIN_RS485_pin, OUTPUT);
  pinMode(ModOUT_RS485_pin, OUTPUT);
  pinMode(PinResetSIM900A, OUTPUT);

  ResetSim900A();
  
  // wdt_enable(WDTO_8S);  //timeout watchdog timer 8 second

  // proses_api.tulisEprom(EP_MODE,"Satu");  
  // proses_api.tulisEprom(EP_MODE,"Tiga");  
  // proses_api.tulisEprom(EP_MODE,"G328");
  
  // proses_tulisEprom(EP_IP,"183.91.67.211");    //pasing riset text
  // proses_api.tulisEprom(EP_PORT,"15055");

  // proses_tulisEprom(EP_IP,"110.50.86.220");    //parsing gabung dev
  // proses_api.tulisEprom(EP_PORT,"9017");
 
  // proses_tulisEprom(EP_IP,"183.91.67.214");    //parsing java prod
  // proses_api.tulisEprom(EP_PORT,"9003");


  proses_tulisEprom(EP_IP,"183.91.67.214");    //parsing java prod
  proses_api.tulisEprom(EP_PORT,"9000");


  // proses_tulisEprom(EP_APN,"internet");
  // proses_tulisEprom(EP_APN,"m2minternet");


  Serial.println("Modem Prima Saver v.07");
  Serial.print("FIRMWARE VERSION:"); Serial.println(FV);

  proses_api.tulisEprom(EID_BIF,"ED0");
  proses_api.tulisEprom(EID_IN,"ED1");
  proses_api.tulisEprom(EID_OUT,"ED2");

  proses_api.tulisEprom(EP_DEV0,EP_IDDEV0);
  proses_api.tulisEprom(EP_DEV1,EP_IDDEV1);
  proses_api.tulisEprom(EP_DEV2,EP_IDDEV2);

  ID_BIF    = proses_api.bacaDataEprom(EID_BIF);   
  ID_IN     = proses_api.bacaDataEprom(EID_IN);  
  ID_OUT    = proses_api.bacaDataEprom(EID_OUT);
  
  IPADDRESS = proses_bacaDataEprom(EP_IP);
  PORT      = proses_api.bacaDataEprom(EP_PORT);
  APN       = proses_bacaDataEprom(EP_APN);

  delay(5000);

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
  Serial2.print("AT+CMGDA=\"DEL ALL\"\r\n");
  waitrespon(3000);

  Serial2.print("AT+CLIP=1\r\n");
  waitrespon(3000);
  // Serial2.print("AT+CLTS=1\r\n");
  // waitrespon(3000);
  // Serial2.print("AT&W\r\n");
  // waitrespon(3000);

  delay(5000);
  waitrespon(5000);
  Serial2.print("AT+CCLK?\r\n");
  waitrespon(3000);
  if(Reply.substring(20,22).toInt()>10){
    tools::setTahun(Reply.substring(20,22).toInt()+2000);
    tools::setBulan(Reply.substring(23,25).toInt());
    tools::setHari(Reply.substring(26,28).toInt());
    tools::setJam(Reply.substring(29,31).toInt());
    tools::setMenit(Reply.substring(32,34).toInt());
    Reply.substring(35,37).toInt();
  }

  cekSignal();
  rutin::sdLoger();
  cekModbus();
  return;
}//setup()



// namespace pros{
//   unsigned long flag;
//   unsigned long timeProsFlag;
//   unsigned long lastFlag = 0;
//   bool diwaktu(unsigned long timeGet){
//     if(millis()-timeProsFlag > 100){
//         timeProsFlag = millis();
//         flag += 100;
//     }
//     if(flag>4000){flag = 0;lastFlag = 0;}
//     if(lastFlag<timeGet&&flag>timeGet){
//     lastFlag = timeGet;
//       return 1;
//     }
//     return 0;
//   }//timePros()
// }

// namespace prosNotSkip{
//   unsigned long flag;
//   unsigned long timeProsFlag; 
//   bool diwaktu(unsigned long timeGet){
//     if(millis()-timeProsFlag > 100){
//         timeProsFlag = millis();
//         flag += 100;
//     }
//     if(flag>4000){flag = 0;}
//     if(flag>timeGet)return 1;
//     return 0;
//   }//timePros()
// }

//  // while(1){
//  //   if(prosNotSkip::diwaktu(3000))Serial.print("c");
//  //   if(prosNotSkip::diwaktu(1000))Serial.print("a");
//  //   if(prosNotSkip::diwaktu(2000))Serial.print("b");
//  // }

void loop() {
  
  // sms::testKirimSms("081282603007","ini teko");
  // sms::testKirimSms("081316091495","ini teko");
  // sms::testKirimSms("085212979873","ini teko");
  // sms::testKirimSms("089605230676","ini no nya");
  
  if(tools::jamH()==23&&tools::jamM()==0&&tools::jamD()<2) asm volatile ("  jmp 0");   
  // if(tools::jamH()==0&&tools::jamM()==1&&tools::jamD()<20) reset();   
  
  rutin::tarikDataLoger();
  if(tools::Tahun()>2010) rutin::reset(1,10,00);

  // Serial2.print("AT+CMGR=1\r\n");
  waitrespon(3000);

  if(Reply.indexOf("RING")>=0){
    Serial2.print("ATH0\r\n");
    waitrespon(3000);
    sms::getNomor(Reply);
    // sms::testKirimSms(sender_phone,"test");
    Reply = " LIST";
    sms::fikturSMS();
    sms::kirimPesan();
  }

  TimetoSend();
  if (getmodbus){
    getmodbus=false;
    cekSignal();
    GETmodbusTigaData();
    send2serverEbetTiga();

    if(sms::RecieveMessage()){
      sms::fikturSMS();
      sms::kirimPesan();
    }
  }

  FM="";
  FM.concat(freeMemory());
  ResetSim900A();
}//loop()

unsigned long timeFlag;
bool TimetoSend(){
  // wdt_reset();  
  if (millis() - previousMillis > 180000) {
      previousMillis = millis();
      if(S1debug){}else{Serial.println("time to send");}
      getmodbus = true;
      TTS       = true;
      sinc      = true;
  }

  if(millis()-timeFlag > (600000)){
    timeFlag = millis();
    if(S1debug){}else{Serial.println("time to Reset");}
    Resetsim = true;
    Serial2.print("AT+CREG=1\r\n");
    waitrespon(3000);
    Serial2.print("AT+CCLK?\r\n");
    waitrespon(3000);
    if(Reply.substring(20,22).toInt()>10){
      tools::setTahun(Reply.substring(20,22).toInt()+2000);
      tools::setBulan(Reply.substring(23,25).toInt());
      tools::setHari(Reply.substring(26,28).toInt());
      tools::setJam(Reply.substring(29,31).toInt());
      tools::setMenit(Reply.substring(32,34).toInt());
      Reply.substring(35,37).toInt();
    }
    
    Serial2.print("AT+CLIP=1\r\n");
    waitrespon(3000);
    Serial2.print("AT+CLTS=1\r\n");
    waitrespon(3000);
    
    pub(MOBI,"hai",3);
    cekSignal();
    rutin::sdLoger();
    cekModbus();
    return 1;
  }
  return 0;
}//TimetoSend()


void GETmodbusTigaData(){
  // wdt_reset();
   modbuskwh_BIF ="";
   modbuskwh_IN  ="";
   modbuskwh_OUT ="";
   modbuskwh_BIF.concat(input_getDataSensor0());
   modbuskwh_IN.concat(input_getDataSensor1());
   modbuskwh_OUT.concat(input_getDataSensor2());
   
   rutin::logerData(modbuskwh_BIF+modbuskwh_IN+modbuskwh_OUT);
   if(S1debug){}else{Serial.print("modbuskwh_BI:");Serial.println(modbuskwh_BIF);}
   if(S1debug){}else{Serial.print("modbuskwh_IN:");Serial.println(modbuskwh_IN );}
   if(S1debug){}else{Serial.print("modbuskwh_OT:");Serial.println(modbuskwh_OUT);}
}//GETmodbus()


void send2serverEbetTiga(){
  for(int i=0;i<5;i++){
    SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
    SendCommand("AT+CIPSHUT\r\n",500,S1debug);
    SendCommand("AT+CIPMUX=1\r\n",500,S1debug);
    SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
    SendCommand("AT+CGATT=1\r\n",500,S1debug);
    SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",500,S1debug);
    SendCommand("AT+CIICR\r\n",3000,S1debug);
    SendCommand("AT+CIFSR\r\n",500,S1debug);
    SendCommand("AT+CIPSTART=0,\"TCP\",\""+IPADDRESS+"\",\""+PORT+"\"\r\n",1000,S1debug);
    read: SendCommand("AT+CIPSTATUS=0\r\n",1000,S1debug);
    if(Reply.indexOf("RING")>=0)break;
    if(Reply.indexOf("CONNECTING")>0)goto read;
    if(Reply.indexOf("CONNECTED")>0)break;
  }

    SendCommand("AT+CIPSEND=0\r\n",1000,S1debug);
    SendCommand(modbuskwh_BIF,1000,S1debug);
    SendCommand(modbuskwh_IN,1000,S1debug);
    SendCommand(modbuskwh_OUT,1000,S1debug);
    SendCipSend(1500);
}//send2serverEbetTiga()

// // bool SendCommand1(String command, unsigned int timeout, boolean debug);
// void send2serverEbetTiga1(){
//     for(int i=0;i<5;i++){
//       SendCommand("AT+CGDCONT=1,\"IP\",\"m2minternet\"\r\n",500,S1debug);
//       SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
//       SendCommand("AT+NETCLOSE\r\n",500,S1debug);
//       SendCommand("AT+CSOCKSETPN=1\r\n",500,S1debug);
//       SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
//       SendCommand("AT+NETOPEN\r\n",500,S1debug);
//       SendCommand("AT+IPADDR\r\n",500,S1debug);
//       SendCommand("AT+CIPOPEN=0,\"TCP\",\""+IPADDRESS+"\","+PORT+"\r\n",1000,S1debug);      
//       SendCommand("AT+CIPOPEN?\r\n",500,S1debug);
//       if(Reply.indexOf("-1")>0)break;
//     }

//       SendCommand("AT+CIPSEND=0,\r\n",500,S1debug);
//       SendCommand(modbuskwh_BIF,500,S1debug);
//       SendCommand(modbuskwh_IN,500,S1debug);
//       SendCommand(modbuskwh_OUT,500,S1debug);
//       SendCipSend(1000);
//   return;
// }//send2serverEbetTiga()
// bool SendCommand1(String command, unsigned int timeout, boolean debug)
// {
//   Serial2.print(command); 
//   Reply = "";
//   unsigned long time = millis();
//   while(Serial2.available()==0) {
//       if(Serial2.availableForWrite()) {
//         Reply = Serial2.readString();
//         break;
//       }
//     if((millis()-time) > 3000 ) break;
//   }
//   Serial.println(Reply);
//   return 0;
// }//SendCommand()

bool SendCommand(String command, unsigned int timeout, boolean debug)
{
  Serial2.print(command); 

  Reply = "";
  
  unsigned long time = millis();
  while ((millis()-time) < timeout ){
      if(Serial2.available()) Reply += (char) Serial2.read();//Reply = Serial2.readString();
  }
  debug=1;
  if(debug==1){
    Serial.println(Reply);
  }else{
    if      (Reply.indexOf("ERROR")>0)  { Serial.print("ERROR proses of>  "); Serial.println(command); return 0;}
    else if (Reply.indexOf("OK")>0)     { Serial.print("OK proses of>  "); Serial.println(command); return 1;}
    else    { Serial.println("SUCSES proses of>>  "); Serial.println(command); return 1;}
  }
  return 0;
}//SendCommand()

void SendCipSend(unsigned int timeout){
  // wdt_reset();
  respondsend = "";
  Serial2.write(0x1A);
  
  unsigned long time = millis();
  while ( (millis()-time) < timeout ){
      if(Serial2.available()) respondsend += (char) Serial2.read();
  }
  if(respondsend.indexOf("SEND OK")>0)respondsend="SEND OK";else respondsend="SEND fail";
  if(S1debug){}else{Serial.println(respondsend);}

}//SendCipSend()

void cekSignal(){
    SendCommand("AT+CSQ\r\n",500,S1debug);
    int f,l;
    f=Reply.indexOf("+CSQ:"); 
    l=Reply.indexOf("\n",f);
    csq = Reply.substring(f+6,l);
    if(S1debug){}else{tools::printData2();}
    if(S1debug){}else{Serial.print("CSQ : ");Serial.println(csq);}
}//cekSignal()

void cekModbus(){
  MOBF = ID_BIF+printHexDate(posix_get_2_Register(0x08,0x04,0x0578,0x0002),8);
  MOBI = ID_IN+printHexDate(posix_get_2_Register(0x01,0x04,0x0578,0x0002),8);
  MOBO = ID_OUT+printHexDate(posix_get_2_Register(0x02,0x04,0x0578,0x0002),8);

  if(S1debug){}else{Serial.println(MOBF);}
  if(S1debug){}else{Serial.println(MOBI);} 
  if(S1debug){}else{Serial.println(MOBO);}

  // if(MOBF.indexOf("00000000")>0)MOBF = "nv"; else MOBF = "v";     
  // if(MOBI.indexOf("00000000")>0)MOBI = "nv"; else MOBI = "v";     
  // if(MOBO.indexOf("00000000")>0)MOBO = "nv"; else MOBO = "v";

  // if(S1debug){}else{Serial.println(MOBF);}
  // if(S1debug){}else{Serial.println(MOBI);} 
  // if(S1debug){}else{Serial.println(MOBO);}
  return;
}//cekModbus()

void pub(String topik, String pesan, int panjang) {
  for(int i=0;i<5;i++){
    SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
    SendCommand("AT+CIPSHUT\r\n",500,S1debug);
    SendCommand("AT+CIPMUX=1\r\n",500,S1debug);
    SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
    SendCommand("AT+CGATT=1\r\n",500,S1debug);
    SendCommand("AT+CSTT=\""+APN+"\",\"\",\"\"\r\n",500,S1debug);
    SendCommand("AT+CIICR\r\n",3000,S1debug);
    SendCommand("AT+CIFSR\r\n",500,S1debug);
    SendCommand("AT+CIPSTART=0,\"TCP\",\"broker.hivemq.com\",\"1883\"\r\n",500,S1debug);
    read: SendCommand("AT+CIPSTATUS=0\r\n",1000,S1debug);
    if(Reply.indexOf("CONNECTING")>0)goto read;
    if(Reply.indexOf("CONNECTED")>0)break;
  }
  SendCommand("AT+CIPSEND=0\r\n",500,S1debug);
  Serial2.write(0x10);
  Serial2.write(0x12);
  Serial2.write(0x00);
  Serial2.write(0x04);
  Serial2.print("MQTT");
  Serial2.write(0x04);
  Serial2.write(0x02);
  Serial2.write(0x00);
  Serial2.write(0x3c);
  Serial2.write(0x00);
  Serial2.write(0x06);
  Serial2.print("ABCDEF");
  Serial2.write(0x1A);
  waitrespon(1000);

    panjang = modbuskwh_BIF.length()+modbuskwh_IN.length()+modbuskwh_OUT.length();
    int X, encodedByte;
    X = topik.length()+panjang + 2;

    SendCommand("AT+CIPSEND=0\r\n",500,S1debug);
    Serial2.write(0x30);
    do {
        encodedByte = X % 128;
        X = X / 128;
        if (X > 0) encodedByte |= 128;
         Serial2.write(encodedByte);
    }while (X > 0);

    Serial2.write(0x00);
    Serial2.write(topik.length());
    Serial2.print(topik);
    Serial2.print(modbuskwh_BIF);
    Serial2.print(modbuskwh_IN);
    Serial2.print(modbuskwh_OUT);
    Serial2.write(0x1A);
    waitrespon(3000);
}

// void pubM(String topik, String pesan, int panjang) {
//     for(int i=0;i<5;i++){
//       SendCommand("AT+CGDCONT=1,\"IP\",\"m2minternet\"\r\n",500,S1debug);
//       SendCommand("AT+CIPCLOSE=0\r\n",500,S1debug);
//       SendCommand("AT+NETCLOSE\r\n",500,S1debug);
//       SendCommand("AT+CSOCKSETPN=1\r\n",500,S1debug);
//       SendCommand("AT+CIPMODE=0\r\n",500,S1debug);
//       SendCommand("AT+NETOPEN\r\n",500,S1debug);
//       SendCommand("AT+IPADDR\r\n",500,S1debug);
//       SendCommand("AT+CIPSTART=0,\"TCP\",\"broker.hivemq.com\",\"1883\"\r\n",500,S1debug);
      
//       SendCommand("AT+CIPOPEN?\r\n",500,S1debug);
//       if(Reply.indexOf("-1")>0)break;
//     }

//     SendCommand("AT+CIPSEND=0\r\n",500,S1debug);
//     Serial2.write(0x10);
//     Serial2.write(0x12);
//     Serial2.write(0x00);
//     Serial2.write(0x04);
//     Serial2.print("MQTT");
//     Serial2.write(0x04);
//     Serial2.write(0x02);
//     Serial2.write(0x00);
//     Serial2.write(0x3c);
//     Serial2.write(0x00);
//     Serial2.write(0x06);
//     Serial2.print("ABCDEF");
//     Serial2.write(0x1A);
//     waitrespon(1000);

//     panjang = modbuskwh_BIF.length()+modbuskwh_IN.length()+modbuskwh_OUT.length();
//     int X, encodedByte;
//     X = topik.length()+panjang + 2;

//     SendCommand("AT+CIPSEND=0\r\n",500,S1debug);
//     Serial2.write(0x30);
//     do {
//         encodedByte = X % 128;
//         X = X / 128;
//         if (X > 0) encodedByte |= 128;
//          Serial2.write(encodedByte);
//     }while (X > 0);

//     Serial2.write(0x00);
//     Serial2.write(topik.length());
//     Serial2.print(topik);
//     Serial2.print(modbuskwh_BIF);
//     Serial2.print(modbuskwh_IN);
//     Serial2.print(modbuskwh_OUT);
//     Serial2.write(0x1A);
//     waitrespon(3000);
// }

void ResetSim900A(){ 
  // wdt_reset();
  if (Resetsim){
    Resetsim=false;
    Serial.println("Reset SIM900A");
    digitalWrite(PinResetSIM900A, HIGH);
    delay(1000);
    digitalWrite(PinResetSIM900A, LOW);
    delay(1000);
    digitalWrite(PinResetSIM900A, HIGH);
    delay(1000);
    // digitalWrite(PinResetSIM900A, LOW);  //  unuk sim 7600
    // delay(1000);                         //
  }
}//ResetSim900A()