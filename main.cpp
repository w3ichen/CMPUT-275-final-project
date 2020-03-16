#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <Wire.h> // for accelerometer abd gyroscope and temp

// width/height of the display when rotated horizontally
#define TFT_WIDTH 480
#define TFT_HEIGHT 320

// touch screen pins, obtained from the documentaion
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM  9  // can be a digital pin
#define XP  8  // can be a digital pin

// calibration data for the touch screen, obtained from documentation
// the minimum/maximum possible readings from the touch point
#define TS_MINX 100
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// thresholds to determine if there was a touch
#define MINPRESSURE  100
#define MAXPRESSURE 1000

const int MPU=0x68; 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ, temperature;

MCUFRIEND_kbv tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
enum {HOME, BEGIN_WORKOUT, END_WORKOUT, GRAPH} currentMode; // modes

void drawHome(); // pre declaration

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
 	drawHome();
}

void drawHome(){
	currentMode = HOME;
	tft.fillScreen(TFT_RED);
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
  temperature = Wire.read()<<8 | Wire.read();
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read();  
  Tmp = temperature/340.00+36.53;

  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = "); Serial.println(AcZ); 

  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.print(" | Z = "); Serial.println(GyZ);

  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
}

int main() {
	setup();
	pinMode(YP, INPUT);
 	pinMode(XM, INPUT);


	while (true){
		TSPoint touch = ts.getPoint();

	 	if (touch.z >= MINPRESSURE && touch.z <= MAXPRESSURE) { 
	 		Serial.print("X: ");Serial.println(touch.x);
	 		Serial.print("Y: ");Serial.println(touch.y);
	 		delay (100);
	 	}

	}
}
