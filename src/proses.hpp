#include <Arduino.h>
#include <avr/eeprom.h> //========================================= EPROM API 

class DataEprom{
  private:                                                              //==========contoh=============
  int c;                                                                // harus masuk fungsi
  public:                                                               //
  int a;                                                                // use DataEprom dataEprom; f{dataEprom.a=13;}
  void inData(int a){c=a;}                                              //
  void printa(){Serial.print(c);Serial.print("  ");Serial.println(a);}  //
  
  String 
  bacaDataEprom(int ngone)
  {
    String hasile="";
    unsigned int adde; 
    char dat[10]="";
    byte e;
    ngone=ngone*8;
    for(int i=0;i<8;i++){
      adde = ngone + i;
      e = eeprom_read_byte((unsigned char *) adde);
      dat[i]= (char )e;
    }
    hasile = (String) dat;
    return hasile;
  }
  
  void 
  tulisEprom(int ngone,String isine)
  {   
    String hasile="";
    unsigned int adde; 
    ngone=ngone*8;
    for(int i=0;i<8;i++) {
      adde= ngone +i;
      eeprom_write_byte((unsigned char *) adde,isine[i]);
    }
  }

  void
  tulisEprom(int ngone,int isine)
  {   
    String hasile="";
    unsigned int adde; 
    String isines = String(isine);
    ngone=ngone*8;
    for(int i=0;i<8;i++) {
      adde= ngone +i;
      eeprom_write_byte((unsigned char *) adde,isines[i]);
    }
  }

};
DataEprom proses_api;


  inline void 
  proses_tulisEprom(int ngone,String isine)
  {   
    String hasile="";
    unsigned int adde; 
    ngone=ngone*8;
    for(int i=0;i<15;i++) {
      adde= ngone +i;
      eeprom_write_byte((unsigned char *) adde,isine[i]);
    }
  }

  inline String 
  proses_bacaDataEprom(int ngone)
  {
    String hasile="";
    unsigned int adde; 
    char dat[20]="";
    byte e;
    ngone=ngone*8;
    for(int i=0;i<15;i++){
      adde = ngone + i;
      e = eeprom_read_byte((unsigned char *) adde);
      dat[i]= (char )e;
    }
    hasile = (String) dat;
    return hasile;
  }




#include <stdio.h>
#include <stdint.h>
#include <math.h>

String printHexDate(uint32_t num, int precision);

namespace konversi
{
	typedef union {

		float f;
		struct
		{
			// Order is important.
			// Here the members of the union data structure
			// use the same memory (32 bits).
			// The ordering is taken
			// from the LSB to the MSB.
			unsigned long mantissa : 23;
			unsigned int exponent : 8;
			unsigned int sign : 1;

		} raw;
	} myfloat;

	// Driver Code
	String toIEEE(float input)
	{
    //Serial.println(input);
		myfloat var;
    var.f = input;
    uint32_t output = (uint32_t)var.raw.sign << 31|(uint32_t)var.raw.exponent<<23|(uint32_t)var.raw.mantissa;
    // Serial.println(output,HEX);
    return printHexDate(output,8);
	}//toIEEE()
  
  float toHUMAN(uint32_t input)
	{
		myfloat varh;
		varh.raw.mantissa = input;
		varh.raw.exponent = input>>23;
		varh.raw.sign = input>>31;
    // Serial.println(varh.f);
    return varh.f;
  }//toHUMAN()
}//konversi

/*
#include "RTClib.h"
RTC_DS3231 rtc;
namespace tools
{
  String getDate()
  {
    DateTime now = rtc.now();
    return printHexDate(now.unixtime()-820454400,8); //data dikurangi 26 tahun
  }

  int jamH()
  {
    DateTime now = rtc.now();
    return (int)now.hour();
  }

  int jamM()
  {
    DateTime now = rtc.now();
    return (int) now.minute();
  }

  int jamD(){
    DateTime now = rtc.now();
    return (int) now.second();  
  }

  int Hari()
  {
    DateTime now = rtc.now();
    return (int) now.day();
  }

  int Bulan()
  {
    DateTime now = rtc.now();
    return (int) now.month();
  }

  int Tahun()
  {
    DateTime now = rtc.now();
    return (int) now.year();
  }

  String Waktu()
  {
    DateTime now = rtc.now();
    return String(now.day())+"-"+String(now.month())+"-"+String(now.year());
  }

  String tarif()
  {
    if (jamH()==17)return "WBP";
    if (jamH()==18)return "WBP";
    if (jamH()==19)return "WBP";
    if (jamH()==20)return "WBP";
    if (jamH()==21)return "WBP";
    if (jamH()==22)return "WBP";
    return "LWBP";
  }

}//tools
*/

#include <DS3231.h>
DS3231 clock;
RTCDateTime dt;
namespace tools
{
  String getDate()
  {
    dt = clock.getDateTime();
    return printHexDate(dt.unixtime-820454400,8); //data dikurangi 26 tahun
  }

  int jamH()
  {
    RTCDateTime now = clock.getDateTime();
    return (int) now.hour;
  }

  int jamM()
  {
    RTCDateTime now = clock.getDateTime();
    return (int) now.minute;
  }

  int jamD(){
    RTCDateTime now = clock.getDateTime();
    return (int) now.second;  
  }

  int Hari()
  {
  RTCDateTime now = clock.getDateTime();
    return (int) now.day;
  }

  int Bulan()
  {
  RTCDateTime now = clock.getDateTime();
    return (int) now.month;
  }

  int Tahun()
  {
    RTCDateTime now = clock.getDateTime();
    return (int) now.year;
  }

  String Waktu()
  {
    RTCDateTime now = clock.getDateTime();
    return String(now.day)+"/"+String(now.month)+"/"+String(now.year);
  }

  String tarif()
  {
    if (jamH()==17)return "WBP";
    if (jamH()==18)return "WBP";
    if (jamH()==19)return "WBP";
    if (jamH()==20)return "WBP";
    if (jamH()==21)return "WBP";
    if (jamH()==22)return "WBP";
    return "LWBP";
  }

  void setHari(uint8_t in){
    clock.setDateTime(Tahun(), Bulan(), in, jamH(), jamM(), 0);
  }

  void setBulan(uint8_t in){
    clock.setDateTime(Tahun(), in, Hari(), jamH(), jamM(), 0);
  }

  void setTahun(uint16_t in){
    clock.setDateTime(in, Bulan(), Hari(), jamH(), jamM(), 0);
  }

  void setJam(uint8_t in){
    clock.setDateTime(Tahun(), Bulan(), Hari(), in, jamM(), 0);
  }

  void setMenit(uint8_t in){
    clock.setDateTime(Tahun(), Bulan(), Hari(), jamH(), in, 0);
  }


  boolean isAlarm = false;
  void alarmFunction()
  {
    Serial.println("*** INT PIN 3 ***");
    isAlarm = true;
  }

  bool alaramIsOn(){
    if (tools::isAlarm){
       //Serial.println(clock.readRegister8Ebet(0xE),BIN);
      tools::isAlarm = !tools::isAlarm;
      clock.clearAlarm1();
      return 1;
    }else{
      return 0;
    }
  }
  
  void alaramBegin(){
    clock.begin();
    clock.armAlarm1(false);
    clock.clearAlarm1();
    clock.writeRegister8Ebet(0x0E,0b00000111); //0b00001110

    // Serial.print(clock.readRegister8Ebet(0x00),BIN);
    // Serial.print(clock.readRegister8Ebet(0x01),BIN);
    // Serial.println(clock.readRegister8Ebet(0x02),BIN);

    // setTahun(2020);                      //
    // setBulan(6);                         //
    // setHari(1);                          //
    // setJam(10);                          //
    // setMenit(0);

    // setTahun(2020);                      //
    // setBulan(6);                         //
    // setHari(17);                          //
    // setJam(10);                          //
    // setMenit(55);

    clock.setAlarm1(1,10,2,0, DS3231_MATCH_DY_H_M_S);
  }

  void printData2(){
    Serial.print(tools::Hari());Serial.print("/");
    Serial.print(tools::Bulan());Serial.print("/");
    Serial.print(tools::Tahun());Serial.print("  ");
    Serial.print(tools::jamH());Serial.print(":");
    Serial.print(tools::jamM());Serial.print(":");
    Serial.print(tools::jamD());Serial.print("  ");
    Serial.print("CSQ : ");
    // Serial.print(tools::getDate();Serial.print("  ");
    // dt = clock.getDateTime();
    // Serial.print(dt.unixtime);Serial.print("  ");
    // Serial.println();
  }

}//tools


float posix_get_1_RegisterOut(uint8_t addres, uint8_t func, uint16_t reg, uint16_t reg2);
float posix_get_1_Register(uint8_t addres, uint8_t func, uint16_t reg, uint16_t reg2);

namespace rutin
{
    void reset(){
      if(tools::Hari()==1&&tools::jamH()==10){
        posix_get_1_Register(0x01,0x05,0x0834,0xFF00);
        posix_get_1_RegisterOut(0x02,0x05,0x0834,0xFF00);
        proses_api.tulisEprom(WBP_IN, String(0.0));
        proses_api.tulisEprom(LWBP_IN, String(0.0));
        proses_api.tulisEprom(WBP_OUT, String(0.0));
        proses_api.tulisEprom(LWBP_OUT, String(0.0));
        
        for(int b=1;b<=12;b++){
          String dir =String(tools::Tahun()-2);
          for(int i=1;i<=31;i++){
            SD.remove(dir+"/"+String(b)+"/"+String(i)+".TXT");   
          }
          SD.rmdir(dir+"/"+String(b));
          SD.rmdir(dir);
        }//for
      }
    }//reset()

   void resetMeteranDanEprom(){
      // Serial.print(tools::Hari());Serial.print("  ");
      // Serial.print(tools::jamH());Serial.print(":");
      // Serial.print(tools::jamM());Serial.print(":");
      // Serial.println(tools::jamD());

      if(tools::Hari()==1){
        if(tools::jamH()==10&&tools::jamM()==0&&tools::jamD()<30){
          reset();
        }//if
      }//if
    }//resetMeteranDanEprom()

   void sdLoger(){
    if (SD.begin(PIN_CS)) {
      sdCard = "SDCARD ON";
    }else{
      sdCard = "SDCARD OFF";  
    }
    Serial.println(sdCard);
   }//sdLoger()

   void logerData(String in){

      String dir = String(tools::Tahun());
      if(!SD.exists(dir)){
        SD.mkdir(dir);
      }
      dir = String(tools::Tahun())+"/"+String(tools::Bulan());
      if(!SD.exists(dir)){
      SD.mkdir(dir);  
      }
      
      myFile = SD.open(String(tools::Tahun())+"/"+String(tools::Bulan())+"/"+String(tools::Hari())+".TXT", FILE_WRITE);
      if (myFile) {
        myFile.println(in);
        myFile.close();
      }

   }//logerData()

   void tarikDataLoger(){
    String in = "DATA#0/0/0;";
     if(Serial.available()){
       in = Serial.readString();
        int f,l;
        f=in.indexOf("#"); 
        l=in.indexOf(";");
        String data = in.substring(f+1,l);
      
        myFile = SD.open(data+".TXT");
        
        Serial.println("<START>");
        if (myFile) {
          while (myFile.available()) {
            char buffData = myFile.read();
            Serial.write(buffData);
            if(buffData =='\n')break;
          }
          myFile.close();
        } 
        Serial.println("<END>");
      }
   }//tarikDataLoger()

   void cekSDCard(){
    if (SD.begin(PIN_CS)) {
      Serial.println("ini bisa");
    }else{
      Serial.println("ini tidak bisa");  
    }
   }//cekSDCard()
}//rutin