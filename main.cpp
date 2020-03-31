#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <Wire.h> // for accelerometer abd gyroscope and temp
#include <string.h>

#define TFT_WIDTH 320 // Width / height oriented vertically
#define TFT_HEIGHT 480

// touch screen pins, obtained from the documentaion
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM  9  // can be a digital pin
#define XP  8  // can be a digital pin

// calibration data for the touch screen, obtained from documentation
// the minimum/maximum possible readings from the touch point
#define TS_MINY 100
#define TS_MINX 120
#define TS_MAXY 920
#define TS_MAXX 940

// thresholds to determine if there was a touch
#define MINPRESSURE  100
#define MAXPRESSURE 1000

#define TIMEROW 60 // y position of clock time row
#define DATEROW 200 // y position of date row

// constant needed to take measurements
const int MPU=0x68; 
int16_t AcX,AcY,AcZ,GyX,GyY,GyZ, Tmp;

MCUFRIEND_kbv tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
enum {HOME, BEGIN_WORKOUT, END_WORKOUT, GRAPH} currentMode; // modes

// variables for home screen time and date
String weekDay,month,date,year;
int CLKhour,CLKminutes,CLKseconds;

// input buffer to store server messages
const uint16_t buf_size = 256;
// current number of chars in buffer, not counting null terminator
uint16_t buf_len = 0;
char* buffer = (char *) malloc(buf_size);

// forward declaration
void measurements(); 
void drawHome(); 
void syncTimeDate();

void setup() {
	init();
	Serial.begin(9600);

	// tft display initialization
	uint16_t ID = tft.readID();
	tft.begin(ID);
	tft.setRotation(0);
	tft.setTextWrap(false);

	// accelerometer/gyroscope/temperature intialization
	Wire.begin();
	Wire.beginTransmission(MPU);
	Wire.write(0x6B); 
	Wire.write(0);    
	Wire.endTransmission(true);

	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);

 	syncTimeDate(); // sync time with PC
 	measurements(); // get first measurents
 	drawHome(); // draw home page
}
/*
	Processes data transferred from the server and stores 
	in corresponding variable
	Arguments: index to indicate which variable to store buffer
*/
void process_line(int index) {
  // store buffer in different variables depending on index
  if (index == 0){
  	weekDay = String(buffer);
  }else if (index == 1){
  	month = String(buffer);
  }else if (index == 2){
  	date = String(buffer);
  }else if (index == 3){
  	year = String(buffer);
  }else if (index == 4){
  	CLKhour = String(buffer).toInt();
  }else if (index == 5){
  	CLKminutes = String(buffer).toInt();
  }else if (index == 6){
  	CLKseconds = String(buffer).toInt();
  }
  Serial.println("A"); // send acknowledge

  // clear the buffer
  buf_len = 0;
  buffer[buf_len] = 0;
}

/*
	Serial communciation with server to get current time and date
*/
void syncTimeDate(){
	char in_char; // to read input
	int counter = 0;
	while (!Serial.available()){
		// keep sending requests to sync
		Serial.println("S"); // intiate sync
		delay(100);
	}
	// receiving data
	while (counter < 7){
	  // expecting 7 messages from server. Keep looping until 7 messages sent
	  if (Serial.available()){
	      // read the incoming byte:
	      in_char = Serial.read();
	      // if end of line is received, waiting for line is done:
	      if (in_char == '#') {
	          // now we process the buffer
	          process_line(counter);
	          counter++; // count as a new message is processed
	      }
	      else {
	          // add character to buffer, provided that we don't overflow.
	          // drop any excess characters.
	          if ( buf_len < buf_size-1 ) {
	              buffer[buf_len] = in_char;
	              buf_len++;
	              buffer[buf_len] = 0;
	          }
	      }
      }
	}
}

/*
	Draws the homepage with: Clock, Date, Temperature, Start workout button
*/
void drawHome(){
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);
	currentMode = HOME; // mode is now Home
	tft.fillScreen(tft.color565(100,200,50));
	tft.setTextSize(3);

	// temperature measurement at top right
	tft.fillCircle(TFT_WIDTH- TFT_WIDTH/8,40,35,TFT_RED);
	tft.drawCircle(TFT_WIDTH-20, 25,4, TFT_BLACK); // degree circle
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
	tft.print(Tmp); // print the temperature

	// start workout button
	tft.setCursor(TFT_WIDTH/8 + 5,TFT_HEIGHT-TFT_HEIGHT/4 + 20);
	tft.print("START WORKOUT");
	tft.fillRect(TFT_WIDTH/8,TFT_HEIGHT-TFT_HEIGHT/4,
				 TFT_WIDTH-TFT_WIDTH/4,3,TFT_BLUE);

	// write time
	tft.setTextSize(5);
	tft.setCursor(30,TIMEROW); tft.print(CLKhour);
	tft.setCursor(85,TIMEROW); tft.print(":");
	tft.setCursor(105,TIMEROW); tft.print(CLKminutes);
	tft.setCursor(160,TIMEROW); tft.print(":");
	tft.setCursor(185,TIMEROW); tft.print(CLKseconds);

	// write date
	tft.setTextSize(3);
	tft.setCursor(TFT_WIDTH/2-30,DATEROW-50); tft.print(weekDay);
	tft.setCursor(50,DATEROW); tft.print(month);
	tft.setCursor(140,DATEROW); tft.print(date);
	tft.setCursor(210,DATEROW); tft.print(year);

	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);
}

/*
	Take measurements from GY-521 module: acceleration in X,Y,Z,
	gyroscope in X,Y,Z, temperature in celsuis. Data stored in global variables
*/
void measurements(){
  // NOTE: measurements written to global variables
  // Read in measurements
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,12,true);  
  AcX=Wire.read()<<8|Wire.read();    
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
  Tmp = Wire.read()<<8 | Wire.read();
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read();  
  Tmp = Tmp/340.00+36.53; // convert to celsius

/*
  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = "); Serial.println(AcZ); 

  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.print(" | Z = "); Serial.println(GyZ);

  Serial.print(" | tmp = "); Serial.print(Tmp);

*/
}

/*
	Updates the time in hours, minutes, and seconds
	Arguments: addSeconds is seconds to increment by
	seconds, minutes, hour are passed in by reference and are modified
	spacing, colorRGB, rowLine control appearances and spacings when printing
*/
void updateTime(int addSeconds, int &seconds, int &minutes, int &hour
				,int spacing[], int colorRGB[], int rowLine){
	tft.setTextSize(5);
	tft.setTextColor(TFT_BLACK, tft.color565(colorRGB[0],colorRGB[1],colorRGB[2]));
	
	// calculations
	seconds += addSeconds; // add new seconds to current seconds
	int addMinutes = seconds/60; // integer division
	seconds -= (addMinutes*60); // subract the seconds moved to mins
	minutes += addMinutes; // add seconds carried over to minutes
	int addHours = minutes/60; // integer division
	minutes -= (addHours*60); 
	hour += addHours;

	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);

	if (seconds < 10){
		// redraw background for 1 digit numbers
		tft.setCursor(spacing[0],rowLine); tft.print("  ");
	}
	// update seconds
	tft.setCursor(spacing[0],rowLine); tft.print(seconds);
	if (addMinutes > 0){
		// need to update minutes if addMinutes is not 0
		if (minutes < 10){
			//redraw background for 1 digit numbers
			tft.setCursor(spacing[1],rowLine); tft.print("  ");
		}
		tft.setCursor(spacing[1],rowLine); tft.print(minutes);
	}
	if (addHours > 0){
		// need to update hours if addHours is not 0
		if (hour < 10){
			//redraw background for 1 digit numbers
			tft.setCursor(spacing[2],rowLine); tft.print("  ");
		}
		tft.setCursor(spacing[2],rowLine); tft.print(hour);
	}
 	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);
}

/*
	Update temperature measurement on home screen
*/
void updateTemp(){
	tft.setTextSize(3);
	pinMode(YP, OUTPUT);
	pinMode(XM, OUTPUT);
	// update temperature periodically
	measurements(); // get new temperature measurement
	tft.setTextColor(TFT_BLACK, TFT_RED);
	tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
	tft.print(Tmp); // update the temperature
	pinMode(YP, INPUT);
	pinMode(XM, INPUT);
}


/*
	Workout Mode
*/
void workout(){
	// stopwatch starts at 0
	int stopWatch_h = 0; int stopWatch_m = 0; int stopWatch_s = 0;
	int STOPROW = 15; // spacing purposes
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);
	tft.fillScreen(tft.color565(200,160,188));

	// draw the end workout button
	tft.setTextSize(3);
	tft.setTextColor(TFT_BLACK, tft.color565(200,160,188));
	tft.setCursor(TFT_WIDTH/8 + 22,TFT_HEIGHT-TFT_HEIGHT/4 + 20);
	tft.print("END WORKOUT");
	tft.fillRect(TFT_WIDTH/8,TFT_HEIGHT-TFT_HEIGHT/4,
				 TFT_WIDTH-TFT_WIDTH/4,3,TFT_BLACK);

	//print the stop watch
	tft.setTextSize(5);
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(TFT_WIDTH/2-120,STOPROW); tft.print(stopWatch_h);
	tft.setCursor(TFT_WIDTH/2-55,STOPROW); tft.print(":");
	tft.setCursor(TFT_WIDTH/2-20, STOPROW); tft.print(stopWatch_m);
	tft.setCursor(TFT_WIDTH/2+45,STOPROW); tft.print(":");
	tft.setCursor(TFT_WIDTH/2+75 ,STOPROW); tft.print(stopWatch_s);

	// spacing and colorRGB used for updated stopwatch
	int spacing[]={TFT_WIDTH/2+75,TFT_WIDTH/2-20,TFT_WIDTH/2-120};
	int colorRGB[]={200,160,188};
	while (true){
		if (millis()%1000 == 0){ // 1000 miliseconds in a second
			// add 1 second
			updateTime(1,stopWatch_s, stopWatch_m, stopWatch_h
				,spacing,colorRGB,STOPROW);
		}


	// TEMPORARY///FOR TESTING PURPOSES//////////////////////
		TSPoint touch = ts.getPoint();
	 	if (touch.z >= MINPRESSURE && touch.z <= MAXPRESSURE) { 
			// ***** probably change this, for testing only
			// stops work out!!
	

			int timeElaspedSeconds = stopWatch_s + (stopWatch_m*60) + (stopWatch_h*3600);
			// add time in workout to clock
			int spacing[] = {185,105,30};int colorRGB[] = {100,200,50};
			updateTime(timeElaspedSeconds,CLKseconds, CLKminutes, CLKhour
	 			,spacing,colorRGB,TIMEROW); // add one second
			drawHome(); // go to home screen
			currentMode = HOME;
			break;
	///////////////////////////////////////////////////////////////////
	 	}
	}
	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);  
}

int main() {
	setup();
	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);
 	int ptx, pty;

	while (true){
		TSPoint touch = ts.getPoint();

	 	if (touch.z >= MINPRESSURE && touch.z <= MAXPRESSURE) { 
			pty = map(touch.y, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
		    ptx = map(touch.x, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
	 		if (pty <= TFT_HEIGHT/4){
	 			// start workout is pressed
	 			currentMode = BEGIN_WORKOUT; // mode is now begin workout
	 		}
	 	}
	 	if (millis()%1000 == 0 && currentMode == HOME){
	 		// every second update temperature and temperature
	 		updateTemp(); // update temp
	 		int spacing[] = {185,105,30}; int colorRGB[] = {100,200,50};
	 		updateTime(1,CLKseconds, CLKminutes, CLKhour
	 			,spacing,colorRGB,TIMEROW); // add one second
	 	}
	 	if (currentMode == BEGIN_WORKOUT){
	 		// if mode is BEGIN_WORKOUT, go to workout page
	 		workout();
	 	}

	}
}
