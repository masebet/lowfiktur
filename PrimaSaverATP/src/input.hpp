// #include "proses.hpp"

String printHexDate(uint32_t num, int precision);
  
  String
  tools_getDate()
  {
    DateTime now      = rtc.now();
    return printHexDate(now.unixtime(),8);
  }

  int
  tools_jamH()
  {
    DateTime now = rtc.now();
    return (int)now.hour();
  }

  String
  tools_tarif()
  {
   if (tools_jamH()==17)return "WBP";
   if (tools_jamH()==18)return "WBP";
   if (tools_jamH()==19)return "WBP";
   if (tools_jamH()==20)return "WBP";
   if (tools_jamH()==21)return "WBP";
   if (tools_jamH()==22)return "WBP";
   return "LWBP";
  }

uint16_t
crc16_update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= (uint16_t)a;
  for (i = 0; i < 8; ++i) {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }

  return crc;
}

uint16_t
crc16(const uint8_t *in,int load)
{
  load ++;
  uint16_t crc;
  crc = 0xFFFF;
  for(int i = 0; i < load; i++) crc = crc16_update(crc, in[i]);

  return crc;
}

String
printHex(uint8_t num, int precision)
{
     String out;
     char tmp[16];
     char format[32];
     sprintf(format, "%%.%dX", precision);
     sprintf(tmp, format, num);
     out=tmp;
  return out;
}

String
printHexDate(uint32_t num, int precision)
{
     String out;
     char tmp[16];
     char format[32];
     sprintf(format, "%%.%dX", precision/2);
     sprintf(tmp, format, num>>16);
     out=tmp;
     sprintf(format, "%%.%dX", precision/2);
     sprintf(tmp, format, num);
     out+=tmp;
  return out;
}

//=================================================================================================
String
posix_get_2_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
  uint8_t sent;
  uint8_t crcCalc[7];

  // digitalWrite(10,1);
  digitalWrite(ModIN_RS485_pin, HIGH);

  crcCalc[0]=sent=addres;
  ModIN.write(sent);
  
  crcCalc[1]=sent=func;
  ModIN.write(sent);

  crcCalc[2]=sent=reg>>8;
  ModIN.write(sent);
  crcCalc[3]=sent=reg;
  ModIN.write(sent);

  crcCalc[4]=sent=reg2>>8;
  ModIN.write(sent);
  crcCalc[5]=sent=reg2;
  ModIN.write(sent);

  uint16_t crc=crc16(crcCalc,5);
  sent=crc;
  ModIN.write(sent);
  sent=crc>>8;
  ModIN.write(sent);

  delay(10);          //harus ada delai untuk menyelamatkan bit terakhir
  // digitalWrite(10,0);
  digitalWrite(ModIN_RS485_pin, LOW);
  delay(150);         //harus ada delai untuk tunggu interup uart
  
  uint8_t dataIn[8]; String dataString="";
  int index=0;
  while(ModIN.available()){
    dataIn[index]=ModIN.read();
    dataString += printHex(dataIn[index], 2);
    index++;
  }
  
  String out="";
  if(crc16(dataIn,6)==int((dataIn[8]<<8)|dataIn[7])){
    out = printHex(dataIn[3], 2)+printHex(dataIn[4], 2)+printHex(dataIn[5], 2)+printHex(dataIn[6], 2);
    }else{
      out = "nullData"; 
    }
  return out;
}

/*
  urutan datanya adalah
  CR1 SerialID execlTime KWH Wtot WBP LWBP IR IS IT FRQ VarTot VAtot VR VS VT WR WS WT VarR VarS VarT VAR VAS VAT
  |     |       |         |
  3chr  8chr    ?char     8char and etc until end
*/

String 
input_getDataSensor()
{
  uint8_t adress  = 0x01;
  String out = ID_IN;

  out += posix_get_2_Register(adress,0x04,0x0578,0x0002); 
  out += tools_getDate(); 
  out += posix_get_2_Register(adress,0x03,0x00dc,0x0002);   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += posix_get_2_Register(adress,0x03,0x0030,0x0002);   // kW     //aktiv power

  if(tools_tarif()=="WBP"){
  // WBP  = KWHTOT-LWBP;
      proses_api.tulisEprom(20,posix_get_2_Register(adress,0x03,0x00dc,0x0002).toDouble() - proses_api.bacaDataEprom(22).toDouble());
  }else{
  // LWBP = KWHTOT-WBP;
      proses_api.tulisEprom(22,posix_get_2_Register(adress,0x03,0x00dc,0x0002).toDouble() - proses_api.bacaDataEprom(20).toDouble());
  }
    
  out += printHex(proses_api.bacaDataEprom(20).toDouble(),8);
  out += printHex(proses_api.bacaDataEprom(22).toDouble(),8);
  
  out += posix_get_2_Register(adress,0x03,0x0002,0x0002);   // A
  out += posix_get_2_Register(adress,0x03,0x0012,0x0002);   // A
  out += posix_get_2_Register(adress,0x03,0x0022,0x0002);   // A

  out += posix_get_2_Register(adress,0x03,0x003c,0x0002);///100.0;   // Hz

  out += posix_get_2_Register(adress,0x03,0x0032,0x0002);   // kVAR   //reaktiv power
  out += posix_get_2_Register(adress,0x03,0x0036,0x0002);   // kVA    //aperent power
  
  out += posix_get_2_Register(adress,0x03,0x003e,0x0002);///10.0;  // V
  out += posix_get_2_Register(adress,0x03,0x0040,0x0002);///10.0;  // V
  out += posix_get_2_Register(adress,0x03,0x0042,0x0002);///10.0;  // V
                                            // aktif-opwer
  out += posix_get_2_Register(adress,0x03,0x0004,0x0002);     // kW
  out += posix_get_2_Register(adress,0x03,0x0014,0x0002);     // kW
  out += posix_get_2_Register(adress,0x03,0x0024,0x0002);     // kW
                                          // reaktif-power
  out += posix_get_2_Register(adress,0x03,0x0006,0x0002);   // kVAR
  out += posix_get_2_Register(adress,0x03,0x0016,0x0002);   // kVAR
  out += posix_get_2_Register(adress,0x03,0x0026,0x0002);   // kVAR
                                          // aperent-power
  out += posix_get_2_Register(adress,0x03,0x000a,0x0002);   // kVA
  out += posix_get_2_Register(adress,0x03,0x001a,0x0002);   // kVA
  out += posix_get_2_Register(adress,0x03,0x002a,0x0002);   // kVA
  // Serial.println(out.length());
  return out;
}//input_getDataSensor()

//=================================================================================================
String
posix_get_2_Register0(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
  uint8_t sent;
  uint8_t crcCalc[7];

  digitalWrite(10,1);
  crcCalc[0]=sent=addres;
  Serial1.write(sent);
  
  crcCalc[1]=sent=func;
  Serial1.write(sent);

  crcCalc[2]=sent=reg>>8;
  Serial1.write(sent);
  crcCalc[3]=sent=reg;
  Serial1.write(sent);

  crcCalc[4]=sent=reg2>>8;
  Serial1.write(sent);
  crcCalc[5]=sent=reg2;
  Serial1.write(sent);

  uint16_t crc=crc16(crcCalc,5);
  sent=crc;
  Serial1.write(sent);
  sent=crc>>8;
  Serial1.write(sent);

  delay(10);          //harus ada delai untuk menyelamatkan bit terakhir
  digitalWrite(10,0);
  delay(150);         //harus ada delai untuk tunggu interup uart
  
  uint8_t dataIn[8]; String dataString="";
  int index=0;
  while(Serial1.available()){
    dataIn[index]=Serial1.read();
    dataString += printHex(dataIn[index], 2);
    index++;
  }
  
  String out="";
  if(crc16(dataIn,6)==int((dataIn[8]<<8)|dataIn[7])){
    out = printHex(dataIn[3], 2)+printHex(dataIn[4], 2)+printHex(dataIn[5], 2)+printHex(dataIn[6], 2);
    }else{
      out = "nullData"; 
    }
  return out;
}

/*
  urutan datanya adalah
  CR1 SerialID execlTime KWH Wtot WBP LWBP IR IS IT FRQ VarTot VAtot VR VS VT WR WS WT VarR VarS VarT VAR VAS VAT
  |     |       |         |
  3chr  8chr    ?char     8char and etc until end
*/

String 
input_getDataSensorOut()
{
  uint8_t adress  = 0x02;
  String out  = ID_OUT;
  out += posix_get_2_Register0(adress,0x04,0x0578,0x0002); 
  out += tools_getDate(); 
  out += posix_get_2_Register0(adress,0x03,0x00dc,0x0002);   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += posix_get_2_Register0(adress,0x03,0x0030,0x0002);   // kW     //aktiv power

   if(tools_tarif()=="WBP"){
  // WBP  = KWHTOT-LWBP;
      proses_api.tulisEprom(24,posix_get_2_Register0(adress,0x03,0x00dc,0x0002).toDouble() - proses_api.bacaDataEprom(26).toDouble());
  }else{
  // LWBP = KWHTOT-WBP;
      proses_api.tulisEprom(26,posix_get_2_Register0(adress,0x03,0x00dc,0x0002).toDouble() - proses_api.bacaDataEprom(24).toDouble());
  }
  out += printHex(proses_api.bacaDataEprom(24).toDouble(),8);
  out += printHex(proses_api.bacaDataEprom(26).toDouble(),8);  
  out += posix_get_2_Register0(adress,0x03,0x0002,0x0002);   // A
  out += posix_get_2_Register0(adress,0x03,0x0012,0x0002);   // A
  out += posix_get_2_Register0(adress,0x03,0x0022,0x0002);   // A

  out += posix_get_2_Register0(adress,0x03,0x003c,0x0002);///100.0;   // Hz

  out += posix_get_2_Register0(adress,0x03,0x0032,0x0002);   // kVAR   //reaktiv power
  out += posix_get_2_Register0(adress,0x03,0x0036,0x0002);   // kVA    //aperent power
  
  out += posix_get_2_Register0(adress,0x03,0x003e,0x0002);///10.0;  // V
  out += posix_get_2_Register0(adress,0x03,0x0040,0x0002);///10.0;  // V
  out += posix_get_2_Register0(adress,0x03,0x0042,0x0002);///10.0;  // V
                                            // aktif-opwer
  out += posix_get_2_Register0(adress,0x03,0x0004,0x0002);     // kW
  out += posix_get_2_Register0(adress,0x03,0x0014,0x0002);     // kW
  out += posix_get_2_Register0(adress,0x03,0x0024,0x0002);     // kW
                                          // reaktif-power
  out += posix_get_2_Register0(adress,0x03,0x0006,0x0002);   // kVAR
  out += posix_get_2_Register0(adress,0x03,0x0016,0x0002);   // kVAR
  out += posix_get_2_Register0(adress,0x03,0x0026,0x0002);   // kVAR
                                          // aperent-power
  out += posix_get_2_Register0(adress,0x03,0x000a,0x0002);   // kVA
  out += posix_get_2_Register0(adress,0x03,0x001a,0x0002);   // kVA
  out += posix_get_2_Register0(adress,0x03,0x002a,0x0002);   // kVA  return out;
  return out;
}//input_getDataSensorOut()

//=================================================================================================

/*
  //   void setup() {           //==================//GAKUSAH UDAH SAMA ATP
  //     Serial.begin( 9600 );
  //     Serial1.begin( 9600 );
  //     pinMode(10,OUTPUT);
  //   }
    
  //   void loop() {           //==================//GAKUSAH UDAH
  //     Serial.println(input_getDataSensor());
  //     delay(10);
  //   }
*/

/*
  // adress       =  0x01;
  // data["VRS1"]   =  posix_get_2_Register(adress,0x03,0x0000,0x0002)/10.0;  // Single Phase
  // data["VST1"]   =  posix_get_2_Register(adress,0x03,0x0010,0x0002)/10.0;  //
  // data["VTR1"]   =  posix_get_2_Register(adress,0x03,0x0020,0x0002)/10.0;  //
  // adress       =  0x02;
  // data["VRS2"]   =  posix_get_2_Register(adress,0x03,0x0000,0x0002)/10.0;  // Single Phase
  // data["VST2"]   =  posix_get_2_Register(adress,0x03,0x0010,0x0002)/10.0;  //
  // data["VTR2"]   =  posix_get_2_Register(adress,0x03,0x0020,0x0002)/10.0;  //

  //  posix_get_2_Register(0x01,0x05,0x0834,0xff00);  //Deleting energies
  //  posix_get_2_Register(0x01,0x05,0x0838,0xff00);  //Deleting maximum and minimum values
  //  posix_get_2_Register(0x01,0x05,0x0839,0xff00);  //Starting maximum demand
  //  posix_get_2_Register(0x01,0x05,0x083d,0xff00);  //Deleting the hour counters (All tariffs)
  //  posix_get_2_Register(0x01,0x05,0x083f,0xff00);  //Deleting the maximum value of the maximum demand
  //  posix_get_2_Register(0x01,0x05,0x0848,0xff00);  //Deleting energies, maximum demand and maximum and minimum values
*/