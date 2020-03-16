#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <Wire.h> // for accelerometer abd gyroscope and temp

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

const int MPU=0x68; 
int16_t AcX,AcY,AcZ,GyX,GyY,GyZ, Tmp;

MCUFRIEND_kbv tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
enum {HOME, BEGIN_WORKOUT, END_WORKOUT, GRAPH} currentMode; // modes

void drawHome(); // forward declaration
void measurements(); 

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
	tft.setTextSize(3);
 	measurements();
 	drawHome();
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

  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = "); Serial.println(AcZ); 

  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.print(" | Z = "); Serial.println(GyZ);

  Serial.print(" | tmp = "); Serial.print(Tmp);
}
void workout(){
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
	 	if (millis()%10000 == 0 && currentMode == HOME){
	 		// update temperature periodically
	 		measurements();
	 		tft.setTextColor(TFT_RED, TFT_BLACK);
			tft.setCursor(TFT_WIDTH-TFT_WIDTH/8 -15,30);
			tft.print(Tmp); // update the temperature
	 	}
	 	if (currentMode == BEGIN_WORKOUT){
	 		workout();
	 	}

	}
}
