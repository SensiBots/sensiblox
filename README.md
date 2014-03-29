sensiblox
=========

Arduino code that runs on the SensiBlox and communicates with BLE

Arduino Setup Instructions
--------------------------
1. Connect Arduino Pro Mini using USB riser with Mini-A
2. Install Arduino and load up sketch
3. Select Tools->Board->Arduino Pro Mini (5V, 16MHZ) w/ ATMEGA 328
3. Verify Code (Checkmark)
4. Install Adafruit_PCD8544 library
	A. https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library
	B. git clone https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library.git
	C. cp -fr dir to Adafruit_PCD8544
	D. Sketch -> Import Library -> Add Library
		-Follow path to copied folder
		-Don't pay attention to status/error window, Verify source again or restart Arduino
		-Get more errors? Resolve the next library:
5. Install Adafruit_GFX library
	A. https://github.com/adafruit/Adafruit-GFX-Library
	B. git clone https://github.com/adafruit/Adafruit-GFX-Library.git
	C cp -fr dir to Adafruit_GFX
	D. Sketch -> Import Library -> Add Library
		-Follow path to copied folder
		-Don't pay attention to status/error window, Verify source again or restart Arduino
6. Verify code again, should be successful
7. Select Tools -> serial port -> /dev/ttyUSB0
8. Don't worry about selecting a Programmer, not using a programmer
9. Click upload, lights should blink on the board, and eventually the LCD will display a sensor reading
