void taskLEDStatus_NOMQTT(void*){
 
  VOID SETUP(){      
      pinMode(xLED2,OUTPUT);
      
   }
  VOID LOOP(){      
    
     digitalWrite(xLED2,LOW);
     DELAY(xTimeBlk_nomqtt);
     digitalWrite(xLED2,HIGH);
     DELAY(xTimeBlk_nomqtt);
    }
   
}
