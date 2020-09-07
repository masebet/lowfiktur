#include <Arduino.h>
#include <avr/eeprom.h> //========================================= EPROM API 

namespace rutin{
  void logerDataExel(String in);
}

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


  inline void proses_tulisEprom(int ngone,String isine)
  {   
    String hasile="";
    unsigned int adde; 
    ngone=ngone*8;
    for(int i=0;i<15;i++) {
      adde= ngone +i;
      eeprom_write_byte((unsigned char *) adde,isine[i]);
    }
  }

  inline String proses_bacaDataEprom(int ngone)
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

namespace konversi {
	typedef union {

		float f;
		struct
		{
			unsigned long mantissa : 23;
			unsigned int exponent : 8;
			unsigned int sign : 1;

		} raw;
	} myfloat;

  uint32_t toHUMAN(uint32_t input)
	{
		myfloat varh;
		varh.raw.mantissa = input;
		varh.raw.exponent = input>>23;
		varh.raw.sign = input>>31;
    return varh.f;
  }//toHUMAN()

	String toIEEE(uint32_t input)
	{
    rutin::logerDataExel(String(input)+"\t");   
    myfloat var;
    var.f =  (float) input;
    uint32_t output = (uint32_t)var.raw.sign << 31|(uint32_t)var.raw.exponent<<23|(uint32_t)var.raw.mantissa;
    return printHexDate(output,8);
	}//toIEEE()

}//konversi

#include <DS3231.h>
DS3231 clock;
RTCDateTime dt;
namespace tools {
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
    return String(now.day)+"/"+String(now.month)+"/"+String(now.year)+" "+String(now.hour)+":"+String(now.minute)+":"+String(now.second);
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
    if(S1debug){}else{Serial.println("*** INT PIN 3 ***");}
    isAlarm = true;
    return;
  }

  bool alaramIsOn(){
    if (tools::isAlarm){
      if(S1debug){}else{Serial.println(clock.readRegister8Ebet(0xE),BIN);}
      tools::isAlarm = !tools::isAlarm;
      clock.clearAlarm1();
      return 1;
    }else{
      return 0;
    }
  }
  
  void alaramBegin(){
    // clock.begin();

    // //=====Date
    // setTahun(2020);
    // setBulan(9);
    // setHari(2);
    // //=====time
    // setJam(9);
    // setMenit(59);


    // pinMode(PIN_INT, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(PIN_INT), tools::alarmFunction, FALLING);
    // clock.writeRegister8Ebet(0x0E,0b00000111); //0b00001110
    // clock.armAlarm1(false);
    // clock.clearAlarm1();
    // clock.setAlarm1(1,10,0,15, DS3231_MATCH_DY_H_M_S);


    // Serial.print(clock.readRegister8Ebet(0x00),BIN);
    // Serial.print(clock.readRegister8Ebet(0x01),BIN);
    // Serial.println(clock.readRegister8Ebet(0x02),BIN);
  
  }

  void printData2(){
    Serial.print(tools::Hari());Serial.print("/");
    Serial.print(tools::Bulan());Serial.print("/");
    Serial.print(tools::Tahun());Serial.print("  ");

    Serial.print(tools::jamH());Serial.print(":");
    Serial.print(tools::jamM());Serial.print(":");
    Serial.print(tools::jamD());Serial.print("  ");
    // Serial.print(tools::getDate();Serial.print("  ");
    // dt = clock.getDateTime();
    // Serial.print(dt.unixtime);Serial.print("  ");
    // Serial.println();
  }

}//tools

float posix_get_1_RegisterOut(uint8_t addres, uint8_t func, uint16_t reg, uint16_t reg2);
float posix_get_1_Register(uint8_t addres, uint8_t func, uint16_t reg, uint16_t reg2);

namespace rutin {

   void reset(int hari,int jam,int menit){
      if(tools::Hari()==hari&&tools::jamH()==jam&&tools::jamM()<menit+5){
        Serial.println("reset Modbus");
        posix_get_1_Register(0x01,0x05,0x0834,0xFF00);
        posix_get_1_Register(0x02,0x05,0x0834,0xFF00);
        posix_get_1_Register(0x08,0x05,0x0834,0xFF00);
        delay(1000);
        posix_get_1_Register(0x01,0x05,0x0834,0xFF00);
        posix_get_1_Register(0x02,0x05,0x0834,0xFF00);
        posix_get_1_Register(0x08,0x05,0x0834,0xFF00);
        //posix_get_1_RegisterOut(0x02,0x05,0x0834,0xFF00);
        delay(60000);
        Serial.println("reset Eprom 8 bit");
        proses_api.tulisEprom(WBP_BIF, String(0.0));
        proses_api.tulisEprom(LWBP_BIF, String(0.0));
        proses_api.tulisEprom(WBP_IN, String(0.0));
        proses_api.tulisEprom(LWBP_IN, String(0.0));
        proses_api.tulisEprom(WBP_OUT, String(0.0));
        proses_api.tulisEprom(LWBP_OUT, String(0.0));
        delay(60000);
        Serial.println("reset Eprom 16 bit");
        proses_tulisEprom(WBP_BIF16, String(0.0));
        proses_tulisEprom(LWBP_BIF16, String(0.0));
        proses_tulisEprom(WBP_IN16, String(0.0));
        proses_tulisEprom(LWBP_IN16, String(0.0));
        proses_tulisEprom(WBP_OUT16, String(0.0));
        proses_tulisEprom(LWBP_OUT16, String(0.0));
        delay(60000);
        Serial.println("reset LOG paket");
        for(int b=1;b<=12;b++){
          String dir =String(tools::Tahun()-2);
          for(int i=1;i<=31;i++){
            SD.remove(dir+"/"+String(b)+"/"+String(i)+".TXT");   
          }
          SD.rmdir(dir+"/"+String(b));
          SD.rmdir(dir);
        }//for
        delay(60000);
        Serial.println("reset LOG EXEL");
        for(int b=1;b<=12;b++){
          String dir ="EXEL/"+String(tools::Tahun()-2);
          for(int i=1;i<=31;i++){
            SD.remove(dir+"/"+String(b)+"/"+String(i)+".TXT");   
          }
          SD.rmdir(dir+"/"+String(b));
          SD.rmdir(dir);
        }//for
        delay(60000);
      }//if
      return;
   }//reset()

   void sdLoger(){
    if (SD.begin(PIN_CS)) {
      sdCard = "SDCARD ON";
    }else{
      sdCard = "SDCARD OFF";  
    }
    if(S1debug){}else{Serial.println(sdCard);}
   }//sdLoger()

   void logerData(String in){
      if(sdCard != "SDCARD ON") return;
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

   void logerDataExel(String in){
      if(sdCard != "SDCARD ON") return;
      String dir = "EXEL";
      if(!SD.exists(dir)){
        SD.mkdir(dir);
      }

      dir = String(tools::Tahun());
      if(!SD.exists(dir)){
        SD.mkdir(dir);
      }

      dir = "EXEL/"+String(tools::Tahun())+"/"+String(tools::Bulan());
      if(!SD.exists(dir)){
      SD.mkdir(dir);  
      }
      
      myFile = SD.open("EXEL/"+String(tools::Tahun())+"/"+String(tools::Bulan())+"/"+String(tools::Hari())+".TXT", FILE_WRITE);
      if (myFile) {
        myFile.print(in);
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
            //if(buffData =='\n')break;
          }
          myFile.close();
        } 
        Serial.println("<END>");
      }
   }//tarikDataLoger()
   
}//rutin
