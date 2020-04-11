//===================================================================================================
// Names: Gurbani Baweja, Steven Jiao, Tyler Mah, Weichen Qiu
// IDs: 1590254, 1442672, 1429548, 1578205
// CMPUT 275 - Winter 2020
// Final Project : Fitness Tracking Device
//===================================================================================================


#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <Wire.h> // for accelerometer abd gyroscope and temp
#include <string.h>
#include <stdio.h>

#define TFT_WIDTH 320 // Width / height oriented vertically
#define TFT_HEIGHT 480
#define CYAN    0x07FF

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

#define TIMEROW 60   // y position of clock time row
#define DATEROW 200  // y position of date row

// constant needed to take measurements
const int MPU=0x68; 
int16_t GyX,GyY,GyZ,Tmp;
int32_t GyXCal, GyYCal, GyZCal, AcX, AcY, AcZ, AcTot, AcAvg;
float pitch, roll, yaw;
float pitch_acc, roll_acc;
float pitchOut = 0; 
float rollOut = 0;
long loop_timer = 0;

MCUFRIEND_kbv tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
enum {HOME, BEGIN_WORKOUT, END_WORKOUT, GRAPH, RESULTS} currentMode; // modes

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
void calibration();

//===================================================================================================

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
	Wire.write(0x00);    
	Wire.endTransmission(true);
	// gyroscope
	Wire.beginTransmission(MPU);
	Wire.write(0x1B); 
	Wire.write(0x08);  // for +/- 500 deg/s
	Wire.endTransmission();
	
	// accelerometer
	Wire.beginTransmission(MPU);
	Wire.write(0x1C); 
	Wire.write(0x08);  // for +/- 8g
	Wire.endTransmission();
	
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);

 	Serial.flush();
 	syncTimeDate();   // sync time with PC
 	calibration();   // for calibrating gyro and accelerometer
 	measurements(); // get first measurents
 	drawHome();    // draw home page

}

//===================================================================================================

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

//===================================================================================================

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

//===================================================================================================

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

//===================================================================================================

// for debugging purposes /// **** REMOVE LATER***** ///
char temp_str[7];
char* int16_to_str(int16_t num) {
	sprintf(temp_str, "%6d", num);
	return temp_str;
}

// 
void calibration() {
	// set all values initially to 0
	GyXCal = 0;
	GyYCal = 0;
	GyZCal = 0;
	pitch = 0;
	roll = 0;
	yaw = 0;
	Serial.print("Calibrating");
	// calibrate using 1000 readings
	for (int i = 0; i < 1000; i++) {
		if (i % 200 == 0) {
			Serial.print(".");
		}
		Wire.beginTransmission(MPU);
		Wire.write(0x3B);  
		Wire.endTransmission(false);
		Wire.requestFrom(MPU,7*2,true); 
		while (Wire.available() < 14); 
		// read in acc data as well and calibrate later if needed
		AcX=Wire.read()<<8|Wire.read();    
		AcY=Wire.read()<<8|Wire.read();  
		AcZ=Wire.read()<<8|Wire.read();  
		Tmp = Wire.read()<<8 | Wire.read();
		// get gyro calibration and add to total 
		GyXCal+=(Wire.read()<<8|Wire.read())/65.5;  
		GyYCal+=(Wire.read()<<8|Wire.read())/65.5;  
		GyZCal+=(Wire.read()<<8|Wire.read())/65.5;  
		Tmp = Tmp/340.00+36.53; // convert to celsius
		delay(3);
	}
	// average it out for final calibration values
	GyXCal /= 1000;
	GyYCal /= 1000;
	GyZCal /= 1000;

	// output the calibrated values to serial mon
	Serial.println();
	Serial.print("Gyroscope Calibrated: ");
	Serial.print("X = "); Serial.print(GyXCal);
	Serial.print("| Y = "); Serial.print(GyYCal);
	Serial.print("| Z = "); Serial.println(GyZCal);
}

//===================================================================================================
/*
 process acceleration readings
*/
void processAc() {
  
  // get measurements
  measurements();
  AcX = abs(AcX);
  AcY = abs(AcY);
  AcZ = abs(AcY);
  AcTot = AcX + AcY + AcZ;

}

//===================================================================================================

/*
 process gyro readings and get them into pitch and roll values
*/
void processGyr() {

	// first get the measurements
	measurements();
	// divide GyX and GyY by 65.5 to convert reading into degrees 
	GyX /= 65.5; GyX -= GyXCal; // subtract calibration reading 
	GyY /= 65.5; GyY -= GyYCal;
	GyZ /= 65.5; GyZ -= GyZCal;

	// calculate pitch by adding the degrees changed divided by 
	// frequency of getting readings 
	pitch += GyX/25;
	roll += GyY/25;
	
}

//===================================================================================================

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
  Wire.requestFrom(MPU,7*2,true);  
  while(Wire.available() < 14);
  AcX=Wire.read()<<8|Wire.read();    
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
  Tmp = Wire.read()<<8 | Wire.read();
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read();  
  Tmp = Tmp/340.00+36.53;  // convert to celsius

}

//===================================================================================================

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
	seconds += addSeconds;  // add new seconds to current seconds
	int addMinutes = seconds/60;  // integer division
	seconds -= (addMinutes*60);  // subract the seconds moved to mins
	minutes += addMinutes;      // add seconds carried over to minutes
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

//===================================================================================================

/*
	Update temperature measurement on home screen
*/
void updateTemp(){
	
	tft.setTextSize(3);
	pinMode(YP, OUTPUT);
	pinMode(XM, OUTPUT);
	
	// update temperature periodically
	measurements();  // get new temperature measurement
	tft.setTextColor(TFT_BLACK, TFT_RED);
	tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
	tft.print(Tmp);  // update the temperature
	pinMode(YP, INPUT);
	pinMode(XM, INPUT);

}

//===================================================================================================

/*
   Draws the workout page.
*/
void drawWorkout(int stopWatch_h, int stopWatch_m, int stopWatch_s) {
	
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

    // draw the graph button
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, tft.color565(200,160,188));
    tft.setCursor(TFT_WIDTH/8 + 32,TFT_HEIGHT*2/5 + 20);
    tft.print("SHOW GRAPH");
    tft.fillRect(TFT_WIDTH/8,TFT_HEIGHT*2/5,
                TFT_WIDTH-TFT_WIDTH/4,3,TFT_BLACK);

	//print the stop watch
	tft.setTextSize(5);
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(TFT_WIDTH/2-120,STOPROW); tft.print(stopWatch_h);
	tft.setCursor(TFT_WIDTH/2-55,STOPROW); tft.print(":");
	tft.setCursor(TFT_WIDTH/2-20, STOPROW); tft.print(stopWatch_m);
	tft.setCursor(TFT_WIDTH/2+45,STOPROW); tft.print(":");
	tft.setCursor(TFT_WIDTH/2+75 ,STOPROW); tft.print(stopWatch_s);

}

//===================================================================================================

/*
   Draws the results page.
*/
void drawResults(int time,int avgAcc) {
	
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);
	
	tft.fillScreen(CYAN);
	tft.setTextSize(3);
    tft.setCursor(TFT_WIDTH/8 - 16 ,TFT_HEIGHT-TFT_HEIGHT*4/5);
	tft.print("WORKOUT RESULTS");

    // Display Average Accelaration
    tft.setTextSize(3);
	tft.setTextColor(TFT_BLACK);
    tft.setCursor(TFT_WIDTH/8 - 20,TFT_HEIGHT-TFT_HEIGHT/3-40);
    tft.print("Avg Accelaration:");
	tft.setCursor(TFT_WIDTH/8 +60,TFT_HEIGHT-TFT_HEIGHT/3 + 8);
	tft.print(avgAcc);
	tft.setCursor(TFT_WIDTH/8 + 135,TFT_HEIGHT-TFT_HEIGHT/3 + 8);
	tft.print("m/s");

    //  Display time
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(TFT_WIDTH/8+ 4 ,TFT_HEIGHT*2/5 );
    tft.print("Workout Time:");
    tft.setCursor(TFT_WIDTH/8 + 75,TFT_HEIGHT*2/5 + 40);
    tft.print(time);
    tft.setCursor(TFT_WIDTH/8 + 120,TFT_HEIGHT*2/5 + 40);
    tft.print("s");
    
    // Display return to homepage message
    tft.setTextSize(2);
    tft.setTextColor(TFT_MAGENTA);
    tft.setCursor(TFT_WIDTH/8 - 20,TFT_HEIGHT-TFT_HEIGHT/3 + 80);
    tft.print("You will enter homepage");
    tft.setCursor(TFT_WIDTH/8 - 20,TFT_HEIGHT-TFT_HEIGHT/3 + 110);
    tft.print("in 10 seconds");
 
}

//===================================================================================================

/*
 Stores total accelerations
*/
struct AcData {
 
  int32_t AcTotal;
 
};
AcData ac_data[6000]; // 6000 stores 10 minutes of data

//===================================================================================================

/*
 Swap function for sorting
*/
void swap(AcData &a, AcData &b) {
    AcData temp = a;
    a = b;
    b = temp;
}

//===================================================================================================

/*
 Actual sorting of quick sort; implemented from pseudocode on ECLASS
*/
int pivot(AcData a[], int n, int pi) {
  
  // swap the pivot with the last term
  swap(a[pi], a[n-1]);
  // store high and low index
  int lo = 0;
  int hi = n-2;

  // iterate until lo > hi
  while (lo <= hi) {
   
    // if lo index distance is less than or equal to pivot's, move low index up
    if (a[lo].AcTotal <= a[n-1].AcTotal) {
      lo++;
   
    // if hi index distance is greater than pivot's, move high down
    } else if (a[hi].AcTotal > a[n-1].AcTotal) {
      hi--;
   
    // else swap high and low 
    } else {
      swap(a[lo], a[hi]);
    }
  }
  
  // swap the pivot back into position
  swap(a[lo], a[n-1]);

  // return the new low index
  return lo;
}

//===================================================================================================

/*
  Quick sort implementation using recursion of a AcData struct 
*/
void qSort(AcData a[], int n) {
  // if n <=1 then do nothing
  if (n > 1) {
    // Initial pivot set to be at the half-way point (a little above half)
    int pi = n/2;
    // store the new pivot after pivoting the array
    int new_pi = pivot(a, n, pi);
    // recursively sort the bottom portion before pivot
    qSort(a, new_pi);
    // recursively sort top portion after pivot
    qSort(&a[new_pi+1], n-new_pi-1);
  }
}

//===================================================================================================

/*
	Workout Mode
*/
void workout(){
	bool screenOn = true;
  	bool showGraph = false;
	int stopWatch_h = 0; int stopWatch_m = 0; int stopWatch_s = 0;
	drawWorkout(stopWatch_h,stopWatch_m,stopWatch_s);
    // stopwatch starts at 0
	int STOPROW = 15;  // spacing purposes

    // spacing and colorRGB used for updated stopwatch
	int spacing[]={TFT_WIDTH/2+75,TFT_WIDTH/2-20,TFT_WIDTH/2-120};
	int colorRGB[]={200,160,188};
	int oldRoll = 0;
	int time_millis = millis();

	// reset values
	AcAvg = 0;
	int AcIndex = 0;
	int graphIndex = 0;

	while (true){
		time_millis = millis();
	    if (time_millis%100 == 0) {
			// save total acceleration every 100ms
			processAc();
			ac_data[AcIndex].AcTotal = AcTot;
			AcIndex++;
			// print points on graph if button was pressed
			// graph is only relative and not in SI units
			if (showGraph == true && screenOn == true) {
				pinMode(YP, OUTPUT);
				pinMode(XM, OUTPUT);
				AcTot /= (2048/16); // reduce value for graph
				// max acceleration to ceiling of graph
				if (AcTot > TFT_HEIGHT*2/5-9) {
				  AcTot = TFT_HEIGHT*2/5-9;
				}
				tft.fillCircle(graphIndex,TFT_HEIGHT*3/5-AcTot-3,2,TFT_RED);
				graphIndex++;
	    	}

			// while in workout mode, get the gyroscope data
			processGyr();
			// "turn off" screen if screen is on its side position
	  		if (abs(roll - oldRoll) > 10) {
	  			if (screenOn == true){
	  				screenOn = false; // turn it off
	          		showGraph = false;
	  				// go from on to off
	  				pinMode(YP, OUTPUT);
	   				pinMode(XM, OUTPUT);  
	  		 		tft.fillScreen(TFT_BLACK);
	  		 		Serial.println("turning screen off");
	  			} else if (screenOn == false){
	  				screenOn = true;
	  				// redraw
	  				drawWorkout(stopWatch_h,stopWatch_m,stopWatch_s); 
	  			}
	  		}
	  		// set the old roll to new reading of roll
	  		oldRoll = roll; 
    	}

		if (time_millis%1000 == 0){
			Serial.println("updating seconds");
			// always update the time every 1000 milliseconds
			if (screenOn == true){
				// only use function if screen is on
				updateTime(1,stopWatch_s, stopWatch_m, stopWatch_h
				,spacing,colorRGB,STOPROW);
			}else{
				// add one but don't print
				stopWatch_s += 1;
			}
	    }

		// I set both graph and stop work out buttons in here so maybe keep?
		// TEMPORARY///FOR TESTING PURPOSES//////////////////////
		TSPoint touch = ts.getPoint();
		if (touch.z >= MINPRESSURE && touch.z <= MAXPRESSURE && screenOn == true) { 
			// ***** probably change this, for testing only
			int pty = map(touch.y, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
			// graph button pushed
			// show graph
			if (pty > TFT_HEIGHT/5 && showGraph == false) {
			pinMode(YP, OUTPUT);
			pinMode(XM, OUTPUT);
			showGraph = true;
			graphIndex = 0;
			tft.fillRect(0,TFT_HEIGHT*1/5,TFT_WIDTH,
			TFT_HEIGHT*2/5,tft.color565(200,160,188));
			tft.fillRect(0,TFT_HEIGHT*1/5,TFT_WIDTH,3,TFT_BLACK);
			tft.fillRect(0,TFT_HEIGHT*3/5,TFT_WIDTH,3,TFT_BLACK);
			pinMode(YP, INPUT);
			pinMode(XM, INPUT);
			}
			// touching graph hides and resets it
			else {
			showGraph = false;
			// redraw
			drawWorkout(stopWatch_h,stopWatch_m,stopWatch_s); 
			}

			// stops work out
			if (pty <= TFT_HEIGHT/5){
			   
			currentMode = END_WORKOUT;
			 
			int timeElaspedSeconds = stopWatch_s + (stopWatch_m*60) + (stopWatch_h*3600);

			int spacing[] = {185,105,30};int colorRGB[] = {100,200,50};
			updateTime(timeElaspedSeconds,CLKseconds, CLKminutes, CLKhour
				   ,spacing,colorRGB,TIMEROW); // add one second

			//calculates average acceleration
			AcAvg = AcTot / timeElaspedSeconds;

			drawResults(timeElaspedSeconds,AcAvg); // go to results page
			delay(10000);
			drawHome();  // go to homepage 10 seeconds after displaying the results page
			break;           	  			
			}
			///////////////////////////////////////////////////////////////////
		}
	}
	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);  
}

//===================================================================================================

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
	 		// every second update temperature and time
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
