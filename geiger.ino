/*Serial Communication for Radiation Detector Arduino Compatible DIY Kit ver 2.01 or higher
* http://radiohobbystore.com/radiation-detector-geiger-counter-diy-kit-second-edition.html
* Allow to connect the kit to computer and use the kit with Radiation Logger PC software
* http://radiohobbystore.com/radiation-logger/
* This Arduino sketch written by Alex Boguslavsky RH Electronics; mail: support@radiohobbystore.com
* CPM counting algorithm is very simple, it just collect GM Tube events during presettable log period.
* For radiation monitoring station it's recommended to use 30-60 seconds logging period. Feel free to modify
* or add functions to this sketch. This Arduino software is an example only for education purpose without any
* warranty for precision radiation measurements. You are fully responsible for your safety in high
* radiation area!!
* --------------------------------------------------------------------------------------
* WHAT IS CPM?
* CPM (or counts per minute) is events quantity from Geiger Tube you get during one minute. Usually it used to 
* calculate a radiation level. Different GM Tubes has different quantity of CPM for background. Some tubes can produce
* about 10-50 CPM for normal background, other GM Tube models produce 50-100 CPM or 0-5 CPM for same radiation level.
* Please refer your GM Tube datasheet for more information. Just for reference here, SBM-20 can generate 
* about 10-50 CPM for normal background.
* --------------------------------------------------------------------------------------
* HOW TO CONNECT GEIGER KIT?
* The kit 3 wires that should be connected to Arduino UNO board: 5V, GND and INT. PullUp resistor is included on
* kit PCB. Connect INT wire to Digital Pin#2 (INT0), 5V to 5V, GND to GND. Then connect the Arduino with
* USB cable to the computer and upload this sketch. 
*/

#include <RF24.h>
#include <nRF24L01.h>
#include <RF24_config.h>

#include <SPI.h>

#define LOG_PERIOD 15000  //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000  //Maximum logging period without modifying this sketch

unsigned long counts;     //variable for GM Tube events
unsigned long cpm;        //variable for CPM
unsigned int multiplier;  //variable for calculation CPM in this sketch
unsigned long previousMillis;  //variable for time measurement
float uSv;            // the measured microSiverts
float ratio = 151.0; // the divide the cpm by ration to get uSv

static RF24 rf(/*ce*/ 8, /*cs*/ 10);
static long int address = 0x66996699L;  // So that's 0x0066996699

void tube_impulse(){       //subprocedure for capturing events from Geiger Kit
  counts++;
}

void setup(){             //setup subprocedure
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD / LOG_PERIOD;      //calculating multiplier, depend on your log period
  Serial.begin(9600);
  attachInterrupt(0, tube_impulse, FALLING); //define external interrupts 
  
  // init RF24
  rf.begin();
  rf.setRetries(15, 15);
  rf.enableDynamicPayloads();
  rf.openWritingPipe(address);
}

void loop(){                                 //main cycle
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LOG_PERIOD){
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    
    Serial.println(cpm);
    uSv = cpm / ratio ;
    Serial.println(uSv);
    counts = 0;
    
    // send over RF24
    unsigned char buf[7];
    buf[0] = 6;
    buf[1] = 'G';
    buf[2] = 'E';
    buf[3] = 'I';
    buf[4] = 'G';
    buf[5] = (cpm >> 8) & 0xFF;
    buf[6] = (cpm >> 0) & 0xFF;
    rf.write(buf, sizeof(buf));
  }
  
}

