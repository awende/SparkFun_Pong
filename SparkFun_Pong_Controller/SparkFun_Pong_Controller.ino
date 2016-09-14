#include <RFM69.h>
#include <SPI.h>

// Addresses for this node. CHANGE THESE FOR EACH NODE!
#define NETWORKID     0   // 0 for one robot/controller pair, 1 for the second pair
#define MYNODEID      1   // ID for the controller
#define TONODEID      2   // ID for the robot

// RFM69 frequency, 915MHZ or 434MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):
#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

// Maximum motor speed (0-9)
#define SPEED 3 

RFM69 radio;
int speed1,speed2, oldSpeed1, oldSpeed2;


void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);

  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);
}

void loop() {
  long sum = 0; // Used for averaging ADC reads
  String motor1, motor2;  // String message to build data packet
  static char sendbuffer[7];  // Data packet sent to robot
  
  //1 Read Pots and average readings
  sum = 0;
  for(byte i=0;i<16;i++)
  {
    sum += analogRead(A1);
  }
  int average = sum/16;
  
  if(average > 800) speed1 = SPEED; // set left motor speed forward
  else if(average < 200) speed1 = -1 * SPEED; // set left motor speed reverse
  else speed1 = 0;  // stop left motor

  sum = 0;
  for(byte i=0;i<16;i++)
  {
    sum += analogRead(A0);
  }
  average = sum/16;
  if(average > 800) speed2 = SPEED; // set right motor speed forward
  else if(average < 200) speed2 = -1 * SPEED; // set left motor speed reverse
  else speed2 = 0;  // stop right motor

  // Only send packet if the data was different from the last packet sent
  if(speed1!=oldSpeed1 || speed2!=oldSpeed2)
  {
    oldSpeed1 = speed1;
    oldSpeed2 = speed2;
    
    // packet structure : motor(0 or 1) direction(f-forward, r-reverse) speed(0 or SPEED)
    
    if(speed1 < 0) motor1 = "1r"+String(abs(speed1));
    else motor1 = "1f"+String(speed1);

    if(speed2 < 0) motor2 = "2r"+String(abs(speed2));
    else motor2 = "2f"+String(speed2);

    // build data packet
    String message = motor1+"&"+motor2;
    for(byte j=0;j<7;j++)
    {
      sendbuffer[j] = message.charAt(j);
    }
    
    // Send data packet
    radio.send(TONODEID,sendbuffer,7);

  }
  delay(100);
}
