#include <DNSServer.h>
#include <SoftwareSerial.h>
SoftwareSerial swSer;   //for communication to teensy
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "esp8266html.h"
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

   void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void setup() {
  Serial.begin(BAUDRATE); //for console output via USB
  swSer.begin(BAUDRATE, SWSERIAL_8N1, 14, 12, false);   //pin 14/D5 is RX, pin12/D6 is TX
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
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  Serial.println("Server listening");
}

void loop() {
  // to handle actual incoming of http requests: handleClient method
    //recvWithEndMarker();
    dnsServer.processNextRequest();
    server.handleClient();
 
}
