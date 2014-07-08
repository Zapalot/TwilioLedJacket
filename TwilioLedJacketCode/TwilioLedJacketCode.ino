////////////////////////////////////////////////////////////////////
// This sketch shows how to remote control wearable LEDs using text messages
//
// Messages sent over the Twilio API can be received by a GSM Module.
// We used a generic 'SIM900' based Module, but anything that understands AT-commands over UART should work as well.
//
// A chain of individually adressable WS2801 LEDs is controlled by the Arduino.
// The code supports current limiting to allow safe powering over USB.
//
// Except for the Rainbow-Fade Effect by Stefan Hintz/TrafoPop, this code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// (2014) by Felix Bonowski / Made for Twilio 
////////////////////////////////////////////////////////////////////

///////////////////////How to wire things up:///////////////////////
// 
// ----------Connection to the GSM Module:--------------------------
// Our module was a bit picky about power.
// It needs AT LEAST 5.0V, anything less and it will crash erratically...
// The Arduino 5V pin actually gives you only about 4.7V because of the voltage drop across the protection diode on the Arduino board.
// The easiest way to make things work reliably is to cut a USB cable in the middle and draw power from it directly.
//
// So here is how you do it...
// GSM Module Ground/'-' <----> Arduino GND/0V (both connections are needed- see below!)
// GSM Module Ground/'-' <----> Cut USB cable GND (usually black) (both connections are needed- see above!)
// GSM Module 5V/'+'     <----> Cut USB cable 5V (usually red)
// GSM Module RX         <----> Arduino TX (Pin 0 on Leonardo/Micro/ProMicro, see below for Software Serial i.e. on Nano)
// GSM Module TX         <----> Arduino RX (Pin 1 on Leonardo/Micro/ProMicro, see below for Software Serial i.e. on Nano)
// GSM Module Power-On   <----> Arduino any digital Pin (#defined by 'gsmPowerPin' below)
//
// ----------Connection to the LEDs:--------------------------
//
// Any kind of WS2801 SPI-type controlled LEDs are supported.
// They can draw a low of power, so make sure you set the current limit and/or have a good USB power supply...
// It is also a good idea to put a large electrolythic capacitor (~1000µF, >=10V) in parallel to the LED Power supply
// 
//
// Unfortunately, there is no standard color scheme for the wires, so you will have to look at the datasheet to find out.
//
// LED strand GND/0V/-      <----> Arduino GND and/or cut USB cable GND (usually black)
// LED strand 5V/+          <----> Arduino 5V and/or cut USB cable 5V (usually red)
// LED strand MOSI/Data in  <----> Arduino MOSI (Pin 16 on Arduino Pro Micro, Pin 11 on Arduino Nano)
// LED strand SCLK/Clock    <----> Arduino SCLK (Pin 15 on Arduino Pro Micro, Pin 13 on Arduino Nano)
/////////////////////Have Fun!//////////////////////////////////////////


//stuff you might have to adjust to fit to your setup:
//LED Related:
#define totalCurrentLimit 1000  // maximum current we allow the leds to draw when they are all white
#define nTotalLeds 34           // the number of LEDs on your jacket

//gsm related:
//!!!!WARNING/DANGER!!!! Using the wrong pin might lock you out of your SIM-Card!
//it's safest to disable PIN-checking of your sim-card entirely.
char* simPinNumber=0;           // Set to PinNumber ("with quotes") to enable pin entering, leave 0 (without quotes!) to disable pin entering.
#define gsmPowerPin 2           // connected to the power-on pin "P" of the GSM module

// Serial connection to GSM Module:
//#define useSoftSerial         // uncomment if you use a software serial. else "Serial1" of a leonardo will be used
//if you don't have a Leonardo or micro, we use a software serial.
#ifdef useSoftSerial
#include "SoftwareSerial.h"
#define rxPin 3                 // pin connected to 'TX' of the GSM Module
#define dxPin 2                 // pin connected to 'RX' of the GSM Module
SoftwareSerial softSerial(rxPin,dxPin);
#define theGsmSerial softSerial
#else
#define theGsmSerial Serial1
#endif

#define DEBUG 0                 // enable debug messages by replacing 'DEBUG 0' with 'DEBUG 1'


// the functionality is split into a few modules:
#include "SPI.h"          // SPI is used to push data to the LEDs on a hardware level
#include "debugTrace.h"   // Debug output on Serial that can be easily switched off 
#include "commands.h"     // a general framework for text based commands on Arduino
#include "vec2d.h"        // some useful functions and structures for calculation in 2D-space
#include "ledPositions.h" // the positions of the LEDs on the jacket
#include "ledControl.h"   // initialization and hardware access to the LEDs
#include "ledEffects.h"   // the LED animations effects
#include "receiveHandler.h" // handles the "message receive" indicator and delays incoming messages while the indicator is active
#include "streamFun.h"    // some convenience functions for receiving data from streams
#include "gsmModule.h"    // a small GSM-Module interface for receiving SMS

#include "images.h"         // images you might want to show   
#include "imageSelection.h" // reception of image show commands

//this buffer is used for communication with the GSM module
#define receiveBufLength 200          //maximum length of data from gsm module:
char receiveBuffer[receiveBufLength]; // Receive Buffer for gsm module

//this buffer is used to store the last incoming text message
#define messageBufferLength 100          //maximum length of a message:
char messageBuffer[messageBufferLength]; //messages are stored here

//these objects do most of our work:
MimimalGSM gsm;               // manages setup&communication with the gsm module
CommandParser smsParser;      // takes care of parsing incoming messages for commands
LedEffectEngine effectEngine; // does the actual graphics
LedImageSelector ledImageEngine; //receives image commands and shows images


// the receive handler stores incoming messages and lets the Twilio logo indicate reception
ReceiveIndicatorHandler receiveHandler(
messageBuffer,  // it will use this buffer for the messages
&smsParser,     // it will invoke this parser once it finished blinking
1500,           // it will blink this many millis in total
50              // the leds will be lit this may millis at a time
); 


//all Commands are handled by so called callback Functions. To reduce clutter, we have put them in a separate file
#include "callbackFunctions.h"

void setup(){
  Serial.begin(115200);
  Serial.println("system startup");
  Serial.setTimeout(5);

  //you can easily add your own commands by writing a function and adding a callback like I do below for all the modes and colors.
  //the functions used here are declared in "callbackFunctions.h"
  smsParser.addCallback(F("#flash"),startFlash);
  smsParser.addCallback(F("#fade"),startFade);
  smsParser.addCallback(F("#glitter"),startGlitter);
  smsParser.addCallback(F("#fill"),startFill);
  smsParser.addCallback(F("#move"),startMove);
  smsParser.addCallback(F("#rainbow"),startRainbow);

  smsParser.addCallback(F("#green"),startGreen);
  smsParser.addCallback(F("#cyan"),startCyan);
  smsParser.addCallback(F("#blue"),startBlue);
  smsParser.addCallback(F("#mauve"),startMauve);
  smsParser.addCallback(F("#pink"),startPink);
  smsParser.addCallback(F("#red"),startRed);
  smsParser.addCallback(F("#orange"),startOrange);
  smsParser.addCallback(F("#yellow"),startYellow);
  smsParser.addCallback(F("#white"),startWhite);
  smsParser.addCallback(F("#black"),startBlack);

  smsParser.addReceiver(&ledImageEngine);  // Image showing is disabled in this version
  
  //LED initialization
  setupDisplayLedIndices(); // make a list of all LEDs that are not part of the logo
  setupLedControl();        // setup the SPI communication with the LEDs;

  //GSM initialization
  theGsmSerial.begin(9600); //initialize gsm serial
  gsm.setup(
  &theGsmSerial, // serial Object the module is connected to (i.e. &Serial1 on a Leonardo , or some software Serial)
  gsmPowerPin,    // pin connected to the Power on pin of the Module
  simPinNumber, // PIN-Number entered into the SIM-Card to unlock it
  receiveBuffer,    // a buffer for incoming data - has to be at least 32 bytes long
  receiveBufLength    // the length of the receive buffer
  );
}

void loop(){
  //  TRACELN(millis()); // show current time for test purposes

  // get new commands from Serial for test purposes
  receiveMessagesFromStream(&Serial,messageBuffer,messageBufferLength,&receiveHandler); 

  // receive sms message
  gsm.update();// receive notifications from gsm module and check connection
  if(gsm.smsWaiting){
    Serial.println(F("Message waiting!"));
    if(gsm.receiveSMS(true,messageBuffer,messageBufferLength)){
      Serial.println(F("got a Message!"));
      Serial.println(messageBuffer);
      receiveHandler.newMessageReceived(); // pass message to receive handler that will start flashing and invoke parsing
    }
    gsm.smsWaiting=false;
  }

  receiveHandler.update();        // blink receive indicator and parse newest message when finished
  effectEngine.updateColors();    // update LED colors according to current effect

  ledImageEngine.applyOnLeds(); // show images 

  //check for gsm connection using flag updated by "gsm.update()"
  if(!gsm.connectionGood){
    gsm.initializeModule(simPinNumber); // maybe the module needs to be initialized again after a reset/loss of power?
    gsm.checkConnection();
  }
  if(!gsm.connectionGood)effectEngine.doConnectionBad(); //a visual indicator for bad connection - overwrites colors from current effect

  sendColorBuffer();           // push the updated colors to the LEDs
  delay(2);                    // led chain needs some time to update its colors
};



















