#include <DNSServer.h>
#include <SoftwareSerial.h>
SoftwareSerial swSer;   //for communication to teensy
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "esp8266html.h"
#include "FS.h"   //include file system header
#define BAUDRATE 9600

const byte DNS_PORT = 53;
const char *ssid = "Assistive Button";
const char *password = "testpassword";  //password has at least to be 8 characters, otherwise open network

IPAddress local_IP(192,168,4,23);
IPAddress gateway(192,168,4,1);
IPAddress subnet (255,255,255,0);
ESP8266WebServer server(80); //create a webserver object
DNSServer dnsServer;

const byte numChars = 50;
char receivedChars[numChars];   // an array to store the received data
String modus;
String selected;
String fileList;

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
   
   while (swSer.available() > 0) {
        rc = swSer.read();
        if (rc != endMarker) {
           receivedChars[ndx] = rc;
           ndx++;
            if (ndx >= numChars)
                ndx = numChars - 1;
            }
        else if (rc == endMarker && ndx > 0){
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            //Serial.println(receivedChars);
            fileList = receivedChars;
            }
     }
   } 

void handleRoot() {
    server.send_P(200, "text/html", index_html);} //the p is for sending from program memory

void handlePlaylist(){
  recvWithEndMarker();
  Serial.println(fileList);
  server.send(200, "text/plain", fileList);
  }

 void handleMode(){
  modus = server.arg("plain");
  server.send(204, "");       //http status 204: no content
  swSer.println(modus);
  }

  void handleSelected(){
    selected=server.arg("plain");
    server.send(204, "");       //http status 204: no content
    swSer.println(selected);
    }

 void handleWebRequests(){
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
 
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }
 
  dataFile.close();
  return true;
}

void setup() {
  Serial.begin(BAUDRATE); //for console output via USB
  swSer.begin(BAUDRATE, SWSERIAL_8N1, 14, 12, false);   //pin 14/D5 is RX, pin12/D6 is TX
  SPIFFS.begin();   //initialize File System
  
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAPIP();

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  server.begin();
  dnsServer.start(DNS_PORT, "www.mybutton.com", local_IP);

  // server functions
  server.on("/", handleRoot);
  server.on("/playlist", HTTP_GET, handlePlaylist);
  server.on("/mode", HTTP_POST, handleMode);
  server.on("/selected", HTTP_POST, handleSelected);
  server.onNotFound(handleWebRequests); //set server all paths are not found so it can handle as per URI

  Serial.println("Server listening");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();  // to handle actual incoming of http requests: handleClient method
 
}
