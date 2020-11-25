bool checkSDcard(){
   if (!SD.begin()) {
     return 0;
   }else{
     return 1;
   }
}
//Function Start SDcard
void sdbegin()
{
  String serr;
  
  if (!SD.begin()) {
    if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
      Serial.println("$7000: Error SDCard Mount Failed");
      serr = "$7000: Error SDCard Mount Failed";
      SendError(serr.c_str());
     }
    Msgstart += "$7000: Error SDCard Mount Failed\n";
    Sdcardforhis = serr;
    return;
  } else {
    //serr = "$7010: SD Card OK\n";
    if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
      Serial.println("$7010:SD Card OK");
      Msgstart += "$7010: SD Card OK\n";
      serr = "$7010: SD Card OK\n";
      SendError(serr.c_str());
      Sdcardforhis = serr;
    }
  }

}

String getSomeConfigAssign(){
  String grp,comp,mach;
   for(int s=0;s<gcm.length();s++){
    if(s<6){
      grp += gcm[s];
      if(s==5){
        grp += "/";
      }
    }
     if((s>=6)&&(s<=8)){
      comp += gcm[s];
       if(s==8){
         comp += "/";
       }
    }
     if((s>8)&&(s<=12)){
      mach += gcm[s];
    }
  }
 //Serial.println(grp+comp+mach);
  return grp+comp+mach;
}
String GroupComp(byte opt){
  String grp,comp;
   if(opt == 1){
    for(int s=0;s<gcm.length();s++){
    if(s<6){
       grp += gcm[s];
      }
    }
    return grp;
   }
   if(opt == 2){
    for(int s=0;s<gcm.length();s++){
      if(s<6){
        grp += gcm[s];
         if(s==5){
           grp += "/";
         }
      }
     if((s>=6)&&(s<=8)){
        comp += gcm[s];
       }
     }
    return grp+comp;
   }
}
bool assignConfig_Reduce(fs::FS &fs, const char* path) { //Modifi on 13/10/20
  File file = fs.open(path);
  if (!file) {
    Serial.println("$7020: Error Failed to open directory sdcard");
    Msgstart += "$7020: Error Failed to open directory sdcard\n";
    //ReadConf_Flash_Reduce(path); //case sdcard fail
    errsd = true;
    return false;
  }else{
  
  Serial.print("$7030:Reading file: ");
  Msgstart += "$7030:Reading file: ";
  Serial.print(path);
  Serial.println(" OK");
  Msgstart += String(path)+" OK \n";
  
    int n =0;
    while (file.available()) {
     eachline = file.readStringUntil('\n');    
     int posi = eachline.indexOf(':');
     String val = eachline.substring(posi+1);
      Serial.println(val);
      Line[n] = val;
      Line[n].trim();
      n++;
    }
   wifi_ssid = Line[0];
   
   wifi_pwd =  Line[1];
  
   mqtt_server = Line[2];
 
   mqttPort = Line[3].toInt();
   
   //Serial.println(mqtt_server);//debug ok
   //Serial.println(mqttPort);//debug ok

   mqttUser = Line[4];

   mqttPassword = Line[5];
   
   String g = Line[6].substring(0,6);
   //Serial.println(g);
   String c = Line[6].substring(7,10);
   //Serial.println(c);
   String m = Line[6].substring(11);
   //Serial.println(m);
    gcm = g+c+m;
     Serial.printf("Client ID: %s\n",gcm);
    clientId = gcm;
   otatimeout = Line[7].toInt();
   sendperiod = Line[8];
   cirmachine = Line[9].toInt();
   return true;
  }
}

//Function Read config in SDcard file conf.txt  and assign value to Global variable

/*------------OLD Config------------
bool assignConfig(fs::FS &fs, const char* path) {
  File file = fs.open(path);
  if (!file) {
    Serial.println("$7020: Error Failed to open directory sdcard");
    Msgstart += "$7020: Error Failed to open directory sdcard\n";
    ReadConf_Flash(path); //case sdcard fail
    errsd = true;
    return false;
  }else{
  
  Serial.print("$7030:Reading file: ");
  Msgstart += "$7030:Reading file: ";
  Serial.print(path);
  Serial.println(" OK");
  Msgstart += String(path)+" OK \n";
  
    int n =0;
    while (file.available()) {
     eachline = file.readStringUntil('\n');    
     int posi = eachline.indexOf(':');
     String val = eachline.substring(posi+1);
      //Serial.println(val);
      Line[n] = val;
      Line[n].trim();
      n++;
    }
   wifi_ssid = (const char*)Line[0].c_str();
   
   wifi_pwd = (const char*)Line[1].c_str();
  
   mqtt_server = (const char*)Line[2].c_str();
 
   mqttPort = Line[3].toInt();
   
   //Serial.println(mqtt_server);//debug ok
   //Serial.println(mqttPort);//debug ok

   mqttUser = (const char*)Line[4].c_str();

   mqttPassword = (const char*)Line[5].c_str();
   
   clientId = (const char*)Line[6].c_str();
     gcm = Line[6];
   
   otatimeout = Line[7].toInt();
   sendtopic = (const char*)Line[8].c_str();
  
   gtopic = (const char*)Line[9].c_str();
   ctopic = (const char*)Line[10].c_str();
   stopic = (const char*)Line[11].c_str();//otamachine 
   
   dbreply =  (const char*)Line[12].c_str(); // add by rjk
   getconf = (const char*)Line[13].c_str();
   //sendperiod = (const char*)Line[14].c_str();// add by rjk
   sendperiod = Line[14];
   ackota = (const char*)Line[15].c_str();
   sendconf = (const char*)Line[16].c_str();
   debug_allow = (const char*)Line[17].c_str();
   debugs = (const char*)Line[18].c_str();
   cirmachine = Line[19].toInt();
   return true;
  }
}
*/

//Function read cerificate SSL in SDcard file certi.txt and Assign to Global variable
String readcert(fs::FS &fs, const char* path){
 File fr = fs.open(path);
  String SL = "";
  if(!fr){
    //SL = ReadCert_Flash(path);//case sdcard fail
    return SL;
  }else{
   while(fr.available()){
     SL += fr.readStringUntil('\n');
    }
   fr.close();
   return SL;
  }
}
void listcountfileindir(fs::FS &fs, const char * dirname) {
  int chkfile = 0;
  const char *historydelname;
  File root = fs.open(dirname);
  String ls;
  if (!root) {
    if((debug_method == 1) || (debug_method == 3)){
       Serial.println("$7040: Error Failed to open directory");
       
    }
    ls = "$7040: Error Failed to open directory\n";
    
    SendMQttDebug(ls.c_str());
     SendError(ls.c_str());
    
    ls = "";
    return;
  }
  if (!root.isDirectory()) {
     if((debug_method == 1) || (debug_method == 3)){
      Serial.println("$7050: Not a directory");
     }
     ls = "$7050: Not a directory\n";
      SendMQttDebug(ls.c_str());
       ls = "";
    return;
  }
  File file = root.openNextFile();
  if (file) {
    delay(50);
    historydelname = file.name();
    delay(50);
    buffilenamedel = historydelname;
  }
  while (file) {
    file = root.openNextFile();
    chkfile++;
  }
  countfileinsd = chkfile;
}

const char* listDir(fs::FS &ff, const char * dirname, uint8_t levels) {
  const char *historyfilename;
  if((debug_method == 1) || (debug_method == 3)){
    Serial.printf("$7060: Listing directory: %s\n", dirname);
  }
  String Ld = "$7060: Listing directory: "+ String(dirname) + "\n";
  SendMQttDebug(Ld.c_str());
  Ld = "";
  
  File root = ff.open(dirname);
  if (!root) {
   if((debug_method == 1) || (debug_method == 3)){ 
     Serial.println("$7070: Error Failed to open directory");
     
    }
     Ld = "$7070: Error Failed to open directory\n";
     SendMQttDebug(Ld.c_str());
     Ld = "";
    return "";
  }
  if (!root.isDirectory()) {
   if((debug_method == 1) || (debug_method == 3)){ 
    Serial.println("$7050: Not a directory");
   } 
     Ld = "$7050: Not a directory\n";
     SendMQttDebug(Ld.c_str());
     Ld = "";
    return "";
  }

  File file = root.openNextFile();
  if (file) {
    if (file.isDirectory()) {
      if((debug_method == 1) || (debug_method == 3)){
        Serial.print("  DIR : ");
        Serial.println(file.name());
      }
      Ld = "  DIR : ";
      SendMQttDebug((Ld + file.name() + "\n").c_str());
      Ld = "";
      if (levels) {
        listDir(ff, file.name(), levels - 1);
      }
    } else {
      if((debug_method == 1) || (debug_method == 3)){
        Serial.print("  FILE: ");
        Serial.println(file.name());
      }
      Ld = "  FILE: ";
      SendMQttDebug((Ld + file.name() + "\n").c_str());
      Ld = "";
      delay(50);
      historyfilename = file.name();
      delay(50);
      if((debug_method == 1) || (debug_method == 3)){
        Serial.print("filenameinsdcard : ");
        Serial.println(historyfilename);
      }
      Ld = "filenameinsdcard : ";
      String fn = String(historyfilename);
      Ld += fn + "\n";
      SendMQttDebug(Ld.c_str());
      Ld = "";
      return historyfilename;
    }
    file = root.openNextFile();
  }
  historyfilename = "0";
  return historyfilename;

}

void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}


void readFileinSD(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  String Ld;
  if (!file) {
    if((debug_method == 1) || (debug_method == 3)){
      Serial.println("$7080: Error Failed to open file for reading");
      
    }
    Ld = "$7080: Error Failed to open file for reading\n";
    SendMQttDebug(Ld.c_str());
    
    Ld = "";
    return;
  }
   if((debug_method == 1) || (debug_method == 3)){
     Serial.print("$7030: Reading file: ");
     Serial.println(path);
   }
   Ld = "$7030: Reading file: ";
   Ld += String(path)+" OK\n";
   SendMQttDebug(Ld.c_str());
   Ld = "";
  while (file.available()) {
    datainfilesd += file.readStringUntil('\n');
  }  file.close();
}

int writeFile(fs::FS &fs, const char * path, const char * message) {
  String wf;
  if((debug_method == 1) || (debug_method == 3)){
    Serial.print("$7090: Writing file: ");
    Serial.println(path);
   
  }
   wf = "$7090: Writing file: " + String(path) + " OK\n";
   SendMQttDebug(wf.c_str());
   wf = "";
   
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
     if((debug_method == 1) || (debug_method == 3)){
      Serial.println("$7100: Error Failed to open file for writing");
     }
     wf = "$7100: Error Failed to open file for writing\n";
     SendMQttDebug(wf.c_str());
     wf = "";
    return file;
  }
  if (file.print(message)) {
    if((debug_method == 1) || (debug_method == 3)){
     Serial.println("$7110: File written OK");
    }
     wf = "$7110: File written OK\n";
     SendMQttDebug(wf.c_str());
     wf = "";
  } else {
     if((debug_method == 1) || (debug_method == 3)){
       Serial.println("$7120: Write failed");
     }
     wf = "$7120: Write failed\n";
     SendMQttDebug(wf.c_str());
     wf = "";
  }
  file.close();
  return file;
}

void WriteSDcfg(fs::FS &fs, const char * path,String cdata){
  String wf;
  
    File iotfmx = fs.open(path, FILE_WRITE);
   
   iotfmx = SD.open(path,FILE_WRITE);
    iotfmx.print(cdata);
       iotfmx.close();
    Serial.print(F("$7160: WRITE NEW CONFIG To SD card"));   
    Serial.println(cdata);
    
     wf = "$7160: WRITE NEW CONFIG To SD card  " + cdata;
     SendMQttDebug(wf.c_str());
     wf = "";
}

void deleteFile(fs::FS &fs, const char * path) {
  if((debug_method == 1) || (debug_method == 3)){
   Serial.printf("$7130: Deleting file: %s OK\n", path);
   
  }
  String Ld;
   Ld = "$7130: Deleting file: ";
   Ld += String(path)+" OK\n";
   SendMQttDebug(Ld.c_str());
   Ld = "";
   
  if (fs.remove(path)) {
    if((debug_method == 1) || (debug_method == 3)){
     Serial.println("$7140: File deleted OK");
    }
     Ld = "$7140: File deleted OK\n";
     SendMQttDebug(Ld.c_str());
     Ld = "";
  } else {
    if((debug_method == 1) || (debug_method == 3)){
      Serial.println("$7150: Error Delete failed");
       
    }
     Ld = "$7150: Error Delete failed\n";
     SendMQttDebug(Ld.c_str());
     Ld = "";
  }
}

void ReadConfig(fs::FS &fs, const char* path) {
  String Ld;
  File file = fs.open(path);
  if (!file) {
    Serial.println("$7040: Failed to open directory");
    Ld = "$7040: Failed to open directory\n";
    SendMQttDebug(Ld.c_str());
    return;
  }
   int n =0;
    while (file.available()) {
     eachline = file.readStringUntil('\n');    
      Serial.println(eachline);
      n++;
    }
  file.close();
}

void ReadSendConfig(fs::FS &fs, const char* path) {
  String Ld;
  File file = fs.open(path);
  if (!file) {
    Serial.println("$7040: Failed to open directory");
    Ld = "$7040: Failed to open directory\n";
    SendMQttDebug(Ld.c_str());
    return;
  }
   int n =0;
    while (file.available()) {
     String L = file.readStringUntil('\n');    
      int posi = L.indexOf(':');
       if(n < 19){
         Eachline += L.substring(posi+1)+",";
       }else{
         Eachline += L.substring(posi+1);
       }
      n++;
    }
  file.close();
  Eachline = "#CF,"+Eachline;
}

void SetSDcfg(fs::FS &fs, const char * path,const char* cdata){
  String p;
   File file = fs.open(path, FILE_WRITE);
        if(!file){
           Serial.println("$7170: Error Failed to open file for writing");
           String er = "$7170: Error Failed to open file for writing";
           SendError(er.c_str());
            return;
          }
         if(file.print(cdata)){
               Serial.printf("$7090: Writing file: %s\n", path);
                p = "$7090: Writing file: ";
                p += String(path)+ "\n";
                SendError(p.c_str());
                p = "";
                p = "$7170: File New config written OK\n";
               Serial.print(p);
                SendError(p.c_str());
               
           } else {
               p = "$7180: Error Write new config failed\n";
               Serial.print(p);
               SendError(p.c_str());
           }
   
}
