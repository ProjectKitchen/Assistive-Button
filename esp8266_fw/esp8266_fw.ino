//TODO: captive portal
//TODO: html/CSS adaptieren




#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "html.h"

#define SERIAL_DEBUG 0 // constant for debugging purposes only
#define SERIALBAUD  115200

#define APNAME "Assistive Button"

/* commands for esp to teensy communication */
#define PLAYMODE    0x00 //
#define RECORDMODE  0x01 // 
#define LOADSONG    0x02 // 
#define PLAYSONG    0x03 // 
#define GETPLAYLIST 0x04 //


// function prototypes 
void handleRoot();
void handleModeSelection();
void handleLoadFile();
void handlePlayFile();
void handlePlayList();

/* Globals */
const byte DNS_PORT = 53;         // Capture DNS requests on port 53
IPAddress apIP(192, 168, 4, 1);   // Private network for server

ESP8266WebServer server(80);  // create web server at HTTP port 80
DNSServer        dnsServer;   // create the DNS object
byte serve_dns_requests = 1;  // shall we react (captive portal)

String playlist = ""; // var to save the incoming directory string from teensy

void setup() {

  Serial.begin(SERIALBAUD);
  while (!Serial); // wait for serial attach

#if SERIAL_DEBUG
  Serial.println("Initializing ESP8266 hotspot!");
#endif

  // Set WiFi SSID
  String ap_name = APNAME;
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_name.c_str());
  
  // Set server callback functions
  server.on("/", handleRoot);    
  server.on("/mode", handleModeSelection);
  server.on("/getList", handlePlayList);
  server.on("/load", handleLoadFile);
  server.on("/play", handlePlayFile);
  server.begin();
  
  dnsServer.start(DNS_PORT, "*", apIP);
#if SERIAL_DEBUG
  Serial.println("HTTP server started");
#endif
}

void loop() {
  
  if (serve_dns_requests) {
    dnsServer.processNextRequest();
  }

  server.handleClient();

  // Checks is there any data in buffer 
  if(Serial.available()>0) {
    // Read playlist
    playlist = Serial.readString();
#if SERIAL_DEBUG    
    Serial.println(playlist);
#endif     
  }
}


/* ------  webserver functions  ---------- */

void handleRoot() {
#if SERIAL_DEBUG
  Serial.println("You called root page");
#endif
  String s = FPSTR(MAIN_page); // Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleModeSelection() {
  String t_state = server.arg("mode");
#if SERIAL_DEBUG  
  Serial.print("Mode: ");
  Serial.println(t_state);
#endif

  if (t_state == "playMode") {
    Serial.write(PLAYMODE);
  } else if(t_state == "recordMode") {
    Serial.write(RECORDMODE);
  }
  server.send(200, "text/html", "switch mode");
}

void handleLoadFile() {
#if SERIAL_DEBUG
  Serial.print("Load: ");
#endif
  String t_state = server.arg("song");
  Serial.write(LOADSONG);
  Serial.println(t_state);
  server.send(200, "text/html", "select song"); 
}

void handlePlayFile() {
#if SERIAL_DEBUG
  Serial.print("Play: ");
#endif
  String t_state = server.arg("song");
  Serial.write(PLAYSONG);
  Serial.println(t_state);
  server.send(200, "text/html", "play song"); 
}

void handlePlayList() {
#if SERIAL_DEBUG
  Serial.println("Getting playlist");
#endif
  Serial.write(GETPLAYLIST);
  delay(500);
  server.send(200, "text/html", playlist);
  playlist="";  
}

/*
*/
