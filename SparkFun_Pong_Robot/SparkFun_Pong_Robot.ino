#include <RFM69.h>
#include <SPI.h>

// Addresses for this node. CHANGE THESE FOR EACH NODE!
#define NETWORKID     0   // 0 for one robot/controller pair, 1 for the second pair
#define MYNODEID      2   // ID for the robot
#define TONODEID      1   // ID for the controller

// RFM69 frequency, 915MHZ or 434MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):
#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// time in milliseconds to wait before checking to see if the robot should be forced to stop
#define INTERVAL      100

double currentTime, oldTime;
RFM69 radio;

void setup() {
  Serial1.begin(115200);
  pinMode(2, INPUT_PULLUP);
  
  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);
  
  oldTime = millis();
}

void loop() {
  String motor[2];
  byte activeCups = 0;
  
  // Read IR sensors
  for(byte i=0; i<6; i++)
  {
    if(analogRead(14+i) < 800)
      activeCups++;
  }

  bool stop = 0;
  byte prob[] = {0,80,50,30,10,5,0};  // Probability of forced stop based on the number of active cups
  byte randomNumber = random(100);

  currentTime = millis();
  if((currentTime-oldTime) > INTERVAL)
  {
    oldTime = currentTime;
    for(byte i=0;i<prob[activeCups];i++)
    {
      if(i==randomNumber)
      {
        stop = 1;
        break;
      }
    }
  }
  
  if (radio.receiveDone()) // Got one!
  {
    String message = "";
    for (byte i = 0; i < radio.DATALEN; i++)
      message += (char)radio.DATA[i];
    // Break apart data packet to individual motors
    byte index = message.indexOf('&');
    motor[0] = message.substring(0,index);
    motor[1] = message.substring(index+1);
  }

  if(stop==1)
  {
    motor[0] = "1f0\r";
    motor[1] = "2f0\r";
  }

  if(motor[0].length() > 0)
  {
    // Send motor speeds to Serial Motor Driver
    delay(50);
    Serial1.print(motor[0]+'\r');
    delay(50);
    Serial1.print(motor[1]+'\r');
  }
}
