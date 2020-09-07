// #include "proses.hpp"

String printHexDate(uint32_t num, int precision);
  
uint16_t crc16_update(uint16_t crc, uint8_t a) {
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

uint16_t crc16(const uint8_t *in,int load) {
  load ++;
  uint16_t crc;
  crc = 0xFFFF;
  for(int i = 0; i < load; i++) crc = crc16_update(crc, in[i]);

  return crc;
}

String printHex(uint8_t num, int precision) {
     String out;
     char tmp[16];
     char format[32];
     sprintf(format, "%%.%dX", precision);
     sprintf(tmp, format, num);
     out=tmp;
  return out;
}

String printHexDate(uint32_t num, int precision) {
     String out;
     char tmp[16];
     char format[32];
     sprintf(format, "%%.%dx", precision/2);
     sprintf(tmp, format, num);
     out=tmp;
     sprintf(format, "%%.%dx", precision/2);
     sprintf(tmp, format, num>>16);
     out+=tmp;
  return out;
}

//=================================================================================================

float posix_get_1_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2) { 
  uint8_t sent;
  uint8_t crcCalc[7];

  digitalWrite(ModIN_RS485_pin, HIGH);      // digitalWrite(10,1);

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

  delay(10);                            //harus ada delai untuk menyelamatkan bit terakhir
  digitalWrite(ModIN_RS485_pin, LOW);   // digitalWrite(10,0);
  delay(150);                           //harus ada delai untuk tunggu interup uart
  
  uint8_t dataIn[8]; String dataString="";
  int index=0;
  while(ModIN.available()){
    dataIn[index]=ModIN.read();
    dataString += printHex(dataIn[index], 2);
    index++;
  }

  float out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24| 
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8 |
                          (uint32_t)dataIn[6];

      out = (float) dataOut;
    }else{
      out = 0x0;
    }
  return out;
}//posix_get_1_Register()

uint32_t posix_get_2_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){ 
  uint8_t sent;
  uint8_t crcCalc[7];

  digitalWrite(ModIN_RS485_pin, HIGH);      // digitalWrite(10,1);

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

  delay(10);                            //harus ada delai untuk menyelamatkan bit terakhir
  digitalWrite(ModIN_RS485_pin, LOW);   // digitalWrite(10,0);
  delay(150);                           //harus ada delai untuk tunggu interup uart

  uint8_t dataIn[8]; String dataString="";
  int index=0;
  while(ModIN.available()){
    dataIn[index]=ModIN.read();
    dataString += printHex(dataIn[index], 2);
    // Serial.print(printHex(dataIn[index],2));  
    index++;
  }
  
  // Serial.println("");
  // Serial.print(printHex(crcCalc[2],2));
  // Serial.print(printHex(crcCalc[3 ],2));
  // Serial.print(">  ");  
  // Serial.print(printHex(dataIn[3],2));
  // Serial.print(printHex(dataIn[4],2));
  // Serial.print("  ");
  // Serial.print(printHex(dataIn[5],2));
  // Serial.println(printHex(dataIn[6],2));
  // Serial.print("  ");
  // Serial.print(printHex(dataIn[7],2));
  // Serial.println(printHex(dataIn[8],2));

  uint32_t out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24| 
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8 |
                          (uint32_t)dataIn[6];
      out = dataOut;
    }else{
      out = 0x0;
    }  
  return out;
}//posix_get_2_Register()

String input_getDataSensor0()
{
  uint8_t adress  = 0x08;
  String out = ID_BIF;

  out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8); 
  rutin::logerDataExel("\r\n");
  rutin::logerDataExel(out+"\t");
  rutin::logerDataExel(tools::Waktu()+"\t");
  out += tools::getDate(); 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  if(tools::tarif()=="WBP"){
      // WBP  = KWh-LWBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float WBP = KWh - proses_api.bacaDataEprom(LWBP_BIF).toFloat();
        proses_api.tulisEprom(WBP_BIF, String((float)WBP) );
        // float WBP = KWh - proses_bacaDataEprom(LWBP_BIF16).toFloat();
        // proses_tulisEprom(WBP_BIF16, String((float)WBP) );
      }
  }else{
      // LWBP = KWh-WBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float LWBP = KWh - proses_api.bacaDataEprom(WBP_BIF).toFloat();
        proses_api.tulisEprom(LWBP_BIF, String((float)LWBP) );
        // float LWBP = KWh - proses_bacaDataEprom(WBP_BIF16).toFloat();
        // proses_tulisEprom(LWBP_BIF16, String((float)LWBP) );
      }
  }   
  out += konversi::toIEEE(proses_api.bacaDataEprom(WBP_BIF).toFloat());
  out += konversi::toIEEE(proses_api.bacaDataEprom(LWBP_BIF).toFloat());  
  // out += konversi::toIEEE(proses_bacaDataEprom(WBP_BIF16).toFloat());
  // out += konversi::toIEEE(proses_bacaDataEprom(LWBP_BIF16).toFloat());  

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002)/(uint32_t)1000);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002)/(uint32_t)100);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002)/(uint32_t)10);  // V
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0004,0x0002));     // kW   // aktif-opwer
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0014,0x0002));     // kW
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0024,0x0002));     // kW
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0006,0x0002));   // kVAR   // reaktif-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0016,0x0002));   // kVAR
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0026,0x0002));   // kVAR

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x000a,0x0002));   // kVA    // aperent-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x001a,0x0002));   // kVA
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x002a,0x0002));   // kVA
  // Serial.println(out.length());
  return out;
}//input_getDataSensor0()

String input_getDataSensor1()
{
  uint8_t adress  = 0x01;
  String out = ID_IN;

  out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8);
  rutin::logerDataExel("\r\n");
  rutin::logerDataExel(out+"\t");
  rutin::logerDataExel(tools::Waktu()+"\t");
  out += tools::getDate(); 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  if(tools::tarif()=="WBP"){
      // WBP  = KWh-LWBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float WBP = KWh - proses_api.bacaDataEprom(LWBP_IN).toFloat();
        proses_api.tulisEprom(WBP_IN, String((float)WBP) );
        // float WBP = KWh - proses_bacaDataEprom(LWBP_IN16).toFloat();
        // proses_tulisEprom(WBP_IN16, String((float)WBP) );
      }
  }else{
      // LWBP = KWh-WBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
       float LWBP = KWh - proses_api.bacaDataEprom(WBP_IN).toFloat();
        proses_api.tulisEprom(LWBP_IN, String((float)LWBP) );
        // float LWBP = KWh - proses_bacaDataEprom(WBP_IN16).toFloat();
        // proses_tulisEprom(LWBP_IN16, String((float)LWBP) );
      }
  } 

  out += konversi::toIEEE(proses_api.bacaDataEprom(WBP_IN).toFloat());
  out += konversi::toIEEE(proses_api.bacaDataEprom(LWBP_IN).toFloat());
  // out += konversi::toIEEE(proses_bacaDataEprom(WBP_IN16).toFloat());
  // out += konversi::toIEEE(proses_bacaDataEprom(LWBP_IN16).toFloat());

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002)/(uint32_t)1000);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002)/(uint32_t)100);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002)/(uint32_t)10);  // V
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0004,0x0002));     // kW   // aktif-opwer
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0014,0x0002));     // kW
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0024,0x0002));     // kW
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0006,0x0002));   // kVAR   // reaktif-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0016,0x0002));   // kVAR
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0026,0x0002));   // kVAR

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x000a,0x0002));   // kVA    // aperent-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x001a,0x0002));   // kVA
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x002a,0x0002));   // kVA
  // Serial.println(out.length());
  return out;
}//input_getDataSensor()

String input_getDataSensor2()
{
  uint8_t adress  = 0x02;
  String out = ID_OUT;
  out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8);
  rutin::logerDataExel("\r\n");
  rutin::logerDataExel(out+"\t");
  rutin::logerDataExel(tools::Waktu()+"\t");
  out += tools::getDate(); 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  if(tools::tarif()=="WBP"){
      // WBP  = KWh-LWBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float WBP = KWh - proses_api.bacaDataEprom(LWBP_OUT).toFloat();
        proses_api.tulisEprom(WBP_OUT, String((float)WBP) );
        // float WBP = KWh - proses_bacaDataEprom(LWBP_OUT16).toFloat();
        // proses_tulisEprom(WBP_OUT16, String((float)WBP) );
      }
  }else{
      // LWBP = KWh-WBP;
      float KWh = (float) posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float LWBP = KWh - proses_api.bacaDataEprom(WBP_OUT).toFloat();
        proses_api.tulisEprom(LWBP_OUT, String((float)LWBP) );
        // float LWBP = KWh - proses_bacaDataEprom(WBP_OUT16).toFloat();
        // proses_tulisEprom(LWBP_OUT16, String((float)LWBP) );
      }
  }   
  out += konversi::toIEEE(proses_api.bacaDataEprom(WBP_OUT).toFloat());
  out += konversi::toIEEE(proses_api.bacaDataEprom(LWBP_OUT).toFloat());  
  
  // out += konversi::toIEEE(proses_bacaDataEprom(WBP_OUT16).toFloat());
  // out += konversi::toIEEE(proses_bacaDataEprom(LWBP_OUT16).toFloat());  

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002)/(uint32_t)1000);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002)/(uint32_t)1000);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002)/(uint32_t)100);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002)/(uint32_t)10);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002)/(uint32_t)10);  // V
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0004,0x0002));     // kW   // aktif-opwer
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0014,0x0002));     // kW
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0024,0x0002));     // kW
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0006,0x0002));   // kVAR   // reaktif-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0016,0x0002));   // kVAR
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0026,0x0002));   // kVAR

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x000a,0x0002));   // kVA    // aperent-power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x001a,0x0002));   // kVA
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x002a,0x0002));   // kVA
  // Serial.println(out.length());
  return out;
}//input_getDataSensor1()
