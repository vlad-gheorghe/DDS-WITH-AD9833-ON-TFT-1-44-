/*
AD9833 Waveform Module vwlowen.co.uk
*/

#include <SPI.h>
#include <Rotary.h>            // Rotary encoder: https://github.com/brianlow/Rotary 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#define dc   A0                // Define pins for TFT display.           
//#define cs   A1                // 
//#define rst  A2  

#include <Adafruit_GFX.h>      // Core graphics library

// include Adafruit library OR QDTech library depending on the display's controller chip.

 // #include <Adafruit_ST7735.h>            // Hardware-specific library
 // Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
 // #include <Adafruit_QDTech.h>              // Hardware-specific library
//  Adafruit_QDTech tft = Adafruit_QDTech(cs, dc, rst);  


// https://github.com/zigwart/Adafruit_QDTech


#define BLACK 0x000                         // Define the display colours we'll be using
#define BLUE 0x001F                         // so they're constants regardless of which
#define GREEN 0x07E0                        // display library we use.
#define YELLOW 0xFFE0
#define GREY 0x632C

const int SINE = 0x2000;                    // Define AD9833's waveform register value.
const int SQUARE = 0x2028;                  // When we update the frequency, we need to
const int TRIANGLE = 0x2002;                // define the waveform when we end writing.    

int wave = 0;
int waveType = SINE;
int wavePin = 7;

int freqUpPin = 2;         //dt                 // Define rotary encoder pins.
int freqDownPin = 3;       // clk
int stepUpPin = 5;         // clk
int stepDownPin = 6;       // dt

const float refFreq = 25000000.0;           // On-board crystal reference frequency

const int FSYNC = 10;                       // Standard SPI pins for the AD9833 waveform generator.
const int CLK = 13;                         // CLK and DATA pins are shared with the TFT display.
const int DATA = 11;

Rotary r = Rotary(freqUpPin, freqDownPin);    // Rotary encoder for frequency connects to interrupt pins
Rotary i = Rotary(stepUpPin, stepDownPin);    // Rotart encoder for setting increment.

unsigned long freq = 1000;               // Set initial frequency.
unsigned long freqOld = freq;

unsigned long incr = 1;
unsigned long oldIncr = 1;

void setup() { 

  pinMode(freqUpPin, INPUT_PULLUP);      // Set pins for rotary encoders as INPUTS and enable
  pinMode(freqDownPin, INPUT_PULLUP);    // internal pullup resistors.
  pinMode(stepUpPin, INPUT_PULLUP);
  pinMode(stepDownPin, INPUT_PULLUP);
  pinMode(wavePin, INPUT_PULLUP);  
  
   // Can't set SPI MODE here because the display and the AD9833 use different MODES.
  SPI.begin();
  delay(50); 
  
  // Initialize either Adafruit OR QDTech display
  //QDTech display
   //  tft.init(); 
 // tft.initR(INITR_BLACKTAB); 
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  //Adafruit display      
  // tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
 display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
 // tft.setRotation(3); 
  display.setTextWrap(false);                                  // Allow text to run off right edge

  //tft.fillScreen(BLACK);
  /*
  tft.drawFastVLine(10, tft.height()-55, 4, BLUE);         // Display 'static' cosmetic text.
  tft.drawFastVLine(30, tft.height()-55, 4, BLUE);
  tft.drawFastVLine(37, tft.height()-55, 4, BLUE);
  tft.drawFastVLine(78, tft.height()-55, 4, BLUE); 
  tft.drawFastVLine(85, tft.height()-55, 4, BLUE); 
  tft.drawFastVLine(124, tft.height()-55, 4, BLUE); 
  
  tft.drawFastHLine(10, tft.height()-52, 20, BLUE); 
  tft.drawFastHLine(37, tft.height()-52, 42, BLUE); 
  tft.drawFastHLine(85, tft.height()-52, 40, BLUE); 
 */ 
 /// tft.setTextColor(BLUE); 
  //tft.setCursor(5, tft.height()-48);
 // tft.print(" MHz    kHz     Hz");
 display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,30);
  display.println(" MHz    kHz     Hz"); 
  display.display();
 // tft.setCursor(2, tft.height() -20);
  //tft.setTextSize(1);
  //tft.drawFastHLine(0, tft.height() - 23, tft.width()-10, GREY);
 // tft.setTextColor(GREY);
  //tft.println("AD9850 10 Hz to 5 MHz ");
 // tft.print("   waveform generator");
  
  
// Configure interrupt for rotary encoder and enable.
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();
  
  AD9833reset();                                   // Reset AD9833 module after power-up.
  delay(50);
  AD9833setFrequency(freq, SINE);                  // Set the frequency and Sine Wave output
  updateDisplay();
}

void updateDisplay() {
  
  // To complicate things, the display uses SPI MODE0 but the AD9833 uses SPI MODE3 so it's
  // necessary to switch modes before each SPI transfer.
  SPI.setDataMode(SPI_MODE0);    
  
//  tft.fillRect(40, 10, 120, 12, BLACK);            // Clear text.
   display.fillRect(0,0,120,9,BLACK);
  display.setTextColor(WHITE);
  display.setCursor(20, 0);
  display.setTextSize(1); 
  switch (waveType) {
    case SINE: display.print("  sine"); break;
    case SQUARE: display.print(" square"); break;
    case TRIANGLE: display.print("triangle"); break;
   // display.setCursor(0,57);
//display.println("AD9850 10 Hz to 5 MHz ");
    display.display();
  }
  
 // tft.fillRect(5, 50, 140, 14, BLACK);           // Clear frequency numerals.
 display.fillRect(0,19,120,18,BLACK);
 // display.setTextColor(BLACK,WHITE);
 
    display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,50);
  display.println("MHz  kHz     Hz"); 
  display.setTextSize(2); 
  display.setCursor(0, 20);
display.drawLine(72,45,107,45,WHITE);  
display.drawLine(72,45,72,43,WHITE);
display.drawLine(107,45,107,43,WHITE);

display.drawLine(0,45,15,45,WHITE); 
display.drawLine(0,45,0,43,WHITE);
display.drawLine(15,45,15,43,WHITE);

display.drawLine(22,45,55,45,WHITE); 
display.drawLine(22,45,22,43,WHITE);
display.drawLine(55,45,55,43,WHITE);

//display.setTextSize(1);
//  display.setTextColor(WHITE);
  //display.setCursor(0,57);
//display.println("AD9850 10 Hz to 5 MHz ");
  display.display();
  format(freq);  // Show frequency in formatted form.  
  
  
display.display();
}

void format(unsigned long value) {
   
   // Break the frequency value down into individual digits & into variable 'digit'.
   // If a digit corresponds with the currently-selected x10 increment, change the 
   // text colour to YELLOW. All other digits and commas are GREEN. 
   
  unsigned long j = 1000000;
  
  for (int i=6; i>=0; i--) {
    int digit = (value / j) % 10;
    incr == j ? display.setTextColor(BLACK,WHITE): display.setTextColor(WHITE);
    display.print(digit);
    if ((i == 6) || (i == 3)) {                  // Add commas at millions and thousands
      display.setTextColor(WHITE); 
      display.print(",");
    }   
    j /= 10;
  }
} 


void loop() {
  
  if (oldIncr != incr) {
    updateDisplay();
    oldIncr= incr;
  }
  
  // Check 'increment' rotary encoder. Increase or decrease 'increment' by a factor of x10
  // if encoder has been turned.
  unsigned char result = i.process();
  if (result) {
    if (result == DIR_CW)  {if (incr < 1000000) incr *= 10;}
    if (result == DIR_CCW) {if (incr >= 10) incr /= 10;}
    updateDisplay();
  }
  
  // Check if push button on 'increment' rotary encoder is pushed and set Wave Type accordingly.
  if (digitalRead(wavePin) == LOW) {
    wave += 1;
    if (wave > 2) wave = 0;
    switch (wave) {
      case 0: waveType = SINE; break;
      case 1: waveType = SQUARE; break;
      case 2: waveType= TRIANGLE; break;
    }    
    AD9833setFrequency(freq, waveType);     // Set AD9833 to frequency and selected wave type.
    updateDisplay();
    delay(200);
  }
  
  if (freq != freqOld) {                    // If frequency has changed, interrupt rotary encoder
    AD9833setFrequency(freq, waveType);     // must have been turned so update AD9833 and display.
    updateDisplay();
    freqOld = freq;                         // Remember new frequency to avoid unwanted display 
  }                                         // and AD9833 updates.
}

// AD9833 documentation advises a 'Reset' on first applying power.
void AD9833reset() {
  WriteRegister(0x100);   // Write '1' to AD9833 Control register bit D8.
  delay(10);
}

// Set the frequency and waveform registers in the AD9833.
void AD9833setFrequency(long frequency, int Waveform) {

  long FreqWord = (frequency * pow(2, 28)) / refFreq;

  int MSB = (int)((FreqWord & 0xFFFC000) >> 14);    //Only lower 14 bits are used for data
  int LSB = (int)(FreqWord & 0x3FFF);
  
  //Set control bits 15 ande 14 to 0 and 1, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000; 
  
  WriteRegister(0x2100);   
  WriteRegister(LSB);                  // Write lower 16 bits to AD9833 registers
  WriteRegister(MSB);                  // Write upper 16 bits to AD9833 registers.
  WriteRegister(0xC000);               // Phase register
  WriteRegister(Waveform);             // Exit & Reset to SINE, SQUARE or TRIANGLE

}

void WriteRegister(int dat) { 
  
  // Display and AD9833 use different SPI MODES so it has to be set for the AD9833 here.
  SPI.setDataMode(SPI_MODE2);       
  
  digitalWrite(FSYNC, LOW);           // Set FSYNC low before writing to AD9833 registers
  delayMicroseconds(10);              // Give AD9833 time to get ready to receive data.
  
  SPI.transfer(highByte(dat));        // Each AD9833 register is 32 bits wide and each 16
  SPI.transfer(lowByte(dat));         // bits has to be transferred as 2 x 8-bit bytes.

  digitalWrite(FSYNC, HIGH);          //Write done. Set FSYNC high
}


// Interrupt service routine for the 'frequency' rotary encoder.

ISR(PCINT2_vect) {

  unsigned char result = r.process();
  if (result) {
    if (result == DIR_CW) {                   // Clockwise rotation so add increment to frequency
       if ((freq + incr) < 6000000) freq+=incr;
       
    } else {
        if (freq > incr) {                    // Counter-clockwise rotation so subtract increment
          freq -= incr;                       // from frequency unless it would result in a negative
        } else {                              // number.
          if (freq >= 1) incr /= 10;
          if (incr < 1) incr = 1;             // Compensate for math rounding error.
        }  
    }
  }
}


