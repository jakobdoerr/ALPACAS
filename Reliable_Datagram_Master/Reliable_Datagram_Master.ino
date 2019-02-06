// This script handles the communication for the master with the slaves/alpacas.
// Only the Master which is attached to a serial port of a computer needs this script.
// The script writes the recieved data to the serial port. This is a string of numbers which is read in by the live_data.py script
// Variables which need to be changed for a different number of active slaves/alpacas:
// int nnodes: Total number of active slaves/alpacas
// uint8_t slaves[]: Array of all client adresses of the slaves/alpacas, these need to be the same as set for every slave/alpaca, also calles client adress, the size of the array needs to be changed depending on the number of active slaves
// client adress of god: 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#define SERVER_ADDRESS 0 // node adress of the master
#define RF95_FREQ 868.0 // change this if the LoRA chip uses a different frequency

// Singleton instance of the radio driver
RH_RF95 driver;
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);
// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB
void setup() 
{
// Rocket Scream Mini Ultra Pro with the RFM95W only:
// Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);
  Serial.begin(115200);
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
}
int nnodes = 11; // number of used slaves/alpacas
unsigned long packetnum = 0;  // packet counter, we increment per xmission
uint8_t slaves[13] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21}; // Array of all client adresses of the arduinos
//client adress of god is 22
uint8_t broadcast[] = "Measurement Request";
uint8_t datarequest[] = "Data Request";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t gottbuftemp[RH_RF95_MAX_MESSAGE_LEN];
uint8_t gottbufgps[RH_RF95_MAX_MESSAGE_LEN];
// main loop for the data communication
// buffer for recieved data is set at the beginning
void loop()
{
    uint8_t len = sizeof(buf);
    uint8_t gottlentemp = sizeof(gottbuftemp);
    uint8_t gottlengps = sizeof(gottbufgps);

    uint8_t from;
    delay(10);
    // looping over all nodes/slaves
    // sends a data request to every slave/alpaca (sendtoWait)
    // waits atleast 500ms for an answer of every slave/alpaca (recvfromAckTimeout)
    // writes the answer to the serialport of the attached computer
    // if the answering adress is "22" (still hardcoded, can be changed to the client adress of god alpaca if needed), the god sends its gps position
    for (int i = 0; i < nnodes; i++)
    {
      from = slaves[i];      
      manager.sendtoWait(datarequest, sizeof(datarequest), from);
      //if (manager.sendtoWait(datarequest, sizeof(datarequest), from))
        //{
      Serial.println((char*)datarequest);
        //}
      if (from == 22) // client adress of god 
      {
        if (manager.recvfromAckTimeout(gottbufgps, &gottlengps, 500, &from))
        {
          //Serial.print("got reply from : slave");^
          //Serial.print(": ");
          Serial.print((char*)gottbufgps);
          Serial.print(from, DEC);
          Serial.println(packetnum);
        }
      }
      else 
      {
        if (manager.recvfromAckTimeout(buf, &len, 500, &from))
        {
          //Serial.print("got reply from : slave");^
          //Serial.print(": ");
          Serial.print((char*)buf);
          Serial.print(from, DEC);
          Serial.println(packetnum);
        }
      }
      delay(10);
    }
    packetnum++;
}

