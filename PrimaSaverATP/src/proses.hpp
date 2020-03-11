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

namespace forDebug
{
  void cobaEprom(){
    //===================================================================//
  // proses_api.tulisEprom(20,200);       //WBP_IN
  // proses_api.tulisEprom(22,200);       //LWBP_IN
  // proses_api.tulisEprom(24,200);       //WBP_OUT
  // proses_api.tulisEprom(26,200);       //LWBP_OUT
  // Serial.println(printHex(proses_ap(i.bacaDataEprom(21).toInt(),8));
  }

  void cobaConverter(){
      while(1){
      // String data = konversi::IEEE(-2.88);
      // konversi::HUMAN(data);
      // float a   = posix_get_2_Register(0x02,0x04,0x0578,0x0002)/10.0;
      // String b  = String(a);
      // float c   = b.toFloat();
      // Serial.println(b);
      // Serial.println(a);
      // Serial.println(c); 
      // Serial.println(konversi::toIEEE(a));
      //Serial.println(input_getDataSensor());
      delay(1000);
    }
  }
}