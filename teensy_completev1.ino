#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <SoftwareSerial.h>

#define HWSERIAL Serial1
#define PLAYBUTTON 2
#define BAUDRATE 9600

const byte numChars = 50;
char receivedChars[numChars];   // an array to store the received data
int programMode;  //0=record, 1=playRecorded, 2=playSelected
String modus = "";
char *selsong;
String fileextension = ".wav";  //file extension for the selected song

int recmode = 0;    //needed for the record function
unsigned long int timeLimit = 120000;    //this is where the rec time limit can be set in ms
elapsedMillis recTime;                  //counting up after rec start

File frec;        // The file where data is recorded
File root;        //for printing the files on the SD card
String fileList="";
String temp="";



// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=105,63
AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw           playSdRaw1;     //xy=116,422.00001525878906
AudioPlaySdWav           playSdWav1;     //xy=142,592
AudioMixer4              mixer1;         //xy=337.0000686645508,474.00005531311035
AudioMixer4              mixer2;         //xy=341,572
AudioOutputI2S           i2s1;           //xy=470,120
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playSdRaw1, 0, mixer1, 1);
AudioConnection          patchCord4(playSdRaw1, 0, mixer2, 1);
AudioConnection          patchCord5(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord6(playSdWav1, 1, mixer2, 0);
AudioConnection          patchCord7(mixer1, 0, i2s1, 0);
AudioConnection          patchCord8(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212
// GUItool: end automatically generated code 

// Bounce objects to easily and reliably read the buttons
//Bounce buttonMode =  Bounce(MODEBUTTON, 8);
Bounce buttonPlay =  Bounce(PLAYBUTTON, 8);  // 8 = 8 ms debounce time

const int myInput = AUDIO_INPUT_MIC;  // which input on the audio shield will be used?
#define PLAYBACK_VOLUME 0.8

#define SDCARD_CS_PIN    10   //configuration for SD card
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
   
   while (HWSERIAL.available() > 0) {
        //Serial.println("entered");
        HWSERIAL.println("from teensy message received");
        rc = HWSERIAL.read();
        if (rc != endMarker) {
           receivedChars[ndx] = rc;
           ndx++;
            if (ndx >= numChars)
                ndx = numChars - 1;
            }
        else if (rc == endMarker && ndx > 0){
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            }
         if ((strcmp(receivedChars, "0")==0)||(strcmp(receivedChars, "1")==0)||(strcmp(receivedChars, "2")==0)){
              modus = receivedChars;
              programMode = modus.toInt();
              Serial.println("programModeInt = " +programMode);
              }
} 
        
void printFileList(File dir){       
while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;}
                
      //Serial.println(entry.name());
      //Serial.println("entryName");
      
       temp = entry.name();
       temp.remove(temp.length()-4, 4);
       
       if ((temp.equalsIgnoreCase("RECORD")) || (temp.equalsIgnoreCase("SYST")))
         continue;
       
      fileList.concat(temp);
      fileList += ',';
      entry.close();
      }
  fileList.remove(fileList.length()-1, 1);  
  //Serial.println(fileList);
   }

void handleMode(int programMode, char *str){
  switch (programMode){
    case 0:                               //record modus   TESTED SUCESSFULLY!
   
     if (buttonPlay.fallingEdge()) {
        Serial.println("Record Button Press");
        recTime = 0;
        if (recmode == 0){
          Serial.println("startRecording");
        if (SD.exists("RECORD.RAW")) {
          SD.remove("RECORD.RAW"); //remove the old file before new one is written
          }
        frec = SD.open("RECORD.RAW", FILE_WRITE);
        if (frec) {
          Serial.println("frec started");
          queue1.begin();
          recmode = 1;
          }
        }
    }
      
    if (recmode == 1) {
    if (queue1.available() >= 2) {
      byte buffer[512];
      memcpy(buffer, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      memcpy(buffer + 256, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      Serial.println(recTime);
      frec.write(buffer, 512);    // write all 512 bytes to the SD card
      }
      if (recTime == timeLimit){
        Serial.println("stopRec from time limit");
        queue1.end();
        while (queue1.available() > 0) {
          frec.write((byte*)queue1.readBuffer(), 256);
          queue1.freeBuffer();
          }
        frec.close();
        recmode = 0;
        }
    }
 
    if (buttonPlay.risingEdge()) {
      Serial.println("Stop Button Press");
      Serial.println("stopRecording");
      queue1.end();
      while (queue1.available() > 0) {
          frec.write((byte*)queue1.readBuffer(), 256);
          queue1.freeBuffer();
          }
      frec.close();
      recmode = 0;
     } 
    break;
    
    case 1:                 /////playRec            TESTED SUCESSFULLY!
     if (buttonPlay.fallingEdge()){
       Serial.println("play recorded");
       playSdRaw1.play("RECORD.RAW");
        }
      break; 
    
    case 2:                           //playSelected    TESTED SUCESSFULLY!
      if (buttonPlay.fallingEdge()){   
        Serial.println("play selected wav");
        if(!playSdWav1.isPlaying()){  //start playing sel song if not already playing     
          playSdWav1.play(str);
        }
        else{                          //stop playing if already playing
          playSdWav1.stop();
        }
      }
      break; 
      
    default:
      Serial.println("not a correct program mode");
      break;  
  }
}


void setup() {
  Serial.begin(BAUDRATE);    // for console ouptut / USB
  HWSERIAL.begin(BAUDRATE,SERIAL_8N1); //for communication with ESP8266
  pinMode(PLAYBUTTON, INPUT_PULLUP);  // Configure the pushbutton pins
  AudioMemory(60);
  
  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(PLAYBACK_VOLUME);
  sgtl5000_1.micGain(30);
  sgtl5000_1.adcHighPassFilterDisable(); 
    
  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}
  
void loop() {
  buttonPlay.update();    // First, read the buttons
  recvWithEndMarker();
  handleMode(programMode, selsong);
  //selsong = "SDTEST4.wav";
  //programMode = 0;
  
    
}
