/* MECH 452
 * CMU Cam2 Sample Program
 * Kevin Hughes, 2013
 *
 *  The circuit: 
 *   RX is digital pin 2 (connect to TX of other device)
 *   TX is digital pin 3 (connect to RX of other device)
 */

#include <SoftwareSerial.h>

SoftwareSerial cmucam(2,3);
unsigned char RcvData[8] = "";


/* 
 * Function to print the data packet
 */
void print_packet(unsigned char * packet)
{
  for(int i = 0; i < 8; i++)
  {
     Serial.print( (int)packet[i] );
     Serial.print(" "); 
  }
  Serial.println();
}


/* 
 * Function for sending commands to the CMU Cam2
 * where no return data aside from the 'ACK' 
 * command is expected. The verbose option
 * will print out exactly what is sent and 
 * recieved from the CMU Cam2
 */
boolean cmucam2_set(char* cmd, boolean verbose=false)
{
  if(verbose)
  {
    Serial.print("Sending CMU Cam2 Command: ");
    Serial.print(cmd);
    Serial.println();
  }

  // send the command
  cmucam.print(cmd);
  cmucam.print("\r");
  cmucam.listen();

  boolean ack = false;

  // get the response
  while( cmucam.available() > 0 ) 
  {
    char inbyte = cmucam.read();

    if(inbyte == ':')
      ack = true;  

    if(verbose)
      Serial.write(inbyte);
  }

  if(verbose)
    Serial.println();

  // flush
  while( cmucam.available() > 0 )
    cmucam.read();

  return ack;
}


/* 
 * Function for sending commands to the CMU Cam2
 * where return data is expected. The packet type
 * must be specified, currently only S and T packets
 * are supported. This code expects the camera to be in
 * raw mode and still sending 'ACK'. The rtrn buffer 
 * must be at least 8 bytes for T packets and 
 * at least 6 bytes for S packets. 
 * The verbose option will print out exactly what 
 * is sent and recieved from the CMU Cam2
 */
boolean cmucam2_get(char* cmd, char packet, unsigned char *rtrn, boolean verbose=false)
{
  if(verbose)
  {
    Serial.print("Sending CMU Cam2 Command: ");
    Serial.print(cmd);
    Serial.println();
  }

  // send the command
  cmucam.print(cmd);
  cmucam.print("\r");
  cmucam.listen();

  // Debug Packet
  // Change to true and
  // turn of raw mode "RM 0"
  // to see the packet
  boolean debug = false;
  if(debug) 
  {
    while(cmucam.available() > 0)
    {
      delay(100);
      Serial.print((char)cmucam.read());    
    }
    Serial.println();
    
    return true;
  }

  // S-Packet
  // raw mode must be on
  if(packet == 'S')
  {
    while(cmucam.read() != 255); // wait for signal bit
    while(cmucam.read() != 83);
    while(cmucam.available() < 6); // wait for data
    for(int i = 0; i < 6; i++) // read the packet
    {
      rtrn[i] = cmucam.read();    
    }
  }

  // T-Packet
  // raw mode must be on
  if(packet == 'T')
  {
    while(cmucam.read() != 255); // wait for signal bit
    while(cmucam.read() != 84); 
    while(cmucam.available() < 8); // wait for data
    for(int i = 0; i < 8; i++) // read the packet
    {
      rtrn[i] = cmucam.read();    
    }
  }

  return true;  
}


void setup()
{
  Serial.begin(9600);

  // Init CMU Cam2
    cmucam.begin(9600);
    cmucam.print("RS"); 
    cmucam.print("\r");
    cmucam.print("RS"); 
    cmucam.print("\r");
    cmucam.listen();
    while( cmucam.available() > 0 ) 
    {
      cmucam.read();
    }
    delay(100);
    while(!cmucam2_set("RS", true));
  // End Init CMU Cam2
}


void loop()
{
  cmucam2_set("L0 0"); // green LED (on camera) off
  cmucam2_set("L1 0"); // red LED (on camera) off

  // White Balance
  while(!cmucam2_set("CR 18 44", true));// RGB white balance on
  for(int i = 0; i < 10; i++) // let the camera adjust for 5 seconds
  {
    cmucam2_set("L0 1"); // green LED (on camera) on
    delay(250);
    cmucam2_set("L0 0"); // green LED (on camera) off
    delay(250);  
  }
  while(!cmucam2_set("CR 18 40", true));// RGB white balance off
  while(!cmucam2_set("CR 19 32", true));// auto gain off

  // set up the camera for color tracking
  while(!cmucam2_set("PM 1", true));// set poll mode
  while(!cmucam2_set("RM 1", true));// set raw mode on
  //while(!cmucam2_set("RM 0", true));// set raw mode off
  
  cmucam2_get("GM",'S', RcvData, true);
  print_packet(RcvData);
  delay(500);
  
  cmucam2_get("TC 140 150 60 80 90 100",'T', RcvData, true);
  print_packet(RcvData);
  delay(500);
  
  cmucam2_get("TW",'T', RcvData, true);
  print_packet(RcvData);
  delay(500);

  // End of Program, Blink Camera LEDs
  while(true) 
  {
    cmucam2_set("L0 1"); // green LED (on camera) on
    cmucam2_set("L1 0"); // red LED (on camera) off
    delay(1000);
    cmucam2_set("L0 0"); // green LED (on camera) off
    cmucam2_set("L1 1"); // red LED (on camera) on
    delay(1000);
  }
}

