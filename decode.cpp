
/***************************************************
# File Name:	decode.cpp
# Abstract:
# Author:	zhangzhelucky@126.com
# Update History:
#
# 2014-06-24	Restart this Project
#
#
****************************************************/

#include <iostream>
#include <fstream>

#include <dirent.h>
#include <vector>
#include <string>

#include <pthread.h>

  using namespace std;


#define _Event_Separator	0xFFFFFFFF
#define _Crate_Header		0xFFFF0000
#defile _Crate_Num		1


#define _GeoMask		0xf8000000
#define _GeoR			27


#define	_MarkMask		0x07000000
#define _MarkR			24

#define _MarKData		0x0
#define _MarkHeader		0x2
#define _MarkEnd		0x4
#define _MarkNotValid		0x6


#define	_MarkMask830		0x04000000
#define _MarkR830		26

#define _MarKData830		0x0
#define _MarkHeader830		0x1


#define _CrateMask		0x00ff0000
#define _CrateR			16


#define _ChlNumMask830		0x00fc0000
#define _ChlNumR830		18


#define _ChlMask830		0xf8000000
#define _ChlR830		27


#define _DataMark830		0x03ffffff


#define _ChlMask32		0x001f0000
#define _ChlR32			16


#define _ChlMask16		0x001e0000
#define _ChlR16			17


#define _DataMask		0x00000fff



#define _isUnderThresMask	0x00001000
#define _isOverflowMask		0x00002000
#define _isDataValidMask	0x00004000		



// ---------------------------------------------------



#define	_DATA_DIRE	"./source/"	// Dir of the data files 
#define _LOG_FILE	"log"		// The log file
#define _MAX_FILE_NUM	10e7		// Max of data files 
#define _BUF_SIZE	(5*1024*1024)	// Buffer size ( byte )
#define _BUF_NUM	2		// buffer number



struct sEventDataRegister
{
    unsigned int EventCounter;
    // Event Counter from the data file, it's different from
    // the global flag 'EC' who indicates the real number of
    // the events, it's discrete.

    int v830ac_3[32];
    short v785n_4[16];
    short v775n_5[16];
    /* 6 */
    short v775n_7[16];
    short v775n_8[16];
    /* 9 */
    short v785_10[32];
    /* 11 */
    short v792_12[32];
    short v792_13[32];
    /* 14 */
    short v792_15[32];
    short v792_16[32];
    /* 17 */
    short v785_18[32];
    short v785_19[32];

};

typedef sEventDataRegister sEData;


struct sDataBuf
{
    unsigned int* p_data;
    unsigned short m_size;	// Size of buffer in fect (int)
    sDataBuf* p_next;
};

struct cirQueue
{
    sDataBuf buf[ _BUF_NUM ];
    int mNumb;			// The number of full buffer
    sDataBuf* p_put;		// Pointer to put next buffer
    sDataBuf* p_get;		// Pointer to get next buffer

};



ofstream logFile;
unsigned int EC;		// Event Counter
unsigned int _FileSize;		// The Size of the file
cirQueue* pQue;			// Pointer to Data buffer
sEData dataReg;			// Data Register




// Scan current directory for data files  
int scanDir( vector<string>* );





int main(int argc, char *argv[])
{
    cout << "zhangzhelucky@126.com" << endl;

    cout << "Creating log file ... ";
    logFile.open( _LOG_FILE );
    if( ! logFile.is_open() )
    {
	cout << endl
	     << "Fail in creating log file. Exit." << endl;
	exit( 0 );
    }
    cout << "Done." << endl;
    logFile << "Log file start tracking." << endl;
    

    // Scan current directory ==========
    vector<string> file_names;
    int fileNum = scanDir( & file_names );
    if( _MAX_FILE_NUM <= fileNum )
    {
	cout << "Too Much Files Found." << endl;
	exit( 0 );
    }
    else if( 0 == fileNum )
    {
	cout << "No data file found." << endl;
	exit( 0 );
    }


    // List all files found to Screan ==========
    /*
    for (int i = 0; i < file_names.size() ; i++) 
    {
	cout << file_names.at(i) << '\t';
    }
    */

    logFile << fileNum << " files found." << endl;
    cout << fileNum << " files found."
	 << "Press Enter to Confirm. " << endl;
    getchar();



    // Prepare data buffer ==========
    for (int i = 0; i < _BUF_NUM ; i++)
    {
	pQue -> buf[i].p_data = new int[ _BUF_SIZE ];
	pQue -> buf[i].m_size = 0;
    } /* i */
    
    for (int i = 0; i < _BUF_NUM-1 ; i++) 
    {
	pQue->buf[i].p_next = & ( pQue->buf[i+1].p_data );
    } /* i */
    pQue->buf[ _BUF_NUM-1 ].p_next = & ( pQue->buf[0].p_data );

    pQue -> mNumb = 0;			// Empty queue
    pQue -> p_put = pQue -> buf[0];
    pQue -> p_get = pQue -> buf[0];




    // Open file for Decoding ==========
    for (int i = 0; i < file_names.size() ; i++)
    {
	logFile << "----- File --- " << i << " -----" << endl
		<< ">> File Name: " << file_names.at(i) << endl;
    	cout << endl 
	     << "--- Preparing for decoding file: " 
	     << file_names[i] << endl;

	// Decode current fule -----
	int temp = Decode( file_names.at(i) );

	cout << temp << "Events decoded in the file." << endl;
	logFile << ">> " << temp 
		<< " Events got in this file." << endl;

    }
















    // Close the log file ==========
    logFile.close();


    // Deletion for data buffer ==========
    for (int i = 0; i < _BUF_NUM ; i++)
    {
	delete[] pQue->buf[i].p_data;
    } /* i */

    return 0;
}



/***************************************************
# Abstract:	
# Input:	(string) fileName
	Data File Name
# Output:	(int) x
	>0	x Events loaded, Succeed in loading the hole file
	0	Fail in decoding the file, NO Events Loaded
	<0	(-x) Events loaded, BUT interrupted inside the file

	    The reason who caused the interruption will be record
	 in the Log file.
****************************************************/
int Decode( string fileName )
{

    // Open the data file ==========
    logFile << ">> Opening data file." << endl;
    string FileStr = "source//" + fileName;
    ifstream file( FileStr.c_str() , ios::binary ); 

    if( ! file.is_open() )
    {
	cout << "File in opening file." << endl;
	logFile << ">> (E) Fail in opening file." << endl;
	return 0;
    }
    logFile << ">> Done." << endl;


    // Prepare the Event Counter and the File Size ========== 
    EC = 0;				// Event Counter
    file.seekg( 0 , ios::end );		// Seek to the end
    _FileSize = file.tellg();		// The Size of the file
    file.seekg( ios::beg );		// Seek back
    

    // Get file header and the comments ==========
    char comment[1024];
    datafile.get( comment , 1024 );
    logFile << ">> File comments: " << comment  << endl;

    datafile.seekg(1024 ,ios::beg);	// Skip the header 
    

    // Check the first Event Separator ==========
    unsigned int temp;
    file.read( (char*) (&temp) , sizeof(temp))
    if( _Event_Separator != temp )
    {
	logFile << ">> (E) First event separator missing. "<< endl;
	return 0;
    }
    
    














}



/***************************************************
# Abstract:	
# Input:	(ifstream*) pFile
	Pointer to the position of the data file will be read
# Output:	(bool)
	Indicates whether current file is end
	TRUE for end
****************************************************/
bool LoadBuf( ifstream* pFile)
{
    assert( (pQue->mNumb)>= 0 && (pQue->mNumb)<= _BUF_NUM );

    // Wait for empty buffer ========== 
    while (  _BUF_NUM == pQue->mNumb ) 
    {
	// Just wait here if all the buffer is full
    } 


    // Load buffer to Memory ========== 
    unsigned int fileRemain;	// Get how much data remain
    fileRemain = _FileSize - pFile->tellg() ;

    // Switch for different case of "File Remain"
    if( 0 == fileRemain )
    {
	// ----- Case 1 ------------------------------------
	// When the file is just end 
	return true;
    }
    else if( fileRemain <= _BUF_SIZE )
    {
	// ----- Case 2 ------------------------------------
	// When the file remaining is NOT enough for a hole buffer
	// which is sized by Mocra _BUF_SIZE
	pFile.read( (char*)(pQue->p_put->p_data) , fileRemain );


	// Load buffer size in fect ==========
	pQue->p_put->m_size = ( fileRemain / sizeof(int) );


	// Change the pointer to the next buffer ==========
	pQue->p_put = pQue->p_put->p_next;


	// Change the number of full empty ==========
	++ ( pQue->p_put->mNumb );

	return true;

    }
    else
    {
	// ----- Case 3 -------------------------------------
	// When there is still enough room for a hole buffer

	pFile.read( (char*)(pQue->p_put->p_data) , _BUF_SIZE );


	// Check the last _Event_Separator ========== 
	int iter;
	for ( iter = _BUF_SIZE -1; iter > 0; iter--) 
	{
	    if( _Event_Separator == pQue->p_put->p_data[iter] )
	    {
		break;
	    }
	} /* iter */

	// Load buffer size in fect ==========
	pQue->p_put->m_size = iter + 1;


	// Change the pointer to the next buffer ==========
	pQue->p_put = pQue->p_put->p_next;


	// Change the file position ==========
	pFile->seekg( -( (_BUF_SIZE- 1- iter)*sizeof(int)) ,
		      ios::end );


	// Change the number of full empty ==========
	++ ( pQue->p_put->mNumb );

	return false;
    }

}


/***************************************************
# Abstract:
# Input:	
# Output:	
****************************************************/
bool BufDecode()
{
    assert( (pQue->mNumb)>= 0 && (pQue->mNumb)<= _BUF_NUM );

    // Wait for prepared buffer ========== 
    while ( 0 == pQue->mNumb )
    {
	// Just wait here if all the buffer is empty
    } 


    // Decode the buffer for series of events ==========
    int iter = 0;
    while ( iter < (pQue->p_get->m_size) )
    {
	
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// Event decode >>>>>>>>>>>>>>>>>>>>>>>>>




    } /* while */


    // Change the pointer to the next buffer ==========
    pQue->p_get = pQue->p_get->p_next;


    // Change the number of full empty ==========
    -- ( pQue->p_get->mNumb );









}



/***************************************************
# Abstract:
# Input:	(unsigned int *) buf
			Pointer to the data buffer
		(short) size
			The size of the data buffer
# Output:	(bool)

****************************************************/
bool EventDecode( unsigned int* buf, short size)
{

    if( size < 9  )
    {
	// The event size is even smaller than the event header
	logFile << ">> (W) Too small event. " << endl;
	return false;
    }

    unsigned int* iter = buf;


    // Decode the event header ==========
    dataReg.EventCounter = *( iter++ );
    

    // Check the crate header and number ==========
    if( _Crate_Header != *( iter ++ ) )
    {
	// Crate header missing ------
	logFile << ">> (W) On Event Number ( " 
		<< dataReg.EventCounter
		<< " ). Crate header missing. "<< endl;
	return false;
    }

    if( _Crate_Num != *( iter ++))
    {
	// Crate number Error ------
	logFile << ">> (W) On Event Number ( "
		<< dataReg.EventCounter
		<< " ). Crate number Error. " << endl;
	return false;
    }

    
    // Skip the crate header ==========
    iter += 6;


    // Decode moduls ========
    while ( iter < ( buf + size ) )
    {
	// Decode the modul header ------
	short geo;		// The 'Goe' of the modul
	short mark;		// The 'Mark'
	short crateNum;		// The 'Crate Number'
	short chlNum;		// How much channel hit on modul

	// Get geo, mark, crateNum and chlNum ------
	geo = ( ( *iter ) & _GeoMask ) >> _GeoR;

	if( 3 == geo )	
	{
	    // For Modul 830ac ///////////////////////////////
	    // 
	    mark = ( ( *iter ) & _MarkMask830 ) >> _MarkR830;

	    // Check mark ------
	    if( _MarkHeader830 != mark )
	    {
		// Check event header mark fail
		logFile << ">> (W) On Event Number ( "
			<< dataReg.EventCounter
			<< " ). Check event header mark Error. "
			<< endl;
		return false;
	    }

	    chlNum =( (*iter) & _ChlNumMask830 ) >>_ChlNumR830;
	    crateNum = 1;

	}
	else
	{
	    // For Modul other than 830ac ///////////////////
	    // 
	    mark = ( ( *iter ) & _MarkMask ) >> _MarkR;

	    // Check mark ------
	    if( _MarkHeader != mark )
	    {
		// Check event header mark fail
		logFile << ">> (W) On Event Number ( "
			<< dataReg.EventCounter
			<< " ). Check event header mark Error. " 
			<< endl;
		return false;
	    }

	    chlNum = ( ( *iter ) & _ChlNumMask ) >> _ChlNumR;
	    crateNum = ( ( *iter ) & _CrateMask ) >> _CrateR;

	}

	// Check gro ------
	// if(  )
	// {
	//     // Check event header mark fail
	//     logFile << ">> (W) On Event Number ( "
	// 	    << dataReg.EventCounter
	// 	    << " ). Check event geo Error. " 
	// 	    << endl;
	//     return false;
	// }


	// Check crate number ------
	if( _Crate_Num != crateNum )
	{
	    // Check event header mark fail
	    logFile << ">> (W) On Event Number ( "
		    << dataReg.EventCounter
		    << " ). Check crate number Error. " 
		    << endl;
	    return false;
	}
	

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>Switch to different modul

	switch( geo )
	{
	  case 3 :

	    break;

	  case 4 :

	    break;

	  case 5 : 

	    break;

	  case 7 : 

	    break;

	  case 8 : 

	    break;

	  case 10 : 

	    break;

	  case 12 : 

	    break;

	  case 13 : 

	    break;

	  case 15 : 

	    break;

	  case 16 : 

	    break;

	  case 18 :

	    break;

	  case 19 :

	    break;


	  default: break;

	}	/* switch */








	


    } /* while */




}



// Decode v830ac ==========
unsigned int* Decode_v830ac( unsigned int* iter,
			     short chlNum, 
			     int* reg)
{
    short chl;
    int tempdata;
    for (int i = 0; i< chlNum; i++) 
    {
	chl = ( (*iter) & _ChlMask830 ) >> _ChlR830;
	tempdata =  (*iter) & _DataMask830 ;

	reg[ chl ] = tempdata;
	++ iter;
    } /* i */
    
    return iter;

}



// Decode v792 ==========
unsigned int* Decode_v792( unsigned int* iter,
			   short chlNum, 
			   int* reg)
{
    short chl;
    int tempdata;
    for (int i = 0; i< chlNum; i++) 
    {
	chl = ( (*iter) & _ChlMask32 ) >> _ChlR32;
	tempdata =  (*iter) & _DataMask ;
	reg[ chl ] = tempdata;

	if( (*iter) & _isUnderThresMask )
	{
	    // Data under lower thershold
	    reg[ chl ] = -1;
	}

	if( (*iter) & _isOverflowMask )
	{
	    // Data overflew
	    reg[ chl ] = -2;
	}

	++ iter;
    } /* i */
    
    return iter;

}



// Decode v785n ==========
unsigned int* Decode_v785n( unsigned int* iter,
			    short chlNum, 
			    int* reg)
{
    short chl;
    int tempdata;
    for (int i = 0; i< chlNum; i++) 
    {
	chl = ( (*iter) & _ChlMask16 ) >> _ChlR16;

	tempdata =  (*iter) & _DataMask ;
	reg[ chl ] = tempdata;

	if( (*iter) & _isUnderThresMask )
	{
	    // Data under lower thershold
	    reg[ chl ] = -1;
	}

	if( (*iter) & _isOverflowMask )
	{
	    // Data overflew
	    reg[ chl ] = -2;
	}

	++ iter;
    } /* i */
    
    return iter;

}



// Decode v785 ==========
unsigned int* Decode_v785( unsigned int* iter,
			   short chlNum, 
			   int* reg)
{
    short chl;
    int tempdata;
    for (int i = 0; i< chlNum; i++) 
    {
	chl = ( (*iter) & _ChlMask32 ) >> _ChlR32;
	tempdata =  (*iter) & _DataMask;
	reg[ chl ] = tempdata;

	if( (*iter) & _isUnderThresMask )
	{
	    // Data under lower thershold
	    reg[ chl ] = -1;
	}

	if( (*iter) & _isOverflowMask )
	{
	    // Data overflew
	    reg[ chl ] = -2;
	}

	++ iter;
    } /* i */
    
    return iter;

}



// Decode v775n ==========
unsigned int* Decode_v775n( unsigned int* iter,
			    short chlNum, 
			    int* reg)
{
    short chl;
    int tempdata;
    for (int i = 0; i< chlNum; i++) 
    {
	chl = ( (*iter) & _ChlMask16 ) >> _ChlR16;
	tempdata =  (*iter) & _DataMask;
	reg[ chl ] = tempdata;

	if( !( (*iter) & _isDataValidMask ) )
	{
	    // Data unvalid
	    reg[ chl ] = -4;
	}

	if( (*iter) & _isUnderThresMask )
	{
	    // Data under lower thershold
	    reg[ chl ] = -1;
	}

	if( (*iter) & _isOverflowMask )
	{
	    // Data overflew
	    reg[ chl ] = -2;
	}


	++ iter;
    } /* i */
    
    return iter;

}



// Scan for data files
int scanDir(vector<string>* file_names)
{
    int file_counter = 0;
    DIR* dp;
    struct dirent* dirp;
    char dirname[] = _DATA_DIRE ;

    if( ( dp = opendir(dirname) ) == NULL )
    {
	cout << "Wrong Directory! Check data directory setting." 
	     << endl;
	return 0;
    }

    while( (dirp=readdir(dp) ) != NULL )
    {
	string tempStr = dirp->d_name;
	if( ("." == tempStr) || (".." == tempStr) )	
	  continue;
	file_names->push_back( dirp->d_name );	    
	file_counter ++;
    }

    sort( file_names->begin() , file_names->end() );
    return file_counter;

}



// Load a data Buffer



