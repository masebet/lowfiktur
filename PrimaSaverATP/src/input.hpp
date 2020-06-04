// #include "proses.hpp"

String printHexDate(uint32_t num, int precision);
  
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
     sprintf(format, "%%.%dx", precision/2);
     sprintf(tmp, format, num);
     out=tmp;
     sprintf(format, "%%.%dx", precision/2);
     sprintf(tmp, format, num>>16);
     out+=tmp;
  return out;
}

//=================================================================================================
float
posix_get_2_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){ 
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

  // Serial.print(printHex(crcCalc[2],2));
  // Serial.print(printHex(crcCalc[3 ],2));
  // Serial.print(">  ");  
  // Serial.print(printHex(dataIn[3],2));
  // Serial.print(printHex(dataIn[4],2));
  // Serial.print("  ");
  // Serial.print(printHex(dataIn[5],2));
  // Serial.println(printHex(dataIn[6],2));

  float out;
  if(crc16(dataIn,6) == int((dataIn[8]<<8)|dataIn[7])){
      uint32_t dataOut =  (uint32_t)dataIn[3]<<24| 
                          (uint32_t)dataIn[4]<<16|
                          (uint32_t)dataIn[5]<<8 |
                          (uint32_t)dataIn[6];

      out = (float) dataOut;
    }else{
      out = 0x0;
      // dataIn[3] = 0x00;
      // dataIn[4] = 0x10;
      // dataIn[5] = 0x00;
      // dataIn[6] = 0x00;
      // uint32_t dataOut = (uint32_t)dataIn[3]|(uint32_t)dataIn[4]<<8|(uint32_t)dataIn[5]<<16|(uint32_t)dataIn[6]<<24;
      // out = (float) dataOut;
    }
  
    //Serial.println(out);
  return out;
}

/*
  urutan datanya adalah
  CR1 SerialID execlTime KWH Wtot WBP LWBP IR IS IT FRQ VarTot VAtot VR VS VT WR WS WT VarR VarS VarT VAR VAS VAT
  |     |       |         |
  3chr  8chr    8chr     8char and etc until end
*/

String 
input_getDataSensor()
{
  uint8_t adress  = 0x01;
  String out = ID_IN;

  out += konversi::toIEEE(posix_get_2_Register(adress,0x04,0x0578,0x0002)); 
  out += tools::getDate(); 
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x00dc,0x0002));   // kWh    //circutor  5e-5f + 88-89 + b2-b3 = dc-dd
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0030,0x0002));   // kW     //aktiv power

  if(tools::tarif()=="WBP"){
      // WBP  = KWh-LWBP;
      float KWh = posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float WBP = KWh - proses_api.bacaDataEprom(LWBP_IN).toFloat();
        proses_api.tulisEprom(WBP_IN, String((float)WBP) );
      }
  }else{
      // LWBP = KWh-WBP;
      float KWh = posix_get_2_Register(adress,0x03,0x00dc,0x0002);
      if(KWh>=1.0){
        float LWBP = KWh - proses_api.bacaDataEprom(WBP_IN).toFloat();
        proses_api.tulisEprom(LWBP_IN, String((float)LWBP) );
      }
  } 

  out += konversi::toIEEE(proses_api.bacaDataEprom(WBP_IN).toFloat());
  out += konversi::toIEEE(proses_api.bacaDataEprom(LWBP_IN).toFloat());

  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0002,0x0002)/1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0012,0x0002)/1000.0);   // A
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0022,0x0002)/1000.0);   // A
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003c,0x0002)/100.0);   // Hz
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x003e,0x0002)/10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0040,0x0002)/10.0);  // V
  out += konversi::toIEEE(posix_get_2_Register(adress,0x03,0x0042,0x0002)/10.0);  // V
  
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

//=================================================================================================
//String
// uint32_t
float
posix_get_2_RegisterOut(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
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
}

/*
  urutan datanya adalah
  CR1 SerialID execlTime KWH Wtot WBP LWBP IR IS IT FRQ VarTot VAtot VR VS VT WR WS WT VarR VarS VarT VAR VAS VAT
  |     |       |         |
  3chr  8chr    8chr     8char and etc until end
*/
String 
input_getDataSensorOut()
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

  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0002,0x0002)/1000.0);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0012,0x0002)/1000.0);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0022,0x0002)/1000.0);   // A
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x003c,0x0002)/100.0);    // Hz
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0032,0x0002));   // kVAR   //reaktiv power
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0036,0x0002));   // kVA    //aperent power
  
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x003e,0x0002)/10.0);  // V
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0040,0x0002)/10.0);  // V
  out += konversi::toIEEE(posix_get_2_RegisterOut(adress,0x03,0x0042,0x0002)/10.0);  // V
  
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
//=================================================================================================

float
posix_get_1_Register(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){ 
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
}

float
posix_get_1_RegisterOut(uint8_t addres,uint8_t func,uint16_t reg,uint16_t reg2){
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
}

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


bool DEBUG =0;
void debug()
{

  while(1){
  if(RecieveMessage()) {
    fikturSMS();
    kirimPesan();
  }
  
    delay(1000);
  }
}//debug()

void SendCommand(String command, const int timeout, boolean debug)
{       
  Serial2.print(command); 

  // Reply = "";
  // delay(timeout);
  // if(Serial2.available()) Reply = Serial2.readString();
  
  Reply = "";
  unsigned long time = millis();
  while ((millis()-time)< timeout ){
      if(Serial2.available()) Reply = Serial2.readString();
          // Reply += char(Serial2.read());
  }

  if(Reply.indexOf("CSQ")>0)  Serial.println(Reply);

  if      (Reply.indexOf("ERROR")>0)  { Serial.print("ERROR proses of>  "); Serial.println(command); }
  else if (Reply.indexOf("OK")>0)     { Serial.print("OK proses of>  "); Serial.println(command); }
  else { Serial.println("SUCSES proses of>>  "); Serial.println(Reply); }
}//SendCommand()

bool RecieveMessage(){
  wdt_reset(); 
  Serial2.write(0x1A); 
  SendCommand("AT\r\n",500,S1debug);
  SendCommand("AT+CMGR=1\r\n",500,S1debug);
  Serial.println("pesan >>"); Serial.println(Reply);
  int f,l;
  f=Reply.indexOf("\",\"+"); 
  l=Reply.indexOf("\"\"");
  sender_phone=Reply.substring(f+3,l-2);
  Serial.print("Nomor Telepon >"); Serial.println(sender_phone);
  if(f>0) return 1; 
  return 0;
}//RecieveMessage()

void fikturSMS()
{
    ResponeSMS="";
    Serial.println(Reply);

    if(Reply.indexOf("RESET")>0) {
       posix_get_1_Register(0x01,0x05,0x0834,0xFF00);
       posix_get_1_RegisterOut(0x02,0x05,0x0834,0xFF00);
       proses_api.tulisEprom(WBP_IN, String(0.0));
       proses_api.tulisEprom(LWBP_IN, String(0.0));
       proses_api.tulisEprom(WBP_OUT, String(0.0));
       proses_api.tulisEprom(LWBP_OUT, String(0.0));
       ResponeSMS+="\r\nRESET ALL OK";
    }

    if(Reply.indexOf("MODE#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Mode = Reply.substring(f+1,l);
        proses_api.tulisEprom(EP_MODE,Mode);
        ResponeSMS+="\r\nMODE > "+Mode;
    }

    if(Reply.indexOf("TAHUN#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Tahun = Reply.substring(f+1,l);
        rtc.adjust(DateTime(Tahun.toInt(), tools::Bulan(), tools::Hari(), tools::jamH(), tools::jamM(), 0));
        ResponeSMS+="\r\nTAHUN > "+Tahun;
    }

    if(Reply.indexOf("BULAN#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Bulan = Reply.substring(f+1,l);
        rtc.adjust(DateTime(tools::Tahun(), Bulan.toInt(), tools::Hari(), tools::jamH(), tools::jamM(), 0));
        ResponeSMS+="\r\nBULAN > "+Bulan;
    }

    if(Reply.indexOf("HARI#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Hari = Reply.substring(f+1,l);
        rtc.adjust(DateTime(tools::Tahun(), tools::Bulan(), Hari.toInt(), tools::jamH(), tools::jamM(), 0));
        ResponeSMS+="\r\nHARI > "+Hari;
    }

    if(Reply.indexOf("JAM#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Jam = Reply.substring(f+1,l);
        rtc.adjust(DateTime(tools::Tahun(), tools::Bulan(), tools::Hari(), Jam.toInt(), tools::jamM(), 0));
        ResponeSMS+="\r\nJAM > "+Jam;
    }
    
    if(Reply.indexOf("MIN#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Min = Reply.substring(f+1,l);
        rtc.adjust(DateTime(tools::Tahun(),tools::Bulan(), tools::Hari(), tools::jamH(), Min.toInt(), 0));
        ResponeSMS+="\r\nMINUTE > "+Min;
    }

    if(Reply.indexOf("IP#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Ip = Reply.substring(f+1,l);
        proses_tulisEprom(EP_IP,Ip);
        ResponeSMS+="\r\nIP > "+Ip;
        IPADDRESS = proses_bacaDataEprom(EP_IP);
    }
    
    if(Reply.indexOf("PORT#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Port = Reply.substring(f+1,l);
        proses_api.tulisEprom(EP_PORT,Port);
        ResponeSMS+="\r\nPORT > "+Port;
        PORT = proses_api.bacaDataEprom(EP_PORT);
    }

    if(Reply.indexOf("APN#")>0) {
        int f,l;
        f=Reply.indexOf("#"); 
        l=Reply.indexOf(";");
        String Apn = Reply.substring(f+1,l);
        proses_tulisEprom(EP_APN,Apn);
        ResponeSMS+="\r\nAPN > "+Apn;
        APN = proses_bacaDataEprom(EP_APN);
    }

    if(Reply.indexOf("HELP")>0) ResponeSMS+="\r\nMODE?TIME?IP?PORT?IP?\r\nHanya bisa satu persatu :\r\nJAM#<x>;MIN#<x>;IP#<x>;PORT#<x>;APN#<x>;\r\nRESET";
    if(Reply.indexOf("TIME")>0) ResponeSMS+="\r\nTIME : "+tools::tarif()+" "+String(tools::jamH())+":"+String(tools::jamM());
    if(Reply.indexOf("IP")>0)   ResponeSMS+="\r\nIP TCP : "+IPADDRESS;
    if(Reply.indexOf("PORT")>0) ResponeSMS+="\r\nIP PORT : "+PORT;
    if(Reply.indexOf("APN")>0)  ResponeSMS+="\r\nAPN : "+APN;
    if(Reply.indexOf("MODE")>0) ResponeSMS+="\r\nMODE : "+proses_api.bacaDataEprom(EP_MODE);
    if(Reply.indexOf("DATE")>0) ResponeSMS+="\r\nDATE : "+tools::Waktu();

}

void kirimPesan()
{
    ResponeSMS+="\r\nSNI: ";   ResponeSMS.concat(ID_IN+konversi::toIEEE(posix_get_2_Register(0x01,0x04,0x0578,0x0002)));
    ResponeSMS+="\r\nSNO: "; ResponeSMS.concat(ID_OUT+konversi::toIEEE(posix_get_2_RegisterOut(0x02,0x04,0x0578,0x0002)));
    
    SendCommand("AT+CMGF=1\r\n",500,S1debug);
    SendCommand("AT+CMGS=\""+sender_phone+"\"\r\n",500,S1debug);
    SendCommand(ResponeSMS+"\r\n",1000,S1debug);
    SendCipSend(9000);
    
    ResponeSMS="";
    sender_phone="";
    SendCommand("AT\r\n",250,S1debug);
    SendCommand("AT+CMGD=1,4\r\n",500,S1debug);
}//kirimPesan()