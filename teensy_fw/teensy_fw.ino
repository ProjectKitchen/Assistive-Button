#include <SD.h>
#include <SPI.h>
#include <Audio.h>
#include <Bounce.h>

#define SDCARD        1

/* Use these with the Teensy Audio Shield */
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN  14
#define SDCARD_CS_PIN   10

/* Use these for serial debug */
#define SONGS       "Songs"
#define AUDIOBOOKS  "Audiobooks"
#define RECORDS     "Records"

/* User defined commands */
#define PLAYMODE    0x00
#define RECORDMODE  0x01
#define LOADSONG    0x02
#define PLAYSONG    0x03
#define GETPLAYLIST 0x04

/* Define the serial peripheral */
#define HWSERIAL     Serial1
#define SWSERIALBAUD  115200
#define HWSERIALBAUD  115200

/* Globals */
File root;
#if SDCARD
String file_list = "";
#else // for testing purposes
String file_list = "SOUND1|SOUND2,SOUND3|SOUND4,SOUND5|SOUND6";
#endif

String fileName = "RECORD";

/* ================= Audio ================= */
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

const int myInput = AUDIO_INPUT_MIC;

#define PLAYBACK_VOLUME 0.8

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing
#define STOP    0x00
#define RECORD  0x01
#define PLAY    0x02

// The file where data is recorded
File frec;
// The file where time data is recorded
File ftime;

/* ================== Buttons ================== */

#define PLAYBUTTON 2
#define MODEBUTTON 3

// Bounce objects to easily and reliably read the buttons
Bounce buttonMode =  Bounce(MODEBUTTON, 8);
Bounce buttonPlay =  Bounce(PLAYBUTTON, 8);  // 8 = 8 ms debounce time

void setup() {
  /* Open serial communications and wait for port to open */
  Serial.begin(SWSERIALBAUD);   // Software serial
  HWSERIAL.begin(HWSERIALBAUD); // Hardware serial
  
  while (!HWSERIAL) {
    ; // wait for serial port to connect.
  }

  // Configure the pushbutton pins
  pinMode(PLAYBUTTON, INPUT_PULLUP);
  pinMode(MODEBUTTON, INPUT_PULLUP);

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(PLAYBACK_VOLUME);
  sgtl5000_1.micGain(30);
  sgtl5000_1.adcHighPassFilterDisable(); 

  Serial.print("Initializing SD card...");

  /* SPI setup */
  SPI.setMOSI(SDCARD_MOSI_PIN); // Audio shield has MOSI on pin 7
  SPI.setSCK(SDCARD_SCK_PIN);   // Audio shield has SCK on pin 14

#if SDCARD // for testing purposes
  // initialize SD card communication via SPI
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("initialization failed!");
    // return;
  }
  Serial.println("initialization done.");

  root = SD.open("/songs/");
  printSongs(root, 1);

  root = SD.open("/abooks/");
  printAudioBooks(root, 1);

  root = SD.open("/records/");
  printRecords(root, 1);

  /*
   * Format the string to get a string like: 
   * SOUND1|SOUND2,SOUND3|SOUND4,SOUND5|SOUND6
   */
  file_list.replace(".RAW", '|');
  file_list.replace("|,", ',');
  file_list = file_list.substring(0, file_list.length() - 1);
  
  Serial.println("File list: " + file_list);
  HWSERIAL.println(file_list);
#endif 
  
  Serial.println("done!");
}

void loop() {
  if(HWSERIAL.available()>0) {
    uint8_t b = HWSERIAL.read();
    String str = HWSERIAL.readString();
    exploitCommand(b, str);
  }

  // First, read the buttons
  buttonMode.update();
  buttonPlay.update();

  if (buttonMode.fallingEdge()) {
    Serial.println("Record Button Press");
    if (mode == PLAY) stopPlaying();
    if (mode == STOP) startRecording();
  }
  
  // Stop recording der geht
  if (buttonMode.risingEdge()) {
    Serial.println("Stop Record Button Press");
    if (mode == RECORD) stopRecording();
    if (mode == PLAY) stopPlaying();
  }
  
  //Play button
  if (buttonPlay.fallingEdge()) {
    Serial.println("Play Button Press");
    switch(mode){
      case RECORD:
        stopRecording();
        break;
      case PLAY:
        stopPlaying();
        break;
      case STOP:
        Serial.println(fileName);
        startPlaying(fileName);
        break;
    }
    /*if (mode == RECORD) stopRecording();
    if (mode == PLAY){
      stopPlaying();
      //startPlaying(fileName);
    }
    if (mode == STOP){
      Serial.println(fileName);
      startPlaying(fileName);
    }*/
  }
  
  // If we're playing or recording, carry on...
  if (mode == RECORD) {
    continueRecording();
  }
  if (mode == PLAY) {
    continuePlaying();
  }
}

void printSongs(File dir, int numTabs) {
  Serial.println(SONGS);
  printDirectory(dir, numTabs);
}

void printAudioBooks(File dir, int numTabs) {
  Serial.println(AUDIOBOOKS);
  printDirectory(dir, numTabs);
}

void printRecords(File dir, int numTabs) {
  Serial.println(RECORDS);
  printDirectory(dir, numTabs);
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      file_list.concat(',');
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    String item = entry.name();

    /* add the file name to the file list */
    file_list.concat(item);
    Serial.println(item);
    entry.close();
  }
}

void exploitCommand(uint8_t cmd, String str) {

  switch(cmd) {  
    case PLAYMODE:
      // TODO: set teensy in play mode
      Serial.println("set teensy in play mode");
      break;
    case RECORDMODE:
      // TODO: set teensy in record mode
      fileName = "RECORD";
      Serial.println("set teensy in record mode");
      break;
    case LOADSONG:
      // TODO: load song via teensy
      Serial.print("load song via teensy: ");
      Serial.println(str.trim());
      fileName = str.trim();
      break;
    case PLAYSONG:
      // TODO: play song via teensy
      Serial.print("play song via teensy: ");
      Serial.println(str);
      fileName = str.trim();
      stopPlaying();
      startPlaying(fileName);
      break;
    case GETPLAYLIST:
      // send playlist to esp via hardware serial
      Serial.println("send playlist to esp via hardware serial");
      HWSERIAL.println(file_list);
      break;
    default:
      break;
  }
}

void startRecording() {
  Serial.println("start recording");
  if (SD.exists("/records/RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("/records/RECORD.RAW");
  }
  frec = SD.open("/records/RECORD.RAW", FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = RECORD;
  }
  fileName="RECORD";
}

void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];

    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer + 256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    
    // write all 512 bytes to the SD card
    frec.write(buffer, 512);
  }
}

void stopRecording() {
  Serial.println("stop recording");
  queue1.end();
  if (mode == RECORD) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }

  mode = STOP;
}

void startPlaying(String fileName) {
  // TODO: delete line
  //fileName = "SOUND6";
  fileName = fileName.trim();
  fileName = fileName.concat(".RAW");

  uint8_t found = 0;
    
  for (int i = 0; i < 3; i++) {
    String path = "";
    File temp;
    switch(i) {
      case 0:
        temp = SD.open("/songs/");
        path = "/songs/";
        break;
      case 1:
        temp = SD.open("/abooks/");
        path = "/abooks/";
        break;
      case 2:
        temp = SD.open("/records/");
        path = "/records/";
        break;
    }
    path = path.concat(fileName);
    path = path.trim();
    char __FileName[100];
    while (true) {
      File entry = temp.openNextFile();
      if (!entry) {
        break;
      }
      String item = entry.name();
      if (item == fileName) {
        found = 1;  
        path.toCharArray(__FileName, sizeof(__FileName));
        playRaw1.play(__FileName);
      }
      entry.close();
    }
    if (found) {
      break;
    }
  }
  mode = PLAY;
}

void continuePlaying() {
  if (!playRaw1.isPlaying()) {
    playRaw1.stop();
    mode = STOP;
  }
}

void stopPlaying() {
  /*if (mode == PLAY) {
    playRaw1.stop();
  }*/
  playRaw1.stop();
  mode = STOP;
}
