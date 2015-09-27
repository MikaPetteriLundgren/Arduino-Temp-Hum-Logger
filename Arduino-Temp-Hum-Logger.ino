/* DS1307 RTC is connected via I2C and Wire lib
   EEPROM AT24C32 memory placed on same board with the DS1307 RTC. I2C address of AT24C32 EEPROM is 0x50
   Size of AT24C32 EEPROM memory is 4096 bytes. EEPROM is not used in this sketch though.
   
   Temperature and humidity readings are read from DHT22 (RHT03) digital temperature & humidity sensor via MaxDetect 1-Wire bus
   Note! MaxDetect 1-Wire bus in not compatible with Dallas OneWire bus!!!
   Data is sent with TX433N RF transmitter using VirtualWire library to Arduino Domoticz Gateway which sends data to Domoticz running on
   Raspberry Pi using MQTT protocol.*/

#include "DHT.h"
#include <Wire.h>
#include <VirtualWire.h>
#include "RTClib.h"
#include <Narcoleptic.h>

const long SleepTime = 30000; //Set narcoleptic sleeping time in milliseconds. Default 30000ms.
const int TxLed = 13; // TxLed will be lit when TX433N is transmitting
const int TxPin = 12; // Data pin of TX433N is connected to this pin
const int MeasInterval = 1800; // Set tempererature measurement interval in seconds. Default 1800s (30min).
unsigned long PreviousTime = 0; // Unix time is compared to PreviousTime in order to find out has MeasInterval elapsed

float temperature = 0; // Variable to store temperature readings
float humidity = 0; // Variable to store humidity readings

const int sensorIDX = 539; // IDX number of sensor connected to this device
const int dtype = 82; // dtype (device type) is a value used by Domoticz server to determine sensor type. 82 means temp+hum and 80 means temp only.
 
#define DHTPIN 2     // DHT22 data pin is plugged into pin 2 of the Arduino 
#define DHTTYPE DHT22   // DHT22 is used

DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 RTC;

//#define RAM_DEBUG // Comment this line out if there is no need to print amount of free RAM memory via serial port

void setup()
{
  Serial.begin(9600); // Start serial port
  Wire.begin(); // Start up Wire (I2C) library
  RTC.begin(); // Start up RTC library
  dht.begin(); // Start up DHT library
  pinMode(TxLed, OUTPUT); //TxLed is set to output
  vw_set_tx_pin(TxPin); // IO is set for data transmission
  vw_setup(2000); // TX433N bits per second rate is set
  
    if (!RTC.isrunning()) 
    {
    Serial.println(F("RTC is NOT running!"));
    RTC.adjust(DateTime(__DATE__, __TIME__)); // Sets the RTC to the date & time this sketch was compiled
    Serial.print(F("Following date and time set to RTC: "));
    Serial.print(__DATE__);
    Serial.print(F(" , "));
    Serial.println(__TIME__);
    }
    
  DateTime now = RTC.now(); // Function that returns a DateTime object that describes the year, month, day, hour, minute and second is called
  PreviousTime = now.unixtime(); // PreviousTime is set to unix time
  Serial.println(F("Setup completed!"));
}
 
 
void loop()
{
  
  DateTime now = RTC.now(); // Function that returns a DateTime object that describes the year, month, day, hour, minute and second is called
  
  // To be checked is it time to measure temperature and humidity
  if (now.unixtime() >= PreviousTime + MeasInterval) // If MeasInterval has elapsed time, date and temperature are measured
   {
    temperature = tempReading(); // TempReading function is called
    delay(500);
    humidity = humReading(); // HumReading function is called
    
    // Measured temperature is printed
    Serial.print(F("\nMeasured temperature: "));
    Serial.print(temperature);
    Serial.println(F("DegC"));
    
    // Measured humidity is printed
    Serial.print(F("Measured humidity: "));
    Serial.print(humidity);
    Serial.println(F("%\n"));
    
    //createDataToBeSent();
    sendData(createDataToBeSent()); // Create data String to be sent and send it data via RF link
       
    PreviousTime = now.unixtime(); // PreviousTime is set to unix time
   }
   
   #if defined RAM_DEBUG
     Serial.print(F("Amount of free RAM memory: "));
     Serial.print(memoryFree()); // Prints the amount of free RAM memory 
     Serial.println(F(" / 2048 bytes")); //ATmega328 has 2kB of RAM memory
   #endif
   
  Serial.print(F("Going to sleep for: "));
  Serial.print(SleepTime);
  Serial.println(F("ms"));
  delay(1000); //Delay needed before sleeping in order to avoid messing up terminal messages
  Narcoleptic.delay(SleepTime); // During this time power consumption is minimised
  Serial.println(F("Awake again!"));
  
  delay(1000);
}


float tempReading() // Function TempReading is declared
{  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t = dht.readTemperature();
  if (isnan(t)) 
  {
    Serial.println(F("Failed to read temperature from DHT22"));
  } 
  return (float) t;
}

float humReading() // Function humReading is declared
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) 
  {
    Serial.println(F("Failed to read humidity from DHT22"));
  } 
  return (float) h;
}

String createDataToBeSent() // Function createDataToBeSent creates a String from sensorIDX, dtype and measured temperature & humidity values. Created String is returned.
{
  String dataString = String(sensorIDX);
  dataString += ':';
  dataString += String(dtype);
  dataString += ':';
  dataString += temperature;
  dataString += ':';
  dataString += humidity;
  dataString += ':';
  Serial.print(F("Created message (dataString): "));
  Serial.println(dataString);
  return dataString;
}

void sendData(String data) // Created String (constructor parameter) is firstly converted to char array and transmitted via RF link
{
  char msg[50]; //char array to store data to be sent via RF link
  data.toCharArray(msg, data.length()+1);
  
  digitalWrite(TxLed, HIGH); // Lit TxLed when transmission is ongoing
  if(vw_send((uint8_t *)msg, strlen(msg))) // Tries to transmits a message. Returns false if message was too long to be transmitted.
  {
    Serial.print(F("Message "));
    Serial.print(msg);
    Serial.println(F(" sent"));
  }
  else
  {
    Serial.println(F("FAIL - Message was too long to transmit!"));
  }
  
  Serial.println();
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(TxLed, LOW); // Switch off TxLed after the transmission
  delay(100);
}

// variables created by the build process when compiling the sketch. Used in memoryFree function
extern int __bss_end;
extern void *__brkval;

int memoryFree() //Function to return the amount of free RAM
{
  int freeValue; 
  if((int)__brkval == 0)
  {
    freeValue = ((int)&freeValue) - ((int)&__bss_end); 
  } 
  else
  {
    freeValue = ((int)&freeValue) - ((int)__brkval);
  } 
  return freeValue;
}
