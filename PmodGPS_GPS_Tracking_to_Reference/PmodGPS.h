/************************************************************************/
/*																											*/
/*	PmodGPS.h  This library supports the PmodGPS Module,					*/
/*                  in particular, providing an easy means of using   			*/
/*					the module                                										*/
/*																											*/
/************************************************************************/
/*	Authors: 	Ian Brech, Thomas Kappenman 									*/
/*	Copyright 2014, Digilent Inc.															*/
/************************************************************************/
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
/************************************************************************/
/*  Module Description: 																		*/
/*																											*/
/*	This the Static PmodGPS Header file         										*/
/*																											*/
/************************************************************************/
/*  Revision History:																			*/
/*																											*/
/*	 5/15/2014(IanB): Created																	*/
/*	 6/23/2014(TommyK): Added data parsing and completed				*/
/*										library with additional documentation				*/
/*	  7/31/2014(SamL): reviewed for release											*/
/*																										*/
/************************************************************************/

#ifndef PmodGPS_H
#define PmodGPS_H

#include "Arduino.h"
#include "HardwareSerial.h"

#define MAX_SIZE  128

/***********************************************
 * Module Object Class Type Declarations       *
 **********************************************/

typedef enum{
	INVALID = 0,
	GGA,	    	//Time, position, and fix type data
	GSA,			//Operating mode, active satellites, DOP values
	GSV,			//Satellites in view, satellite ID numbers, elevation, azimuth, SNR values
	RMC,			//Recommended minimum navigation information
	VTG				//course and speed relative to ground
} NMEA;

typedef struct SATELLITE_T{
	int ID;		//Satellite ID
	int ELV;	//Satellite Elevation in degrees (90° max)
	int AZM;	//Satellite Azimuth, degrees from true north (0° to 359°)
	int SNR;	//Satellite Signal to noise ratio, 0-99 dB
}SATELLITE;

typedef struct GGA_DATA_T{
	char UTC[11];				//UTC Time
	char LAT[14];				//Latitude
	char NS;						//North or south indicator
	char LONG[15];			//Longitude
	char EW;					//East or west indicator
	char PFI;						//Position fixed indicator
	char NUMSAT[3];		//Number of satellites used
	char HDOP[5];			//HDOP
	char ALT[10];				//MSL Altitude
	char AUNIT;				//Units
	char GSEP[5];			//Geoidal Separation
	char GUNIT;				//Units
	char AODC[11];			//Age of Diff. Corr.
	char CHECKSUM[3];	//Checksum
} GGA_DATA;

typedef struct GSA_DATA_T{
	char MODE1;				//'M' for manual- forced to operate in 2D or 3D mode
									//'A' for 2D automatic- switches automatically b/w 2D/3D
	char MODE2;				// 1 - fix not available
									// 2 - 2D (<4SVs used)
									// 3 - 3D(>=4 SVs used)
	char SAT1[3];				// Satellite Used (SV) (channel 1)
	char SAT2[3];				// Satellite Used (SV) (channel 2)
	char SAT3[3];				// Satellite Used (SV) (channel 3)
	char SAT4[3];				// Satellite Used (SV) (channel 4)
	char SAT5[3];				// Satellite Used (SV) (channel 5)
	char SAT6[3];				// Satellite Used (SV) (channel 6)
	char SAT7[3];				// Satellite Used (SV) (channel 7)
	char SAT8[3];				// Satellite Used (SV) (channel 8)
	char SAT9[3];				// Satellite Used (SV) (channel 9)
	char SAT10[3];			// Satellite Used (SV) (channel 10)
	char SAT11[3];			// Satellite Used (SV) (channel 11)
	char SAT12[3];			// Satellite Used (SV) (channel 12)
	char PDOP[4];			// Positional dilution of precision
	char HDOP[4];			// Horizontal dilution of precision
	char VDOP[4];			// Vertical Dilution of precision
	char CHECKSUM[3];	//checksum
} GSA_DATA;

typedef struct GSV_DATA_T{
	int NUMM;					//Number of messages
	int MESNUM;				//Message number
	int SATVIEW;				//Satellites in view
	SATELLITE SAT[15];	//Satellite info
	char CHECKSUM[3];	//checksum

} GSV_DATA;

typedef struct RMC_DATA_T{
	char UTC[11];				//UTC Time
	char STAT;					//Status: A = data valid, V = data not valid
	char LAT[14];				//Latitude
	char NS;						//N/S indicator
	char LONG[14];			//Longitude
	char EW;					//E/W indicator
	char SOG[10];			//Speed over ground (knots)
	char COG[7];				//Course over ground (degrees)
	char DATE[7];				//Date
	char MVAR[7];				//Magnetic Variation (degrees)
	char MVARDIR;			//Magnetic Variation direction
	char MODE;				//A: Autonomous mode
									//D: Differential mode
									//E: Estimated mode
	char CHECKSUM[3];	//checksum
} RMC_DATA;

typedef struct VTG_DATA_T{
	char COURSE_T[7];	//measured heading
	char REF_T;				//True (T)
	char COURSE_M[7];	//measured heading
	char REF_M;				//Magnetic (M)
	char SPD_N[7];			//Measured speed (knots)
	char UNIT_N;				//Knots (N)
	char SPD_KM[7];		//Measured speed (km/h)
	char UNIT_KM;			//km/hr (K)
	char MODE;				//A: Autonomous mode
									//D: Differential mode
									//E: Estimated mode
	char CHECKSUM[3];	//checksum
} VTG_DATA;


/*******************
 * GPS Class
 ******************/

class GPS
{
	public:
	void GPSinit(HardwareSerial &serialPort, unsigned long baud, uint8_t DF, uint8_t PPS);
	void GPSinit(HardwareSerial &serialPort, unsigned long baud, uint8_t DF, uint8_t PPS, uint8_t RST);
	
	NMEA getData(HardwareSerial &serialPort);
	
	bool isFixed();	
	char* getLatitude();
	char* getLongitude();
	char* getDate();
	double getAltitude();
	char* getAltitudeString();
	double getTime();
	int getNumSats();
	double getPDOP();
	double getSpeedKnots();
	double getSpeedKM();
	double getHeading();
	SATELLITE* getSatelliteInfo();
	
	GGA_DATA getGGA();
	GSA_DATA getGSA();
	GSV_DATA getGSV();
	RMC_DATA getRMC();
	VTG_DATA getVTG();

	private:	
	NMEA chooseMode(char recv[MAX_SIZE]);
	void formatGGA(char* data_array);
	void formatGSA(char* data_array);
	void formatGSV(char* data_array);
	void formatRMC(char* data_array);
	void formatVTG(char* data_array);
	void formatCOORDS(char* coords);
	


	GGA_DATA GGAdata;
	GSA_DATA GSAdata;
	GSV_DATA GSVdata;
	RMC_DATA RMCdata;
	VTG_DATA VTGdata;
	
};

#endif //PmodGPS_H
