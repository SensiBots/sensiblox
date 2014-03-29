

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code is for the SensiBloxTM developed by Dynepic, LLC 
//28 March 2014
//
//
//This code is distributed for personal and educational use only.  No commercial use is authorized.
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Arduino Pro Mini (5V, 16MHZ) w/ ATMEGA 328
int screen = 4; // Select screen to be shown: 0 - nothing, 1 - light, 2 - temp, 3 - sound, 4 - accel, 5 - all

#include <avr/pgmspace.h>  // Library with functions to access flash memory
#include <Adafruit_GFX.h> // Adafruit graphics library - must download from adafruit.com and put in arduino/libraries directory
#include <Adafruit_PCD8544.h>  // Adafruit library for 5110 LCD & PCD8544 controller - must download from adafruit.com and put in arduino/libraries directory

// Define Arduino pins
const int  LIGHT_PIN = A0;
const int TEMP_PIN = A1;
const int SOUND_PIN = A3;
const int ACCELX_PIN = A4;
const int ACCELY_PIN = A5;
const int ACCELZ_PIN = A6;
const int BUZZER_PIN = 3;
const int DISPLAY_LED_PIN = 4;
const int BUTTON_PIN = 5;
const int LED1_PIN = 6;
const int LED2_PIN = 7;

boolean GRAVITY_enabled = false;
boolean TEMP_enabled = false;
boolean LIGHT_enabled = false;
boolean SOUND_enabled = false;

// Define pins for 5110 library
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 12, 11);
// pin 8 - Serial clock out (SCLK)
// pin 9 - Serial data out (DIN)
// pin 10 - Data/Command select (D/C)
// pin 12 - LCD chip select (CS)
// pin 11 - LCD reset (RST)
int lcd_refresh_int = 333; // Set interval between screen refreshes
unsigned long lcd_timestamp=0;

// Define variables for BLE Mini
uint16_t value;

//Define variables for light sensor
int lightLevel, high = 0, low = 1023;
int lightLevelnew;
float lux;
int lux_int; 
int i;
int j;
int duration;
int tempLevel;
int soundLevel;
unsigned long light_timestamp=0;
int light_sample_int=100;

//Define variables for SPL meter
int spl_v = 0;     
int spl = 0;       
int average = 60;       
int avg_count = 1;
unsigned long sound_timestamp=0;
int sound_sample_int=10;

//Define variables for temperature sensor
unsigned long temp_timestamp=0;
int temp_sample_int=100;

//Define variables for accelerometer
unsigned long accel_timestamp=0;
int accel_sample_int=100;
int gx1;
int gy1;
int gz1;
float gx;
float gy;
float gz;
float g;

void setup()
{
  // Set up output pins, set them all low
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED2_PIN, LOW);
  pinMode(DISPLAY_LED_PIN, OUTPUT);
  digitalWrite(DISPLAY_LED_PIN, LOW);

  // Set up input pins
  pinMode(LIGHT_PIN, INPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  // Start 5110 LCD
  display.begin();
  display.setContrast(60);  // sets screen contrast
  display.clearDisplay();   // clears the screen and buffer

  Serial.begin(57600);  //Begin serial comm with BLE Mini via pin 0 and 1.
}

void loop()
{

  /*** Read sensors ***/
  read_button();
  read_light();
  read_temp();
  read_sound();
  read_accel();

  /*** Receive data ***/
  while (Serial.available())
  {    
    // Read serial data
    byte data0 = Serial.read();
    byte data1 = Serial.read();
    byte data2 = Serial.read();

    if (data0 == 0x01)  // Command is to control Display Backlight pin
    {
      if (data1 == 0x01)
        digitalWrite(DISPLAY_LED_PIN, HIGH);
      else
        digitalWrite(DISPLAY_LED_PIN, LOW);
    }
    else if (data0 == 0x02) // Command is to control PWM Buzzer pin
    {
      buzzer_beep(); // Plays some beeping noises
    }
    else if (data0 == 0x03)  // Command is to control LED
    {
      if (data1 == 0x01)
      {
        digitalWrite(LED1_PIN, HIGH);
        digitalWrite(LED2_PIN, HIGH);
      }
      else
      {
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
      }
    }
    else if (data0 == 0x05)  // Command is to play "The Star-Spangled Banner"
    {
      play_banner();
    }
    else if (data0 == 0x06)  // Command is to flash lights
    {
      if (data1 == 0x01)
      {
        int index;
        int delayTime = 400; // milliseconds to pause between LED flashes
        for(index = 0; index <= 5; index++)        //flash 10x
        {
          digitalWrite(LED1_PIN, HIGH); 
          digitalWrite(LED2_PIN, HIGH); // turn LED on
          delay(delayTime);            // pause to slow down
          digitalWrite(LED1_PIN, LOW);// turn LED off
          digitalWrite(LED2_PIN, LOW);  
          delay(delayTime/2); 
        }
      }                                                
    }
    else if (data0 == 0xA0) // Command is to enable TEMP Analog
    {
      if (data1 == 0x01)
      { 
        TEMP_enabled = true;
        screen=2;
      }
      else
        TEMP_enabled = false;
    }
    else if (data0 == 0xA1) // Command is to enable Light Analog
    {
      if (data1 == 0x01)
      {
        LIGHT_enabled = true;
        screen=1;
      }
      else
        LIGHT_enabled = false;
    }
    else if (data0 == 0xA2) // Command is to enable Sound Analog
    {
      if (data1 == 0x01)
      {
        SOUND_enabled = true;
        screen=3;
      }
      else
        SOUND_enabled = false;
    }
    else if (data0 == 0xA3) // Command is to enable Accelerometer Analog
    {
      if (data1 == 0x01)
      {
        GRAVITY_enabled = true;
        screen=4;
      }
      else
        GRAVITY_enabled = false;
    }
  }

  /*** Transmit data ***/


  if (GRAVITY_enabled == true)
  {
    value = gx1; 
    Serial.write((byte) 0x0E);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
    value = gy1; 
    Serial.write((byte) 0x0F);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
    value = gz1; 
    Serial.write((byte) 0x10);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
  }
  if (TEMP_enabled == true)
  {
    value = tempLevel; 
    Serial.write((byte) 0x0B);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
  }
  if (LIGHT_enabled == true)
  {
    value = lux; 
    Serial.write((byte) 0x0C);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
  }
  if (SOUND_enabled == true)
  {
    value = spl;
    Serial.write((byte) 0x0D);
    Serial.write((byte) (value >> 8));
    Serial.write((byte) (value & 0x00FF));
  }

  /*** Display data on 5110 LCD ***/
  // 0 - nothing, 1 - light, 2 - temp, 3 - sound, 4 - accel, 5 - all
  if (millis() - lcd_timestamp >= lcd_refresh_int) // Implement screen refresh rate 
  {
    lcd_timestamp=millis();

    if (screen==1)
    {
      display_light();
    }
    else if (screen==2)
    {
      display_temp();
    }
    else if (screen==3)
    {
      display_sound();
    }
    else if (screen==4)
    {
      display_accel();
    }
    else if (screen==5)
    {
      display_all();
    }
    else
    {
      display_logo();
    }
  }

} // end void() loop

void read_button() // Function that checks if button is being pressed
{
  static boolean buttonState = false;
  static boolean gone_low = true;
  int button_debounce=250; // Set minimum time between button presses (debounce time)
  static unsigned long button_timestamp=0;

  if (digitalRead(BUTTON_PIN)==HIGH && gone_low==true && millis() - button_timestamp >= button_debounce)
  {
    button_timestamp=millis();
    buttonState=!buttonState;
    digitalWrite(DISPLAY_LED_PIN, buttonState); // Toggle the LCD backlight when the button is pressed
    //    screen+=1;
    //    if (screen>=4)
    //    {
    //      screen=0;
    //    }
    gone_low=false;
  }
  if (digitalRead(BUTTON_PIN)==LOW)  // Prevents flashing when button is held
  {
    gone_low=true;
  }
}
void read_light() // Analog photoresistor read function
{
  if (millis() - light_timestamp >= light_sample_int)
  {
    light_timestamp=millis();
    lightLevel = analogRead(LIGHT_PIN);  // 0 (0 V) to 1023 (5 V)
    lux=lightLevel;      
    // make negative values 0
    if (lux<0)
    {
      lux=0;
    }
    if (lux>0 && lux<900)
    {
      lux=.7247*pow(2.71828,.0071*lux);
    }
    if (lux >= 900)
    {
      lux=.000000007*pow(2.71828,.0278*lux);
    }
    lux = floor(lux + 0.5);  // Round to nearest integer
    lux_int = (int) lux; // Convert to integer
  }
}
void display_light() // Display brightness data on 5110 LCD
{
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(15,0);
  display.println("Currently");
  display.setCursor(20,10);
  display.println("reading:");
  display.setCursor(25,22);
  display.setTextSize(2);
  display.println(lux_int);
  display.setTextSize(1);
  display.setCursor(29,40);
  display.print("lux");
  display.display();
}
void read_temp() // Analog temperature sensor read function
{
  if (millis() - temp_timestamp >= temp_sample_int)
  {
    temp_timestamp=millis();
    tempLevel = analogRead(TEMP_PIN)*.488759;
  }

}
void display_temp() // Display temperature data on 5110 LCD
{
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(15,0);
  display.println("Currently");
  display.setCursor(20,10);
  display.println("reading:");
  display.setCursor(25,22);
  display.setTextSize(2);
  display.println(tempLevel);
  display.setTextSize(1);
  display.setCursor(29,40);
  display.print("deg F");
  display.display();
}
void read_sound() // SPL meter function
{
  if (millis() - sound_timestamp >= sound_sample_int)
  {
    sound_timestamp=millis();
    spl_v = analogRead(SOUND_PIN);
    if (spl_v > 10)  //discard 0-10 readings
    {
      average = (average*9 + spl_v)/10;   // moving average
    }
  }
  spl = 46.4*pow(average,.13);  // Best-fit power curve from dB-meter calibration testing
  if (spl<70)
  {
    spl = map(spl, 50, 70, 30, 70);  // Calibration for low SPL values
  }
}
void display_sound() // Display SPL data on 5110 LCD
{
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(15,0);
  display.println("Currently");
  display.setCursor(20,10);
  display.println("reading:");
  display.setCursor(25,22);
  display.setTextSize(2);
  display.println(spl);
  display.setTextSize(1);
  display.setCursor(31,40);
  display.print("dB");
  display.display();

}
void read_accel() // Accelerometer function
{
  if (millis() - accel_timestamp >= accel_sample_int)
  {
    accel_timestamp=millis();
    //read ADXL335 sensor
    gx1 = analogRead(ACCELX_PIN);       // read analog input pin for accel pin X
    gy1 = analogRead(ACCELY_PIN);       // read analog input pin for accel pin Y
    gz1 = analogRead(ACCELZ_PIN);       // read analog input pin for accel pin Z

    //  Calibrate values
    gx1=map(gx1,265,398,1000,-1000);
   // gx1=constrain(gx1,-1000,1000); //contrains it to 1g (sensor good for 3gs)
    gy1=map(gy1,264,400,-1000,1000);
   // gy1=constrain(gy1,-1000,1000);
    gz1=map(gz1,270,405,-1000,1000);
   // gz1=constrain(gz1,-1000,1000);

    gx=(float)gx1/1000;
    gy=(float)gy1/1000;
    gz=(float)gz1/1000;
   // gx=abs(gx); //use only if you want absolute values
    //gy=abs(gy);
    //gz=abs(gz);

    // Calculate magnitude
    g=sqrt(sq(gx)+sq(gy)+sq(gz));
  }
}
void display_accel() // Display acceleration / gravity data on 5110 LCD
{
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(6,0);
  display.print("gx   gy   gz");
  display.setCursor(0,12);
  display.print(gx);
  display.print(" ");  
  display.print(gy);
  display.print(" ");  
  display.println(gz);
  display.println("   Magnitude:");  
  display.print("    ");  
  display.print(g);  
  display.print(" g");  
  display.display();
}
void display_all() // Display all data on 5110 LCD
{
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(10,4);
  display.print(lux_int);
    display.print("     ");  
  display.println(tempLevel);
    display.setCursor(10,14);
  display.println("lux    deg F");
    display.setCursor(14,30);
  display.print(spl);
  display.print("    ");  
  display.println(g);  
  display.print("  dB      g");  
display.drawLine(42,0,42,48,BLACK);
display.drawLine(0,24,84,24,BLACK);
  display.display();
}
void display_logo()
{

}

//Buzzer functions
int frequency(char note) 
{
  // This function takes a note character (a-g), and returns the
  // corresponding frequency in Hz for the tone() function.
  int j;
  const int numNotes = 8;  // number of notes we're storing

  // The following arrays hold the note characters and their
  // corresponding frequencies. The last "C" note is uppercase
  // to separate it from the first lowercase "c". If you want to
  // add more notes, you'll need to use unique characters.

  // For the "char" (character) type, we put single characters
  // in single quotes.

  char names[] = { 
    'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'                                                                                           };
  int frequencies[] = {
    262, 294, 330, 349, 392, 440, 494, 523                                                                                          };

  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.

  for (j = 0; j < numNotes; j++)  // Step through the notes
  {
    if (names[j] == note)         // Is this the one?
    {
      return(frequencies[j]);     // Yes! Return the frequency
    }
  }
  return(0);  // We looked through everything and didn't find it,
  // but we still need to return a value, so return 0.
}

void playTone(int tone, int duration) {  // Function to play a musical tone
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(tone);
  }
}

void play_banner() // Play the Star-Spangled Banner
{
  int length = 28; // the number of notes
  char notes[] = "gcccbaaadfedccbggcdefGcdefdc"; // a space represents a rest
  int beats[] = { 
    2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 4, 1, 1, 3, 1, 2, 4                                                                                          };
  int tempo = 350;

  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote_banner(notes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
}

void playNote_banner(char note, int duration) {  // Set notes for Star-Spangled Banner
  char names[] = { 
    'g', 'a', 'b', 'c', 'd', 'e', 'f', 'G'                                                                                               };
  int tones[] = { 
    2551, 2273, 2024, 1915, 1700, 1515, 1433, 1275                                                                                           }; // Note frequencies - range from G3 to G4

  for (int i = 0; i < 9; i++) {   // Play the tone corresponding to the note name
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void buzzer_beep() // Beeping function
{
  //SETUP Buzzer
  const int songLength = 12;
  // Notes is an array of text characters corresponding to the notes
  // in your song. A space represents a rest (no tone)
  //char notes[] = "ccggaagffeeddcggffeedggffeedccggaagffeeddc"; // a space represents a rest
  char notes[] = "CbgCbe CcgaC";
  // Beats is an array of values for each note and rest.
  // A "1" represents a quarter-note, 2 a half-note, etc.
  // Don't forget that the rests (spaces) need a length as well.
  int beats[] = {
    2, 2, 2, 4, 4, 2, .5, 2, 2, 2, 1, 1                  };
  // The tempo is how fast to play the song.
  // To make the song play faster, decrease this value.
  int tempo = 100;
  //PLAY A TUNE
  int j, duration;

  for (j = 0; j < songLength; j++) // step through the song arrays
  {
    duration = beats[j] * tempo;  // length of note/rest in ms

    if (notes[i] == ' ')          // is this a rest? 
    {
      delay(duration);            // then pause for a moment
    }
    else                          // otherwise, play the note
    {
      tone(BUZZER_PIN, frequency(notes[i]), duration);
      delay(duration);            // wait for tone to finish
    }
    delay(tempo/10);              // brief pause between notes
  }
}














