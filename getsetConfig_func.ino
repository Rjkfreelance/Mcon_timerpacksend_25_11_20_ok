//Function Send Config to Web Server use in IOTCallback Function
void sendconfig(){ 
         chipid=ESP.getEfuseMac();
         (uint16_t)(chipid>>32);
         String cID = String((uint16_t)(chipid>>32),HEX);
         String eID = String((uint32_t)chipid,HEX);
         cID.toUpperCase();
         eID.toUpperCase();
         String id = "id|" +cID + eID; id += "$"; //pack chip id
         String v = String(v_fw[0])+String(v_fw[1])+String(v_fw[2])+String(v_fw[3]); v = "v|" + v + "$";//pack version firmware
         String w = String(wifi_ssid); w = "w|"+w+"$";// pack WiFi SSID
         String wp = String(wifi_pwd); wp = "p|"+ wp + "$";// pack WiFi Password
         String ms = String(mqtt_server); ms = "ms|"+ ms + "$";// pack mqtt server
         String mp = String(mqttPort,DEC); mp = "mp|" + mp + "$"; //pack mqtt port 
         String u = String(mqttUser); u = "u|" + u + "$";//pack mqtt user 
         String mw = String(mqttPassword); mw = "mw|" + mw +"$"+String(getconf);//pack mqtt password
         String cfg = id + v + w + wp + ms + mp + u + mw; //Concatination 
        
         client.publish(sendconf,("CF$" +cfg).c_str()); //Publish to Cloud
          String scf = "$3000: Send Board Config: CF$ "+cfg+" OK\n";
          Serial.print(scf);
           SendError(scf.c_str());
}

//Function Send  use in IOTCallback Function
void sendId(){
         chipid=ESP.getEfuseMac();
         (uint16_t)(chipid>>32);
         String cID = String((uint16_t)(chipid>>32),HEX);//Revise By RJK
         String eID = String((uint32_t)chipid,HEX);//Revise By RJK
         cID.toUpperCase();
         eID.toUpperCase();
         client.publish(sendconf,(cID+eID).c_str()); //Publish to Cloud
         String sID = "$3010: Send ChipID From Board  "+ cID + eID + " OK\n";
           Serial.print(sID);
             SendError(sID.c_str());
}
/*
     Machine Publish Configs to cloud if subscribe CONFIG_TOPIC Callback function call this function
     For mat data packet with delimeter $ between each value and | each key|value
     such as CF$id|chipid$v|version$w|WiFi SSID$p|WiFi Password$ms|mqttserver$mp|mqtt port$u|mqtt user$mw|mqtt password$config topic 
     CF is config header , id is chip id , v is version , w is WiFi SSID , p is WiFi password , ms is mqttserver ,mp is mqtt port , u is mqtt user
        mw is mqtt password , CFG Topic 
     such as get config  of machine G00001/C01/M001 
     CF$id|c49b38e0d84$v|AB01V1$w|FMX-Wireless$p|fmxfmxfmx$ms|m20.cloudmqtt.com$mp|11580$u|jqgehwzu$mw|Xmrb3tebPNY6$CFG/G00001/C01/M001         
*/

//Function for Send Certificate(SSL) use in IOTCallback Function
String CheckCertSSL(){
   String url = "https://www.iotfmx.com/iot/api/iotapi_checkcert.php?cid="+ String(clientId);
   String cerh = "$3020: Check use Certificate SSL from Server: " + url + " OK\n";
   Serial.print(cerh);
   SendError(cerh.c_str());
   HTTPClient http;
   http.begin(url, certssl);
   int httpCode = http.GET();
   if (httpCode == 200){
    String reply;
    reply = "$3030: Server Reply  ";
    reply += http.getString();
      Serial.print(reply + " \n");
      SendError(reply.c_str());
    return reply;
   } else {
    String serer;
    serer = "$3040: Error From Server Fail. code " + String(httpCode) + "\n";
    Serial.print(serer);
    SendError(serer.c_str());
     return "Fail";
  }
}

//Function for change config by web server use in IOTCallback Function and Restart Board
void ChangeConfig_Restart(){
  String url = "https://www.iotfmx.com/iot/api/iotapi_getconfig.php?getcfg="+ String(clientId);
 Serial.println();
 String getcff = "$3050: Get New Config from Server ";
        getcff += url + "\n";
 Serial.print(getcff);
 SendError(getcff.c_str());
 
 HTTPClient http;
 http.begin(url, certssl);
  int httpCode = http.GET();
 if (httpCode == 200){
  String content = "$3060: Machine received config ";
   content += http.getString() + " OK\n";
   Serial.print(content);
   
   SendError(content.c_str());
   
    WriteSDcfg(SD,"/conf.txt",content);
    
    client.publish(sendconf,"Write Config OK",Qos);
    digitalWrite(RST,!digitalRead(RST));
    delay(500);
    
   
  } else {
    String ercf = "$3070: Error From Server Reply Config" + String(httpCode) + "\n";
    Serial.print(ercf);
      SendError(ercf.c_str());
  }
}

//Function for change config by web server use in IOTCallback Function
void ChangeConfig(){
 String url = "https://www.iotfmx.com/iot/api/iotapi_getconfig.php?getcfg="+ String(clientId);
 Serial.println();
 Serial.println("Get Coonfig from Server: " + url);
 HTTPClient http;
 http.begin(url, certssl);
  int httpCode = http.GET();
 if (httpCode == 200){
   String content = http.getString();
   Serial.println(content);
    WriteSDcfg(SD,"/conf.txt",content);
    client.publish(sendconf,"Write Config OK",Qos);
  } else {
   Serial.println("Fail. error code " + String(httpCode));
  }
}

//Function for check config by web server use in IOTCallback Function
void currentConfig(fs::FS &fs, const char* path){

  String Content = "$3080: Machine send current config to Webserver\n";
  File frc = fs.open(path);
  if (!frc) {
    String oer = "$3090: Error Failed to open directory config\n";
    Serial.print(oer);
    SendError(oer.c_str());
    return;
  }
  while(frc.available()){
     Content += frc.readStringUntil('\n');
    }
   frc.close();
  Serial.println(Content);
  
  client.publish(sendconf,Content.c_str());

  SendError(Content.c_str());
  
}
