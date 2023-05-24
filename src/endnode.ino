#include <Adafruit_BME280.h>
#include <LiquidCrystal.h>
#include <lorawan.h>
#include <SPI.h>
#include <TLI4971.h>

/***************************************************************

- File:   endnode.ino
- Author: Brendan Dickerson
- Date:   2023-05-31
- Course: Capstone Project Spring 2023
- School: Oregon Institue of Technology
- Description:
    End node takes an analog current reading, environment status,
    and additional information and transmits it back to a 
    LoraWAN gateway. Typical applications for this include
    remote alternative energy sources such as photovoltaic 
    installations or wind energy farms. 

****************************************************************/

// RP2040 pin definitions
#define D0      0
#define D1      1
#define LCD_D7  2
#define LCD_D6  3
#define LCD_D5  4
#define LCD_D4  5
#define LCD_E   6
#define LCD_RS  7
#define SELBTN  8
#define TXLED   9
#define LCDBL   10
#define D11     11
#define SW_CS   12
#define LOAD    13
#define D14     14
#define RF_D2   15
#define BME_CS  17
#define RF_RES  18
#define RF_D5   19
#define POCI    20
#define RF_CS   21
#define SCK     22
#define PICO    23
#define RF_D0   24
#define RF_D1   25
#define CURRENT A0
#define VREF    A2
#define OCD1    A3


// //LCD Display
// LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
// bool displayActive = false;

// //BME280
// //Adafruit_BME280 bme(BME_CS, POCI, PICO, SCK);
// long temp = 0;
// long humidity = 0;

//ABP Credentials 
const char *devAddr = "10101010";
const char *nwkSKey = "00000000000000000000000000000000";
const char *appSKey = "00000000000000000000000000000000";

//Timing for transmission
const unsigned long interval = 10000;    // interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

//Storage for current calculation
int current = 0;
int avg,ref = 0;

//Storage for transmission data
char data[2] = {0, 1};
byte recvStatus = 0;

//Pinout for RFM95
const sRFM_pins RFM_pins = {
  .CS = RF_CS,
  .RST = RF_RES,
  .DIO0 = RF_D0,
  .DIO1 = RF_D1,
  .DIO2 = RF_D2,
  .DIO5 = RF_D5,
};

/******************
  Setup Function
******************/
void setup() {
  Serial.begin(9600);
  delay(100);
  //while(!Serial);
  loraSetup();
  //bmeSetup();
  //lcd.begin(16, 2);

  pinMode(LCDBL, OUTPUT);
  pinMode(VREF, INPUT);
  pinMode(CURRENT, INPUT);
  pinMode(TXLED, OUTPUT);
  analogWrite(VREF, 1028);
}

/*******************
  Loop Function
*******************/
void loop() {
  transmit();
  digitalWrite(TXLED, LOW);
  digitalWrite(LCDBL, HIGH);
}

/*******************
  Read TLI4971
*******************/
void getCurrent() {
  avg = 0;
  for(int i = 0; i<500; i++) {
    analogRead(CURRENT);
    avg += analogRead(CURRENT);
  }
  avg = avg/500.0;
  
  ref = 0;
  for(int i = 0; i<500; i++) {
    analogRead(VREF);
    ref += analogRead(VREF);
  }
  ref = ref/500.0;
  
  current = round(((avg-ref)/6));
}

/*******************
  LoRa Transmit
*******************/
void transmit() {
  // Check interval overflow
  if(millis() - previousMillis > interval) {
    //getEnvironment();
    getCurrent();
    Serial.println(current);
    digitalWrite(TXLED, HIGH);
    digitalWrite(LCDBL, HIGH);
    //lcd.clear();
    //lcd.print("Transmitting...");
    previousMillis = millis(); 
    data[0] = current;     //change to current
    Serial.print("Sending: ");
    Serial.println((int)data[0]);

    lora.sendUplink(data, strlen(data), 0, 1);
    counter++;
  }
  lora.update(); //needed 
}

/*******************
  LoRa Setup/Config
*******************/
void loraSetup() {
  while(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(1000);
  }
  Serial.println("Starting...");
  
  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_A);

  // Set Data Rate
  lora.setDataRate(SF9BW125);
  
  // set channel to random
  lora.setChannel(MULTI);
  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  lora.setDevEUI("00000000");
}

/*******************
  Setup BME280
*******************/
// void bmeSetup() {
//     unsigned status;
//     // default settings
//    // status = bme.begin();  
//     // You can also pass in a Wire library object like &Wire2
//     // status = bme.begin(0x76, &Wire2)
//     if (!status) {
//         Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
//         Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
//         Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
//         Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
//         Serial.print("        ID of 0x60 represents a BME 280.\n");
//         Serial.print("        ID of 0x61 represents a BME 680.\n");
//         while (1) delay(10);
//     }
// }

/*******************
  Read BME280
*******************/
// void getEnvironment() {
//   //temp = bme.readTemperature();
//   Serial.print("Temperature = ");
//   Serial.print(temp);
//   Serial.println(" °C");
    
//   humidity = bme.readHumidity();
//   Serial.print("Humidity = ");
//   Serial.print(humidity);
//   Serial.println(" %");
// }