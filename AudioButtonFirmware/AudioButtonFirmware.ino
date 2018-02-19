// Record sound as raw data to a SD card, and play it back.
//

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=105,63
AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw           playRaw1;       //xy=302,157
AudioOutputI2S           i2s1;           //xy=470,120
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playRaw1, 0, i2s1, 0);
AudioConnection          patchCord4(playRaw1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212
// GUItool: end automatically generated code

// Bounce objects to easily and reliably read the buttons
Bounce buttonRecordAndStop = Bounce(0, 8);
Bounce buttonPlay =   Bounce(1, 8);  // 8 = 8 ms debounce time



// which input on the audio shield will be used?
const int myInput = AUDIO_INPUT_MIC;

#define PLAYBACK_VOLUME 0.8


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14


// Remember which mode we're doing
int programMode = 1; // 1= record & play 2= timer function
int mode = 0;  //In programMode 1 0=stopped, 1=recording, 2=playing


// The file where data is recorded
File frec;

// The file where time data is recorded
File ftime;

// String where the Mode from PC is stored
String fromPC;
// String where the minutes for timer mode is stored
String timeFromPC;


void setup() {

  // configure Serial
  Serial.begin(115200);

  // Configure the pushbutton pins
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);


  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(PLAYBACK_VOLUME);
  sgtl5000_1.micGain(30);
  sgtl5000_1.adcHighPassFilterDisable(); 
  //sgtl5000_1.lineInLevel(10, 10);

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
  Serial.println("Audio system ready.");
}


void loop() {

  // send data only when you receive data:
  if (Serial.available() > 0)
  {
    // read the incoming byte:
    fromPC = Serial.readString();

    //Serial.println(fromPC);
    if (fromPC == "Mode 1")
    {
      Serial.print("Mode 1: OK");
      programMode = 1;

    }
    if (fromPC == "Mode 2")
    {
      Serial.print("Mode 2: OK");
      Serial.print("Give me time in minutes bigger than 0 and equal or less than 120:  ");
      while (!Serial.available()) {

      }
      timeFromPC = Serial.readString();
      Serial.end();

      programMode = 2;
    }

    if (fromPC != "Mode 1")
    {
      if (fromPC != "Mode 2") {
        Serial.print(" Invalid command ");
      }

    }
  }

  // First, read the buttons
  buttonRecordAndStop.update();
  buttonPlay.update();

  // Mode 1: record and play
  if (programMode == 1)
  {

    // Respond to button presses

    // RECORDING: Der geht, check
    if (buttonRecordAndStop.fallingEdge()) {
      Serial.println("Record Button Press");
      if (mode == 2) stopPlaying();
      if (mode == 0) startRecording();

    }

    // Stop recording der geht
    if (buttonRecordAndStop.risingEdge()) {
      Serial.println("Stop Button Press");
      if (mode == 1) stopRecording();
      if (mode == 2) stopPlaying();
    }

    //Play button
    if (buttonPlay.fallingEdge()) {
      Serial.println("Play Button Press");
      if (mode == 1) stopRecording();
      if (mode == 0) startPlaying();
    }

    // If we're playing or recording, carry on...
    if (mode == 1) {
      continueRecording();
    }
    if (mode == 2) {
      continuePlaying();
    }
  }

  //Mode 2: Timer

  if (programMode == 2)
  {

    int minutes = 0;

    // Getting the time as string from UART, converting to intager
    minutes = timeFromPC.toInt();

    int howOftenPlayTimeFiles = howMany5steps(minutes);

    if ( !SD.exists("/TIME") )
    {
      SD.mkdir("/TIME");
    }

    if (buttonRecordAndStop.fallingEdge())
    {
      Serial.println("Start recording noch 5 min");
      if (SD.exists("/TIME/5MINUTES.RAW")) {

        Serial.println("5MINUTES.RAW is existsing so removing it");
        SD.remove("/TIME/5MINUTES.RAW");
      }

      //SD.open() Opens a file on the SD card. If the file is opened for writing, it will be created if it doesn't already exist
      ftime = SD.open("/TIME/5MINUTES.RAW", FILE_WRITE);
      if (SD.exists("/TIME/5MINUTES.RAW"))
      {
        Serial.println("Juhu, 5_Minutes RAW angelegt");
      }
      if (ftime) {
        Serial.println("ftime is true");
        queue1.begin();
        mode = 1;
      }
    }
    if (buttonRecordAndStop.risingEdge())
    {
      Serial.println("Released button record");
    }


    //Code for recording the TIME files, it is only made once
    // The code + functions can be find in projekt_0.2 sketch
    // The code was removed in sketch projekt_0.3 and higher
    // due to better readability of code.


    // End of Mode 2 IF
  }

  // End of void loop
}


// FUNCTIONS:

int howMany5steps(int x)
{
  int result;
  result = x / 5;
  // 120 minutes is the maximum -> 120 / 5 = 24
  if (result > 24)
  {
    result = 24;
  }
  return result;
}


void startRecording() {
  //sgtl5000_1.volume(0,0);
  //sgtl5000_1.dacVolume(0,0);
  //sgtl5000_1.muteHeadphone();

  Serial.println("startRecording");
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = 1;
  }
}

void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];

    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer + 256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    elapsedMicros usec = 0;
    int x = millis();
    frec.write(buffer, 512);
    Serial.println(millis()-x);

  }
}

void stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  //sgtl5000_1.volume(PLAYBACK_VOLUME);
  //sgtl5000_1.dacVolume(0.5,0.5);
  //sgtl5000_1.unmuteHeadphone();


  mode = 0;
}


void startPlaying() {
  Serial.println("startPlaying");
  playRaw1.play("RECORD.RAW");
  mode = 2;
  Serial.println("Am Ende von startPlaying");
}

void continuePlaying() {
  if (!playRaw1.isPlaying()) {
    playRaw1.stop();
    mode = 0;
  }
}

void stopPlaying() {
  Serial.println("stopPlaying");
  if (mode == 2) playRaw1.stop();
  mode = 0;
}



