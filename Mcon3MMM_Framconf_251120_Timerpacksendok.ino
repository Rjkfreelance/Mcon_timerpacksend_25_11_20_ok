/*-------------------------Code modify  19/11/2020 -------------------------*/
//Modify Test on 19/11/2020 Timer pack send use HW Timer1
/*-------------------------Firmware Feature---------------------------------*/
//Send realtime have config sendperiod (5,10,15 sec)
//Task Packdata IF Write SDcard 
//Can READ,DELETE HISTORY BY WEB APP  www.iotfmx.com/history 
//Can OTA if option 1 OTA By Webserver www.iotfmx.com (if option 0 OTA By Local WiFi Network)
//Can Debug  Via MQTT ,Serial && Mqtt config Topic www.iotfmx.com/debugmonitor
//Additional  Task WDT (Clear Watchdog)
//Additional OTA Save Log
//Additinal Read config,Certificate from Flash Memory if SDcard fail open 07/09/2020
//Modify with show ERROR Code on and support commend reset from web debug 28/09/2020 
//Modify Pack Send with Timer1 and Reduce config (SDcard & FlashFile System)13/10/2020
//Modify Read config from SDcard write to FRAM  12/11/2020
//Modify Task Pack Send Data period send  12/11/2020

#include <WiFi.h>
#include <PubSubClient.h> 
#include <HttpFOTA.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "time.h"
#include <RTClib.h>
#include <Adafruit_MCP3008.h>
#include <Machine.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <TridentTD_EasyFreeRTOS32.h>
#include <esp_task_wdt.h>
#include "SPIFFS.h"

#define WDT_TIMEOUT 180 //watchdog timeout 90 sec
int last = millis(); //use in ResetWDT function
long countSec; //show seconds reset wdt

/*------------GPIO---------------*/
#define xLED2 13 //CS2# GPIO 13 
unsigned int xTimeBlink = 500; //LED Blink Status
unsigned int xTimeBlk_nomqtt = 100;
#define RST 12 //GPIO 12 for reset


String cfgs[10];
String Certssl;

#define Cert_addr 25700  //Start address

#define FRAM_Flag_addr 31000 //start address

/*------------FRAM ADDRESS FOR STORAGE CONFIG Modify Read config from SDcard write to FRAM---------------*/
#define ssid_addr 30000 // WiFi SSID MaxLength  20 characters start address
#define wpass_addr 30020 // WiFi PASS MaxLength  20 characters
#define mqserv_addr 30040 // WiFi PASS MaxLength  30 characters
#define mqport_addr 30070 // MQTTPORT MaxLength  5 characters
#define mquser_addr 30075 // MQTTUSER MaxLength  30 characters
#define mqpass_addr 30105 // MQTTPASS MaxLength  30 characters
#define clientid_addr 30135 // CLIENTID MaxLength  15 characters
#define otatimeout_addr 30150 // OTATIMEOUT MaxLength  5 characters
#define sendperiod_addr 30155 // SENDPERIOD MaxLength  5 characters
#define cirmachine_addr 30160 // CIRMACHINE MaxLength  1 characters

#define ssid_size 20
#define wpass_size 20
#define mqserv_size 30 //Byte
#define mqport_size 5 //Byte
#define mquser_size 30 //Byte
#define mqpass_size 30 //Byte
#define clientid_size 15 //Byte
#define otatimeout_size 5 //Byte
#define sendperiod_size 5 //Byte
#define cirmachine_size 1 //Byte


/*--------------Config Variable---------------------*/
String wifi_pwd; //WiFi Password
String wifi_ssid;//WiFi SSID
String mqtt_server;//Mqtt cloudserver
int mqttPort;//Mqtt cloudport
String mqttUser;//Mqtt Username
String mqttPassword;//Mqtt Password
String clientId;//Client ID for this Machine
int otatimeout; // OTA Timeout limit 45 sec
String sendh = "MMM/";
const char* sendtopic; // Machine send data Topic
String otah = "OTA/";
const char* gtopic; //OTA Group Topic 
const char* ctopic; //OTA Sub Companny Topic
const char* stopic; //OTA Self Machine Topic
const char* ackota = "OTA/ACK"; //OTA Acknowledge use for Machine confirm received OTA
String cfgh = "CFG/";
const char* getconf; // //Topic of this machine subscribe (or listen) for receive command from web socket command getcf(get config)
const char* sendconf = "GETCFG"; // Topic for Machine send config back to(publish to) web server (use web socket)
String rm = "RM/";
const char* dbreply;//Topic for check db active  Server Reply OK if Insert data already  
String sendperiod;//Config send every msec (Additional 18/06/2020)
int cirmachine;//Config Type of OTA (Additional 03/07/2020)
/*-----------------Modify on 22/07/2020--------------------------*/
String debugallow = "DEBUG/ALLOW/";
String debug_allow; //for Board Debug Mode Monitor(mode 1,2,3) Via MQTT
String dbugh = "DEBUG/";
const char* debugs; // Topic Send Debug Monitor Via MQTT
String rstm = "DEBUG/RST/";
const char* resetm; //Topic for reset from web debug (290820)
/*--------------Config Variable---------------------*/



/*--------------------------For Demo G90000----------------------------*/
/*-------------------------Topic For Check History(revise 27/07/2020)----------------------*/
/*--History check by web app  www.iotfmx.com/history --*/

/*----------------------Not Include to Config SDcard-----------------------*/
/*
const char* history = "HFILES/G90000/C01/M001"; //Topic subscribe command gethist  from web send historyfile.txt and amount of file (see function IOTCallback)
const char* readhis = "RFILES/G90000/C01/M001"; //Topic subscribe command for readfile Rhistoryfile  from web send (see function IOTCallback)
const char* delFile = "DFILES/G90000/C01/M001"; //Topic subscribe command for delete file Dhistoryfile  from web send (see function IOTCallback)
const char* delAFile = "DAFILES/G90000/C01/M001"; //Topic subscribe command delhis for delete all files history (see function IOTCallback)
*/

/*----------------------See Tab history_func---------------------------*/


/*--------------------------For check history config----------------------------*/
String gcm = "";
String h1 = "HFILES/";
String h2 = "RFILES/";
String h3 = "DFILES/";
String h4 = "DAFILES/";
const char* history;
const char* readhis; 
const char* delFile; 
const char* delAFile; 



int debug_method = 0; //Flag for debug mode
String Msgstart; // log message for restart
String Sdcardforhis; // log for open sdcard history

int counttestWdt = 1; //For test WDT 

/*-----------------Modify on 16/07/2020--------------------------*/

/*--------------Config Variable---------------------*/

String eachline;// String  Object for receiving each line in file conf.txt

char* certssl; // SSL Certification for download firmware OTA Load from certi.txt
String  Certs = "";// String Object for concatination certification from file certi.txt


/* String Object array for keep config each line in file conf.txt in SD Card */
String Line[10]; //config lines array

File iotfmx; //create object root from Class File use SD Card use write new config to SD Card
File root; //create object root from Class File use SD Card read from SD Card and assign to config variable

/*----------TIME NTP Server-------------*/
//const char* ntpServer = "time.uni.net.th";
//const char* ntpServer = "pool.ntp.org";
const char* ntpServer = "1.th.pool.ntp.org";//NTP Server Use Current
const long  gmtOffset_sec = 7 * 3600; // GMT+7 Thailand Time
const int   daylightOffset_sec = 0; 

volatile char datareceivedmqtt[2]; // receive OK from DBserv

#define Qos  1 //Quality of Service Mqtt

uint64_t chipid;  //Declaration for storage ChipID
unsigned long tnow = 0; //Init time for plus current time
unsigned long  startota = 0; //Initial time for counter 
int ota_t; //time difference
int prog; //percent download progress
String Percent;//for keep % finish OTA (add 27/07/20)

bool errsd = false;
  
WiFiClient FMXClient; //Create Object WiFi Client

PubSubClient client(FMXClient); // Use Mqtt Constructor

Machine mac;// Create Machine Object (see Lib Class Machine  Machine.h , Machine.cpp)

RTC_DS3231 RTC; // Create RTC Object

Adafruit_MCP3008 adc; //Create  I/O Port Expander Object

/*-----------------------Machine-------------------------*/
/*----------Define IO Use Method int Machine::READ_DATASW(int pin) in Machine.cpp ---------*/
#define IO_1 mac.READ_DATASW(sw1)
#define IO_2 mac.READ_DATASW(sw2)
#define IO_3 mac.READ_DATASW(sw3)
#define IO_4 mac.READ_DATASW(sw4)
#define IO_5 mac.READ_DATASW(sw5)
#define IO_6 mac.READ_DATASW(sw6)
#define IO_7 mac.READ_DATASW(sw7)
#define IO_8 mac.READ_DATASW(sw8)
/*----------Define IO Use Method int Machine::READ_DATASW(int pin) in Machine.cpp ---------*/
#define writeaddr_eeprom1 32001 // See define in Machine.h
#define writeaddr_eeprom2 32002 // See define in Machine.h
#define FILE_COUNT_INHISTORYSD 31250 //1GB:1,000,000KB
#define addrsize 128 // Data in page 128 Byte (see Machine.h #define TOTAL_PAGE 128)

#define time_limitwifi  5 // wifi timeout not use

char DATA_PACKHEADHIS[16];//History Protocol(#R) Header Use in Function packdata_HEADSDCARD() see tab Machine_func
char DATA_PACKHEAD[21];// Realtime Protocol(#M) Headr Use in Function packdata_HEAD() see tab Machine_func
char DATA_PACKDATE[6];// Pack datetime current see tab Machine_func
char DATA_PACKIO[2];// Pack data input signal see tab Machine_func
char Chipid_buf[12]; // Char Array for Keep ChipID Of MCU ESP32 see void loop() chipid = ESP.getEfuseMac();
char filnamechar[12];
char buf_date[12];//Buffer datetime use in tab Machine_func function packdata_DATE()
char buf_io[4]; //Buffer I/O Use in  function sendmqtt()
char v_fw[4];//Keep Firmware version type

/*---------------------------------Pack Data Signal-----------------------------------------*/
unsigned char DATA_PACKPWM1[4];
unsigned char DATA_PACKPWM2[4];
unsigned char DATA_PACKRELAY[1];
unsigned char DATA_PACKPWM3[4];
unsigned char DATA_PACKPWM4[4];
unsigned char DATA_PACKAD1[4];
unsigned char DATA_PACKAD2[4];
unsigned char DATA_PACKAD3[4];
unsigned char DATA_PACKGPS[6];
unsigned char DATA_PACKM1[8];
unsigned char DATA_PACKM2[3];
unsigned char DATA_PACKM3[3];

const char* datasaveram;//For keep packdata pre write to FRAM
const char* datamqtt;// Keep data format pre publish to cloudMqtt sendrealtime
const char* datamqttinsdcard;//Keep data format pre publish to cloudMqtt sendhistory
const char* filenamesavesd;//Keep path /history/filehistory.txt

unsigned int data_IO;
unsigned int write_addeeprom;
int countfileinsd = 0;
int buf_head = 0;
int bufwrite_eeprom1, bufwrite_eeprom2;
int read_packADD;
int time_outwifi = 0;
int checksettime = 0;
int filename = 0;
int checksendmqtt = 0;

String sDate;
String filenames;
String datainfilesd;
String Headerhistory = "";
String buffilenamedel;

long time_out = 0;
long time_limit = 100;

/*-----Additional Var---------*/

unsigned long periodSend;// keep period send realtime
uint64_t delays; // tics/sec (18/10/20)
uint64_t Delays;

unsigned long last_time = 0; // Start in millis()
unsigned long last_err = 0;
unsigned long periodSend_error = 30000;//Period for send error sd card send 15 sec (29/09/20)
bool flag_sd;//Flag check sd card (29/09/20)


String xdataContinue[23];//String array keep in task packdata
//const char* xdataContinue;
const char* dataToram; 
/*-----Additional Var---------*/

/*---------------Serial Command----------------*/
String inputCmd = "";         // a string to hold incoming data
boolean DinComplete = false;  // whether the string is complete

String Eachline;// String  Object receiving each line in file conf.txt

/*-------------- String Array for Write Config By Serial Interrupt Use in SerialReadConfig() ----------------------*/
String headcfg[] = {"wifissid:","wifipass:","mqserv:","mqport:","mquser:","mqpass:","clientId:","otatimeout:","send:","otagroup:","otacompany:","otamachine:","DBREPLY:","getconfg:","sendperiod:","otaack:","sendconfig:","debug_allow:","debug_send:","cirmachine:"};

/*-----------------------Use in Case OTA See void Loop() case -----------------------------------*/

volatile int interruptCounter1;
int totalInterruptCounter1;
volatile int interruptCounter2;
int totalInterruptCounter2; 
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


typedef enum {
    Runnning_e = 0x01,
    Fota_e  
}SysState;

SysState state = Runnning_e;  //Create an instance state

/*-----------------Firmware Source Download------------------------*/
char url[100];//Url firmware download 
char md5_1[50];// md5 checksum firmware filename .bin
String Fw;//Firmware Filename (.bin)


//Add  22-06-20
//Check Period send 
void checkSendTime(){ //Set Send Period 3 Levels send every 5,10,15 Sec 
   if(sendperiod.length() == 5){
    //Serial.print(sendperiod[0]);
    //Serial.println(sendperiod[1]);
    if((sendperiod[0] == '1')&&(sendperiod[1] == '0')){
       periodSend = 10000;
       Serial.println("$1050: Send Every 10 Sec. OK");
       Msgstart += "$1050: Send Every 10 Sec. OK\n";
     }
     if((sendperiod[0] == '1')&&(sendperiod[1] == '5')){
       periodSend = 15000;
        Serial.println("$1060: Send Every 15 Sec. OK");
        Msgstart += "$1060: Send Every 15 Sec. OK\n";
     } 
     if((sendperiod[0] == '2')&&(sendperiod[1] == '0')){
       periodSend = 20000;
       Serial.println("$1050: Send Every 20 Sec. OK");
       Msgstart += "$1050: Send Every 20 Sec. OK\n";
     }
      if((sendperiod[0] == '3')&&(sendperiod[1] == '0')){
       periodSend = 30000;
       Serial.println("$1050: Send Every 30 Sec. OK");
       Msgstart += "$1050: Send Every 30 Sec. OK\n";
     }
    
   }
   if(sendperiod.length() == 4){
     if((sendperiod[0] == '5')&&(sendperiod[1] == '0')){
       periodSend = 5000;
       Serial.println("$1070: Send Every 5 Sec. OK");
       Msgstart += "$1070: Send Every 5 Sec. OK\n";
     }
   }
}

void ChipID(){//Show Chip ID
  chipid=ESP.getEfuseMac();//The chip ID is  MAC address(length: 6 bytes).
  Serial.printf("$1080: Read Machine Board Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  
  String ch = "$1080: Read Machine Board Chip ID = ";
  String cID = String((uint16_t)(chipid>>32),HEX);
         String eID = String((uint32_t)chipid,HEX);
         cID.toUpperCase();
         eID.toUpperCase();
    Msgstart += ch + cID + eID + " OK\n";
    Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
 
}

//Call Back Function

void IOTCallback(char *topic, byte *payload, unsigned int length){
  
  //Serial.println((char*)payload);//print payload (or message) in topic received
  
  /*-------------------------------Topic for OTA-------------------------------------*/
  
  if((strncmp(gtopic, topic, strlen(gtopic)) == 0)||(strncmp(ctopic, topic, strlen(ctopic)) == 0)||(strncmp(stopic, topic, strlen(stopic)) == 0)){
   String stota = "$1210: Prepare OTA \n";
    if((debug_method == 1) || (debug_method == 3)){
      Serial.println((char*)payload);//print payload (or message) in topic received
    }
  
    memset(url, 0, 100);
    memset(md5_1, 0, 50);
    char *tmp = strstr((char *)payload, "url:");//Query url: in payload(message received) and assign to pointer tmp
    char *tmp1 = strstr((char *)payload, ",");//Query , in payload(message received) and assign to pointer tmp1
    memcpy(url, tmp+strlen("url:"), tmp1-(tmp+strlen("url:")));
    
    char *tmp2 = strstr((char *)payload, "md5:");//Query md5: in payload(message received) and assign to pointer tmp2
    memcpy(md5_1, tmp2+strlen("md5:"), length-(tmp2+strlen("md5:")-(char *)&payload[0]));
    
    if((debug_method == 1) || (debug_method == 3)){
       Serial.print(stota);
       Serial.printf("started fota url: %s\n", url);
       Serial.printf("started fota md5: %s\n", md5_1);
    }
    SendMQttDebug(stota.c_str());
    SendMQttDebug(url); //Send to Mqtt debug
    SendMQttDebug(md5_1);//Send to Mqtt debug
    
    client.publish(ackota,stopic);// Publish message OTA TOPIC if acknowledge OTA backward to web server 
    state = Fota_e; //Start state Firmware OTA
   }
   
   /*  Check topic received and payload (message) is equal command for getconfig (getcf)  */
  if(strncmp(getconf, topic, strlen(getconf)) == 0){//Cmd from php or websocket getcf (Get config)
   if((debug_method == 1) || (debug_method == 3)){
    Serial.println((char*)payload);//print payload (or message) in topic received
   }
    
    if((char)payload[0] == 'g' && (char)payload[1] == 'e' && (char)payload[2] == 't' && (char)payload[3] == 'c' && (char)payload[4] == 'f'){
        sendconfig();
    }
    if((char)payload[0] == 'g' && (char)payload[1] == 'e' && (char)payload[2] == 't' && (char)payload[3] == 'i' && (char)payload[4] == 'd'){
        sendId();
    }
    if((char)payload[0] == 'c' && (char)payload[1] == 'f'){//Change config by https get to server iotfmx.com
        ChangeConfig();
     }
    if((char)payload[0] == 'c' && (char)payload[1] == 'f' && (char)payload[2] == 'b' ){//Change config by https get to server iotfmx.com
        ChangeConfig_Restart();
      } 
    if((char)payload[0] == 'c' && (char)payload[1] == 'c' && (char)payload[2] == 'f'){//Check current config
       if(checkSDcard() == 1){
         currentConfig(SD,"/conf.txt");
       }else{
         currentConfigFRAM();
       }
     }
    if((char)payload[0] == 'c' && (char)payload[1] == 's'){// Check Cert SSL
       String rep = CheckCertSSL();
       client.publish(sendconf,rep.c_str(),Qos);
     }
    if((char)payload[0] == 'c' && (char)payload[1] == 'l' && (char)payload[2] == 'r'){
     if(checkSDcard()){
      const char* Clrhis;
      Clrhis = delAll(SD,"/history");
      CleraFRAM2();
       String msg = "$1250: Clear memory success";
       client.publish(debugs,msg.c_str());
       Serial.println("Clear Buffer");
       
     }else{
      CleraFRAM2();
       String msg = "$1250: Clear memory success";
       client.publish(debugs,msg.c_str());
       Serial.println("Clear Buffer");
     }
    }
   }
   
   /*--------------------------For Check DataBase Server Reply -----------------------------*/
   if (strncmp(dbreply, topic, strlen(dbreply)) == 0) {
     //Serial.println((char*)payload);//print payload (or message) in topic received
     for (int r = 0; r < length; r++) {
      //Serial.write((char)payload[r]);
        datareceivedmqtt[r] = (char)payload[r];
      //         delay(20);
    }

    Serial.println();
   }
  if (strncmp(history, topic, strlen(history)) == 0){
      
      Serial.println((char*)payload);//print payload (or message) in topic received
      
      if((char)payload[0] == 'g' && (char)payload[1] == 'e' && (char)payload[2] == 't' && (char)payload[3] == 'h' && (char)payload[4] == 'i' && (char)payload[5] == 's'){
         const char* histfiles; 
          histfiles = listDirHist(SD,"/history",0);
          Serial.println(histfiles);
          client.publish(history,histfiles);
      }
   }

   if (strncmp(readhis, topic, strlen(readhis)) == 0){
    Serial.println((char*)payload);//print payload (or message) in topic received
      if((char)payload[0] == 'R'){
      String fda,Fpub;
     
      for(int p=0;p<length;p++){
        if(p>0){
          fda += (char)payload[p];
        }
      }
       Fpub = readHistFile(SD,fda.c_str());
       Serial.println(Fpub);
       client.publish(readhis,Fpub.c_str());
      }
    }
   if (strncmp(delFile, topic, strlen(delFile)) == 0){
    Serial.println((char*)payload);//print payload (or message) in topic received
       if((char)payload[0] == 'D'){
          String dFa;
          const char* Dpub;
          for(int p=0;p<length;p++){
            if(p>0){
              dFa += (char)payload[p];
            }
          }
          Dpub = deleteHistFile(SD,dFa.c_str());
          client.publish(delFile,Dpub);
       }
     }
   if (strncmp(delAFile, topic, strlen(delAFile)) == 0){
    Serial.println((char*)payload);//print payload (or message) in topic received
    if((char)payload[0] == 'd' && (char)payload[1] == 'e' && (char)payload[2] == 'l' && (char)payload[3] == 'h' && (char)payload[4] == 'i' && (char)payload[5] == 's'){
       const char* DApub;
       DApub = delAll(SD,"/history");
       client.publish(delAFile,DApub);   
     }
   }

   /*--------------------Debug Method(3 Modes)---------------------*/
    if (strncmp((const char*)debug_allow.c_str(), topic, strlen((const char*)debug_allow.c_str())) == 0){
      
      Serial.println((char*)payload);//print payload (or message) in topic received
      
      if((char)payload[0] == '1'){
       
       debug_method = 1; //debug via serial port only
         
      }
      if((char)payload[0] == '2'){
       
        debug_method = 2; //debug via mqtt only
      }
      if((char)payload[0] == '3'){
        
        debug_method = 3; //debug via serial port and mqtt 
      }
    
   }   
   /*--------------------Reset From Web Debug---------------------*/
    if (strncmp(resetm,topic, strlen(resetm)) == 0){
      if((char)payload[0] == 'r' && (char)payload[1] == 's' && (char)payload[2] == 't'){
        delay(100);
        ESP.restart(); 
      }
    }
}
 
 //Function Mqtt Connection
int mqttconnect_FRAM(){
  //while(1){
   /* connect now */
    if (client.connect((const char*)clientId.c_str(),(const char*)mqttUser.c_str(),(const char*)mqttPassword.c_str())) {
      
      Serial.println("Mqtt....connected");
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
         return 1;
      }
     
   //}
}
int mqttconnect() {
  /* Loop until reconnected */
 //while(1){
 
   /* connect now */
    if (client.connect((const char*)clientId.c_str(),(const char*)mqttUser.c_str(),(const char*)mqttPassword.c_str())) {
      
      Serial.println("Mqtt....connected");
      Msgstart += "Mqtt....connected\n";
      /* subscribe topic */
      //Serial.println(stopic);
        
        String Ts = "OTA/"+getSomeConfigAssign();
        client.subscribe((const char*)Ts.c_str(),Qos);
          
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
        
       return 1;
      
    }
   
  //}
  
}

//Function WiFi Connection



void wifi_setup()
{ 

  ResetWDT(15000); //Clear Watchdog every 15 sec.
  Serial.print("Connecting to ");
  Msgstart += "Connecting to ";
  Serial.println(wifi_ssid);
   Msgstart += String(wifi_ssid)+"\n";
   
  WiFi.mode(WIFI_STA);
  WiFi.begin((const char*)wifi_ssid.c_str(),(const char*)wifi_pwd.c_str()); //assign wifi ssid , pass
  int countloss = 0;
  while (WiFi.status() != WL_CONNECTED) {//Loop is not connect until connected exit loop
    countloss++;
    delay(500);
    Serial.print(".");
     Msgstart += ".";
     if(countloss >10){
      goto Exitpass;
     }
  }
  Serial.println("");
  Serial.println("WiFi connected");
   Msgstart += "\nWiFi connected\n";
  Serial.print("IP address: ");
   Msgstart += "IP address: " + WiFi.localIP().toString() + "\n";
  Serial.println(WiFi.localIP());
  Serial.println("Connect ok");
   Msgstart += "Connect ok\n";
   Exitpass: Serial.println("Wifi Not connect...");
}

/*------------------------For StampTime Log OTA to DataBase------------------------------*/
String TimeStampOTA(){
  DateTime now = RTC.now();
  String DMY = String(now.day())+"/"+String(now.month())+"/"+String(now.year())+ "%20" +String(now.hour())+":"+String(now.minute())+":"+String(now.second())+"%20";
   return DMY; 
}
/*------------------------For StampTime Log OTA to DataBase------------------------------*/
//Function for show Datetime
String DateTimeNOW(){
   DateTime now = RTC.now();
   String DMY = String(now.day())+"/"+String(now.month())+"/"+String(now.year())+" "+String(now.hour())+":"+String(now.minute())+":"+String(now.second());
   return DMY;
}

//Function DateTime and Set RTC
void data_time()
{
  DateTime now = RTC.now();
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("$1220: Error Failed to obtain time");
    
    return;
  }
  //  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //    Serial.printf("NTP TIME : %02d/%02d/%04d ",timeinfo.tm_mday,timeinfo.tm_mon + 1,timeinfo.tm_year + 1900);
  //    Serial.printf("%02d:%02d:%02d \r\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  if (checksettime == 0 ) {
    RTC.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    checksettime = 1;
  }
  else
  {
    if (timeinfo.tm_wday == 0 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec <= 60)
    {
      RTC.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
      Serial.println("$1110: Update Time Success");
    }
  }
  if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
  Serial.printf("%02d", now.day());
  Serial.print('/');
  Serial.printf("%02d", now.month());
  Serial.print('/');
  Serial.printf("%02d", now.year());
  Serial.print(' ');
  Serial.printf("%02d", now.hour());
  Serial.print(':');
  Serial.printf("%02d", now.minute());
  Serial.print(':');
  Serial.printf("%02d", now.second());
  Serial.println();
  }
  
}

void WriteSDhistory(){
  
  if (write_addeeprom >= 25599) //  //ถ้าเขียนถึง address ที่ 25599 ให้เอาข้อมูลทั้งหมดใส่ใน file sdcard
  {
    interruptCounter1 = 0;
     
    if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){ 
      Serial.println("Please wait for read RAM To SDCARD");
    }
    String pw = "$1240: Please wait for read RAM To SDCARD\n";
    Serial.print(pw);
    SendError(pw.c_str());
  
    pw = "";
    String datab;
    const char * datasavesdcard;
    datab = mac.read_all();
    datasavesdcard = datab.c_str();
    listcountfileindir(SD, "/history");
    //delay(100);
    //delay(10);
    if (countfileinsd >= FILE_COUNT_INHISTORYSD) { //FILE_COUNT_INHISTORYSD
      const char * delfile;
      delfile = buffilenamedel.c_str();
      deleteFile(SD, delfile);
      countfileinsd = 0;
    }
    String a = "/history/" + filenames + ".txt";
    File file = SD.open("/history");
    if (!file)
    {
      if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
        Serial.println("Create Directory");
      }
      pw = "Create Directory\n";
      SendMQttDebug(pw.c_str());
      pw = "";
      SD.mkdir("/history");
    }
    filenamesavesd = a.c_str() ;
    //    writeFile(SD, filenamesavesd , datasavesdcard);
    if (!writeFile(SD, filenamesavesd , datasavesdcard)) {
      if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
        Serial.println("$1180: ******** Write DATA TO SDCARD Success ********");
      }
       pw = "$1180: ******** Write DATA TO SDCARD Success ********\n";
       SendMQttDebug(pw.c_str());
       pw = "";
      
      
       CleraFRAM2();
       
      //Create file in sd card success update address eeprom = 0
      mac.writeAddress(writeaddr_eeprom1, 0); 
      mac.writeAddress(writeaddr_eeprom2, 0);
      write_addeeprom = 0;
      filename++;
      
      
       
       
    } else {
       if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
          Serial.println("$1140: Error Can't Save SD Card To RAM");
       }
       pw = String("$1140: Error Can't Save SD Card To RAM\n");
       SendMQttDebug(pw.c_str());
       pw = "";
    }
   
 }
}

//Function  Send data Mqtt
String sendmqtt()
{
  bufwrite_eeprom1 = mac.readAddress(writeaddr_eeprom1);
  bufwrite_eeprom2 = mac.readAddress(writeaddr_eeprom2);
  write_addeeprom = (bufwrite_eeprom2 << 8) + bufwrite_eeprom1;
  String sText;
  String dbfr;
  sText += sDate;
  for (int i = 0; i < sizeof(buf_io); i++)
  {
    sText += buf_io[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKPWM1); i++)
  {
    sText += DATA_PACKPWM1[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKPWM2); i++)
  {
    sText += DATA_PACKPWM2[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKRELAY); i++)
  {
    sText += DATA_PACKRELAY[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKPWM3); i++)
  {
    sText += DATA_PACKPWM3[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKPWM4); i++)
  {
    sText += DATA_PACKPWM4[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKAD1); i++)
  {
    sText += DATA_PACKAD1[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKAD2); i++)
  {
    sText += DATA_PACKAD2[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKAD3); i++)
  {
    sText += DATA_PACKAD3[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKGPS); i++)
  {
    sText += DATA_PACKGPS[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKM1); i++)
  {
    sText += DATA_PACKM1[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKM2); i++)
  {
    sText += DATA_PACKM2[i];
  }
  sText += ";" ;
  for (int i = 0; i < sizeof(DATA_PACKM3); i++)
  {
    sText += DATA_PACKM3[i];
  }
  sText += ";" ;

  datasaveram = sText.c_str();
 // delay(50);
 //delay(10);
 if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){ // Debug mode 0 serial-port and mqtt
  Serial.print("$1110: Data For PackSendMQTT : ");
  Serial.println(sText);
 }
  SendMQttDebug(DateTimeNOW().c_str());
  SendMQttDebug(("$1120: Data For PackSendMQTT : "+sText).c_str());
  
 if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){ 
    Serial.print("$1130: EEPROM ADDR : ");
    Serial.print(write_addeeprom);
    Serial.println(" OK");
 }
 
  String EEdebug = "$1130: EEPROM ADDR : ";
  SendMQttDebug((EEdebug + String(write_addeeprom) + " OK\n").c_str());
  EEdebug = "";
  
  
 
   
   
 for (int i = 0; i < addrsize; i++)
  {
    mac.writeAddress(write_addeeprom, datasaveram[i]);
    write_addeeprom++;
  }

  /*--------------------Start FRAM DEBUG 000H -> 200H----------------------------*/

 Serial.print("BYTE:         ");
 dbfr += "BYTE:                ";
 Serial.print("1  2   3   4   5   6   7   8  9  10  11  12  13 14  15  16  17  18 19 20  21  22  23 24  25  26  27  28 29  30");//Fix position 

 Serial.println();
 
 dbfr += "1   2    3    4    5    6   7   8   9   10  11  12  13 14 15 16  17  18 19 20  21 22 23 24  25  26  27  28 29  30";
 dbfr += "\n";
 
 /*
  Serial.printf("%d ",1);
 for(int h=2;h<=30;h++){
  Serial.printf("%d    ",h);
 }
 */
  

 
int StartAddr = 0;
String digit;
for(int i = 0;i<10;i++){   
  /*
  if(i == 0){
    Serial.print("              ");
    dbfr += "                        ";
    for(int h=1;h<=30;h++){
      if(h == 1){
        Serial.printf("%d   ",h);
         dbfr += "   "+String(h);
      }else{
        Serial.printf("%d  ",h);
        dbfr += " "+String(h);
      }
      
     }
     Serial.println();
     dbfr += "\n";
  }
   */
  
  if(StartAddr == 0){
    digit = "000";
  }
  if(StartAddr == 128){
    digit = "00";
  }
   if(StartAddr == 256){
    digit = "0";
  }
  if(StartAddr == 512){
    digit = "0";
  }
  if((StartAddr > 512)&&(StartAddr<1153)){
    digit = "0";
  }
  Serial.print("ADDR: ");
  dbfr += "ADDR: ";
  dbfr += digit;
  dbfr += String(StartAddr,HEX);
  dbfr += "H  ";
  Serial.printf("%s%X",digit,StartAddr);
  Serial.print("H  ");
   for(int m=0;m<60;m+=2){
    char d = mac.readAddress(StartAddr+m);
    char d1 = mac.readAddress(StartAddr+(m+1));
    if((d == ';')||(d1 == ';')){
     if(d == ';'){
      d = '\0';
      }
     if(d1 == ';'){
        d1 = '\0';
      }
       Serial.printf("%c%c  ",d,d1);
       
    }else{
       Serial.printf("%c%c  ",d,d1);
       
    }
        dbfr += String(d)+"";
        dbfr += String(d1)+"  ";
   }
   Serial.println();
   StartAddr += 128;
   dbfr += "\n";
  }
SendMQttDebug(dbfr.c_str());



  mac.writeAddress(writeaddr_eeprom1, write_addeeprom & 0xFF); //ระบุ ADDRESS
  mac.writeAddress(writeaddr_eeprom2, (write_addeeprom >> 8) & 0xFF);
 
   //checkandsendmqtt(sText, write_addeeprom);
 return sText;
 
}
void PackSend(void * parameter){
    
    int delays;
    //String packDatasend;
    for(;;){ 
      
    
    if(periodSend == 10000){
         delays = 10000;
       }
       if(periodSend == 5000){
         delays = 5000;
       }
       if(periodSend == 15000){
         delays = 15000;
       }
   
   
   data_time();  
   
  packdata_HEAD();
  packdata_DATE();
  packdata_IO();
  packdata_PWM();
  packdata_RELAY();
  packdata_PWM2();
  packdata_AD();
  packdata_GPS();
  packdata_M1();
  packdata_M2();
  packdata_M3();
  
    sendmqtt();
    //checkandsendmqtt(packDatasend,write_addeeprom);
    
    vTaskDelay(delays / portTICK_PERIOD_MS);
  }
}

int checkandsendmqtt(String sdatamqtt, int write_addr)
{
  int buf_lasteeporm = write_addr;
  time_out = 0;
  //while (1)
  //
  if (client.connected())
    {
      
    if (write_addeeprom > 0)
      {
        String datahistory;
        String datamakemqtt;
       
        mac.Scan_data_sstv(write_addr);
        datamakemqtt = mac.make_send_string(sdatamqtt);
        datamqtt = datamakemqtt.c_str();
        if((datareceivedmqtt[0] == '\0')&&(datareceivedmqtt[1] == '\0')){
          data_time();
          client.publish(sendtopic,datamqtt);
         }
        if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
           Serial.print("$1150: DATAPACK  Send: ");
           Serial.print(datamakemqtt);
           Serial.println(" OK");
        }
        
        SendMQttDebug(("$1150: DATAPACK Send: " +  datamakemqtt + " OK").c_str());
        
        
        if((datareceivedmqtt[0] == 'O')&&(datareceivedmqtt[1] == 'K')){ 
         if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){ 
           Serial.println("$1160: *********Send Mqtt Data Realtime Success Insert Data OK********");
          }
          String Sok = "$1160: *********Send Mqtt Data Realtime Success Insert Data OK********\n";
           SendMQttDebug(Sok.c_str());
           Sok = "";
           
         if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){  
          Serial.print("$1170: WRITE_ADDR : ");
          Serial.print(write_addr);
          Serial.println(" OK");
          }   
            Sok = "$1170: WRITE_ADDR : ";
            Sok += String(write_addr)+" OK\n";
            SendMQttDebug(Sok.c_str());
            Sok = "";
          if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){   
            Serial.print("$1180: Response from Server: ");
            Serial.write(datareceivedmqtt[0]);
            Serial.write(datareceivedmqtt[1]);
            Serial.println();
          }
          Sok = "$1180: Response from Server: " + String(datareceivedmqtt[0]) + String(datareceivedmqtt[1]) + "\n";
          SendMQttDebug(Sok.c_str());
          Sok ="";
          
          mac.Check_senddata_fram(write_addr);
          
          datareceivedmqtt[0] = '\0';
          datareceivedmqtt[1] = '\0';
          checksendmqtt = 1;
            if(flag_sd == '0'){//Check SDcard
                 Serial.println("$7000: Error SD card Mount Failed");
                     String Er = "$7000: Error SD card Mount Failed\n";
                            SendError(Er.c_str());
            }
        } else {
          
         // Serial.println("******** NO OK Reply From Server (can't insert to database) ********");
        if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){  
          Serial.println("******** ");
        } 
          String Star = "******** \n";
          SendMQttDebug(Star.c_str());
          Star = "";
        }
         
        //delay(100);
         //delay(10);
        
       if (checksendmqtt == 1) {
         
           sendsdcardtomqtt();
         
          checksendmqtt = 0;
          
        }
        
        client.loop();
      }
      
      return 1;
   }else {
      if(checkSDcard() == 1){
         mqttconnect();
      }else{
         mqttconnect_FRAM();
      }
      
       time_out++;
       if (time_out > time_limit)
        {
      
          Serial.println("Can't Conect TO MQTT Can't Send");
     
            //return 0;
         }
       Serial.println("Can't Conect TO MQTT Can't Send");
    }
     
  //}
}

//Function Send history in SDcard
void sendsdcardtomqtt() {
  const char *filenameinsd;
  String buffilename;
  if(flag_sd == 0){
    SendError(Sdcardforhis.c_str());
    return;
  }
  if(flag_sd == 1){
    sdbegin();//In function have debug condition (Debug Mode)
    SendMQttDebug(Sdcardforhis.c_str());
    Sdcardforhis = "";
     String sk;
      
     /*------------------- List Filename In SDCARD ---------------------*/
      filenameinsd = listDir(SD, "/history", 0); //In function have debug condition (Debug Mode)
      
  buffilename = filenameinsd;
  if (filenameinsd != "0") {
    readFileinSD(SD, filenameinsd);
    
    packdata_HEADSDCARD();
    
    delay(10);
    datainfilesd = Headerhistory + datainfilesd;
    datamqttinsdcard = datainfilesd.c_str();
    client.publish(sendtopic,datamqttinsdcard);
    delay(250);
    //if((datareceivedmqtt[0] == 'O')&&(datareceivedmqtt[1] == 'K')){ 
      if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
       Serial.println("$1190: ***************** Send File In SDCARD OK *****************");
       
      }
      sk = "$1190: ***************** Send File In SDCARD OK *****************\n";
      SendMQttDebug(sk.c_str());
      sk = "";
      
      const char *delfilename;
      delfilename = buffilename.c_str();
      datainfilesd = "";
      
      deleteFile(SD, delfilename);
      
   //}
  } else
  {
   if((debug_method == 1) || (debug_method == 3)||(debug_method == 4)){
    Serial.println("$1200: **************** No File In SDCARD *****************");
   } 
     sk = "$1200: **************** No File In SDCARD *****************\n";
     SendMQttDebug(sk.c_str());
     sk = "";
   }
  }  
}

/*----------------Declare Tasks---------------------*/
TridentOS   task_CheckWiFiMqtt,task_LedStatus,task_LedStaNOMQ;
            
   void taskCheckWiFi_and_Mqtt(void*),taskLEDStatus(void*),taskLEDStatus_NOMQTT(void*);

/*----------------Declare Tasks---------------------*/

//Add  07-07-20
//Function Receive command via serial port for setting config
void SerialReadConfig(){ //Function for receive command from Serial-port (read config,set config,reset) Use with Program c# Mconfig
 while (Serial.available()) {
       char Datacmd = (char)Serial.read();
        Serial.write(Datacmd);
        inputCmd += Datacmd;
         
           if(inputCmd == "readcfg"){
               ReadSendConfig(SD,"/conf.txt");
               
               Serial.println(Eachline);
               SendMQttDebug(Eachline.c_str());
               Eachline = "";
               
            }
            
            if(inputCmd == "rst"){//For serial command reset MCU
              ESP.restart();
            }
            if(inputCmd == "sp"){  //For serial command show debug on serial-port
              debug_method = 4;
            }
            if(inputCmd == "sp0"){//For serial command stop debug on serial-port
              debug_method = 0;
            }
            
            if(inputCmd == "clr"){//For serial command stop debug on serial-port
               CleraFRAM2();
            }
            
           if (Datacmd == '\n') {
             DinComplete = true;
           }
     }
   if (DinComplete) {
   Serial.println(inputCmd);
   SendMQttDebug(inputCmd.c_str());
     
         if(inputCmd.substring(0,3) == "#SD"){ // Separate and write SDcard for New config
          String newConfig="";
           String Valcfg = inputCmd.substring(3);//cut string position 3 -> assign to Valcfg 
           //Serial.print(Valcfg);
            int str_len = Valcfg.length() + 1; 
               char char_array[str_len];
                  Valcfg.toCharArray(char_array, str_len);
                  //Serial.println(char_array);
                    char* ptr = strtok(char_array, ",");
                      byte i = 0;
                      while (ptr) {
                        // Serial.println(ptr);
                        //Serial.print(headcfg[i]);
                        newConfig +=  headcfg[i] + String(ptr)+"\r\n";
                        ptr = strtok(NULL, ",");
                         i++;
                      }
             
             //Serial.print(newConfig);
             String nCf = "New Config Set: " + newConfig + "\n";
             SendMQttDebug(nCf.c_str());
             nCf == "";
             Valcfg = "";
             deleteFile(SD, "/conf.txt");
             //SetSDcfg(SD, "/conf.txt"," ");
             SetSDcfg(SD, "/conf.txt",newConfig.c_str()); 
            
           
         }
    
    inputCmd = "";
    DinComplete = false;
   }
}

void SendError_continue(const char* fname){ // Function for read 
  String ERL;
  
  if(!SPIFFS.begin(true)){
      
      Serial.println("Error has occurred while mounting SPIFFS");
        
   }else{
        //Serial.println("Mounting SPIFFS");
   }
  File fss = SPIFFS.open(fname);
   if(!fss){
        Serial.println("Failed to open file for reading");
        
    }
  
   while(fss.available()){
    ERL = fss.readStringUntil('\n');
    }
   fss.close();
   if(ERL != ""){
     if(client.connected()){
       client.publish(debugs,ERL.c_str());
     }
   }
}

void SendError(const char* errdata){
   
  if(client.connected()){
    client.publish(debugs,errdata);
   }
}


//Function Send Debug Mqtt where start(or reset)
void SendDebugStart(const char* datainit){ //Send debug via Mqtt on start & restart
   if(client.connected()){
    client.publish(debugs,datainit);
   }
}

//Function Send Debug Mqtt
void SendMQttDebug(const char* Data){ //Send debug via Mqtt only or Serial-port and Mqtt
 if(client.connected()){
   if((debug_method == 2)||(debug_method == 3)){ 
      client.publish(debugs,Data);
    }
  }
 
}

//Function send status WDT work on Mqtt
void SendMqtt_Rstwdt(String rw){
  if(client.connected()){
       client.publish(debugs,rw.c_str());
      }
}

//Function Reset WDT
void ResetWDT(unsigned long Timeclear){// For Clear Watchdog Timer,Send result auto to mqtt
  String Rst_result;
  if (millis() - last >= Timeclear){
   // Serial.print("Reset WDT ");
    // Serial.println("$1230: Reset WDT ");
     // countSec = millis()/1000.0;
      esp_task_wdt_reset();
      last = millis();
      //Serial.print(countSec);
      //Serial.println(" Sec");
     //Rst_result = "Reset WDT "+String(countSec)+" Sec";
       //Rst_result = "$1230: Reset WDT OK\n";
       //SendMqtt_Rstwdt(Rst_result);
   }
}

void TestWDT(float nc){
  
  if((float)counttestWdt/2.0 >nc){
     Serial.println("Condition Test Wait..For WDT reset MCU");
     delay(51000); // Test WDT
   }
 counttestWdt++;
 delay(1000);
}

void IRAM_ATTR onTimer1() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter1++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
void IRAM_ATTR onTimer2() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter2++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
void Timer1Config(){
   if(timer1){
        timerEnd(timer1);
        timer1 = NULL;
    } else {
       if(periodSend == 10000){
         delays = 10*1000000;
       }
       if(periodSend == 5000){
         delays = 5*1000000;
       }
       if(periodSend == 15000){
         delays = 15*1000000;
       }
       if(periodSend == 20000){
         delays = 20*1000000;
       }
       if(periodSend == 30000){
         delays = 30*1000000;
       }
        timer1 = timerBegin(1,80, true);//Timer1
        timerAttachInterrupt(timer1, &onTimer1,true);
       
        timerAlarmWrite(timer1,delays, true);
        timerAlarmEnable(timer1);
    }
}
void Timer2Config(){
   if(timer2){
        timerEnd(timer2);
        timer2 = NULL;
    } else {
       
        timer2 = timerBegin(2,80, true);//Timer2
        timerAttachInterrupt(timer2, &onTimer2,true);
       
        timerAlarmWrite(timer2,Delays, true);
        timerAlarmEnable(timer2);
    }
}


void setup() {
  Serial.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  inputCmd.reserve(3500);
  v_fw[0] = 0x30;
  v_fw[1] = 0x31;
  v_fw[2] = 0x31;
  v_fw[3] = 0x30;
  sdbegin();
  Wire.begin();
  
  mac.begin(); // Call Method begin see Machine.cpp boolean Machine::begin(void)
  
 if(checkSDcard() == 1){ // Case SDcard work
 // WriteCertSSL(true);
 // Serial.println(ReadCert());
  
  ReadCfg_fromSD_forWriteFRAM(SD,"/conf.txt");
  
    Write_ssid_FRAM(true);
    Write_wpass_FRAM(true);
    Write_mqserv_FRAM(true);
    Write_mqport_FRAM(true);
    Write_mquser_FRAM(true);
    Write_mqpass_FRAM(true);
    Write_clientid_FRAM(true);
    Write_otatimeout_FRAM(true);
    Write_sendperiod_FRAM(true);
    Write_cirmachine_FRAM(true);
    Serial.println();

   if(assignConfig_Reduce(SD,"/conf.txt")){
    Serial.println(F("$1000: Load config from SD card file:conf.txt OK"));
    Msgstart += "$1000: Load config from SD card file:conf.txt OK\n";
    certssl = (char*)readcert(SD,"/certi.txt").c_str(); //Load certificate and convert to char* datatype
    //Serial.println(certssl);
    Serial.println(clientId);
   
       sendh += getSomeConfigAssign();
       sendtopic = (const char*)sendh.c_str();
       Serial.println(sendtopic);
       otah += getSomeConfigAssign();
       stopic = (const char*)otah.c_str();
       Serial.println(stopic);
       otah = "OTA/";
       otah += GroupComp(1);
       gtopic = (const char*)otah.c_str();
       Serial.println(gtopic);
       otah = "OTA/";
       otah += GroupComp(2);
       ctopic = (const char*)otah.c_str();
       Serial.println(ctopic);
       rm += getSomeConfigAssign();
       dbreply = (const char*)rm.c_str();
       Serial.println(dbreply);
       cfgh += getSomeConfigAssign();
       getconf = (const char*)cfgh.c_str();
       Serial.println(getconf);
       dbugh += getSomeConfigAssign();
       debugs = (const char*)dbugh.c_str();
       Serial.println(debugs);
       h1 += getSomeConfigAssign();
       history = (const char*)h1.c_str();
       Serial.println(history);
       h2 += getSomeConfigAssign();
       readhis = (const char*)h2.c_str();
       Serial.println(readhis);
       h3 += getSomeConfigAssign();
       delFile = (const char*)h3.c_str();
       Serial.println(delFile);
       h4 += getSomeConfigAssign();
       delAFile = (const char*)h4.c_str();
       Serial.println(delAFile);
       Serial.println(ackota);
       Serial.println(sendconf);
       debugallow += getSomeConfigAssign();
       debug_allow = debugallow;
       Serial.println(debug_allow);
       rstm += getSomeConfigAssign();
       resetm = (const char*)rstm.c_str();
       Serial.println(resetm);
   
        wifi_setup();
        client.setServer((const char*)mqtt_server.c_str(),mqttPort); 
        client.setCallback(IOTCallback);
        mqttconnect();
       
     }
  }else{
  // Case SDcard fail!!!
     errsd = true;
     Serial.println("-------------------------SDcard Fail!-------------------------");
      
      //String WS = Read_ssid_FRAM();
      wifi_ssid = Read_ssid_FRAM();;
      Serial.printf("Global Var WiFi ssid: %s\n",wifi_ssid);
      //String WP = Read_wpass_FRAM();
      wifi_pwd = Read_wpass_FRAM();
      Serial.printf("Global Var WiFi pass: %s\n",wifi_pwd);
      
      String Mqserv = Read_mqserv_FRAM();
      mqtt_server = Mqserv;
      //mqtt_server = (const char*)Mqserv.c_str();
      Serial.printf("Global Var MQTT SERVER: %s\n",mqtt_server);
      mqttPort = Read_mqport_FRAM();
      Serial.printf("Global Var MQTT PORT: %d\n",mqttPort);
      String mu = Read_mquser_FRAM();
      mqttUser = mu;
      //mqttUser = (const char*)mu.c_str();
      Serial.printf("Global Var MQTT USER: %s\n",mqttUser);
      String mp = Read_mqpass_FRAM();
      mqttPassword = mp;
      //mqttPassword = (const char*)mp.c_str();
      Serial.printf("Global Var MQTT PASSWORD: %s\n",mqttPassword);
      
       String cd = Read_clientid_FRAM();
       clientId = cd;
       //clientId = (const char*)cd.c_str();
       Serial.printf("Global Var CLIENT ID: %s\n",clientId);

       otatimeout = Read_otatimeout_FRAM();
       Serial.printf("Global Var OTA TIMEOUT: %d\n",otatimeout);

       sendperiod = Read_sendperiod_FRAM();
       Serial.printf("Global Var OTA SEND PERIOD: %s\n",sendperiod);

       cirmachine = Read_cirmachine_FRAM();
       Serial.printf("Global Var CIRMACHINE: %d\n",cirmachine);

       sendh += ReadClidSlash_FRAM();
       sendtopic = (const char*)sendh.c_str();
       Serial.println(sendtopic);
       otah += ReadClidSlash_FRAM();
       stopic = (const char*)otah.c_str();
       Serial.println(stopic);
       otah = "OTA/";
       otah += GroupCompFRAM(1);
       gtopic = (const char*)otah.c_str();
       Serial.println(gtopic);
       otah = "OTA/";
       otah += GroupCompFRAM(2);
       ctopic = (const char*)otah.c_str();
       Serial.println(ctopic);
       rm += ReadClidSlash_FRAM();
       dbreply = (const char*)rm.c_str();
       Serial.println(dbreply);
       cfgh += ReadClidSlash_FRAM();
       getconf = (const char*)cfgh.c_str();
       Serial.println(getconf);
       dbugh += ReadClidSlash_FRAM();
       debugs = (const char*)dbugh.c_str();
       Serial.println(debugs);
       h1 += ReadClidSlash_FRAM();
       history = (const char*)h1.c_str();
       Serial.println(history);
       h2 += ReadClidSlash_FRAM();
       readhis = (const char*)h2.c_str();
       Serial.println(readhis);
       h3 += ReadClidSlash_FRAM();
       delFile = (const char*)h3.c_str();
       Serial.println(delFile);
       h4 += ReadClidSlash_FRAM();
       delAFile = (const char*)h4.c_str();
       Serial.println(delAFile);
       Serial.println(ackota);
       Serial.println(sendconf);
       debugallow += ReadClidSlash_FRAM();
       debug_allow = debugallow;
       Serial.println(debug_allow);
       rstm += ReadClidSlash_FRAM();
       resetm = (const char*)rstm.c_str();
       Serial.println(resetm);
       
       errsd = false;
       wifi_setup();
       client.setServer((const char*)mqtt_server.c_str(),mqttPort); 
       client.setCallback(IOTCallback);  
        mqttconnect_FRAM();
  }
  
   
  
  if(cirmachine == 0){
    Serial.println("$1030: Mode OTA By WiFi Local Network OK");
     Msgstart += "$1030: Mode OTA By WiFi Local Network OK\n";
  }
  if(cirmachine == 1){
    Serial.println("$1040: Mode OTA By Web server OK");
    Msgstart += "$1040: Mode OTA By Web server OK\n";
  }
  checkSendTime();  
  ChipID();//Show Chip ID (IMEI)
  RTC.begin();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(100);
  Serial.println("$1090: NOW: "+DateTimeNOW()+" RTC OK");
  String DTnow = "$1090: NOW: "+DateTimeNOW()+" RTC OK\n";
 
   
  if(errsd){
    //Serial.println("Have Error SD card");
    //RecordErrorLog("/errorlog.txt","$1100: Error SD card open directory " + DateTimeNOW());
    //ReadError("/errorlog.txt");
    String errs = "$1100: Error SD card open directory " + DateTimeNOW();
    SendError((const char*)errs.c_str());
    errsd = false;
  }else{
    //DeleteError_Log("/errorlog.txt");
  }
  
  Msgstart += DTnow;
  SendDebugStart(Msgstart.c_str());
  Msgstart = "";
  
  
   /*---------Task Run-----------*/
   task_LedStatus.start(taskLEDStatus);
   task_CheckWiFiMqtt.start(taskCheckWiFi_and_Mqtt);
  
 
   
  /*
  xTaskCreatePinnedToCore(
    PackSend,    // Function that should be called
    "Packdat_Send",   // Name of the task (for debugging)
    20000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL, // Task handle
    1
  );
  */
  
   Timer1Config();
   //Timer2Config(); 
   
  

}
void loop() {
 String packDatasend;
 switch (state)
  {
  case Runnning_e:
  flag_sd = checkSDcard();
  ResetWDT(15000); //Clear Watchdog every 15 sec.
  //TestWDT(19.0);
   SerialReadConfig();
  
  /*-------------------Start GET CHIPID-------------------------- Use in task_packSend */
 chipid = ESP.getEfuseMac();
 sprintf(Chipid_buf, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)(chipid));
 

   /*-----------------------Packata Send Timer1------------------ Modify on 12/10/20*/
  
  if (interruptCounter1 > 0) {
 
  data_time();  
   
  packdata_HEAD();
  packdata_DATE();
  packdata_IO();
  packdata_PWM();
  packdata_RELAY();
  packdata_PWM2();
  packdata_AD();
  packdata_GPS();
  packdata_M1();
  packdata_M2();
  packdata_M3();
  
    packDatasend = sendmqtt();
    if(write_addeeprom >= 25599){
      timerStop(timer1);
       WriteSDhistory();
       timerRestart(timer1);
    }
   
    checkandsendmqtt(packDatasend,write_addeeprom);
     
    portENTER_CRITICAL(&timerMux);
    interruptCounter1--;
    portEXIT_CRITICAL(&timerMux);
    totalInterruptCounter1++;
  }
  
  
    
 
  if(millis() - last_err >  periodSend_error){// 1 Min check error sd card
         if(flag_sd == 0){
           //Serial.println(flag_sd);
            //RecordErrorLog("/errorlog.txt","$1100: Error SD card open directory " + DateTimeNOW());
            String errs = "$1100: Error SD card open directory " + DateTimeNOW();
            Serial.println(errs);
            SendError((const char*)errs.c_str());
            last_err = millis();
          }
          /*
          if(flag_sd == 1){
            //Serial.println(flag_sd);
            String St = "$7010: SD Card OK\n";
            SendError(St.c_str());
            last_err = millis();
          }
          */
       }
  
       
   client.loop(); 
   break;
   if(cirmachine != 0){
    case Fota_e:
      Fw = String(url).substring(40);
      DlInfo info;
      info.url = url;
      // info.caCert = NULL;//if only use http then remember to set this to NULL
      info.caCert =  certssl; //SSL Cert iotfmx.com (secure server load from certi.txt)
      info.md5 = md5_1; // info.md5 is argument of setMD5(const char * expected_md5) in Update.h
      info.startDownloadCallback =  startDl;
      info.endDownloadCallback =    endDl;
      info.progressCallback  = progress;
      info.errorCallback     = error;
      int result = httpFOTA.start(info); //OTA Method
       /*--------------------Case OTA Fail--------------------------------*/
      if(result < 0){ // Check error return from class HttpFOTA
        DELAY(100);
        ESP.restart(); 
      }
      if(result == 1){
        /*--------------------Case OTA Success 100%--------------------------------*/
       String DT =  DateTimeNOW();
               DT += " OTA OK ";
               DT += String(clientId);
        client.publish(ackota,DT.c_str(),Qos);
        DELAY(1000);
        ESP.restart();  
      }
    
     break;
   }
  }
  
}
