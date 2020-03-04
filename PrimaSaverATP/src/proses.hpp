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