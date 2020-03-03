  #include <avr/io.h>
  #include <avr/interrupt.h>
  
  const int latch = 0;  // latch and output shift registers
  const int b1 = 2;     // button 1
  const int b2 = 1;     // button 2
  const int data = 3;   // serial data output
  const int clk = 4;    // serial clock output

  int number[] = {  // 7-segment digit bytes
    B11111100, // 0
    B01100000, // 1
    B11011010, // 2
    B11110010, // 3
    B01100110, // 4
    B10110110, // 5
    B10111110, // 6
    B11100000, // 7
    B11111110, // 8
    B11110110  // 9
  };

  int digit[] = {  // 7-segment position bytes
    B00000000, // blank
    B10000000, // first
    B01000000, // second
    B00001000, // third
    B00000100, // fourth
  };

  int b1State = 1;
  int b2State = 1;
  int lastb1State = b1State;
  int lastb2State = b2State;
  
  volatile int second = 0;
  volatile int minute = 34;
  volatile int hour = 12;

void setup() {
  // put your setup code here, to run once:
  pinMode(latch, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(b1, INPUT);
  pinMode(b2, INPUT);
}

static inline void initTimer1(void)
{
  TCCR1 |= (1 << CTC1);  // clear timer on compare match
  TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11); //clock prescaler 8192
  OCR1C = 124; // compare match value attempting to match at 1 second
  TIMSK |= (1 << OCIE1A); // enable compare match interrupt
}

ISR(TIMER1_COMPA_vect){ // interupt on OCR1C compare and add a second to time
  if (second < 59) {
    second++; } else {
      second = 0;
      if (minute < 59) {
        minute++; } else {
          minute = 0;
          if (hour < 12) {
            hour++; } else {
            hour = 1;
            }
        }
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  initTimer1();   // initialize timer
  sei();          // google this, i forgot what it does
  b1State = digitalRead(b1);
  b2State = digitalRead(b2);

  if (b2State != lastb2State) {   // button 2 on rising edge
    if (b2State == LOW) {
      second = 0;
      if (minute < 59) {
        minute++; } else {
          minute = 0;
        }
        delay(10);    // delay to debounce the switch. should build r-c circuit instead
    }
  lastb2State = b2State;
  }

  if (b1State != lastb1State) {   // button 1 on rising edge
    if (b1State == LOW) {
      if (hour < 12) {
        hour++; } else {
          hour = 1;
        }
        delay(10);    // delay to debounce the switch. should build r-c circuit instead
    }
  lastb1State = b1State;
  }

   digitalWrite(latch, LOW);    // stop outputting from shift registers
   if ((hour / 10 % 10) == 0) {  // don't show a leading "0" on hour
    void();
   } else {
   shiftOut(data, clk, LSBFIRST, digit[1]);   // serial output position
   shiftOut(data, clk, LSBFIRST, number[(hour / 10) % 10]);   // serial output first digit of hour
   digitalWrite(latch, HIGH);   // start outputting from shift registers
   }

   // repeat for each digit of 7-segment display
   digitalWrite(latch, LOW);
   shiftOut(data, clk, LSBFIRST, digit[2]);
   shiftOut(data, clk, LSBFIRST, number[hour % 10]);
   digitalWrite(latch, HIGH);
   digitalWrite(latch, LOW);
   shiftOut(data, clk, LSBFIRST, digit[3]);
   shiftOut(data, clk, LSBFIRST, number[(minute / 10) % 10]);
   digitalWrite(latch, HIGH);
   digitalWrite(latch, LOW);
   shiftOut(data, clk, LSBFIRST, digit[4]);
   shiftOut(data, clk, LSBFIRST, number[minute % 10]);
   digitalWrite(latch, HIGH);
  
}
