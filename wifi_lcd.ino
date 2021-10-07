/*
 WiFiEsp test: ClientTest
http://www.kccistc.net/
작성일 : 2019.12.17 
작성자 : IoT 임베디드 KSH
*/
#define DEBUG
//#define DEBUG_WIFI

#define DEBUG
//#define DEBUG_WIFI
#define AP_SSID "iotsystem0"
#define AP_PASS "iotsystem00"
#define SERVER_NAME "192.168.10.56"
#define SERVER_PORT 5000  
#define LOGID "LJY_ARD"
#define PASSWD "PASSWD"

#define WIFITX 7  //7:TX -->ESP8266 RX
#define WIFIRX 6 //6:RX-->ESP8266 TX
#define LED_BUILTIN_PIN 13

#define D3_PIN 3
#define CDSPIN A0

#define BUTTON 2

#define CMD_SIZE 50


#define ARR_CNT 5           

#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <TimerOne.h>
#include <Wire.h>

char sendBuf[CMD_SIZE];

bool timerIsrFlag = false;
unsigned int secCount;
int sensorTime;

SoftwareSerial wifiSerial(WIFIRX, WIFITX); 
WiFiEspClient client;

boolean debounce(boolean last)
{
  boolean current = digitalRead(BUTTON);  // 버튼 상태 읽기
  if (last != current)      // 이전 상태와 현재 상태가 다르면...
  {
    delay(5);         // 5ms 대기
    current = digitalRead(BUTTON);  // 버튼 상태 다시 읽기
  }
  return current;       // 버튼의 현재 상태 반환
}

void setup() {
  // put your setup code here, to run once:
    
    pinMode(LED_BUILTIN_PIN, OUTPUT); //D13
        
    Serial.begin(115200); //DEBUG
   // wifi_Setup();

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(timerIsr); // timerIsr to run every 1 seconds
}

void loop() {
  // put your main code here, to run repeatedly:
  if(client.available()) {
    socketEvent();
  }
  if (timerIsrFlag)
  {
    timerIsrFlag = false; 
       
    if(!(secCount%5))
    {
      if (!client.connected()) { 
        lcdDisplay(0,1,"Server Down");
        server_Connect();
      }
    }   
  }

}
void socketEvent()
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0}; 
  int len;

  sendBuf[0] ='\0';
  len =client.readBytesUntil('\n',recvBuf,CMD_SIZE); 
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
  if((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    sprintf(lcdLine2,"%s %s",pArray[1],pArray[2]);
    lcdDisplay(0,1,lcdLine2);
  }
  if(!strncmp(pArray[1]," New",4))  // New Connected
  {
    Serial.write('\n');
    return ;
  }
  else if(!strncmp(pArray[1]," Alr",4)) //Already logged
  {
    Serial.write('\n');
    client.stop();
    server_Connect();
    return ;
  }   
  else if(!strcmp(pArray[1],"LED")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(LED_BUILTIN_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF")) {
      digitalWrite(LED_BUILTIN_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }

    else if(!strcmp(pArray[1],"LAMP")) {
    if(!strcmp(pArray[2],"ON")) {
      digitalWrite(D3_PIN,HIGH);
    }
    else if(!strcmp(pArray[2],"OFF")) {
      digitalWrite(D3_PIN,LOW);
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }

  else if(!strcmp(pArray[1],"GETSTATE")) {
    if(!strcmp(pArray[2],"DEV")) {
      sprintf(sendBuf,"[%s]DEV@%s\n",pArray[0],digitalRead(LED_BUILTIN_PIN)?"ON":"OFF");
    }
  }
  client.write(sendBuf,strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}
void timerIsr()
{
//  digitalWrite(LED_BUILTIN_PIN,!digitalRead(LED_BUILTIN_PIN));
  timerIsrFlag = true;
  secCount++;
}
void wifi_Setup() {
  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}
void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI    
      Serial.println("WiFi shield not present");
#endif 
    }
    else
      break;   
  }while(1);

#ifdef DEBUG_WIFI    
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif     
  while(WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {   
#ifdef DEBUG_WIFI  
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);   
#endif   
  }
  sprintf(lcdLine1,"ID:%s",LOGID);  
  lcdDisplay(0,0,lcdLine1);
  sprintf(lcdLine2,"IP:%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  lcdDisplay(0,1,lcdLine2);
  
#ifdef DEBUG_WIFI      
  Serial.println("You're connected to the network");    
  printWifiStatus();
#endif 
}
int server_Connect()
{
#ifdef DEBUG_WIFI     
  Serial.println("Starting connection to server...");
#endif  

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI     
    Serial.println("Connected to server");
#endif  
    client.print("["LOGID":"PASSWD"]"); 
  }
  else
  {
#ifdef DEBUG_WIFI      
     Serial.println("server connection failure");
#endif    
  } 
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}