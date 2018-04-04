
// ftFuino TX/TXT-Kopplung
//
// (C) 2018 Christian Bergschneider, Stefan Fuss

#include <Wire.h>
#include <FtduinoSimple.h>

#define MyI2CBusAddress   0x10

#define CMD_nop           0x00
#define CMD_input_get     0x01
#define CMD_output_set    0x02
#define CMD_motor_set     0x03

#define Max_CommandBuffer 4
#define Max_ReturnValues  4

uint8_t CommandBuffer [Max_CommandBuffer];
uint8_t ReturnValues  [Max_ReturnValues];
uint8_t ReturnBytes = 0;

void setup() {
  Wire.begin(MyI2CBusAddress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(receiveRequest);
  Serial.begin( 9600 );
}

void debugValue( char* Msg, int Value) {
  char Buffer[255];
  sprintf( Buffer, Msg, Value );
  Serial.println( Buffer );
}

void Do_input_get() {
  // process CMD_input_get
  // Parameter1: Input Port
  ReturnValues[0] = ftduino.input_get( CommandBuffer[1] );
  ReturnBytes = 1;
}

void Do_output_set() {
  // process CMD_output_set
  // Parameter1: Output Port
  // Parameter2: Mode
  ftduino.output_set( CommandBuffer[1], CommandBuffer[2] );
}

void Do_motor_set() {
  // process CMD_output_set
  // Parameter1: Motor Port
  // Parameter2: Mode
  ftduino.motor_set( CommandBuffer[1], CommandBuffer[2] );
}

void receiveEvent(int bytesReceived){

  // store received data into CommandBuffer
  for (int i=0; i<bytesReceived; i++) {
    if (i<Max_CommandBuffer) {
      CommandBuffer[i] = Wire.read();
    } else {
      // skip received data if needed
      Wire.read();
    }
  }

  // Fill rest of CommandBuffer
  for (int i=bytesReceived; i<Max_CommandBuffer; i++ ) {
    CommandBuffer[i] = 0;
  }

  // Clear Return Buffer
  ReturnBytes = 0;

  // process commands
  switch (CommandBuffer[0]) {
    case CMD_nop:
      break;
    case CMD_input_get:
      Do_input_get();
      break;
    case CMD_output_set:
      Do_output_set();
      break;
    case CMD_motor_set:
      Do_motor_set();
      break;
  }

}

void receiveRequest(){
  // just send the already collected data
  
  if ( ReturnBytes <= 0 ) {
    // nothing to send available
    ReturnValues[0] = 0;
    ReturnBytes = 1;
    } 
    
  // send collected data
  Wire.write( ReturnValues, ReturnBytes );
  
}

void loop() {

  // just waiting for TX(T)...
  delay(500);

  }

