// This script is only used be the god alpaca which has the job to send its gps position to the master
// To use the script the arduino needs to have a gps chip
// The script works on the same principle as the normal slave/alpaca script
// The master sends a data request to every specified client adress and the adressed slave/alpaca answers with its measured data
// In this case the god sends back its gps position
#include <SoftwareSerial.h>
#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

TinyGPS gps;
SoftwareSerial ss(4, 3);
#define LED 13
#define SEALEVELPRESSURE_HPA (1013.25)
#define RF95_FREQ 868.0
#define CLIENT_ADDRESS 22 // has to be changed if the client adress of the god should be changed
#define SERVER_ADDRESS 0  // has to be changed if the server adress of the master is changed
bool l=true;
unsigned long delayTime;
RH_RF95 driver;


// Change to 434.0 or other frequency, must match RX's freq!


RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup()
{
  Serial.begin(4800);
  ss.begin(9600); // software serial of the gps chip
  
  Serial.print("Simple TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println("by Mikal Hart");
  Serial.println();

  Serial.println("Arduino LoRa TX Test!");
  if (!manager.init())
    Serial.println("init failed");

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(23, false);
  driver.setFrequency(RF95_FREQ);
}

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
float flat=22.22222, flon=22.22222;
float ele = 0.99;
int newgpsdata = 0;
void loop()
// main loop of the script
// sizes of buffers need to be set at the beginning, gps datapacket buffer size is 45 for lat, lon, elevation and a flag if new data is present
// if a data request from the master is available and the gps data is available datapackets with the gps coordinates is send to the master
// incase no gps fix is achieved, error values are send to the master to differentiate between true and false values
{
  uint8_t len = sizeof(buf);
  uint8_t from; // Server adress   
  char datapacket[45];   
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 500;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  
  if (manager.available())
  {
    if (manager.recvfromAckTimeout(buf, &len, 1000, &from))
    { 
        unsigned long age;
        ele = gps.f_altitude();
        gps.f_get_position(&flat, &flon, &age);
        newgpsdata = 1;
        itoa(int(newgpsdata),datapacket,10);
        ltoa(long(flat*100000), datapacket+1,10);
        ltoa(long((flon)*100000), datapacket+8,10);
        ltoa(long((ele+1000)*100), datapacket+15,10);    
      if (manager.sendtoWait((uint8_t *)datapacket, sizeof(datapacket), from))
      {
      digitalWrite(LED, HIGH);
      digitalWrite(LED, LOW);
      }
    } 

  }
  newgpsdata = 0;
}
