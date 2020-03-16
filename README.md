CMPUT 275 Final Project
Fitness Tracking Device

Parts: 
- Arduino / Breadboard / Wires
- TFT display
- GY-521 Module
- Battery power

Step 1:
- User interface on the touchscreen with the following buttons:
      i. temperature icon that updates ~10 seconds
      ii. Current time and date at top (if possible) probably gets time/date from computer at upload
      iii. Start workout button
      iv. End workout button
      v. View realtime graph
- Use gyroscope to turn screen off to save power when device is not facing up

Step 2:
- When start workout is pressed:
      i. Use a data structure to store acceleration data
      ii. Display time elapsed in realtime (prob using milis())
      iii. Button to view realtime plot of acceleration data

Step 3:
Here is where most of algorithms is used to analyze recorded data in a meaningful way
- When end workout is pressed:
      i. Show time elapsed
      ii. (?) use sort algorithm to find time of most activity (highest acceleration) (?)
** analyze data using algorithms in a meaningful way**
maybe comparing to previous workouts to see if they improved.


___________________
note Arduino desktop IDE has a plotting called "serial plotter"
https://learn.adafruit.com/experimenters-guide-for-metro/circ08-using%20the%20arduino%20serial%20plotter 
Useful for visualizing acceleration data!!