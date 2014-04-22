/*
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
  pinMode(STATUS_PIN, OUTPUT);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  
    Serial.print("Received unknown code, saving as raw, count ");
    Serial.println(count, DEC);
    
    for (unsigned long mask = 0x80000000; mask; mask >>= 1) {
      Serial.print(mask&results->data[0]?'1':'0');
    }
    Serial.print(" ");
    for (unsigned long mask = 0x80000000; mask; mask >>= 1) {
      Serial.print(mask&results->data[1]?'1':'0');
    }
    Serial.print(" ");
    for (unsigned long mask = 0x80000000; mask; mask >>= 1) {
      Serial.print(mask&results->data[2]?'1':'0');
    }
    Serial.print(" ");
    for (unsigned long mask = 0x80000000; mask; mask >>= 1) {
      Serial.print(mask&results->data[3]?'1':'0');
    }
    Serial.println("");
    
    int temp = getNibble(results->data, 17);
    int mode = getNibble(results->data, 18);
    int fan = getNibble(results->data, 20);
    Serial.print("Temp: ");
    Serial.println(temp, DEC);
    Serial.print("Mode: ");
    Serial.println(mode, DEC);
    Serial.print("Fan: ");
    Serial.println(fan, DEC);
}

int getNibble(unsigned long data[], int index){
  int el = index/8;
  index = (7 - (index % 8)) * 4;
    
  int val = 0;
  for(int i=0; i<4; i++){
    int multiplier = 1<<(3-i);
    int bitVal = bitRead(data[el], index+i); //(0x80000000>>(index+i)) & data[el];
//    Serial.print("[");
//    Serial.print(index+i, DEC);
//    Serial.print(":");
//    Serial.print(multiplier, DEC);
//    Serial.print("*");
//    Serial.print(bitVal, DEC);
//    Serial.print("]");
    val += multiplier * bitVal;
  }
  
  return val;
}

void sendCode(int repeat) {
  
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  
}

int lastButtonState;

void loop() {
  // If button pressed, send the code.
  int buttonState = !digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
  }

  if (buttonState) {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState == buttonState);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  } 
  else if (irrecv.decode(&results)) {
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);
  }
  lastButtonState = buttonState;
}
