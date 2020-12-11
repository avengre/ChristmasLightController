#include <Arduino.h>

#define USE_NEOPIXEL 1      // 1 == NEOPIXEL, 0 == FastLED

#define DEBUG_LCD 0         // Using SSD1306 for debugging, may cause problem with interupts

#if USE_NEOPIXEL == 1
  #include "Adafruit_NeoPixel.h"
#else
  #include "FastLED.h"
#endif

#if DEBUG_LCD == 1
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  #define LCD_WIDTH 128
  #define LCD_HEIGHT 32
  #define OLED_RESET -1
  Adafruit_SSD1306 display(LCD_WIDTH, LCD_HEIGHT, &Wire, OLED_RESET);
#endif

#define NUM_LED_STRIPS 1   // Coded for up to 2
#define PIXEL_GROUPING 2 // 1:1 pixel grouping, 2 = 1:2 (1 pixel command is grouped to the next #n pixels), useful if having trouble with USB speeds or you want to simplify the display

#define LED1_DATA_PIN 22
#define LED2_DATA_PIN 24

#define LED_DATA_BUFFERED 1   // If buffered, all LED data is loaded before it is sent to the library
#define MAX_POSSIBLE_LED 800  // Based on memory of controller (24 bits per LED, )

#define NUM_RELAYS 16   // Using Saintsmart 8 or 16 bank
#define RELAY_START_OFFSET 2 // Assuming your relays are on sequential pins, what pin does it start with, in my case Relay1 is on pin 2

#define PERFORM_LIGHT_TESTS 1  // Do we want to perform a quick light test on power-on

#define UART_BAUD_RATE 115200 // Controller rate

class LEDController
{
public:

  LEDController(unsigned char inPinNum)
  {
    bStringDirty = true;  // assume its dirty for first round, needs update
    PinNum = inPinNum;
    #if USE_NEOPIXEL
      LEDStrip = NULL;        
      numActivePixels = 0;
    #else
      for (int i = 0; i < MAX_POSSIBLE_LED; i++)
      { 
        ledArray[i] = 0;       // Clear out pixel buffer for FastLED
        numActivePixels = 0; 
      };
    #endif

  };
  void Initiate(unsigned short inLEDNum)
  {
      numActivePixels = inLEDNum;
    
      // Neopixel is instanced
      #if USE_NEOPIXEL
        LEDStrip = new Adafruit_NeoPixel(inLEDNum, PinNum, NEO_RGB + NEO_KHZ800);
        LEDStrip->begin();
      #else
        FastLED.addLeds<WS2811,22,RGB>(ledArray,inLEDNum);

      #endif 

  };

  // Initiate a light test for this LED Strip
  void LightTest()
  {
    // Turn them all off
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,0,0,0);};
    UpdateDisplay();

    // set all to red
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,255,0,0);};
    UpdateDisplay();
    delay(500); 

    // set all to red, for some reason fastLED needs a double show the first time?
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,255,0,0);};
    UpdateDisplay();
    delay(500); 

    // set all to green
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,0,255,0);};
    UpdateDisplay();
    delay(500);

    // set all to blue
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,0,0,255);};
    UpdateDisplay();
    delay(500);

    // Turn them all off
    for (int i = 0; i<numActivePixels; i++){SetPixelColor(i,0,0,0);};
    UpdateDisplay();
    
  };

  unsigned short getNumLEDs()
  {
    return numActivePixels;
  }
  void SetPixelColor(unsigned short inPixelNum, unsigned char inR, unsigned char inG, unsigned char inB)
  {
    #if USE_NEOPIXEL
      uint32_t pixelColor = LEDStrip->getPixelColor(inPixelNum);
      

      unsigned char r = (pixelColor >> 16) & 0xFF;
      unsigned char g = (pixelColor >> 8 ) & 0xFF;
      unsigned char b = pixelColor & 0xFF;

      // New color, dirty string
      if ((inR != r) || (inG != g) || (inB != b))
      {
        LEDStrip->setPixelColor(inPixelNum,inR,inG,inB);
        bStringDirty = true;
      }
      
      
    #else

     if ((ledArray[inPixelNum].r != inR) || (ledArray[inPixelNum].g != inG) || (ledArray[inPixelNum].b != inB))
     {
      bStringDirty = true;  // Needs update next round
     }
      ledArray[inPixelNum].r = inR;
      ledArray[inPixelNum].g = inG;
      ledArray[inPixelNum].b = inB;
    #endif
  };

  
  // Check if our LED Length has changed
  // This may happen if we are using headers to determine the length of the strip
  // FastLED: Uses constants for pins, thus the ugly Switch statement
  // @return: Returns True if length was different, false if same
  ///
  bool CheckLengthForUpdate(unsigned short inNumPixels)  
  {
      if ((inNumPixels * PIXEL_GROUPING) != numActivePixels)
      {
        numActivePixels = inNumPixels *PIXEL_GROUPING;   // Account for increased actual LEDs based on pixel grouping
        #if USE_NEOPIXEL
            // Neopixel needs to be recreated when length changes
            delete LEDStrip;
            LEDStrip = new Adafruit_NeoPixel(numActivePixels, PinNum, NEO_GRB + NEO_KHZ800);
        #else

            switch (inNumPixels)
            {
              case 1:
                FastLED.addLeds<WS2811,1,RGB>(ledArray,numActivePixels);
                break;
              case 2:
                FastLED.addLeds<WS2811,2,RGB>(ledArray,numActivePixels);
                break;
              case 3:
                FastLED.addLeds<WS2811,3,RGB>(ledArray,numActivePixels);
                break;
              case 4:
                FastLED.addLeds<WS2811,4,RGB>(ledArray,numActivePixels);
                break;
              case 5:
                FastLED.addLeds<WS2811,5,RGB>(ledArray,numActivePixels);
                break;
              case 6:
                FastLED.addLeds<WS2811,6,RGB>(ledArray,numActivePixels);
                break;
              case 7:
                FastLED.addLeds<WS2811,7,RGB>(ledArray,numActivePixels);
                break;
              case 8:
                FastLED.addLeds<WS2811,8,RGB>(ledArray,numActivePixels);
                break;
              case 9:
                FastLED.addLeds<WS2811,9,RGB>(ledArray,numActivePixels);
                break;
              case 10:
                FastLED.addLeds<WS2811,10,RGB>(ledArray,numActivePixels);
                break;
              case 11:
                FastLED.addLeds<WS2811,11,RGB>(ledArray,numActivePixels);
                break;
              case 12:
                FastLED.addLeds<WS2811,12,RGB>(ledArray,numActivePixels);
                break;
              case 13:
                FastLED.addLeds<WS2811,13,RGB>(ledArray,numActivePixels);
                break;
              case 14:
                FastLED.addLeds<WS2811,14,RGB>(ledArray,numActivePixels);
                break;
              case 15:
                FastLED.addLeds<WS2811,15,RGB>(ledArray,numActivePixels);
                break;
              case 16:
                FastLED.addLeds<WS2811,16,RGB>(ledArray,numActivePixels);
                break;
              case 17:
                FastLED.addLeds<WS2811,17,RGB>(ledArray,numActivePixels);
                break;
              case 18:
                FastLED.addLeds<WS2811,18,RGB>(ledArray,numActivePixels);
                break;
              case 19:
                FastLED.addLeds<WS2811,19,RGB>(ledArray,numActivePixels);
                break;
              case 20:
                FastLED.addLeds<WS2811,20,RGB>(ledArray,numActivePixels);
                break;
              case 21:
                FastLED.addLeds<WS2811,21,RGB>(ledArray,numActivePixels);
                break;
              case 22:
                FastLED.addLeds<WS2811,22,RGB>(ledArray,numActivePixels);
                break;
              case 23:
                FastLED.addLeds<WS2811,23,RGB>(ledArray,numActivePixels);
                break;
              case 24:
                FastLED.addLeds<WS2811,24,RGB>(ledArray,numActivePixels);
                break;
              case 25:
                FastLED.addLeds<WS2811,25,RGB>(ledArray,numActivePixels);
                break;
              case 26:
                FastLED.addLeds<WS2811,26,RGB>(ledArray,numActivePixels);
                break;
              case 27:
                FastLED.addLeds<WS2811,27,RGB>(ledArray,numActivePixels);
                break;                                                            
              case 28:
                FastLED.addLeds<WS2811,28,RGB>(ledArray,numActivePixels);
                break;               
            };
            
        #endif

        return true;
      }
      else
      {
        return false; 
      };
  };

  // Updates the light display
  // returns true if it was ACTUALLY updated, returns false if no update was performed
  bool UpdateDisplay()
  {
    // Only update the display if the string is dirty, should help from getting bogged down and increase apparent FPS
    if (bStringDirty)
    {
      #if USE_NEOPIXEL
        LEDStrip->show();
      #else
        FastLED.show();
      #endif

      bStringDirty = false;
      return true;
    }
    return false;
  };

private:
bool bStringDirty;

#if USE_NEOPIXEL
Adafruit_NeoPixel *LEDStrip;
unsigned short ledArray[MAX_POSSIBLE_LED];
#else
CRGB ledArray[MAX_POSSIBLE_LED];
#endif

unsigned short numActivePixels;
unsigned char PinNum;
};

#if NUM_RELAYS > 0
class RelayController
{
public:
  RelayController()
  {
    for (int i = 0; i < NUM_RELAYS; i++)
    {
        Relays[i] = 0;
    };
  }
  void Initiate(){
    for (int i = 0; i < NUM_RELAYS; i++)
    {
       pinMode(i+RELAY_START_OFFSET, OUTPUT);
    }
  };

  void Test()
  {
  for (int i=0; i< NUM_RELAYS; i++)
    {
      digitalWrite(i+RELAY_START_OFFSET, HIGH);
      delay(250);
      digitalWrite(i+RELAY_START_OFFSET, LOW);
    }
  };

  // By relay number, not pin number
  void SetRelayOn(unsigned char inRelay)
  {
      Relays[inRelay + RELAY_START_OFFSET] = true;    // add 2 since our relays start at pin 2+
      digitalWrite(inRelay + RELAY_START_OFFSET, HIGH);
  };

  void SetRelayOff(unsigned char inRelay)
  {
      Relays[inRelay + RELAY_START_OFFSET] = false;
      digitalWrite(inRelay + RELAY_START_OFFSET, LOW);
  };

  void SetRelayBatch(bool Relays[] )
  {
    for (int i = 0; i < NUM_RELAYS; i++)
    {
      if (Relays[i] == true)
        digitalWrite(i+RELAY_START_OFFSET, HIGH);
      else 
        digitalWrite(i+RELAY_START_OFFSET,LOW);
    };
  };
private:
  bool Relays[NUM_RELAYS];  // Each of the relays

};

#endif //NUM_RELAYS

void setup() {
  Serial.begin(UART_BAUD_RATE);

  #if DEBUG_LCD == 1
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));};
    display.display();  // Show default splash screen, basically to show wiring is hooked up correctly
    delay(200);
    display.clearDisplay(); // Then Clear
    display.display();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.cp437(true);
  #endif

  #if NUM_LED_STRIPS > 0
    LEDController LEDStrip1(LED1_DATA_PIN);
    LEDStrip1.Initiate(500);
      #if PERFORM_LIGHT_TESTS == 1
      LEDStrip1.LightTest();
      #endif
  
    #if NUM_LED_STRIPS == 2
    LEDController LEDStrip2(LED2_DATA_PIN);
    LEDStrip2.Initiate(100);
      #if PERFORM_LIGHT_TESTS == 1
      LEDStrip2.LightTest();
      #endif
    #endif
  #endif // Num LED STrips

  #if NUM_RELAYS > 0
  RelayController RelayManager;
  RelayManager.Initiate();
  

  #if PERFORM_LIGHT_TESTS == 1
  RelayManager.Test();
  #endif 
  #endif 

  unsigned int d1, d2, d3;
  unsigned char rgb[2];
  unsigned int num_leds = 0;

  // Second string of leds?
  #if NUM_LED_STRIPS == 2
  unsigned int e1,e2,e3;
  unsigned int num_leds2 = 0;
  #endif

  int cnt = 0;
  // Loop indefinately
  while(true)
  {
      cnt = 0;
      d1 = 0;
      d2 = 0;
      d3 = 0;    
      
      #if NUM_LED_STRIPS == 2
      e1 = 0;
      e2 = 0;
      e3 = 0;
      #endif
      
      int processedLEDs = 0;
    
    #if DEBUG_LCD
      display.clearDisplay();
      display.setCursor(0,0);
      
    #endif
     
    #if NUM_LED_STRIPS > 0
    
      //Begin waiting for the header to be received on the serial bus
      //1st character
      while(!Serial.available()){};
      if(Serial.read() != '>') {
        continue;
        }
      //second character
      while(!Serial.available()){};
      if(Serial.read() != '>') {
        continue;
        }
      //get the first digit from the serial bus for the number of pixels to be used
      while(!Serial.available()){};
      d1 = Serial.read();
      //get the second digit from the serial bus for the number of pixels to be used
      while(!Serial.available()){};
      d2 = Serial.read();
      //get the third digit from the serial bus for the number of pixels to be used
      while(!Serial.available()){};
      d3 = Serial.read();
      //get the end of the header
  
      #if NUM_LED_STRIPS == 2
        // If we are using multiple strips look for sub-header
        while (!Serial.available()){};
        if(Serial.read()!= '/') {
          continue;
          }
        while (!Serial.available()){};
        if(Serial.read()!= '/') {
          continue;
          }
        //get the first digit from the serial bus for the number of pixels to be used
        while(!Serial.available()){};
        e1 = Serial.read();
        //get the second digit from the serial bus for the number of pixels to be used
        while(!Serial.available()){};
        e2 = Serial.read();
        //get the third digit from the serial bus for the number of pixels to be used
        while(!Serial.available()){};
        e3 = Serial.read();            
    
    
      #endif
    
        while(!Serial.available()){};
        if(Serial.read() != '<') {
          continue;
          }
        while(!Serial.available()){};
        if(Serial.read() != '<') {
          continue;
          }
        // calculate the number of pixels based on the characters provided in the header digits
        num_leds = (d1-'0')*100+(d2-'0')*10+(d3-'0');
    
        #if NUM_LED_STRIPS == 2
          num_leds2 = (e1-'0')*100+(e2-'0')*10+(e3-'0');
        #endif
    
          #if DEBUG_LCD == 1
            display.setCursor(0,0);
            display.print("Num LED: ");
            display.print(num_leds);
          #endif
    
        // ensure the number of pixels does not exceed the number allowed
        ///if(num_leds > MAX_POSSIBLE_LED) {
        ///  continue;
        ///  }
        // Let the FastLED library know how many pixels we will be addressing
        
        // Loop through each of the pixels and read the values for each color
        while (num_leds > 0) {
          while(!Serial.available()){};
            rgb[0] = Serial.read();
          while(!Serial.available()){};
            rgb[1] = Serial.read();
          while(!Serial.available()){};
            rgb[2] = Serial.read();
  
            processedLEDs++;

            // If we are intentionally pixel grouping, apply to the next pixel(s) as well
            for (unsigned char loop = 0; loop < PIXEL_GROUPING; loop++)
            {
              LEDStrip1.SetPixelColor(cnt++,rgb[0],rgb[1],rgb[2]);
            }
            
            num_leds--;
          } 
  
          #if DEBUG_LCD == 1
          display.setCursor(0,20);
          display.print("Prc ");
          display.print(processedLEDs);
        #endif 
  
      // Extract second LED strip leds
       #if NUM_LED_STRIPS == 2
        if(num_leds2 + num_leds > MAX_POSSIBLE_LED) {
        continue;
        }
     
       cnt = 0;  // Reset LED counter for 2nd strip
       while (num_leds2 > 0) {
        while(!Serial.available()){};
          rgb[0] = Serial.read();
        while(!Serial.available()){};
          rgb[1] = Serial.read();
        while(!Serial.available()){};
          rgb[2] = Serial.read();
  
         for (unsigned char loop = 0; loop < PIXEL_GROUPING; loop++)
          {
            LEDStrip2.SetPixelColor(cnt+loop,rgb[0],rgb[1],rgb[2]);
          }
          cnt++;
          num_leds2--;
        } 
        
       #endif //NUM_LED_STRIPS == 2
      
      // Tell the FastLED Library it is time to update the strip of pixels
      LEDStrip1.UpdateDisplay();
  
      #if NUM_LED_STRIPS == 2
      LEDStrip2.UpdateDisplay();
      #endif
      // WOO HOO... We are all done and are ready to start over again!
    #endif // LED Strips > 0
    // Check relay data, should be next 16 digits

    // create a buffer and batch set relays

    #if NUM_RELAYS > 0
      bool buf[NUM_RELAYS-1];

      #if DEBUG_LCD == 1
      display.setCursor(0,10);
      #endif
      
      while(!Serial.available()){};
      for (int i = 0; i < NUM_RELAYS; i++)
      {
          while(!Serial.available()){};
          if (Serial.read() < 255)
          {
            buf[i] = 0;
            //RelayManager.SetRelayOff(i);

            #if DEBUG_LCD == 1
            display.print("0");
            #endif
          }
          else
          {
            buf[i] = 1;
            //RelayManager.SetRelayOn(i);
            #if DEBUG_LCD == 1
            display.print("1");
            #endif
          }
          
        
      };
      RelayManager.SetRelayBatch(buf);
      #endif // NUM_RELAYS

    #if DEBUG_LCD == 1
    display.display();
    #endif
  }; // end while loop


}

// This should never be entered as we don't leave Setup()
void loop() {}