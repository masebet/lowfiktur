namespace sms{

  void SendCommand(String command, unsigned int timeout, boolean debug){
    Serial2.print(command); 
    Reply = "";
    unsigned long time = millis();
    while ((millis()-time) < timeout ){
        if(Serial2.available())  Reply += (char) Serial2.read();
    }
    Serial.println(Reply);
  }//SendCommand()

  void SendCipSend(int timeout){
    wdt_reset();
    Serial2.write(0x1A);
    delay(6000);
  }//SendCipSend()

  bool RecieveMessage()
  {
    wdt_reset(); 
    // Serial2.write(0x1A); 
    SendCommand("AT\r\n",500,S1debug);
    SendCommand("AT+CMGR=1\r\n",3000,S1debug);
    if(S1debug){}else{Serial.println("pesan >>"); Serial.println(Reply);}
    int f,l;
    f=Reply.indexOf("\",\"+"); 
    l=Reply.indexOf("\"\"");
    sender_phone=Reply.substring(f+3,l-2);
    if(S1debug){}else{Serial.print("Nomor Telepon >"); Serial.println(sender_phone);}
    if(f>0) return 1; 
    return 0;
  }//RecieveMessage()

// +CLIP: "089605230676",129,"",,"",0
  bool getNomor(String in){
    wdt_reset();
    int f,l;
    f=in.indexOf(" \""); 
    l=in.indexOf("\",");
    sender_phone=in.substring(f+2,l);
    
    Serial.print("Nomor Telepon > "); 
    Serial.println(sender_phone);
    return 0;
  }//getNomor()

  void fikturSMS(){
    wdt_reset();
      ResponeSMS="";
      Serial.println(Reply);
      ResponeSMS+="\r\nSNI: "; 
      ResponeSMS.concat(ID_IN+konversi::toIEEE(posix_get_2_Register(0x01,0x04,0x0578,0x0002)));
      ResponeSMS+="\r\nSNO: "; 
      ResponeSMS.concat(ID_OUT+konversi::toIEEE(posix_get_2_Register(0x02,0x04,0x0578,0x0002)));
      
      if(Reply.indexOf("RESET")>0) {
        posix_get_1_Register(0x01,0x05,0x0834,0xFF00);
        posix_get_1_Register(0x02,0x05,0x0834,0xFF00);
        proses_api.tulisEprom(WBP_IN, String(0.0));
        proses_api.tulisEprom(LWBP_IN, String(0.0));
        proses_api.tulisEprom(WBP_OUT, String(0.0));
        proses_api.tulisEprom(LWBP_OUT, String(0.0));
        ResponeSMS+="\r\nRESET ALL OK";
      }

      if(Reply.indexOf("KILL")>0) {
        asm volatile ("  jmp 0");
      }

      if(Reply.indexOf("MODE#")>0) {
          int f,l;
          f=Reply.indexOf("#"); 
          l=Reply.indexOf(";");
          String Mode = Reply.substring(f+1,l);
          proses_api.tulisEprom(EP_MODE,Mode);
          ResponeSMS+="\r\nMODE > "+Mode;
      }

      // if(Reply.indexOf("TAHUN#")>0) {
      //     int f,l;
      //     f=Reply.indexOf("#"); 
      //     l=Reply.indexOf(";");
      //     String Tahun = Reply.substring(f+1,l);
      //     tools::setTahun(Tahun.toInt());
      //     ResponeSMS+="\r\nTAHUN > "+Tahun;
      // }

      // if(Reply.indexOf("BULAN#")>0) {
      //     int f,l;
      //     f=Reply.indexOf("#"); 
      //     l=Reply.indexOf(";");
      //     String Bulan = Reply.substring(f+1,l);
      //     tools::setBulan(Bulan.toInt());
      //     ResponeSMS+="\r\nBULAN > "+Bulan;
      // }

      // if(Reply.indexOf("HARI#")>0) {
      //     int f,l;
      //     f=Reply.indexOf("#"); 
      //     l=Reply.indexOf(";");
      //     String Hari = Reply.substring(f+1,l);
      //     tools::setHari(Hari.toInt());
      //     ResponeSMS+="\r\nHARI > "+Hari;
      // }

      // if(Reply.indexOf("JAM#")>0) {
      //     int f,l;
      //     f=Reply.indexOf("#"); 
      //     l=Reply.indexOf(";");
      //     String Jam = Reply.substring(f+1,l);
      //     tools::setJam(Jam.toInt());
      //     ResponeSMS+="\r\nJAM > "+Jam;
      // }
      
      // if(Reply.indexOf("MIN#")>0) {
      //     int f,l;
      //     f=Reply.indexOf("#"); 
      //     l=Reply.indexOf(";");
      //     String Min = Reply.substring(f+1,l);
      //     tools::setMenit(Min.toInt());
      //     ResponeSMS+="\r\nMINUTE > "+Min;
      // }

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

      if(Reply.indexOf("DEV#")>0) {
          int f,l;
          f=Reply.indexOf("#"); 
          l=Reply.indexOf(";");
          String dev = Reply.substring(f+1,l);
          proses_api.tulisEprom(EID_IN,dev+"1");
          proses_api.tulisEprom(EID_OUT,dev+"2");
          ResponeSMS  +="\r\nDEV > "+dev;
          ID_IN       = proses_api.bacaDataEprom(EID_IN);  
          ID_OUT      = proses_api.bacaDataEprom(EID_OUT);
          
      }

      // if(Reply.indexOf("LIST")>0) Reply = "Replay#TIME#IP#PORT#APN#MODE#DATE#";
      if(Reply.indexOf("LIST")>0) Reply = "Replay#SIM#CSQ#STA#MBI#MBO#MBF#DATE#FV#";
      
      // if(Reply.indexOf("HELP")>0) ResponeSMS+="\r\nMODE?TIME?IP?PORT?IP?\r\nHanya bisa satu persatu :\r\nJAM#<x>;MIN#<x>;IP#<x>;PORT#<x>;APN#<x>;\r\nRESET";
      // if(Reply.indexOf("TIME")>0) ResponeSMS+="\r\nTIME : "+tools::tarif()+" "+String(tools::jamH())+":"+String(tools::jamM());
      if(Reply.indexOf("IP")>0)   ResponeSMS+="\r\nIP TCP : "+IPADDRESS;
      if(Reply.indexOf("PORT")>0) ResponeSMS+="\r\nIP PORT : "+PORT;
      if(Reply.indexOf("APN")>0)  ResponeSMS+="\r\nAPN : "+APN;
      // if(Reply.indexOf("MODE")>0) ResponeSMS+="\r\nMODE : "+proses_api.bacaDataEprom(EP_MODE);
      // if(Reply.indexOf("DATE")>0) ResponeSMS+="\r\nDATE : "+tools::Waktu();

      if(Reply.indexOf("SIM")>0)  ResponeSMS+="\r\nSIM : READY";
      if(Reply.indexOf("CSQ")>0)  ResponeSMS+="\r\nCSQ : "+csq;
      // if(Reply.indexOf("SDCARD")>0) ResponeSMS+="\r\nSDCARD : "+sdCard;
      if(Reply.indexOf("STA")>0)  ResponeSMS+="\r\nSTA : "+respondsend;
      if(Reply.indexOf("MBF")>0)  ResponeSMS+="\r\nMBF : "+MOBF;
      if(Reply.indexOf("MBI")>0)  ResponeSMS+="\r\nMBI : "+MOBI;
      if(Reply.indexOf("MBO")>0)  ResponeSMS+="\r\nMBO : "+MOBO;
      // if(Reply.indexOf("FM")>0)   ResponeSMS+="\r\nFM : "+FM;
      if(Reply.indexOf("FV")>0)   ResponeSMS+="\r\nFV : PSM ( "+FV+" )\r\n";
  }//fikturSMS()

  void kirimPesan()
  {
    wdt_reset();
    sms::SendCommand("AT+CMGF=1\r\n",500,S1debug);
    sms::SendCommand("AT+CMGS=\""+sender_phone+"\"\r\n",500,S1debug);
    sms::SendCommand(ResponeSMS+"\r\n",1000,S1debug);
    sms::SendCipSend(1000);
    ResponeSMS="";
    sender_phone="";
    sms::SendCommand("AT\r\n",250,S1debug);
    sms::SendCommand("AT+CMGD=1,4\r\n",500,S1debug);
    delay(100);
    sms::SendCommand("AT\r\n",250,S1debug);
    sms::SendCommand("AT+CMGDA=\"DEL ALL\"\r\n",500,S1debug);
  }//kirimPesan()

  void testKirimSms(String no,String pesan){
    wdt_reset();
    sms::SendCommand("AT+CMGF=1\r\n",500,S1debug);
    sms::SendCommand("AT+CMGS=\""+no+"\"\r\n",500,S1debug);
    sms::SendCommand(pesan+"\r\n",1000,S1debug);
    sms::SendCipSend(1000);
    sms::SendCommand("AT\r\n",250,S1debug);
    sms::SendCommand("AT+CMGD=1,4\r\n",500,S1debug);
    return;
  }

}//sms
