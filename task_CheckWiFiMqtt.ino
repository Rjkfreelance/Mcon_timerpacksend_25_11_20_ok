void taskCheckWiFi_and_Mqtt(void*){
  int countLoss = 0;
 VOID SETUP(){      
  }
 VOID LOOP(){    // Infinite Loop
  
   if(WiFi.status() != WL_CONNECTED){
     ResetWDT(30000); //Clear Watchdog every 30 sec.
       task_LedStatus.stop();
       digitalWrite(xLED2,LOW);// เมื่อ WiFi หลุดการเชื่อมต่อ LED2 ติดค้าง GPIO Active Low
          countLoss++;
          Serial.println(String("wifi Loss reconnecting...")+ countLoss);
           //if(countLoss >= 199) ESP.restart();
         DELAY(100); //delay count
       
   }else{  
    Serial.println();
    Serial.println("WiFi Reconnected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Connection ok");
    task_LedStatus.resume();
   }
   if(!client.connected()){
         ResetWDT(30000); //Clear Watchdog every 30 sec.
          task_LedStatus.stop();
           if(WiFi.status() != WL_CONNECTED){
             digitalWrite(xLED2,LOW);
           }else{
            task_LedStaNOMQ.start(taskLEDStatus_NOMQTT);
           }
        Serial.println("Try to connecting MQTT...");
        /* connect now */
         if(checkSDcard() == 1){
           if (client.connect((const char*)clientId.c_str(),(const char*)mqttUser.c_str(), (const char*)mqttPassword.c_str())){
      
              Serial.println("Mqtt....connected case SDcard");
              /* subscribe topic */
              client.subscribe(stopic,Qos);
             //client.subscribe("OTA/G90000/C01/M001",Qos);
            //client.subscribe(stopic,Qos);
             String Ts = "OTA/"+getSomeConfigAssign();
             client.subscribe((const char*)Ts.c_str(),Qos); // Topic OTA self machine 
        
            //client.subscribe(gtopic,Qos);
            String Tg = "OTA/"+GroupComp(1);
            client.subscribe((const char*)Tg.c_str(),Qos);
        
            //client.subscribe(ctopic,Qos);
             String Tc = "OTA/"+GroupComp(2);
             client.subscribe((const char*)Tc.c_str(),Qos);
          
             client.subscribe(getconf,Qos);
             //String Rp = "RM/"+getSomeConfigAssign();
             client.subscribe(dbreply,Qos);
            //client.subscribe((const char*)Rp.c_str(),Qos);
        
        
           //client.subscribe(history,Qos);
           String H = "HFILES/"+getSomeConfigAssign();
           client.subscribe((const char*)H.c_str(),Qos);
        
           client.subscribe(readhis,Qos);
           client.subscribe(delFile,Qos);
           client.subscribe(delAFile,Qos);
        
           // client.subscribe(debug_allow,Qos);
           String dal = "DEBUG/ALLOW/"+getSomeConfigAssign();
           client.subscribe((const char*)dal.c_str());
        
           client.subscribe(resetm,Qos);
           task_LedStatus.resume();
           task_LedStaNOMQ.stop();
           }
        }else{
          if (client.connect((const char*)clientId.c_str(),(const char*)mqttUser.c_str(), (const char*)mqttPassword.c_str())){
            Serial.println("Mqtt....connected case FRAM");
            Msgstart += "Mqtt....connected\n";
            String Ts = "OTA/"+ReadClidSlash_FRAM();;
            client.subscribe((const char*)Ts.c_str(),Qos);
          
            //client.subscribe(gtopic,Qos);
            String Tg = "OTA/"+GroupCompFRAM(1);
            client.subscribe((const char*)Tg.c_str(),Qos);
        
            //client.subscribe(ctopic,Qos);
            String Tc = "OTA/"+GroupCompFRAM(2);
            client.subscribe((const char*)Tc.c_str(),Qos);
        
            client.subscribe(getconf,Qos);
            //String Rp = "RM/"+ReadClidSlash_FRAM();
             client.subscribe(dbreply,Qos);
            //client.subscribe((const char*)Rp.c_str(),Qos);
        
        
            //client.subscribe(history,Qos);
            String H = "HFILES/"+ReadClidSlash_FRAM();
            client.subscribe((const char*)H.c_str(),Qos);
          
            client.subscribe(readhis,Qos);
            client.subscribe(delFile,Qos);
            client.subscribe(delAFile,Qos);
        
           // client.subscribe(debug_allow,Qos);
           String dal = "DEBUG/ALLOW/"+ReadClidSlash_FRAM();
           client.subscribe((const char*)dal.c_str());
        
           client.subscribe(resetm,Qos);
           task_LedStatus.resume();
           task_LedStaNOMQ.stop();
          }
       }
     } 
     DELAY(5000);
    }
 
}
