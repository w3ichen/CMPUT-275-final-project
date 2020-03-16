#include <iostream>
#include <ctime>
#include <string.h>
#include "serialport.h"

using namespace std;

// SerialPort Serial("/dev/ttyACM0"); //intiialize serial communication


// Serial.writeline(line);
// Serial.readline();


int main(){
	// current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   char* dt = ctime(&now);

   string weekDay, month, date, hour, minute, year, time;
   weekDay = strtok(dt," ");
   month = strtok(NULL," ");
   date = strtok(NULL," ");
   time = strtok(NULL," ");
   year = strtok(NULL," ");

cout<<"week day: "<<weekDay<<endl;
cout<<"month: "<<month<<endl;
cout<<"date: "<<date<<endl;
cout<<"year: "


   cout << "The local date and time is: " << dt << endl;


	
}