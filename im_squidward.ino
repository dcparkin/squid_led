#include <Adafruit_NeoPixel.h>

#define PIN           11
#define NO_OF_PIXELS  72
#define V_FACTOR      0.00489

//int  pir_pin = A3;
const int pir_ip = 2;
int  PHOTO_PIN = A6;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NO_OF_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t squid_green = strip.Color(0, 255, 50);
uint32_t rgb_shift = 255;

uint8_t r_shift = 16;
uint8_t g_shift = 8;


enum state {
  st_off,
  st_squid,
  st_squid_eyes,
  st_rainbow,
  st_rainbow_cycle,
  st_colour_wipe,
  st_disco,
  st_chase_slow,
  st_chase_mid,
  st_chase_fast
};

state system_state = st_squid_eyes;
int pir_state = 0;
int last_pir_state = 0;
bool trigger = false;
bool break_from_state = false;

void pir_trigger() {
  
  if (system_state == st_chase_fast) {
    system_state = 0;
  }
  else {
    system_state = system_state + 1;
  }
  break_from_state = true;
  Serial.print("trigger\n\r");
}
  

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pir_ip, INPUT_PULLUP);
  noInterrupts();
  //attachInterrupt(pir_ip, pir_trigger, RISING);
  delay(100);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  Serial.begin(9600);
  Serial.print("hello\n\r");
  block(squid_green, false);
  for (int l=0; l<18000; l++){
    delay(5);
  }
  attachInterrupt(digitalPinToInterrupt(pir_ip), pir_trigger, RISING);
  interrupts();
}

void loop() {
 
//  pir_state = digitalRead(pir_ip);
//  Serial.println(pir_state);

  switch (system_state) {
    case st_off:
      blank();
      break;

    case st_squid:
      block(squid_green, false);
      break;
    
    case st_squid_eyes:
      block(squid_green,true);      
      break;

    case st_rainbow:
      rainbow(20);
      break;

    case st_rainbow_cycle:
      rainbowCycle(20) ; 
      break;

    case st_colour_wipe:
      colorWipe(squid_green, 20);
      for (int i=0; i<250; i++)
      {
        delay(1);
        if (break_from_state){ break;}
      }
      blank();
      colorWipeRev(squid_green, 20);
      blank();
      break;

    case st_disco:
      disco();
      break;

    case st_chase_slow:
      chase(squid_green, 20);
      break;

    case st_chase_mid:
      chase(squid_green, 10);
      break;

    case st_chase_fast:
      chase(squid_green, 5);
      break;
  }
}

//sets all pixels off
void blank()
{
  for(uint16_t i=0; i<strip.numPixels(); i++){
    if (break_from_state){
      break;
    }
    strip.setPixelColor(i, 0, 0, 0);
  }
  delay(20);
  strip.show();
  //break_from_state = false;
}


void block(uint32_t c, bool flash_eyes){
  for(uint16_t i=0; i<strip.numPixels()-2; i++){
    if (break_from_state){break;}
    strip.setPixelColor(i, c);
  }
  if (flash_eyes){
    strip.setPixelColor(strip.numPixels()-1, rgb_shift);
    strip.setPixelColor(strip.numPixels()-2, rgb_shift);
    if (rgb_shift == 0xFF000000){
      rgb_shift = 255;
    }
    else{
      rgb_shift = rgb_shift << 8;
    }
  }
  else{
    strip.setPixelColor(strip.numPixels()-1, c);
    strip.setPixelColor(strip.numPixels()-2, c);
  }
  delay(100);
  strip.show();
  break_from_state = false;
}


void disco(){
  for(uint16_t i=0; i<strip.numPixels(); i++){
    if (break_from_state){break;}
    uint8_t r = uint8_t(random(256));
    uint8_t g = uint8_t(random(256));
    uint8_t b = uint8_t(random(256));
    strip.setPixelColor(i, r, g, b);
  }
  delay(20);
  strip.show();
  break_from_state = false;
}

//starts from the given colour in the middle and 
//spreads out to the edges symetrically 
void middleOut(uint32_t c, uint8_t wait){
 // blank();
 // strip.show();
  
  bool odd = strip.numPixels()%2;
  uint16_t first_pixel = strip.numPixels()/2;
  if(odd){
    first_pixel+=1;
    strip.setPixelColor(first_pixel, c);
  }
  else{
    strip.setPixelColor(first_pixel, c);
    strip.setPixelColor(first_pixel+1, c);
  }
  strip.show();
  delay(wait);
  for(uint16_t i=1; i<=strip.numPixels()/2; i++){
    if (break_from_state){
      break;
    }
     if(odd){
      strip.setPixelColor(first_pixel-i, c);
      strip.setPixelColor(first_pixel+i, c);
     }
     else{
      strip.setPixelColor(first_pixel-i, c);
      strip.setPixelColor(first_pixel+i+1, c);
     }
     strip.show();
     delay(wait);
  }
  break_from_state = false;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      if (break_from_state){
        break;
      }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
  //break_from_state = false;
}

// Fill the dots one after the other with a color
void colorWipeRev(uint32_t c, uint8_t wait) {
  for(int16_t i=strip.numPixels()-1; i>=0; i--) {
      if (break_from_state){
        break;
      }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
  break_from_state = false;
}


void fade(uint8_t wait){
  //strip.begin();
  //strip.show();
  //loop through white gradient
  for (uint16_t f=0; f<=255; f++){
    if (break_from_state){
      break;
    }
    //set each pixel to that gradient
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      if (break_from_state){
        break;
      }
      strip.setPixelColor(i, strip.Color(f, f, f));
    }
    delay(wait); 
    strip.show();
    break_from_state = false;   
  }
}

void gradient(uint32_t start_col, uint32_t end_col, uint8_t wait){
  //Serial.print("start\n\r");
  uint8_t r_start_col = start_col >> r_shift;
  uint8_t r_end_col = end_col >> r_shift;
  uint8_t g_start_col = start_col >> g_shift;
  uint8_t g_end_col = end_col >> g_shift;
  uint8_t b_start_col = start_col;
  uint8_t b_end_col = end_col;
  
  for(uint8_t start=0; start<strip.numPixels(); start++){
     if (break_from_state){
        break;
      }
      strip.setPixelColor(start, strip.Color(r_start_col, g_start_col, b_start_col));
    }
    strip.show();
    delay(wait);
  
  bool colour_mismatch = true;
  while(colour_mismatch){
    
    if(r_start_col > r_end_col){
      r_start_col--;
    }
    else if(r_start_col < r_end_col){
      r_start_col++;
    }
    
    if(g_start_col > g_end_col){
      g_start_col--;
    }
    else if(g_start_col < g_end_col){
      g_start_col++;
    }
    
    if(b_start_col > b_end_col){
      b_start_col--;
    }
    if(b_start_col < b_end_col){
      b_start_col++;
    }

    if(r_start_col == r_end_col && g_start_col == g_end_col
        && b_start_col == b_end_col){
        colour_mismatch = false;
        }
        
    for(uint8_t i=0; i<strip.numPixels(); i++){
      if (break_from_state){
        break;
      }
      strip.setPixelColor(i, strip.Color(r_start_col, g_start_col, b_start_col));
    }
    strip.show();
    delay(wait);
    break_from_state = false;
  }
}

void chase(uint32_t col, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    if (break_from_state){break;}
    if(i > 0){
      strip.setPixelColor(i-1, strip.Color(0, 0, 0));
    }
    strip.setPixelColor(i, col);
    delay(wait);
    strip.show(); 
  }
  for(int16_t i2=strip.numPixels(); i2>=0; i2--){
    if (break_from_state){break;}
    if(i2 < strip.numPixels()){
      strip.setPixelColor(i2+1, strip.Color(0, 0, 0)); 
    }
    strip.setPixelColor(i2, col);
    delay(wait);
    strip.show();
  }
  break_from_state = false;
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    if (break_from_state){break;}
    for(i=0; i<strip.numPixels(); i++) {
      if (break_from_state){break;}
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
  break_from_state = false;
} 

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    if (break_from_state){break;}
    for(i=0; i< strip.numPixels(); i++) {
      if (break_from_state){break;}
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
  break_from_state = false;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
