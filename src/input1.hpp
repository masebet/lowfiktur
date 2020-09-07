/*
  urutan datanya adalah
  CR1 SerialID execlTime KWH Wtot WBP LWBP IR IS IT FRQ VarTot VAtot VR VS VT WR WS WT VarR VarS VarT VAR VAS VAT
  |     |       |         |
  3chr  8chr    8chr     8char and etc until end
*/
//=================================================================================================
float posix_get_1_RegisterOut(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
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
  
  float out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24|
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8  |
                          (uint32_t)dataIn[6];
      out = (float) dataOut;
    }else{
      out = 0x0;
    }
  return out;
}//posix_get_1_RegisterOut()

float posix_get_2_RegisterOut(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
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
  
  //String out="";
  float out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      //out = printHex(dataIn[3], 2)+printHex(dataIn[4], 2)+printHex(dataIn[5], 2)+printHex(dataIn[6], 2);
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24|
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8  |
                          (uint32_t)dataIn[6];
      out = (float) dataOut;
    }else{
      //out = "nullData"; 
      out = 0x0;
      // dataIn[3] = 0x00;
      // dataIn[4] = 0x10;
      // dataIn[5] = 0x00;
      // dataIn[6] = 0x00;
      // uint32_t dataOut = (uint32_t)dataIn[3]|(uint32_t)dataIn[4]<<8|(uint32_t)dataIn[5]<<16|(uint32_t)dataIn[6]<<24;
      // out = (float) dataOut;
    }
  return out;
}//posix_get_2_RegisterOut()

String input_getDataSensorOut()
{
  uint8_t adress  = 0x02;
  String out  = ID_OUT;
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x04,0x0578,0x0002)); 
  out += tools::getDate(); 
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  if(tools::tarif()=="WBP"){
      // WBP  = KWh-LWBP;
      float KWh = posix_get_2_RegisterOut(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float WBP = KWh - proses_api.bacaDataEprom(LWBP_OUT).toFloat();
        proses_api.tulisEprom(WBP_OUT, String((float)WBP) );
      }
  }else{
      // LWBP = KWh-WBP;
      float KWh = posix_get_2_RegisterOut(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float LWBP = KWh - proses_api.bacaDataEprom(WBP_OUT).toFloat();
        proses_api.tulisEprom(LWBP_OUT, String((float)LWBP) );
      }
  }   
  out += konversi::toIEEE(proses_api.bacaDataEprom(WBP_OUT).toFloat());
  out += konversi::toIEEE(proses_api.bacaDataEprom(LWBP_OUT).toFloat());  

  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0002,0x0002)/1000);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0012,0x0002)/1000);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0022,0x0002)/1000);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x003c,0x0002)/100);    // Hz
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x003e,0x0002)/10);  // V
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0040,0x0002)/10);  // V
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0042,0x0002)/10);  // V
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0004,0x0002));   // kW      // aktif-opwer
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0014,0x0002));   // kW
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0024,0x0002));   // kW
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0006,0x0002));   // kVAR    // reaktif-power
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0016,0x0002));   // kVAR
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0026,0x0002));   // kVAR

  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x000a,0x0002));   // kVA     // aperent-power
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x001a,0x0002));   // kVA
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x002a,0x0002));   // kVA  return out;
  return out;

}//input_getDataSensorOut()
