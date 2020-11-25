void clearFRAMConf(){
  for(int ad=ssid_addr;ad<=cirmachine_addr;ad++){
    mac.writeAddress(ad,'\0');
  }
}
void SetFlagConf(char sta_conf){
   mac.writeAddress(FRAM_Flag_addr,sta_conf);
}

char ReadStatusConf(){
    char Sta = mac.readAddress(FRAM_Flag_addr);
    return Sta;
}

void CleraFRAM2(){
  Serial.println("Start Clear FRAM");
  write_addeeprom = 0;
  for (int m = 0; m < 200; m++)
  {
   for (int i = 0; i < 6; i++)
     {
       mac.writeAddress(write_addeeprom+i,0);
       Serial.print(write_addeeprom+i,HEX);
       Serial.print(" ");
       Serial.print(mac.readAddress(write_addeeprom+i),HEX);
       Serial.print(" ");
      
     }
     Serial.println();
     write_addeeprom += 128;
  }
      mac.writeAddress(writeaddr_eeprom1, 0); 
      mac.writeAddress(writeaddr_eeprom2, 0);
      write_addeeprom = 0;
  
}
void ClearFRAM(){
   Serial.println("Start Clear FRAM");
   for(int r=0;r<=25599;r++){
    
       mac.writeAddress(r,0);
     
     
   }
      mac.writeAddress(writeaddr_eeprom1, 0); 
      mac.writeAddress(writeaddr_eeprom2, 0);
      write_addeeprom = 0;
}
void WriteCertSSL(bool debugs){
  Certssl = readcert(SD,"/certi.txt");
  Serial.print(Certssl);
  Serial.println("WRITE: Certificate SSL");
   for (int i = 0 ; i < Certssl.length(); i++){
        mac.writeAddress(Cert_addr+i,Certssl[i]);
        if(debugs){
          Serial.printf("%d %X %c\n",Cert_addr+i,Cert_addr+i,Certssl[i]);
        }
    }
    mac.writeAddress(Cert_addr+Certssl.length(),'\0');
}


String ReadCert(){
  char txt[2265];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(Cert_addr);
  while(Cr != '\0' && len<2265){
    Cr = mac.readAddress(Cert_addr+len);
   
      Serial.printf("ADDR: %d ",Cert_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
     Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt);
}



void ReadCfg_fromSD_forWriteFRAM(fs::FS &fs, const char* path){
 
   File file = fs.open(path);
    if (!file) {
       return;
    }
    
    int n =0;
    while (file.available()) {
     eachline = file.readStringUntil('\n');    
      int posi = eachline.indexOf(':');
          String val = eachline.substring(posi+1);
           val.trim();
           Serial.println(val);
           cfgs[n] = val;
         n++;
    }
  file.close();
}  
void Write_ssid_FRAM(bool debugs){
  
  
  Serial.println("WRITE: SSID");
   for (int i = 0 ; i < cfgs[0].length(); i++){
        mac.writeAddress(ssid_addr+i,cfgs[0][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",ssid_addr+i,ssid_addr+i,cfgs[0][i]);
        }
    }
    mac.writeAddress(ssid_addr+cfgs[0].length(),'\0');
}
void Write_wpass_FRAM(bool debugs){

  Serial.println("WRITE: WIFI PASS");
   for (int i = 0 ; i < cfgs[1].length(); i++){
        mac.writeAddress(wpass_addr+i,cfgs[1][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",wpass_addr+i,wpass_addr+i,cfgs[1][i]);
        }
    }
    mac.writeAddress(wpass_addr+cfgs[1].length(),'\0');
}
String Read_ssid_FRAM(){
char txt[ssid_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(ssid_addr);
  while(Cr != '\0' && len < ssid_size){
    Cr = mac.readAddress(ssid_addr+len);
   
      Serial.printf("ADDR: %d ",ssid_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
     Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 
 return String(txt);
}

String Read_wpass_FRAM(){
char txt[wpass_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(wpass_addr);
  while(Cr != '\0' && len < wpass_size){
    Cr = mac.readAddress(wpass_addr+len);
   
      Serial.printf("ADDR: %d ",wpass_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
  
 return String(txt);

}
void Write_mqserv_FRAM(bool debugs){

  Serial.println("WRITE: MQTT SERVER");
   for (int i = 0 ; i < cfgs[2].length(); i++){
        mac.writeAddress(mqserv_addr+i,cfgs[2][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",mqserv_addr+i,mqserv_addr+i,cfgs[2][i]);
        }
    }
    mac.writeAddress(mqserv_addr+cfgs[2].length(),'\0');
}
String Read_mqserv_FRAM(){
char txt[mqserv_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(mqserv_addr);
  while(Cr != '\0' && len < mqserv_size){
    Cr = mac.readAddress(mqserv_addr+len);
   
      Serial.printf("ADDR: %d ",mqserv_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt);

}
void Write_mqport_FRAM(bool debugs){
 
  Serial.println("WRITE: MQTT PORT");
   for (int i = 0 ; i < cfgs[3].length(); i++){
        mac.writeAddress(mqport_addr+i,cfgs[3][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",mqport_addr+i,mqport_addr+i,cfgs[3][i]);
        }
    }
    mac.writeAddress(mqport_addr+cfgs[3].length(),'\0');
}

int Read_mqport_FRAM(){
char txt[mqport_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(mqport_addr);
  while(Cr != '\0' && len < mqport_size){
    Cr = mac.readAddress(mqport_addr+len);
   
      Serial.printf("ADDR: %d ",mqport_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt).toInt();

}

void Write_mquser_FRAM(bool debugs){
 
  Serial.println("WRITE: MQTT USER");
   for (int i = 0 ; i < cfgs[4].length(); i++){
        mac.writeAddress(mquser_addr+i,cfgs[4][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",mquser_addr+i,mquser_addr+i,cfgs[4][i]);
        }
    }
    mac.writeAddress(mquser_addr+cfgs[4].length(),'\0');
}

String Read_mquser_FRAM(){
char txt[mquser_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(mquser_addr);
  while(Cr != '\0' && len < mquser_size){
    Cr = mac.readAddress(mquser_addr+len);
   
      Serial.printf("ADDR: %d ",mquser_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt);

}
void Write_mqpass_FRAM(bool debugs){
  
  Serial.println("WRITE: MQTT PASSWORD");
   for (int i = 0 ; i < cfgs[5].length(); i++){
        mac.writeAddress(mqpass_addr+i,cfgs[5][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",mqpass_addr+i,mqpass_addr+i,cfgs[5][i]);
        }
    }
    mac.writeAddress(mqpass_addr+cfgs[5].length(),'\0');
}

String Read_mqpass_FRAM(){
char txt[mqpass_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(mqpass_addr);
  while(Cr != '\0' && len < mqpass_size){
    Cr = mac.readAddress(mqpass_addr+len);
   
      Serial.printf("ADDR: %d ",mqpass_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt);

}
void Write_clientid_FRAM(bool debugs){
  
  Serial.println("WRITE: MQTT PASSWORD");
   for (int i = 0 ; i < cfgs[6].length(); i++){
        mac.writeAddress(clientid_addr+i,cfgs[6][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",clientid_addr+i,clientid_addr+i,cfgs[6][i]);
        }
    }
    mac.writeAddress(clientid_addr+cfgs[6].length(),'\0');
}

String GroupCompFRAM(byte opt){
  String grp,comp;
   if(opt == 1){
    for(int s=0;s<String(clientId).length();s++){
    if(s<6){
       grp += clientId[s];
      }
    }
    return grp;
   }
   if(opt == 2){
    for(int s=0;s<String(clientId).length();s++){
      if(s<6){
        grp += clientId[s];
         if(s==5){
           grp += "/";
         }
      }
     if((s>=6)&&(s<=8)){
        comp += clientId[s];
       }
     }
    return grp+comp;
   }
}



String ReadClidSlash_FRAM(){
char C[clientid_size];
String txt;
 
 for (int i= 0 ; i< clientid_size; i++){
     C[i] = (char)mac.readAddress(clientid_addr+i);
     
     // Serial.printf("ADDR: %d ",clientid_addr+i);
     // Serial.write(C[i]);
      //Serial.printf("\t");
     
      txt += C[i];
 }

   Serial.println();
  
   //Serial.println(txt);
   return txt;
}



String Read_clientid_FRAM(){
char C[clientid_size];
String txt;
 
 for (int i= 0 ; i< clientid_size; i++){
     C[i] = (char)mac.readAddress(clientid_addr+i);
     
      Serial.printf("ADDR: %d ",clientid_addr+i);
      Serial.write(C[i]);
      Serial.printf("\t");
     
      txt += C[i];
 }

   Serial.println();
  
  // Serial.println(txt);
 
  String Tcid = TransformClientID(txt);
  
    Serial.println(Tcid);
  
  return Tcid;
}



String TransformClientID(String cd){
  
   String g = cd.substring(0,6);
   String c = cd.substring(7,10);
   String m = cd.substring(11);
   return g+c+m;
}

void Write_otatimeout_FRAM(bool debugs){
  
  Serial.println("WRITE: OTA TIMEOUT");
   for (int i = 0 ; i < cfgs[7].length(); i++){
        mac.writeAddress(otatimeout_addr+i,cfgs[7][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",otatimeout_addr+i,otatimeout_addr+i,cfgs[7][i]);
        }
    }
    mac.writeAddress(otatimeout_addr+cfgs[7].length(),'\0');
}
int Read_otatimeout_FRAM(){
char txt[otatimeout_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(otatimeout_addr);
  while(Cr != '\0' && len < otatimeout_size){
    Cr = mac.readAddress(otatimeout_addr+len);
   
      Serial.printf("ADDR: %d ",otatimeout_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt).toInt();
}

void Write_sendperiod_FRAM(bool debugs){
  
  Serial.println("WRITE: SENDPERIOD");
   for (int i = 0 ; i < cfgs[8].length(); i++){
        mac.writeAddress(sendperiod_addr+i,cfgs[8][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",sendperiod_addr+i,sendperiod_addr+i,cfgs[8][i]);
        }
    }
    mac.writeAddress(sendperiod_addr+cfgs[8].length(),'\0');
}

String Read_sendperiod_FRAM(){
char txt[sendperiod_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(sendperiod_addr);
  while(Cr != '\0' && len < sendperiod_size){
    Cr = mac.readAddress(sendperiod_addr+len);
   
      Serial.printf("ADDR: %d ",sendperiod_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt);
}
void Write_cirmachine_FRAM(bool debugs){
  
  Serial.println("WRITE: CIRMACHINE");
   for (int i = 0 ; i < cfgs[9].length(); i++){
        mac.writeAddress(cirmachine_addr+i,cfgs[9][i]);
        if(debugs){
          Serial.printf("%d %X %c\n",cirmachine_addr+i,cirmachine_addr+i,cfgs[9][i]);
        }
    }
    mac.writeAddress(cirmachine_addr+cfgs[9].length(),'\0');
}

int Read_cirmachine_FRAM(){
char txt[cirmachine_size];
 unsigned char Cr;
  int len=0;
  Cr = mac.readAddress(cirmachine_addr);
  while(Cr != '\0' && len < cirmachine_size){
    Cr = mac.readAddress(cirmachine_addr+len);
   
      Serial.printf("ADDR: %d ",cirmachine_addr+len);
       Serial.write(Cr);
       Serial.printf("\t");
        Serial.println();
    txt[len] = Cr;
     len++;
  }
  txt[len] = '\0';
 return String(txt).toInt();
}




void currentConfigFRAM(){
  String ssid = Read_ssid_FRAM()+"\n";
  String spwd = Read_wpass_FRAM()+"\n";
  String mqserv = Read_mqserv_FRAM()+"\n";
  int mqport = Read_mqport_FRAM();
  String Mqport = String(mqport)+"\n"; 
  String mquser = Read_mquser_FRAM()+"\n";
  String mqpass = Read_mqpass_FRAM()+"\n";
  String clid = Read_clientid_FRAM()+"\n";
  int ota_tout = Read_otatimeout_FRAM();
  String otatimeout = String(ota_tout)+"\n";
  String send_period = Read_sendperiod_FRAM()+"\n";
  int  cirm = Read_cirmachine_FRAM();
  String cirmach = String(cirm)+"\n";
  String aconf = ssid+spwd+mqserv+Mqport+mquser+mqpass+clid+otatimeout+send_period+cirmach;
  client.publish(sendconf,aconf.c_str());
}
