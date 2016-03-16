#pragma warning(disable:4530)

#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <soratime.h>

#include "umxsdr.h"
#include "phy.h"
#include "DebugPlotU.h"

// performance monitor
TIMESTAMPINFO        tsinfo;
ULONGLONG            tts1, tts2;

int gDisplayMode    = DISPLAY_PAGE;
int gPHYMode		= PHY_802_11A;	
int gSampleRate		= 40;
int gDataRateK		= 6000; // in Kbps
ULONG gChannelFreq  = 5200;
bool WRITE_FLAG=0;
void Dot11_main ();

bool bWarning = false;

void usage () {
	printf ( "umxsdr cmd parameter \n" );
	printf ( "--------------------------------------------------\n" );
	printf ( "   -s  [40|44]  -    sampling rate (MHz) \n" );
	printf ( "   -f  freq     -    central frequency (MHz)\n" );
	printf ( "   -r  rate     -    sending data rate (Kbps)\n" );
	printf ( "   -b           -    use 802.11b PHY\n" );
	printf ( "   -o  [0|1]    -    0 - status; 1 - logs \n" );
	printf ( "   -h           -    display this page\n" );
}

bool CheckSampleRate () {
	if ( gSampleRate != 40 &&
 		 gSampleRate != 44 ) {
 		printf("Invalid SampleRate\n");
 		 return false;
	}	 

	return true;
}

bool CheckDataRate () {
	if ( gPHYMode == PHY_802_11A ) {
		switch(gDataRateK) {
		case  6000:
		case  9000:
		case 12000:
		case 18000:
		case 24000:
		case 36000:
		case 48000:
		case 54000:
			break;
		default:
			printf( "Warning: data rate (%dKbps) is not supported in 802.11a.\n"
				    "Data rate 6Mbps is used instead.\n\n",
				    gDataRateK );
			gDataRateK = 6000;
			bWarning = true;
		}
	}	else {
		switch(gDataRateK) {
		case  1000:
		case  2000:
		case  5500:
		case 11000:
			break;
        default:
			printf("Warning: data rate (%dKbps) is not supported in 802.11b.\n",
				gDataRateK );
			gDataRateK = 1000;
			printf( "Data rate 1Mbps is used instead.\n\n" );
			bWarning = true;
		}
	}
	return true;
}

bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	while ( i< argc ) {
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 's' ) 
		{
			if ( ++i < argc ) {
				gSampleRate = atoi(argv[i++]);
				if (!CheckSampleRate ()) return false;
			} else {
				return false;
			}
		} else 
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'r' ) 
		{
			if ( ++i < argc ) {
				gDataRateK = atoi(argv[i++]);
			} else {
				return false;
			}
		} else if ( argv[i][0] == '-' 
			 && argv[i][1] == 'f' ) 
		{
			if ( ++i < argc ) {
				gChannelFreq = atoi(argv[i++]);
			} else {
				return false;
			}
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'b')
		{
			gPHYMode = PHY_802_11B; 
			i++;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'o')
		{
			if ( ++i < argc ) {
				gDisplayMode = atoi(argv[i++]);
			} else return false;
			if ( gDisplayMode > 1 ) return false;
		} else if ( argv[i][0] == '-'
			&& argv[i][1] == 'h')
		{
			return false;
		}
		else {
			return false;
		}
	}
	return true;
}


bool CheckParameters () 
{
	if ( gPHYMode == PHY_802_11B ) {
		// 11b
		if ( gSampleRate == 40 ) {
			printf ( "Warning: sampling rate (%dMHz) is not compatible with 802.11b PHY.\n"
					 "You may use this configurate to communicate to another Sora node with the same setting. "
					 "But it may fail to communicate with a standard 802.11b NIC.\n\n", 
				gSampleRate );
		}

		bWarning = true;
		return CheckDataRate ();
		
	} else {
		// 11a
		return CheckDataRate ();
	}
}

int __cdecl main(int argc, const char *argv[])
{
	if ( ParseCmdLine (argc, argv) == false )
	{
		usage();
		return 0;
	}

	if ( CheckParameters () == false ) {
		return 0;
	}

	// init DebugPlot library
	DebugPlotInit();

	InitializeTimestampInfo ( &tsinfo, true );
	
	// begin receive or transmit packets
	if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
		printf ( "Error: fail to find the hwtest driver!\n" );
		getchar();
		return -1;
	}

	if ( bWarning ) {
		printf ( "Warnings - press enter to continue...\n" );
		getchar();
	}	
	
	// clear screen	
	system( "cls" );	
	
	// Start the main procedure
	Dot11_main ();

	// clear screen
	// system( "cls" );	
	
	SoraUCleanUserExtension();
	DebugPlotDeinit();

    return 0;
}
