
// ftFuino TX/TXT-Kopplung
//
// (C) 2018 Christian Bergschneider, Stefan Fuss
//
// ftpi@gmx.de
//
// V0.0.2 refactoring, implementation of all Ftduino-cmds
// V1.20 2019-05-18 don't wait on serial during bootup

#include <Wire.h>
#include <Ftduino.h>

// My own I2C address - must fit to RoboPro I2C cmds
#define MyI2CBusAddress   0x20

// My serial baud rate
#define MySerialBaudRate  9600

// my I2C cmds
#define CMD_nop                     0x00
#define CMD_input_get               0x01
#define CMD_output_set              0x02
#define CMD_motor_set               0x03
#define CMD_input_set_mode          0x04
#define CMD_motor_counter           0x05
#define CMD_motor_counter           0x05
#define CMD_motor_counter_active    0x06
#define CMD_motor_counter_set_brake 0x07
#define CMD_counter_set_mode        0x08
#define CMD_counter_get             0x09
#define CMD_counter_clear           0x0A
#define CMD_counter_get_state       0x0B
#define CMD_ultrasonic_enable       0x0C
#define CMD_ultrasonic_get          0x0D
#define CMD_init                    0x0E
#define CMD_Watchdog                0x0F

// The command buffer stores all received I2C data from TX(T)
#define Max_CommandBuffer 6
uint8_t CommandBuffer [Max_CommandBuffer];

// The return buffer stores all data to send via I2C to TX(T)
#define Max_ReturnBuffer  4
uint8_t ReturnBuffer [Max_ReturnBuffer];
uint8_t ReturnBytes = 0;

// Watchdog to stop motors if communication stops
uint16_t Watchdog = 0;
boolean  WatchdogEnabled = false;

// Setup the arduino
void setup() {

  // initialize on board LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // register I2C
  Wire.begin(MyI2CBusAddress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(receiveRequest);

  // register serial debugging
  Serial.begin( MySerialBaudRate );

  // initialize ftduino
  ftduino.init();

  // Boot message
  Serial.println("");
  Serial.println("------------------------------------------------------------------------------");
  Serial.println("ftDuino_TXT" );
  Serial.println("RoboPro interface to run a ftDuino as I2C Slave device on an TX(T) controller.");
  Serial.println("Version 1.20");
  Serial.println("(C) 2018 Christian Bergschneider & Stefan Fuss");
  Serial.println("------------------------------------------------------------------------------");
  Serial.println("");
  debugValue("Waiting on I2C interface with address %i to receive TX(T) commands...", MyI2CBusAddress );
  
  
}

void debugValue( char* Msg, int Value) {
  // print a debug value via serial
  char Buffer[255];
  sprintf( Buffer, Msg, Value );
  Serial.println( Buffer );
}


void debugValue( char* Msg, int Value1, int Value2) {
  // print a debug value via serial
  char Buffer[255];
  sprintf( Buffer, Msg, Value1, Value2 );
  Serial.println( Buffer );
}

void debugValue( char* Msg, int Value1, int Value2, int Value3) {
  // print a debug value via serial
  char Buffer[255];
  sprintf( Buffer, Msg, Value1, Value2, Value3 );
  Serial.println( Buffer );
}

void debugValue( char* Msg, int Value1, int Value2, int Value3, int Value4) {
  // print a debug value via serial
  char Buffer[255];
  sprintf( Buffer, Msg, Value1, Value2, Value3, Value4 );
  Serial.println( Buffer );
}

void Process_input_set_mode() {
  // process CMD_input_set_mode
  // Parameter1: Input Port
  // Parameter2: Mode (RESISTANCE, VOLTAGE, SWITCH) 

  debugValue("Process_input_set_mode(%i, %i)", CommandBuffer[1], CommandBuffer[2] );
  ftduino.input_set_mode( CommandBuffer[1], CommandBuffer[2] );

}

void Process_input_get() {
  // process CMD_input_get
  // Parameter1: Input Port
  // Parameter2: Mode (RESISTANCE, VOLTAGE, SWITCH) 
  // Return0: MSB
  // Return1: LSB

  // read input value
  uint16_t Value = ftduino.input_get( CommandBuffer[1] );
  debugValue("Process_input_get(%i)=%i", CommandBuffer[1], Value);
  
  // set return values
  ReturnBuffer[0] = Value / 0x100;
  ReturnBuffer[1] = Value % 0x100;
  ReturnBytes = 2;
  
}

void Process_output_set() {
  // process CMD_output_set
  // Parameter1: Output Port
  // Parameter2: Mode
  // Parameter3: PWM Speed 0..255

  debugValue("Process_output_set(%i, %i, %i)", CommandBuffer[1], CommandBuffer[2], CommandBuffer[3] );
  ftduino.output_set( CommandBuffer[1], CommandBuffer[2], CommandBuffer[3] );  

}

void Process_motor_set() {
  // process CMD_motor_set
  // Parameter1: Motor Port
  // Parameter2: Mode
  // Parameter3: PWM Speed 0..255

  debugValue("Process_motor_set(%i, %i, %i)", CommandBuffer[1], CommandBuffer[2], CommandBuffer[3] );
  ftduino.motor_set( CommandBuffer[1], CommandBuffer[2], CommandBuffer[3] );
  
}

void Process_motor_counter() {
  // process CMD_motor_counter
  // Parameter1: Motor Port
  // Parameter2: Mode
  // Parameter3: PWM Speed 0..255
  // Parameter4: Counter MSB
  // Parameter5: Counter LSB

  uint16_t Counter = CommandBuffer[4]*256 + CommandBuffer[5];
  
  debugValue("Process_motor_counter(%i, %i, %i, %i)", CommandBuffer[1], CommandBuffer[2], CommandBuffer[3], Counter );
  ftduino.motor_counter( CommandBuffer[1], CommandBuffer[2], CommandBuffer[3], Counter );
  
}

void Process_motor_counter_active() {
  // process CMD_motor_counter_active
  // Return1 boolean

  ReturnBuffer[0] = ftduino.motor_counter_active( CommandBuffer[1] );
  debugValue("Process_motor_counter_active(%i)=%i", CommandBuffer[1], Process_motor_counter_active );
  ReturnBytes = 1;
  
}

void Process_motor_counter_set_brake() {
  // process CMD_motor_counter_set_brake
  // Parameter1: Motor Port
  // Parameter2: on/off
  
  debugValue("Process_motor_counter_set_brake(%i, %i)", CommandBuffer[1], CommandBuffer[2] );
  ftduino.motor_counter_set_brake( CommandBuffer[1], CommandBuffer[2] );
  
}

void Process_counter_set_mode() {
  // process CMD_counter_set_mode
  // Parameter1: Channel
  // Parameter2: mode
  
  debugValue("Process_counter_set_mode(%i, %i)", CommandBuffer[1], CommandBuffer[2] );
  ftduino.counter_set_mode( CommandBuffer[1], CommandBuffer[2] );
  
}

void Process_counter_get() {
  // process CMD_counter_get
  // Parameter1: Channel
  // Return1: Counter MSB
  // Return2: Counter LSB
  
  uint16_t Counter = ftduino.counter_get( CommandBuffer[1] );
  debugValue("Process_counter_get(%i)=%i", CommandBuffer[1], Counter );
  
  // set return values
  ReturnBuffer[0] = Counter / 0x100;
  ReturnBuffer[1] = Counter % 0x100;
  ReturnBytes = 2;

}

void Process_counter_clear() {
  // process CMD_counter_clear
  // Parameter1: Channel
  
  debugValue("Process_counter_clear(%i)", CommandBuffer[1] );
  ftduino.counter_clear( CommandBuffer[1] );
  
}

void Process_counter_get_state() {
  // process CMD_counter_get_stae
  // Parameter1: Channel
  // Return1: 0
  // Return2: bool
  
  // set return values
  ReturnBuffer[0] = 0;
  ReturnBuffer[1] = ftduino.counter_get_state( CommandBuffer[1] );
  debugValue("Process_counter_get(%i)=%i", CommandBuffer[1], ReturnBuffer[1] );
  ReturnBytes = 2;

}

void Process_ultrasonic_enable() {
  // process CMD_ultrasonic_enable
  // Parameter1: On/Off
  
  debugValue("Process_ultrasonic_enable(%i)", CommandBuffer[1] );
  ftduino.ultrasonic_enable( CommandBuffer[1] );
  
}

void Process_ultrasonic_get() {
  // process CMD_ultrasonic_get
  // Return1: distance MSB
  // Return2: distance LSB
  
  uint16_t distance = ftduino.ultrasonic_get( );
  debugValue("Process_ultrasonic_get(%i)=%i", CommandBuffer[1], distance );
  
  // set return values
  ReturnBuffer[0] = distance / 0x100;
  ReturnBuffer[1] = distance % 0x100;
  ReturnBytes = 2;

}

void Process_init() {
  // process CMD_init

  WatchdogEnabled = true;
  Watchdog =0;

  Serial.println("Init()");

}

void Process_Watchdog() {
  // process CMD_init

  WatchdogEnabled = true;
  Watchdog = 0;

  Serial.println("Process_Watchdog()");

}

void receiveEvent(int bytesReceived){
  // is called when I2C data is received

  digitalWrite(LED_BUILTIN, HIGH);

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

  digitalWrite(LED_BUILTIN, LOW);
  
  // process commands
  switch (CommandBuffer[0]) {
    case CMD_nop:
      break;
    case CMD_input_get:
      Process_input_get();
      break;
    case CMD_output_set:
      Process_output_set();
      break;
    case CMD_motor_set:
      Process_motor_set();
      break;
    case CMD_input_set_mode:
      Process_input_set_mode();
      break;
    case CMD_motor_counter:
      Process_motor_counter();
      break;
    case CMD_motor_counter_active:
      Process_motor_counter_active();
      break;
    case CMD_motor_counter_set_brake:
      Process_motor_counter_set_brake();
      break;
    case CMD_counter_set_mode:
      Process_counter_set_mode();
      break;
    case CMD_counter_get:
      Process_counter_get();
      break;
    case CMD_counter_clear:
      Process_counter_clear();
      break;
    case CMD_counter_get_state:
      Process_counter_get_state();
      break;
    case CMD_ultrasonic_enable:
      Process_ultrasonic_enable();
      break;
    case CMD_ultrasonic_get:
      Process_ultrasonic_get();
      break;
    case CMD_init:
      Process_init();
      break;
    case CMD_Watchdog:
      Process_Watchdog();
      break;
    default:
      debugValue( "Unkown Cmd %i", CommandBuffer[0] );
    }

}

void receiveRequest(){
  // just send the already collected data


  digitalWrite(LED_BUILTIN, HIGH);
  if ( ReturnBytes <= 0 ) {
    // nothing to send available
    ReturnBuffer[0] = 0;
    ReturnBytes = 1;
    } 
    
  // send collected data
  Wire.write( ReturnBuffer, ReturnBytes );

  digitalWrite(LED_BUILTIN, LOW);
  
}

void loop() {
    
  // just waiting for TX(T)...
  delay(100);

  if (WatchdogEnabled) {
    Watchdog = Watchdog + 100;
    if ( Watchdog == 1500 ) {
      ftduino.motor_set( Ftduino::M1, Ftduino::BRAKE, 0);
      ftduino.motor_set( Ftduino::M2, Ftduino::BRAKE, 0);
      ftduino.motor_set( Ftduino::M3, Ftduino::BRAKE, 0);
      ftduino.motor_set( Ftduino::M4, Ftduino::BRAKE, 0);
      Serial.println("Watchdog is barking...");
    }
    
  }

}
