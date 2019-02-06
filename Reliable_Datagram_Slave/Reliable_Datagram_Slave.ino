// This script handles the communication of the slave/alpaca with the master
// Every slave/alpaca needs to have this script with a unique client adress

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C
#define RF95_FREQ 868.0
#define LED 13
#define CLIENT_ADDRESS 21 // this client adress needs to be unique!!
#define SERVER_ADDRESS 0
// Singleton instance of the radio driver
RH_RF95 driver;
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB
void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);
  pinMode(LED, OUTPUT);     
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
    driver.setTxPower(23, false);
    driver.setFrequency(RF95_FREQ);
  // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
//  driver.setTxPower(14, true);
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);
  bool status;
  status = bme.begin();  
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }
}
//uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
// main loop for the data communication
// buffers for the recieved and sent data needs to be set at the beginning
void loop()
{
  uint8_t len = sizeof(buf);
  uint8_t from; // Server adress   
  char datapacket[20];
  // if a message at the manager is available:
  if (manager.available())
  {
    // if the this slave/alpaca is adressed by the master it answers with its measurements of the temperature, humidity and pressure
    // these are converted to ints and longs and a offset is added so the length of the variables does not change
    // this datapacket, containing the temperature, humidity and pressure is then send back to the master
    // if the massage is send the LED on the slave/alpaca blinks once
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    { 
      itoa(int(bme.readTemperature()*100+27315), datapacket, 10);
      itoa(int(bme.readHumidity()*100+10000), datapacket+5, 10);
      ltoa(long(bme.readPressure()*100+10000000), datapacket+10, 10);
      if (manager.sendtoWait((uint8_t *)datapacket, sizeof(datapacket), from))
      {
      digitalWrite(LED, HIGH);
      digitalWrite(LED, LOW);
      }
    } 

  }
}
