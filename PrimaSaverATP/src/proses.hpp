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
	}
  
  float toHUMAN(uint32_t input)
	{
		myfloat varh;
		varh.raw.mantissa = input;
		varh.raw.exponent = input>>23;
		varh.raw.sign = input>>31;
    // Serial.println(varh.f);
    return varh.f;
  }
}
