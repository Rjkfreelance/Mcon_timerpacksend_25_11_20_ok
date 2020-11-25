/*-------------------------------------------Use in IOTCallback Function------------------------------------------------*/
//Function for  List Files in Directory 
const char* listDirHist(fs::FS &fs, const char * dirname, uint8_t levels){
  const char* Fhis;
  String Lfile = "";
  int countfiles = 0;
  Serial.printf("Listing directory: %s\n", dirname);
  
   File root = fs.open(dirname);
   if(!root){
    String els = "$4030: Error Failed to open history directory\n";
     Serial.print(els);
      SendError(els.c_str());
      return "F";
   }
   if(!root.isDirectory()){
    Serial.println("Not a directory");
    return "N";
   }
  File file = root.openNextFile();
        if(!file){
          String emp = "$4040: Status Empty file history\n";
          Serial.print(emp);
          SendError(emp.c_str());
          return "E";//Empty file in Dir
        }
  while(file){
    if(file.isDirectory()){
      Serial.print(" DIR : ");
        Serial.println(file.name());
        
       if(levels){
         listDirHist(fs, file.name(), levels -1);
        }
     } else {
        //Serial.print(" FILE: ");
          //Serial.print(file.name());
           String L = file.name();
            unsigned int  S = file.size();
           Lfile  += L.substring(9)+":"+String(S)+" "; //substring from position 9 (/history/)
          countfiles++;    
     }
    file = root.openNextFile();
   }
   Lfile  += String(countfiles);
    String lh = "$4000: Event List history files in SDcard  and file size\n";
      lh += Lfile + " OK\n";
      Serial.print(lh);
      SendError(lh.c_str());
   Fhis = Lfile.c_str();
   
   return Fhis;
}

//Function for  Read data in Files history 
String readHistFile(fs::FS &fs, const char * path){
 ResetWDT(30000);// clear every 30 sec
   Serial.printf("$4010 Event Reading history file: %s\n", path);
    String rf = "$4010 Event Reading history file: " + String(path) + "\n";
    SendError(rf.c_str());
    File file = fs.open(path);
     if(!file){
        String erf = "$4030: Error Failed to open history directory\n";
        Serial.print(erf);
         SendError(erf.c_str());
        return "F";
     }
    
    String Datas;
    Datas = "$4020: Event Show Data histoy in file ";
    while(file.available()){
      Datas += file.readStringUntil('\n');
      //Serial.write(file.read());
        SendError(Datas.c_str());
    } 
    file.close();
    //Serial.println(Datas);
    
    return Datas;
 }

//Function for  Delete Files history 
const char* deleteHistFile(fs::FS &fs, const char * path){
  String df = "$4050: Event Deleting file history ";
         df += String(path) + "\n";
     SendError(df.c_str());
   Serial.printf("$4050: Event Deleting file history %s\n", path);
     const char* result;
     if(fs.remove(path)){
        String fd = "$4060: File history deleted "+String(path) +" OK";
        Serial.print(fd);
        SendError(fd.c_str());
        result = "1";
        return result;
     } else {
        String fdf = "$4070: Error File history deleted failed\n";
        Serial.println(fdf);
        SendError(fdf.c_str());
        result = "0";
        return result;
     }
}

//Function for  Delete AllFiles history 
const char* delAll(fs::FS &fs, const char * dir){
  const char* res;
  String delAll;
   File root = fs.open(dir);
   if(!root){
    String derf = "$4030: Error Failed to open history directory\n";
    Serial.print(derf);
    SendError(derf.c_str());
    }
   if(!root.isDirectory()){
    Serial.println("Not a directory");
   }
    File file = root.openNextFile();
      while(file){
        delAll = String("$4080: Prepare Delete history all ") + file.name() + "\n";
        Serial.print(delAll);
           SendError(delAll.c_str());
           delAll = "";
         if(fs.remove(file.name())){
            delAll = String("$4090: File history Deleted ( ") + file.name() + " ) OK\n";
            SendError(delAll.c_str());
            file = root.openNextFile();
         }
      }
      
     res = "1";
   return res;
}
