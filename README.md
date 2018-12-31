# PmodGPS_Tracking_to_Reference
GPS tracking system using Arduino Uno, Digilent PmodGPS, and a Digilent PmodCLS LCD screen.

This project is set up to capture the latitude and longitude of the system upon restart then report the distance and 
bearing to that initial reference point as the system changes location.
The Arduino Uno can be powered with a USB battery to make the system mobile. 
Since the PmodGPS uses the serial port on the Arduino Uno, it must be connected after programming the board. 
The PmodCLS was used because it was conveniently available. A different LCD screen should be implementable without much difficulty.
