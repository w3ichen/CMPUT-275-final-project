#include <iostream>
#include <ctime>
#include <string.h>
#include "serialport.h"

using namespace std;

SerialPort Serial("/dev/ttyACM0"); //intiialize serial communication

int main(){ 
   string weekDay, month, date, hour, minute, year, second, message;

   message = Serial.readline(); // wait on arduino
   cout<<"Received: "<<message;
   // current date/time based on current system
   time_t now = time(0);
   char* dt = ctime(&now); // convert to char*

   cout<<"Syncing Time and Date"<<endl;
   cout<<dt<<endl;
   // separate string into substrings
   weekDay = strtok(dt," ");
   month = strtok(NULL," ");
   date = strtok(NULL," ");
   hour = strtok(NULL,":");
   minute = strtok(NULL,":");
   second = strtok(NULL," ");
   year = strtok(NULL," ");

   	// sending date
	Serial.writeline(weekDay+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message<<endl;

	cout<<"Sending: "<<weekDay<<endl;
	Serial.writeline(month+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;

	cout<<"Sending: "<<month<<endl;
	Serial.writeline(date+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;

	cout<<"Sending: "<<date<<endl;
	Serial.writeline(year+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;
	cout<<"Sending: "<<year<<endl;

	// sending time
	Serial.writeline(hour+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;
	cout<<"Sending: "<<hour<<endl;

	Serial.writeline(minute+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;
	cout<<"Sending: "<<minute<<endl;

	Serial.writeline(second+"#"); 
	message = Serial.readline();
	cout<<"Received: "<<message;
	cout<<"Sending: "<<second<<endl;
	
}