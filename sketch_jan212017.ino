#include <SD.h>

#include <Adafruit_FONA.h>

//#include <FonaSMS.h>

#include <Adafruit_INA219.h>

#include <Wire.h>
#include <RTClib.h>

#include <SPI.h>
#include <SD.h>

char phoneNumber[14]={'6','1','7','4','6','6','9','3','0','9'};

// This has to do with the SD card, on the UNO it's 10 on the Mega its 53
const int chipSelect = 53;

//const int ledPin = 13;

// This is the reference to the Clock we'll use to get the time
RTC_DS1307 RTC;

// Our two voltage monitors. A is the one with the A0 FILLED, B is the one with the A1 FILLED
Adafruit_INA219 ina219_a(0x41); // We need to update this for the right address, A0 is 0x41
//Adafruit_INA219 ina219_b(0x44); // We need to update this for the right address, A1 is 0x44


// To use the MEGA, it is necessary to jump the TX and RX ports.
#define FONA_RX 11
#define FONA_TX 12
#define FONA_RST 4

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
//HardwareSerial *fonaSerial = &Serial;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type; 

void setup () {
    Serial.begin(115200);
    ina219_a.begin();
    //ina219_b.begin();
    Wire.begin();
    RTC.begin();
    
    Serial.println ("1");
    Serial.println(SS);
    
    pinMode(SS, OUTPUT);
    //pinMode(13, OUTPUT);

fonaSerial->begin(4800);
  while (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
    delay(1000);
  }

    char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  
    if (! RTC.isrunning()) {
      // TODO remove this before final ship
      Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      // uncomment it & upload to set the time, date and start run the RTC!
     
    }
  // RTC.adjust(DateTime(2016,3,26,12,07,00));

/* Currently causing SMS not to send
    // see if the card is present and can be initialized:
    //if(!SD.begin(10, 11, 12, 13)) {
    if (!SD.begin(53,50,51,52)) {
    //if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      //return;
    } 
*/
}

 
void loop () {
    // Blink an LED while we work
    //digitalWrite(ledPin, HIGH);
    //delay(1000);
  Serial.println ("2");

    // Get volts / amps from IN219-A
    float a_shuntVolts = ina219_a.getShuntVoltage_mV();
    float a_busVolts = ina219_a.getBusVoltage_V();
    float a_loadVolts = a_busVolts + (a_shuntVolts / 1000);
    float a_amps = ina219_a.getCurrent_mA();
    // http://icircuit.net/arduino-boards-pin-mapping/141
    // Get volts / ampts from IN219-B
    //int b_shuntVolts = 0;//ina219_b.getShuntVoltage_mV();
    //int b_busVolts = 0;//ina219_b.getBusVoltage_V();
    //int b_loadVolts = 0;//b_busVolts + (b_shuntVolts / 1000);
    //int b_amps = 0;//ina219_b.getCurrent_mA();
    
    // Write the line
    writeLine(a_shuntVolts,a_busVolts,a_loadVolts,a_amps,0,0,0,0);
    // 1000 * 60 * 60 is 1 hour
    // Blink off
    //digitalWrite(ledPin, LOW);
    // CHANGE FREQUENCY HERE
    // 1000 ms == 1 second
    delay(10 * 120000); //currently at 20 min for testing
    //delay(1000 * 60 * 60); // uncomment this line and comment the line above to do hourly testing
}

void writeLine(float a_shuntVolts, float a_busVolts, float a_loadVolts, float a_amps, float b_shuntVolts, float b_busVolts, float b_loadVolts, float b_amps) {
    // Get datetime now
    DateTime now = RTC.now();
    // Build a file name - we'll use todays date
    //String fileName = String(now.year()) + String(now.month()) + String(now.day()) + F(".csv");
    String fileName = F("data.csv");
    // The open call on SD requires a char-array buffer, so we need to do some copying...
    char fileNameChars[sizeof(fileName)+1];
    // Copy the file to the buffer
    fileName.toCharArray(fileNameChars, sizeof(fileName)+1);
    // Create the line of text we're going to log
    String logLine = String(now.year()) +'/'+ String(now.month()) +'/' + String(now.day()) + ' ' + String(now.hour()) + ':' + String(now.minute()) + ':' + String(now.second());
    logLine += ',' + String(a_shuntVolts) + ',' + String(a_busVolts) + ',' + String(a_loadVolts) + ',' + String(a_amps);
    
    
    Serial.println(logLine);
    //logLine += ',' + String(b_shuntVolts) + ',' + String(b_busVolts) + ',' + String(b_loadVolts) + ',' + String(b_amps);
    // Open the file on the SD card
    //


      char cMessage[141];
    
    /*if (a_amps <= 0 || a_amps >= 10){
      String uhoh = "ALERT - AMPS ARE OUTSIDE OF NORMAL VALUES : " + String(a_amps);
      uhoh.toCharArray(cMessage,141);
      Serial.println ("3");
      sendSMS("16174669309", cMessage);
      
      memset(cMessage, 0, sizeof(cMessage));
    }*/
    

    
    File dataFile = SD.open(fileNameChars, FILE_WRITE);
    
    // If that worked, write it out
    if (dataFile) {
          Serial.println(F("A"));
      // Write the line to the file
      dataFile.println(logLine);
      // Close the file - the SD system only supports a single file being opened at once, since everything is highly serial in nature
      dataFile.close();
    }
          // TODO remove this before final ship
          logLine.toCharArray(cMessage, 141);
      sendSMS(phoneNumber, cMessage);
    Serial.println ("5");
    // We don't have a good way to report errors writing to the card...
}

void sendSMS(char *sendto, char *message) {
   if (!fona.sendSMS(sendto, message)) {
       Serial.println(F("Failed"));
    } else {
       Serial.println(F("Sent!"));
    }
    Serial.println ("6");
}