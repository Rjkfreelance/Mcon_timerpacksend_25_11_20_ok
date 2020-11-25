//Function Show Percent Progress OTA
void progress(DlState state, int percent){ //Show % Progress Download Firmware from Server
   if(percent == 100){
    Percent = String(percent);
   }
   Serial.printf("state = %d - percent = %d %c\n", state, percent,'%');//Print format show % progress
 
     prog = percent; 
     ota_t = millis() - startota;
     chk_ota_timeout(ota_t);//Call function for check timeout
}
 
/*  Refer to errorCallback method in HttpFOTA.h   */
void error(char *message){ //Show Error Message During OTA
  if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
    printf("$5030: Error OTA  %s\n", message);
  }
  SendError(message);
  
}

/*  Refer to startDownloadCallback method in HttpFOTA.h  */
void startDl(void){ // Start Download Firmware Function
  startota = millis();
}

void endDl(void){ //Show Time to OTA Finish Function 
  ota_t = millis() - startota;
  float otafinish = ota_t/1000.0;  //Sec
   ResetWDT(20000);
 if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){ 
  Serial.print("$5000: OTA Finish ");
  Serial.print(" use time "+String(ota_t) + " ms ");
  Serial.printf("%02f",otafinish);
  Serial.println(" Sec. OK");
 }
 String Fin = "$5000: OTA Finish "+Percent + "% " + " use time " + String(ota_t) + " ms " +String(otafinish) + " Sec OK\n";
 SendError(Fin.c_str());
 Fin = "";
 Serial.print("$5010: ");
 Serial.println(getOtaLog());//Run OTA Log database table otastatus_log 
 String api_reply = "$5010: ";
 api_reply += getOtaLog();
  
  SendError(api_reply.c_str());
 
}

void chk_ota_timeout(unsigned long tm){ //Check TimeOut OTA Function 
  if((tm >= otatimeout)&&(prog < 100)){
   Serial.printf("$5020: Error OTA Time out! %d\n",tm);
   String tmo = "$5020: Error OTA Time out! " + String(tm) + "\n";
    SendError(tmo.c_str());
   delay(50);
   ESP.restart();
  }
}

//Function for keep log OTA
String getOtaLog(){
  String urlapi = "https://www.iotfmx.com/iot/api/iotapi_otastatus.php?cid="+ String(clientId);
         urlapi += "&osta="+TimeStampOTA()+"%20OTA%20OK";
         urlapi += "&fw="+Fw;
   HTTPClient http;
   http.begin(urlapi, certssl);
   int httpCode = http.GET();
   if (httpCode == 200){
    String reply = http.getString();
    return reply;
   } else {
   Serial.println("$5040: Error code from server api " + String(httpCode));
   return "$5040: Error code from server api " + String(httpCode) + "\n";
  }      
}
