
#include"DragonFireSDK.h"

///////////////////////////////////////////////////////////////////////

/*
typedef struct _GAMEDATA
{
	// high scores
	int HighScores[5];

	// high score names
	char Name1[16];
	char Name2[16];
	char Name3[16];
	char Name4[16];
	char Name5[16];

	// audio options
	bool UseSoundFX;
	bool UseMusic;

} GAMEDATA,*PGAMEDATA;

// game data objects
GAMEDATA gdGlobal;
GAMEDATA gdDefault;

void SetDefaultGameData(void)
{
	// set the default high scores
	gdDefault.HighScores[0] =10000;
	gdDefault.HighScores[1] =12345;
	gdDefault.HighScores[2] =24680;
	gdDefault.HighScores[3] =33333;
	gdDefault.HighScores[4] =54321;

	// set the default names
	sprintf(gdDefault.Name1, "Big Bob");
	sprintf(gdDefault.Name2, "Steve-o");
	sprintf(gdDefault.Name3, "Jimmy");
	sprintf(gdDefault.Name4, "Hank");
	sprintf(gdDefault.Name5, "Momma");

	// set default audio options
	gdDefault.UseSoundFX =true;
	gdDefault.UseMusic   =true;
}
*/

///////////////////////////////////////////////////////////////////////

/*
typedef struct _PLAYERDATA
{
	// player name
	char Name[16];
	
	// game stats
	int Gold;
	int Experience;
	int HitPoints;
	bool HasBigSword;

} PLAYERDATA,*PPLAYERDATA;

typedef struct _GAMEDATA
{
	// data for 3 players
	PLAYERDATA Players[3];

	// audio options
	bool UseSoundFX;
	bool UseMusic;

} GAMEDATA,*PGAMEDATA;

// game data objects
GAMEDATA gdGlobal;
GAMEDATA gdDefault;

void SetDefaultGameData(void)
{
	// set default player data
	for(int n=0;n<3;n++)
	{
		sprintf(gdDefault.Players[n].Name, "(Empty)");
		gdDefault.Players[n].Gold        =0;
		gdDefault.Players[n].Experience  =0;
		gdDefault.Players[n].HitPoints   =10;
		gdDefault.Players[n].HasBigSword =false;
	}

	// set default audio options
	gdDefault.UseSoundFX =true;
	gdDefault.UseMusic   =true;
}
*/

///////////////////////////////////////////////////////////////////////

typedef struct _GAMEDATA
{
	// Include variables for all data that needs to be saved here.
	// Remember NOT to include transient data, such as class objects
	// and/or pointers.  Also remember to include a default value for
	// each variable in the SetDefaultGameData() function.

	// example: (you can delete this)
	int nHiScore;

} GAMEDATA,*PGAMEDATA;

// game data objects
GAMEDATA gdGlobal;
GAMEDATA gdDefault;

// SetDefaultGameData() -- Use this function to provide a default
//						   value for every variable contained in
//						   the GAMEDATA structure.  This information
//						   will be used to provide game data for
//						   the first time your app is run on the
//                         device or any time the data becomes
//						   corrupted (which should be never, but
//						   we all know how computers can be).

void SetDefaultGameData(void)
{
	// Fill every variable in the gdDefault object here.  The values
	// in gdDefault should not be used or modified anywhere else in
	// your app.  For accessing and changing global data values
	// elsewhere, use the gdGlobal structure, since that is the
	// information that will be saved to disk.  Just to emphasize:
	// Only use gdDefault in this function!  Use gdGlobal everywhere
	// else in your app.

	// example: (you can delete this)
	gdDefault.nHiScore=10000;
}

///////////////////////////////////////////////////////////////////////
// The functions below perform all of the necessary operations for
// saving and loading the data.  When you want to save the data,
// such as when the user makes an options selection, reaches a
// significant point in the game, or during AppExit(), call the
// SaveGameData() function.  The LoadGameData() function should be
// called early in AppMain() and in most cases will not need to be
// called again.  Remember to provide a filename for SaveGameData()
// and LoadGameData() that does NOT have a file extension!  The file
// extension will be added for you (.sav for the main save and .bak
// for the backup).  For example, use SaveGameData("scores"), but do
// not use SaveGameData("scores.dat").

// includes //

#include<string.h>
#include<time.h>

// constants //

#define MAX_GD_STRING 27 // sets the maximum length of the game data filename

// structures //

// game data plus timestamp
typedef struct _GAMEDATA_TS
{
	// game data
	GAMEDATA gdData;
	// timestamp
	time_t ttStamp;
} GAMEDATA_TS,*PGAMEDATA_TS;

// prototypes //

void SaveGameData(const char* pFilename);

void LoadGameData(const char* pFilename);

char ComputeGameDataHash(PGAMEDATA_TS pGTData);

void CreateGameDataChecksums(PGAMEDATA_TS pGTData,
							 PGAMEDATA_TS pGTCheck);

void ValidateGameData(PGAMEDATA_TS pGTData,
					  PGAMEDATA_TS pGTCheck);

void InterlaceGameData(PGAMEDATA_TS pGTData,
					   PGAMEDATA_TS pGTCheck,
					   unsigned short* p16Data);

void DeinterlaceGameData(PGAMEDATA_TS pGTData,
					     PGAMEDATA_TS pGTCheck,
					     unsigned short* p16Data);

void SaveGameDataFile(char* pFilename,
					  unsigned short* p16Data);

void LoadGameDataFile(char* pFilename,
					  unsigned short* p16Data);

// functions //

// SaveGameData() -- redundantly save timestamped & encrypted game data to disk
void SaveGameData(const char* pFilename)
{
	// make sure there is data to save
	if(sizeof(GAMEDATA)==0)
		return;

	// make sure the filename is not too long
	if(strlen(pFilename)>MAX_GD_STRING)
	{
		// output debug message
		printf("SaveGameData() : Filename contains too many characters.\n");

		// stop here
		return;
	}

	// save both files (original and backup)
	for(int n=0;n<2;n++)
	{
		// game data save objects
		GAMEDATA_TS gtSaveData;
		GAMEDATA_TS gtSaveCheck;

		// we need a buffer 2x the size of GAMEDATA_TS, so use an array
		GAMEDATA_TS gtBuffer[2];

		// now cast the buffer to an array of 16-bit values
		unsigned short* p16Data=(unsigned short*)gtBuffer;

		// copy global data to local variable
		gtSaveData.gdData=gdGlobal;

		// set timestamp for save
		gtSaveData.ttStamp=time(0);
		
		// create checksums for save
		CreateGameDataChecksums(&gtSaveData,
								&gtSaveCheck);

		// encode the data
		InterlaceGameData(&gtSaveData,
						  &gtSaveCheck,
						  p16Data);

		// string for filename maniplation
		char pStr[MAX_GD_STRING+5]; // +5 allows bytes for .ext and null terminator

		// add the extenstion to the filename
		sprintf(pStr,
				"%s%s",
				pFilename,
				(n==1) ? ".sav" : ".bak");

		// save the file
		SaveGameDataFile(pStr,
						 p16Data);
	}
}

// LoadGameData() -- load most recent & valid game data from disk
void LoadGameData(const char* pFilename)
{
	// make sure there is data to load
	if(sizeof(GAMEDATA)==0)
		return;

	// make sure the filename is not too long
	if(strlen(pFilename)>MAX_GD_STRING)
	{
		// output debug message
		printf("LoadGameData() : Filename contains too many characters.\n");

		// stop here
		return;
	}

	// objects for storing loaded game data
	GAMEDATA_TS gtLoadData[2];

	// load both files (original and backup)
	for(int n=0;n<2;n++)
	{
		// we need a buffer 2x the size of GAMEDATA_TS, so use an array
		GAMEDATA_TS gtBuffer[2];

		// now cast the buffer to an array of 16-bit values
		unsigned short* p16Data=(unsigned short*)gtBuffer;

		// string for filename maniplation
		char pStr[MAX_GD_STRING+5]; // +5 allows bytes for .ext and null terminator

		// add the extenstion to the filename
		sprintf(pStr,
				"%s%s",
				pFilename,
				(n==1) ? ".sav" : ".bak");

		// load the file
		LoadGameDataFile(pStr,
						 p16Data);

		// this holds the checksums
		GAMEDATA_TS gtLoadCheck;

		// decode the data image
		DeinterlaceGameData(&gtLoadData[n],
						    &gtLoadCheck,
						    p16Data);
		
		// validate the loaded data
		ValidateGameData(&gtLoadData[n],
						 &gtLoadCheck);
	}

	// copy the most recent validated data to the global struct
	gdGlobal=(gtLoadData[0].ttStamp>gtLoadData[1].ttStamp) ? gtLoadData[0].gdData : gtLoadData[1].gdData;
}

// ComputeGameDataHash() -- create a unique number based on global game data
char ComputeGameDataHash(PGAMEDATA_TS pGTData)
{
	// set hash value to 0
	char cHash=0;
	
	// cast the game data to a char buffer
	char* pBuf=(char*)pGTData;

	// add all the bytes together to produce a unique number
	for(int n=0;n<sizeof(GAMEDATA_TS);n++)
		cHash+=pBuf[n];

	// return the number
	return(cHash);
}

// CreateGameDataChecksums() -- create checksum list for game data
void CreateGameDataChecksums(PGAMEDATA_TS pGTData,
							 PGAMEDATA_TS pGTCheck)
{
	// get the starting number
	char cHash=ComputeGameDataHash(pGTData);

	// cast the checksum data to a char buffer
	char* pCheckBuf=(char*)pGTCheck;

	// process each byte
	for(int n=0;n<sizeof(GAMEDATA_TS);n++)
	{
		// get a random number from 0-7
		char cRand=Random(8);

		// add the random value to the hash & truncate to 4 bits
		cHash=(cHash+cRand)&0x0F;

		// add the lower 3 bits of the byte number to the random number
		cRand+=(n&0x07);

		// store the combined hash (lower 4 bits) and the random value (upper 4 bits)
		pCheckBuf[n]=(cRand<<4)|cHash;
	}
}

// ValidateGameData() -- check the game data and set to default if needed
void ValidateGameData(PGAMEDATA_TS pGTData,
					  PGAMEDATA_TS pGTCheck)
{
	// flag for valid default data
	static bool bDefDataIsValid=false;

	// check default data
	if(!bDefDataIsValid)
	{
		// set the default data
		SetDefaultGameData();

		// set the flag
		bDefDataIsValid=true;
	}

	// get the starting number
	char cHash=ComputeGameDataHash(pGTData);

	// cast the checksum data to a char buffer
	char* pCheckBuf=(char*)pGTCheck;

	// flag to indicate failure
	bool bFail=false;

	// validate each byte
	for(int n=0;n<sizeof(GAMEDATA_TS);n++)
	{
		// extract the random value from the buffer
		int cRand=(pCheckBuf[n]&0xF0)>>4;

		// subract the lower 3 bits of the byte number from the random number;
		// this yields the original random number
		cRand-=(n&0x07);

		// add the random value to the hash & truncate to 4 bits
		cHash=(cHash+cRand)&0x0F;

		// compare this value with the stored hash (lower 4 bits)
		if(cHash!=(pCheckBuf[n]&0x0F))
		{
			// invalid byte, so throw the flag
			bFail=true;

			// stop the loop
			break;
		}
	}

	// check for failure
	if(bFail)
	{
		// output debug message
		printf("ValidateGameData() : Invalid checksum; using default game data.\n");

		// copy the default data
		pGTData->gdData=gdDefault;

		// set the timestamp to 0 (this way, if the other file is still
		// valid, its data will be used instead)
		pGTData->ttStamp=0;
	}
	else
	{
		// the data is valid, so leave it alone

		// output debug message
		printf("ValidateGameData() : Successfully loaded game data.\n");
	}
}

// InterlaceGameData() -- encode data and checksums for saving
void InterlaceGameData(PGAMEDATA_TS pGTData,
					   PGAMEDATA_TS pGTCheck,
					   unsigned short* p16Data)
{
	// cast data & checksums as uchar buffers
	unsigned char* cDataBuf  =(unsigned char*)pGTData;
	unsigned char* cCheckBuf =(unsigned char*)pGTCheck;

	// process each value
	for(int n=0;n<sizeof(GAMEDATA_TS);n++)
	{
		// encode the game data with a bitwise not (okay, so it's not exactly Ft. Knox)
		cDataBuf[n]=~cDataBuf[n];

		// shred the data for interlacing
		unsigned short n16Val1 =(((unsigned short)cDataBuf[n]) &0x000F)<<12; // bits 12-15
		unsigned short n16Val2 =(((unsigned short)cDataBuf[n]) &0x00F0)>>4;  // bits 0-3
		unsigned short n16Val3 =(((unsigned short)cCheckBuf[n])&0x000F)<<8;  // bits 8-11
		unsigned short n16Val4 =(((unsigned short)cCheckBuf[n])&0x00F0);     // bits 4-7

		// store the interlaced value
		p16Data[n]=n16Val1|n16Val2|n16Val3|n16Val4;
	}
}

// DeinterlaceGameData() -- decode the image back into data and checksums
void DeinterlaceGameData(PGAMEDATA_TS pGTData,
					     PGAMEDATA_TS pGTCheck,
					     unsigned short* p16Data)
{
	// cast data & checksums as uchar buffers
	unsigned char* cDataBuf  =(unsigned char*)pGTData;
	unsigned char* cCheckBuf =(unsigned char*)pGTCheck;

	// process each value
	for(int n=0;n<sizeof(GAMEDATA_TS);n++)
	{
		// un-shred the interlaced data
		unsigned short n16Val1 =(p16Data[n]&0xF000)>>12;
		unsigned short n16Val2 =(p16Data[n]&0x000F)<<4;
		unsigned short n16Val3 =(p16Data[n]&0x0F00)>>8;
		unsigned short n16Val4 =(p16Data[n]&0x00F0);

		// put the data back in its proper places
		cDataBuf[n]  =(unsigned char)(n16Val1|n16Val2);
		cCheckBuf[n] =(unsigned char)(n16Val3|n16Val4);

		// decode the game data with a bitwise not
		cDataBuf[n]=~cDataBuf[n];
	}
}

// SaveGameDataFile() -- save the encoded data image to disk
void SaveGameDataFile(char* pFilename,
					  unsigned short* p16Data)
{
	// create the file
	int hFile=FileCreate(pFilename);

	// output the data
	FileWrite(hFile,
			  (char*)p16Data,
			  sizeof(GAMEDATA_TS)*sizeof(short));

	// close the file
	FileClose(hFile);
}

// LoadGameDataFile() -- load encoded game data image from disk
void LoadGameDataFile(char* pFilename,
					  unsigned short* p16Data)
{
	// open the file
	int hFile=FileOpen(pFilename);

	// input the data
	FileRead(hFile,
			 (char*)p16Data,
			 sizeof(GAMEDATA_TS)*sizeof(short));

	// close the file
	FileClose(hFile);
}

///////////////////////////////////////////////////////////////////////

void AppMain(void)
{
	LoadGameData("scores");
}

void OnTimer(void)
{
	_asm
	{
	}
}

void AppExit(void)
{
	SaveGameData("scores");
}

// eof //