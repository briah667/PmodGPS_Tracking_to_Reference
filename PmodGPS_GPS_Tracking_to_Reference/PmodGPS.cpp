/****************************************************************************************/
/*																																	*/
/*	PmodGPS.cpp  This library supports the PmodGPS, in particular, 			*/
/*							providing an easy means of using the module          						*/
/*																																	*/
/****************************************************************************************/
/*	Author: 	Ian Brech, Thomas Kappenman             												*/
/*	Copyright 2014, Digilent Inc.																					*/
/****************************************************************************************/
/****************************************************************************************/
/*  Module Description: 																								*/
/*																																	*/
/*	This the PmodGPS CPP file          																		*/
/*																																	*/
/************************************************************************/
/*  Revision History:																									*/
/*																																	*/
/*	 5/15/2014(IanB): Created																						*/
/*	 6/23/2014(TommyK): Added data parsing and completed										*/
/*										library with additional documentation										*/
/*	  7/31/2014(SamL): reviewed for release																	*/
/*																															*/
/************************************************************************/
/*	Needs work:																											*/
/*																																	*/
/*  	Does not yet support other messages that can be received on startup			*/
/*		These messages are not important to the general operation of the PmodGPS			*/
/*																														*/
/*																																	*/
/*		The data sent from the PmodGPS includes a checksum. a private							*/
/*		function could be created to check the checksum against the							*/
/*		packet, and throw a flag if the packet is in error.												*/
/*		Because the data is updated every second, this isn't important.						*/
/*																																	*/
/*		A send command packet function still needs to be implemented.					*/
/*																																	*/
/*		A timeout needs to be implemented in the gatData function in case the PmodGPS 	*/
/*		is unplugged or interrupted during receive.														*/
/*																																	*/
/*																																	*/
/*		For the PmodGPS datasheet, refer to:															*/
/*		https://www.maritex.com.pl/media/uploads/products/wi/GPS-GMS-U1LP.pdf */
/*																																	*/
/****************************************************************************************/
/*	
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "PmodGPS.h"

/* ------------------------------------------------------------ */
/*  GPSinit()
**
**  Parameters:
**    serPort: The HardwareSerial port (Serial1, Serial2, etc) from
**				MPIDE that will be used to communicate with the PmodGPS
**	  baud: The baud rate to communicate to the PmodGPS with
**    DF: The digital pin number of the 3DF pin on the PmodGPS
**    PPS: The digital pin number of the 1PPS pni on the PmodGPS
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Initialize the system to communicate with the PmodGPS.
*/
void GPS::GPSinit(HardwareSerial &serPort, unsigned long baud, uint8_t DF, uint8_t PPS)
{
	serPort.begin(baud);
	pinMode(DF, INPUT);
	pinMode(PPS, INPUT);
}

/* ------------------------------------------------------------ */
/*  GPSinit()
**
**  Parameters:
**    serPort: The HardwareSerial port (Serial1, Serial2, etc) from
**				MPIDE that will be used to communicate with the PmodGPS
**	  baud: The baud rate to communicate to the PmodGPS with
**    DF: The digital pin number of the 3DF pin on the PmodGPS
**    PPS: The digital pin number of the 1PPS pni on the PmodGPS
**	  RST: The digital pin number of the RESET pin on the PmodGPS
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Initialize the system to communicate with the PmodGPS. Resets
**	  the PmodGPS before beginning normal operation.
*/
void GPS::GPSinit(HardwareSerial &serPort, unsigned long baud, uint8_t DF, uint8_t PPS, uint8_t RST)
{
	serPort.begin(baud);
	pinMode(DF, INPUT);
	pinMode(PPS, INPUT);
	pinMode(RST, OUTPUT);
	digitalWrite(RST, LOW);
	digitalWrite(RST, HIGH);
	getData(serPort);//Receive restart command 
	getData(serPort);//Receive start up command
	getData(serPort);//PGACK command sent
	getData(serPort);//PGACK command sent
}

/* ------------------------------------------------------------ */
/*  getData()
**
**  Parameters:
**	  serPort: The HardwareSerial port (Serial1, Serial2, etc) from
**				MPIDE that will be used to communicate with the PmodGPS
**
**  Return Value:
**    The type of sentence that was recieved.
**
**  Errors:
**    none
**
**  Description:
**    Does a read of data. finishes when a value of decimal
**	  10 (ASCII <LF>) is detected. Reads the first three characters to
**	  verify the packet starts with $GP, then checks the next three
**	  to decide which struct to parse the data into.
*/
NMEA GPS::getData(HardwareSerial &serPort)
{
	char recv[MAX_SIZE]={0};
	int i;
	int count = 0;
	char checksum[3];
	NMEA mode= INVALID;

	if (serPort.available()){	//If there is a sentence
		recv[0] = serPort.read();
		//Get the sentence
		for(i = 1; i < MAX_SIZE; i++){
			while(!serPort.available());//Wait for serial to be ready
			recv[i] = serPort.read();
			if(recv[i] == 10){//End of sentence
				break;
			}
		}
	}
	else{
		return INVALID;
	}
	//Decide what kind of sentence was received
	mode=chooseMode(recv);
	
	//Debugging purposes
	//Serial.print("\n\n Message received: ");Serial.println(recv); //This is the full sentence sent from the PmodGPS

	//Format the sentence into structs
		switch(mode){
			case(GGA):formatGGA(recv);
				break;
			case(GSA):formatGSA(recv);
				break;
			case(GSV):formatGSV(recv);
				break;
			case(RMC):formatRMC(recv);
				break;
			case(VTG):formatVTG(recv);
				break;
			case(INVALID):return INVALID;
		}
	
	
	return(mode);//Return the type of sentence that was sent
}

/* ------------------------------------------------------------ */
/* 	getGGA(), getGSA(), getGSV(), getRMC(), getVTG()
**
**  Parameters:
**	  none
**
**  Return Value:
**    The struct containing the data from the $GPXXX sentence
**
**  Errors:
**    none
**
**  Description:
**    Get functions for the private structs in the PmodGPS class
*/
GGA_DATA GPS::getGGA()
{
	return GGAdata;
}
GSA_DATA GPS::getGSA()
{
	return GSAdata;
}
GSV_DATA GPS::getGSV()
{
	return GSVdata;
}
RMC_DATA GPS::getRMC()
{
	return RMCdata;
}
VTG_DATA GPS::getVTG()
{
	return VTGdata;
}


/* ------------------------------------------------------------ */
/*  isFixed()
**
**  Parameters:
**	  none
**
**  Return Value:
**    bool PFI
**
**  Errors:
**    none
**
**  Description:
**    Returns a true if PFI is 1, else 0
*/
bool GPS::isFixed(){
	if ((int)(GGAdata.PFI-'0')==1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/* ------------------------------------------------------------ */
/*  getLatitude(), getLongitude()
**
**  Parameters:
**	  none
**
**  Return Value:
**    A string containing the data requested
**
**  Errors:
**    none
**
**  Description:
**    Get functions for several data members in string form
*/
char* GPS::getLatitude(){
	return GGAdata.LAT;
}

char* GPS::getLongitude(){
	return GGAdata.LONG;
}


/* ------------------------------------------------------------ */
/*  getAltitudeString()
**
**  Parameters:
**	  none
**
**  Return Value:
**    A string containing the altitude with appended units
**
**  Errors:
**    none
**
**  Description:
**    Gets altitude and returns its value with units in string form
*/
char* GPS::getAltitudeString(){
	char altitude[11]={0};
	char unit[2]={0};
	
	unit[0]=GGAdata.AUNIT;
	strcpy(altitude, GGAdata.ALT);
	strcat(altitude," ");
	strcat(altitude,unit);
	return altitude;
}

/* ------------------------------------------------------------ */
/*  getDate()
**
**  Parameters:
**	  none
**
**  Return Value:
**    A string containing the date in correct format
**
**  Errors:
**    none
**
**  Description:
**    Formats the date and returns it as a char string
*/
char* GPS::getDate(){
	char date[9]={0};
	char null[1]={0};
	date[0]=RMCdata.DATE[2];
	date[1]=RMCdata.DATE[3];
	date[2]='/';
	date[3]=RMCdata.DATE[0];
	date[4]=RMCdata.DATE[1];
	date[5]='/';
	date[6]=RMCdata.DATE[4];
	date[7]=RMCdata.DATE[5];
	strcat(date,null);
	return date;
}

/* ------------------------------------------------------------ */
/*  getTime(), getNumSats(), getPDOP(), getAltitude(), getSpeedKnots(),
**		getSpeedKM(), getHeading()
**
**  Parameters:
**	  	none
**
**  Return Value:
**    The data in the structs in either integer or double form
**
**  Errors:
**    none
**
**  Description:
**    Get functions for several items in the PmodGPS structs. These
**		values are converted to integers or doubles before being 
**		returned.
*/
double GPS::getTime(){
	return atof(GGAdata.UTC);
}

int GPS::getNumSats(){
	return atoi(GGAdata.NUMSAT);
}

double GPS::getPDOP(){
	return atof(GSAdata.PDOP);
}

double GPS::getAltitude(){
	return atof(GGAdata.ALT);
}

double GPS::getSpeedKnots(){
	return atof(VTGdata.SPD_N);
}

double GPS::getSpeedKM(){
	return atof(VTGdata.SPD_KM);
}

double GPS::getHeading(){
	return atof(VTGdata.COURSE_T);
}



/* ------------------------------------------------------------ */
/*  getSatelliteInfo()
**
**  Parameters:
**	 	none
**
**  Return Value:
**    The array of SATELLITE structs in GSVdata
**
**  Errors:
**    none
**
**  Description:
**    A get function for the SATELLITE structs containing
**		satellite info.
*/
SATELLITE* GPS::getSatelliteInfo(){
	return GSVdata.SAT;
}


/* ------------------------------------------------------------ */
/*					Private Functions							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*  chooseMode()
**
**  Parameters:
**	  char recv[MAX_SIZE]
**
**  Return Value:
**    NMEA mode: The format of the sentence, for use in deciding
**	   which struct to format into
**
**  Errors:
**    none
**
**  Description:
**    Reads the third, fourth, and fifth character in the sentence, and outputs an NMEA mode.
*/
NMEA GPS::chooseMode(char recv[MAX_SIZE]){
	NMEA mode=INVALID;
	if (((recv[3]) == 'G') && ((recv[4]) == 'G') && (recv[5] == 'A'))
	{
		mode=GGA;
	}
	else if (((recv[3]) == 'G') && ((recv[4]) == 'S') && (recv[5] == 'A'))
	{
		mode=GSA;
	}
	else if (((recv[3]) == 'G') && ((recv[4]) == 'S') && (recv[5] == 'V'))
	{
		mode=GSV;
	}
	else if (((recv[3]) == 'R') && ((recv[4]) == 'M') && (recv[5] == 'C'))
	{
		mode=RMC;
	}
	else if (((recv[3]) == 'V') && ((recv[4]) == 'T') && (recv[5] == 'G'))
	{
		mode=VTG;
	}
	return mode;
}


/* ------------------------------------------------------------ */
/*  formatGGA()
**
**  Parameters:
**	  data: an array to the data to be formatted
**	  GGAdata: a pointer to the GGA_DATA structure to store the parsed data in
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Formats a mode's data into elements in a struct.
**		NOTE: ',' will separate all values
*/
void GPS::formatGGA(char* data_array)
{
	enum cases {UTC, LAT, NS, LONG, EW, PFI, NUMSAT, HDOP, ALT, AUNIT, GSEP, GUNIT, AODC};
	cases datamember= UTC;
	char* start_ptr;
	char* end_ptr = data_array+7;//Set start pointer after the message ID ("$GPGGA,")
	bool flag=1;
	char COORDbuf[14]={0};
	char checksum[3]={0};

	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *end_ptr!='*')end_ptr++;//Increment ptr until a comma is found
		
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')){//End reached
			flag=0;
			break;
		}
		
		switch(datamember){
				case UTC:
					memcpy(GGAdata.UTC, start_ptr, (end_ptr - start_ptr));
					GGAdata.UTC[end_ptr - start_ptr] = '\0';//End null char
					datamember = LAT;
					break;
				case LAT:
					memcpy(COORDbuf, start_ptr, (end_ptr - start_ptr));
					if (*COORDbuf)
					{
						formatCOORDS(COORDbuf);
						memcpy(GGAdata.LAT, COORDbuf, 13);
						memset(COORDbuf, 0, 13);	
					}
					datamember=NS;
					break;
				case NS:
					if (*start_ptr!=','){
						GGAdata.NS = *start_ptr;
						strncat(GGAdata.LAT, start_ptr, 1);
					}
					datamember=LONG;
					break;
				case LONG:
					memcpy(COORDbuf, start_ptr, (end_ptr - start_ptr ));
					if (*COORDbuf){
						formatCOORDS(COORDbuf);
						memcpy(GGAdata.LONG, COORDbuf, 14);
					}
					datamember=EW;
					break;
				case EW:
					if (*start_ptr!=','){
						GGAdata.EW = *start_ptr;
						strncat(GGAdata.LONG, start_ptr, 1);
					}
					datamember=PFI;
					break;
				case PFI:
					if (*start_ptr!=',')GGAdata.PFI = *start_ptr;
					datamember=NUMSAT;
					break;
				case NUMSAT:
					memcpy(GGAdata.NUMSAT, start_ptr, (end_ptr - start_ptr));
					GGAdata.NUMSAT[end_ptr - start_ptr] = '\0';
					datamember=HDOP;
					break;
				case HDOP:
					memcpy(GGAdata.HDOP, start_ptr, (end_ptr - start_ptr));
					GGAdata.HDOP[end_ptr - start_ptr] = '\0';
					datamember=ALT;
					break;
				case ALT:
					memcpy(GGAdata.ALT, start_ptr, (end_ptr - start_ptr));
					GGAdata.ALT[end_ptr - start_ptr] = '\0';
					datamember=AUNIT;
					break;
				case AUNIT:
					if (*start_ptr!=',')GGAdata.AUNIT= *start_ptr;
					datamember=GSEP;
					break;
				case GSEP:
					memcpy(GGAdata.GSEP, start_ptr, (end_ptr - start_ptr));
					GGAdata.GSEP[end_ptr - start_ptr] = '\0';
					datamember=GUNIT;
					break;
				case GUNIT:
					if (*start_ptr!=',')GGAdata.GUNIT=*start_ptr;
					datamember=AODC;
					break;
				case AODC:
					memcpy(GGAdata.AODC, start_ptr, (end_ptr - start_ptr));
					GGAdata.AODC[end_ptr - start_ptr] = '\0';
					flag=0;
					break;

					}
			end_ptr++;//Increment past the last comma
	}
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(GGAdata.CHECKSUM, checksum, 3);
	return;
}

/* ------------------------------------------------------------ */
/*  formatGSA()
**
**  Parameters:
**	  data: an array to the data to be formatted
**	  GSAdata: a pointer to the GSA_DATA structure to store the parsed data in
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Formats a mode's data into elements in a struct.
*/
void GPS::formatGSA(char* data_array)
{
	enum cases {MODE1, MODE2, SAT1, SAT2, SAT3, SAT4, SAT5, SAT6, SAT7, SAT8, SAT9, SAT10, SAT11, SAT12, PDOP, HDOP, VDOP, AODC};
	cases datamember=MODE1;
	char* start_ptr;
	char* end_ptr = data_array+7;//Set start pointer after the message ID ("$GPGGA,")
	bool flag=1;
	char COORDbuf[14]={0};
	char checksum[3]={0};
	
	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *end_ptr!='*')end_ptr++;//Increment ptr until a comma is found
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')){//End reached
			flag=0;
			break;
		}
		switch(datamember){
		
		    
			case MODE1:
				if (*start_ptr!=',')GSAdata.MODE1 = *start_ptr;
				datamember = MODE2;
				break;
			case MODE2:
				if (*start_ptr!=',')GSAdata.MODE2 = *start_ptr;
				datamember=SAT1;
				break;
			case SAT1:
				memcpy(GSAdata.SAT1, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT1[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT2;
				break;
			case SAT2:
				memcpy(GSAdata.SAT2, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT2[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT3;
				break;
			case SAT3:
				memcpy(GSAdata.SAT3, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT3[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT4;
				break;
			case SAT4:
				memcpy(GSAdata.SAT4, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT4[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT5;
				break;
			case SAT5:
				memcpy(GSAdata.SAT5, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT5[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT6;
				break;
			case SAT6:
				memcpy(GSAdata.SAT6, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT6[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT7;
				break;
			case SAT7:
				memcpy(GSAdata.SAT7, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT7[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT8;
				break;
			case SAT8:
				memcpy(GSAdata.SAT8, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT8[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT9;
				break;
			case SAT9:
				memcpy(GSAdata.SAT9, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT9[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT10;
				break;
			case SAT10:
				memcpy(GSAdata.SAT10, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT10[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT11;
				break;
			case SAT11:
				memcpy(GSAdata.SAT11, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT11[end_ptr - start_ptr] = '\0';//End null char
				datamember=SAT12;
				break;
			case SAT12:
				memcpy(GSAdata.SAT12, start_ptr, (end_ptr - start_ptr));
				GSAdata.SAT12[end_ptr - start_ptr] = '\0';//End null char
				datamember=PDOP;
				break;
			case PDOP:
				memcpy(GSAdata.PDOP, start_ptr, (end_ptr - start_ptr));
				GSAdata.PDOP[end_ptr - start_ptr] = '\0';//End null char
				datamember=HDOP;
				break;
			case HDOP:
				memcpy(GSAdata.HDOP, start_ptr, (end_ptr - start_ptr));
				GSAdata.HDOP[end_ptr - start_ptr] = '\0';//End null char
				datamember=VDOP;
				break;
			case VDOP:
				memcpy(GSAdata.VDOP, start_ptr, (end_ptr - start_ptr));
				GSAdata.VDOP[end_ptr - start_ptr] = '\0';//End null char
				flag=0;
				break;
				}
		end_ptr++;//Increment past the last comma

	} //end of while 
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(GSAdata.CHECKSUM, checksum, 3);
	return;
}

/* ------------------------------------------------------------ */
/*  formatGSV()
**
**  Parameters:
**	  data: an array to the data to be formatted
**	  GSVdata: a pointer to the GSV_DATA structure to store the parsed data in
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Formats GSV messages into their corresponding structs.
*/
void GPS::formatGSV(char* data_array)
{
enum cases {NUMM, MESNUM, SATVIEW, SATID1, ELV1, AZM1, SNR1, SATID2, ELV2, AZM2, SNR2, SATID3, ELV3, AZM3, SNR3, SATID4, ELV4, AZM4, SNR4};
	cases datamember=NUMM;
	char* start_ptr;
	char* end_ptr = data_array+7;//Set start pointer after the message ID ("$GPGGA,")
	bool flag=1;
	char checksum[3]={0};
	int mesnum = 0;
	char buffer[4];
	
	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *(end_ptr)!='*')end_ptr++;//Increment ptr until a comma is found
		
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')){//End reached
			flag=0;
			break;
		}
		switch(datamember){
			case NUMM:
				if (*start_ptr!=',')GSVdata.NUMM = (int)(*start_ptr-'0');
				datamember = MESNUM;
				break;
			case MESNUM:
				if (*start_ptr!=','){
					GSVdata.MESNUM = (int)(*start_ptr-'0');
					mesnum=GSVdata.MESNUM;
					}
				datamember=SATVIEW;
				break;
			case SATVIEW:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SATVIEW=atoi(buffer);
				datamember=SATID1;
				break;
			case SATID1:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4].ID=atoi(buffer);
				datamember=ELV1;
				break;
			case ELV1:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4].ELV=atoi(buffer);
				datamember=AZM1;
				break;
			case AZM1:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4].AZM=atoi(buffer);
				datamember=SNR1;
				break;
			case SNR1:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4].SNR=atoi(buffer);
				datamember=SATID2;
				break;
			case SATID2:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+1].ID=atoi(buffer);
				datamember=ELV2;
				break;
			case ELV2:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+1].ELV=atoi(buffer);
				datamember=AZM2;
				break;
			case AZM2:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+1].AZM=atoi(buffer);
				datamember=SNR2;
				break;
			case SNR2:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+1].SNR=atoi(buffer);
				datamember=SATID3;
				break;
			case SATID3:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+2].ID=atoi(buffer);
				datamember=ELV3;
				break;
			case ELV3:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+2].ELV=atoi(buffer);
				datamember=AZM3;
				break;
			case AZM3:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+2].AZM=atoi(buffer);
				datamember=SNR3;
				break;
			case SNR3:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+2].SNR=atoi(buffer);
				datamember=SATID4;
				break;
			case SATID4:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+3].ID=atoi(buffer);
				datamember=ELV4;
				break;
			case ELV4:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+3].ELV=atoi(buffer);
				datamember=AZM4;
				break;
			case AZM4:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+3].AZM=atoi(buffer);
				datamember=SNR4;
				break;
			case SNR4:
				memcpy(buffer, start_ptr, (end_ptr - start_ptr));
				buffer[end_ptr - start_ptr] = '\0';//End null char
				GSVdata.SAT[(mesnum-1)*4+3].SNR=atoi(buffer);
				flag=0;
				break;
				}
		end_ptr++;
	} //end of while 
	
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(GSVdata.CHECKSUM, checksum, 3);
	//Serial.print("CHECKSUM: ");Serial.println(GSVdata.CHECKSUM);
	return;
}

/* ------------------------------------------------------------ */
/*  formatRMC()
**
**  Parameters:
**	  data: an array to the data to be formatted
**	  RMCdata: a pointer to the RMC_DATA structure to store the parsed data in
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Formats a mode's data into elements in a struct.
*/
void GPS::formatRMC(char* data_array)
{
	enum cases {UTC, STAT, LAT, NS, LONG,EW, SOG, COG, DATE, MVAR, MVARDIR, MODE};
	cases datamember= UTC;
	char* start_ptr;
	char* end_ptr = data_array+7;//Set start pointer after the message ID ("$GPGGA,")
	bool flag=1;
	char COORDbuf[14]={0};
	char checksum[3]={0};
	
	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *end_ptr!='*')end_ptr++;//Increment ptr until a comma is found
		
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')){//End reached
			flag=0;
			break;
		}
		switch(datamember){
			case UTC:
				memcpy(RMCdata.UTC, start_ptr, (end_ptr - start_ptr));
				RMCdata.UTC[end_ptr - start_ptr] = '\0';//End null char
				datamember = STAT;
				break;
			case STAT:
				if (*start_ptr!=',')RMCdata.STAT = *start_ptr;
				datamember=LAT;
				break;
			case LAT:
				memcpy(RMCdata.LAT, start_ptr, (end_ptr - start_ptr));
				RMCdata.LAT[end_ptr - start_ptr] = '\0';//End null char
				datamember=NS;
				break;
			case NS:
				if (*start_ptr!=',')RMCdata.NS = *start_ptr;
				datamember=LONG;
				break;
			case LONG:
				memcpy(RMCdata.LONG, start_ptr, (end_ptr - start_ptr));
				RMCdata.LONG[end_ptr - start_ptr] = '\0';//End null char
				datamember=EW;
				break;
			case EW:
				if (*start_ptr!=',')RMCdata.EW = *start_ptr;
				datamember=SOG;
				break;
			case SOG:
				memcpy(RMCdata.SOG, start_ptr, (end_ptr - start_ptr));
				RMCdata.SOG[end_ptr - start_ptr] = '\0';//End null char
				datamember=COG;
				break;
			case COG:
				memcpy(RMCdata.COG, start_ptr, (end_ptr - start_ptr));
				RMCdata.COG[end_ptr - start_ptr] = '\0';//End null char
				datamember=DATE;
				break;
			case DATE:
				memcpy(RMCdata.DATE, start_ptr, (end_ptr - start_ptr));
				RMCdata.DATE[end_ptr - start_ptr] = '\0';//End null char
				datamember=MVAR;
				break;
			case MVAR:
				memcpy(RMCdata.MVAR, start_ptr, (end_ptr - start_ptr));
				RMCdata.MVAR[end_ptr - start_ptr] = '\0';//End null char
				datamember=MVARDIR;
				break;
			case MVARDIR:
				if (*start_ptr!=',')RMCdata.MVARDIR = *start_ptr;
				datamember=MODE;
				break;
			case MODE:
				if (*start_ptr!=',')RMCdata.MODE = *start_ptr;
				flag=0;
				break;
			}
		end_ptr++;
	} //end of while 
	
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(RMCdata.CHECKSUM, checksum, 3);
	//Serial.print("CHECKSUM: ");Serial.println(RMCdata.CHECKSUM);
	return;
}

/* ------------------------------------------------------------ */
/*  formatVTG()
**
**  Parameters:
**	  data: an array to the data to be formatted
**	  VTGdata: a pointer to the VTG_DATA structure to store the parsed data in
**
**  Return Value:
**    none
**
**  Errors:
**    none
**
**  Description:
**    Formats a mode's data into elements in a struct.
*/
void GPS::formatVTG(char* data_array)
{
	enum cases {COURSE_T, REF_T, COURSE_M, REF_M, SPD_N, UNIT_N, SPD_KM, UNIT_KM, MODE, CHECKSUM};
	cases datamember=COURSE_T;
	char* start_ptr;
	char* end_ptr = data_array+7;//Set start pointer after the message ID ("$GPGGA,")
	bool flag=1;
	char checksum[3]={0};
	
	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *end_ptr!='*')end_ptr++;//Increment ptr until a comma is found
		
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')){//End reached
			flag=0;
			break;
		}
		//Choose where to put this data
		switch(datamember){
			case COURSE_T:
				memcpy(VTGdata.COURSE_T, start_ptr, (end_ptr - start_ptr));
				VTGdata.COURSE_T[end_ptr - start_ptr] = '\0';//End null char
				datamember = REF_T;
				break;
			case REF_T:
				if (*start_ptr!=',')VTGdata.REF_T = *start_ptr;
				datamember=COURSE_M;
				break;
			case COURSE_M:
				memcpy(VTGdata.COURSE_M, start_ptr, (end_ptr - start_ptr));
				VTGdata.COURSE_M[end_ptr - start_ptr] = '\0';//End null char
				datamember=REF_M;
				break;
			case REF_M:
				if (*start_ptr!=',')VTGdata.REF_M = *start_ptr;
				datamember=SPD_N;
				break;
			case SPD_N:
				memcpy(VTGdata.SPD_N, start_ptr, (end_ptr - start_ptr));
				VTGdata.SPD_N[end_ptr - start_ptr] = '\0';//End null char
				datamember=UNIT_N;
				break;
			case UNIT_N:
				if (*start_ptr!=',')VTGdata.UNIT_N = *start_ptr;
				datamember=SPD_KM;
				break;
			case SPD_KM:
				memcpy(VTGdata.SPD_KM, start_ptr, (end_ptr - start_ptr));
				VTGdata.SPD_KM[end_ptr - start_ptr] = '\0';//End null char
				datamember=UNIT_KM;
				break;
			case UNIT_KM:
				if (*start_ptr!=',')VTGdata.UNIT_KM = *start_ptr;
				datamember=MODE;
				break;
			case MODE:
				if (*start_ptr!=',')VTGdata.MODE = *start_ptr;
				datamember=CHECKSUM;
				flag=0;
				break;
			
			}
		end_ptr++;
	} //end of while 
		
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(VTGdata.CHECKSUM, checksum, 3);
	//Serial.print("CHECKSUM: ");Serial.println(VTGdata.CHECKSUM);
	return;
}

/* ------------------------------------------------------------ */
/*  char* formatCOORDS()
**
**  Parameters:
**	  coords: the un-formatted decimal representation of latitude or longitude
**
**  Return Value:
**    Correctly formatted coordinates in degrees, minutes, and seconds
**
**  Errors:
**    none
**
**  Description:
**    Formats a set of coordinates of the form XXXX.XXXX or XXXXX.XXXX
**	  into XX°XX'XX.XX" (or XXX°XX'XX.XX")
*/
void GPS::formatCOORDS(char* coords)
{
	char formatted[14]={0};
	int i=0;
	char* coordsstart= coords;
	
	while (*(coords)){
		
		formatted[i]=*coords;
		formatted[++i]; coords++;
		if (*(coords+2)=='.')
		{
			formatted[i]='°';//degrees symbol
			i++;
		}
		else if (*coords=='.')
		{
			formatted[i] = 39;// ' symbol for minutes
			i++;
			coords++;
		}
		else if (*(coords-3)=='.')
		{
			formatted[i]='.';//Decimal for seconds
			i++;
		}
		else if (*(coords-5)=='.')
		{
			formatted[i]='"';// " for seconds
			i++;
			formatted[i]=0;//Null char
		}
	}

	strcpy(coordsstart, formatted);

	//coords=formatted;
	return;
	
}
