// #include "proses.hpp"

String printHexDate(uint32_t num, int precision);
  
uint16_t crc16_update(uint16_t crc, uint8_t a) {
  wdt_reset();
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
  wdt_reset();
  load ++;
  uint16_t crc;
  crc = 0xFFFF;
  for(int i = 0; i < load; i++) crc = crc16_update(crc, in[i]);
  return crc;
}

String printHex(uint8_t num, int precision) {
  wdt_reset();
  String out;
  char tmp[16];
  char format[32];
  sprintf(format, "%%.%dX", precision);
  sprintf(tmp, format, num);
  out=tmp;
  return out;
}

String printHexDate(uint32_t num, int precision) {
  wdt_reset();
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
  wdt_reset();
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

int32_t posix_get_2_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){ 
  wdt_reset();
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

  int32_t out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24| 
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8 |
                          (uint32_t)dataIn[6];
      out = (int32_t)dataOut;
    }else{
      out = -1;
    }  
  return out;
}//posix_get_2_Register()

String input_getDataSensor0()
{
  wdt_reset();
  // uint8_t adress  = 0x08;
  // String out = ID_BIF;

  // out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8);
  // out += "59ac2e70"; 
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  // out += konversi::toIEEE(-1);
  // out += konversi::toIEEE(-1);  

  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002),1000.0);   // A
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002),1000.0);   // A
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002),1000.0);   // A
  
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002),100.0);   // Hz
  
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002),10.0);  // V
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002),10.0);  // V
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002),10.0);  // V
  
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0004,0x0002));     // kW   // aktif-opwer
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0014,0x0002));     // kW
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0024,0x0002));     // kW
  
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0006,0x0002));   // kVAR   // reaktif-power
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0016,0x0002));   // kVAR
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0026,0x0002));   // kVAR

  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x000a,0x0002));   // kVA    // aperent-power
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x001a,0x0002));   // kVA
  // out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x002a,0x0002));   // kVA
  // // Serial.println(out.length());
  // return out;
  return "ED0000000001bfe078700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

}//input_getDataSensor()

String input_getDataSensor1()
{
  wdt_reset();
  uint8_t adress  = 0x01;
  String out = ID_IN;

  out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8);
  out += "59ac2e70"; 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  out += konversi::toIEEE(-1);
  out += konversi::toIEEE(-1);  

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002),1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002),1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002),1000.0);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002),100.0);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002),10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002),10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002),10.0);  // V
  
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
  wdt_reset();
  uint8_t adress  = 0x02;
  String out = ID_OUT;
  out += printHexDate(posix_get_2_Register(adress,0x04,0x0578,0x0002),8);
  out += "59ac2e70"; 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  out += konversi::toIEEE(-1);
  out += konversi::toIEEE(-1);  

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002),1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002),1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002),1000.0);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002),100.0);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002),10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002),10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002),10.0);  // V
  
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
