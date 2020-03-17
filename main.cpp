#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <Wire.h> // for accelerometer abd gyroscope and temp
#include <string.h>

// width/height of the display when rotated horizontally
#define TFT_WIDTH 320
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

#define TIMEROW 60
#define DATEROW 200

const int MPU=0x68; 
int16_t AcX,AcY,AcZ,GyX,GyY,GyZ, Tmp;

MCUFRIEND_kbv tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
enum {HOME, BEGIN_WORKOUT, END_WORKOUT, GRAPH} currentMode; // modes

String weekDay,month,date,year;
int hour,minutes,seconds;

// input buffer to store server messages
const uint16_t buf_size = 256;
// current number of chars in buffer, not counting null terminator
uint16_t buf_len = 0;
char* buffer = (char *) malloc(buf_size);

void measurements(); 
void drawHome(); // forward declaration
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

 	syncTimeDate();
	tft.setTextSize(3);
 	measurements();
 	drawHome();
}

void process_line(int index) {

  if (index == 0){
  	weekDay = String(buffer);
  }else if (index == 1){
  	month = String(buffer);
  }else if (index == 2){
  	date = String(buffer);
  }else if (index == 3){
  	year = String(buffer);
  }else if (index == 4){
  	hour = String(buffer).toInt();
  }else if (index == 5){
  	minutes = String(buffer).toInt();
  }else if (index == 6){
  	seconds = String(buffer).toInt();
  }
  Serial.println("A"); // send acknowledge

  // clear the buffer
  buf_len = 0;
  buffer[buf_len] = 0;
}

void syncTimeDate(){
	char in_char;
	int counter = 0;
	while (!Serial.available()){
		// keep sending request to sync
		Serial.println("S"); // intiate sync
		delay(100);
	}
	// receiving data
	// except 7 messages from server
	while (counter < 7){
	  if (Serial.available()){
	      // read the incoming byte:
	      in_char = Serial.read();
	      // if end of line is received, waiting for line is done:
	      if (in_char == '#') {
	          // now we process the buffer
	          process_line(counter);
	          counter++;
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

void drawHome(){
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);
	currentMode = HOME;
	tft.fillScreen(tft.color565(100,200,50));

	// temperature at top right
	tft.fillCircle(TFT_WIDTH- TFT_WIDTH/8,40,35,TFT_RED);
	tft.drawCircle(TFT_WIDTH-20, 25,4, TFT_BLACK); // degree circle
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
	tft.print(Tmp); // print the temperature

	// start work out button
	tft.setCursor(TFT_WIDTH/8,TFT_HEIGHT-TFT_HEIGHT/4 + 20);
	tft.print("START WORKOUT");
	tft.fillRect(TFT_WIDTH/8,TFT_HEIGHT-TFT_HEIGHT/4,
				 TFT_WIDTH-TFT_WIDTH/4,3,TFT_BLUE);

	// write time
	tft.setTextSize(5);
	tft.setCursor(30,TIMEROW); tft.print(hour);
	tft.setCursor(85,TIMEROW); tft.print(":");
	tft.setCursor(105,TIMEROW); tft.print(minutes);
	tft.setCursor(160,TIMEROW); tft.print(":");
	tft.setCursor(185,TIMEROW); tft.print(seconds);

	// write date
	tft.setTextSize(3);
	tft.setCursor(TFT_WIDTH/2-30,DATEROW-50); tft.print(weekDay);
	tft.setCursor(50,DATEROW); tft.print(month);
	tft.setCursor(140,DATEROW); tft.print(date);
	tft.setCursor(210,DATEROW); tft.print(year);

	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);
}
void measurements(){
// NOTE: measurements written to global variables
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
void updateTime(int addSeconds){
	tft.setTextSize(5);
	tft.setTextColor(TFT_BLACK, tft.color565(100,200,50));
	seconds += addSeconds;
	int addMinutes = seconds/60; //integer division
	seconds -= (addMinutes*60); // subract seconds moved to mins
	minutes += addMinutes;
	int addHours = minutes/60; // integer division
	minutes -= (addHours*60); 
	hour += addHours;

	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);

	if (seconds < 10){
		//redraw background for 1 digit numbers
		tft.setCursor(185,TIMEROW); tft.print("  ");
	}
	// update seconds
	tft.setCursor(185,TIMEROW); tft.print(seconds);
	if (addMinutes > 0){
		// need to update minutes
		if (minutes < 10){
			//redraw background for 1 digit numbers
			tft.setCursor(105,TIMEROW); tft.print("  ");
		}
		tft.setCursor(105,TIMEROW); tft.print(minutes);
	}
	if (addHours > 0){
		// need to update hours
		if (minutes < 10){
			//redraw background for 1 digit numbers
			tft.setCursor(30,TIMEROW); tft.print("  ");
		}
		tft.setCursor(30,TIMEROW); tft.print(hour);
	}

 	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);
}
void updateTemp(){
	tft.setTextSize(3);
	pinMode(YP, OUTPUT);
	pinMode(XM, OUTPUT);
	// update temperature periodically
	measurements();
	tft.setTextColor(TFT_BLACK, TFT_RED);
	tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
	tft.print(Tmp); // update the temperature
	pinMode(YP, INPUT);
	pinMode(XM, INPUT);
}
void workout(){
	//*********start workout stuff here***/
	pinMode(YP, OUTPUT);
 	pinMode(XM, OUTPUT);
	// start workout
	tft.fillScreen(TFT_GREEN);
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
	 			// start workout pressed
	 			currentMode = BEGIN_WORKOUT;
	 		}
	 	}
	 	if (millis()%1000 == 0 && currentMode == HOME){
	 		// every second update temperature and tie
	 		updateTemp();
	 		updateTime(1); // add one second

	 	}
	 	if (currentMode == BEGIN_WORKOUT){
	 		workout();
	 	}

	}
}
