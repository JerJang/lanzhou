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




#define	_DATA_DIRE	"./source/"	// Dir of the data files 
#define _LOG_FILE	"log"		// The log file
#define _MAX_FILE_NUM	10e7		// Max of data files 
#define _BUF_SIZE	5*1024*1024	// Buffer size ( byte )
#define _BUF_NUM	2		// buffer number




struct sDataBuf
{
    unsigned int* p_data;
    unsigned short m_size;
    item* p_next;
};

struct cirQueue
{
    sDataBuf buf[ _BUF_NUM ];
    int mNumb;			// The number of full buffer
    unsigned int* p_put;	// Pointer to put next buffer
    unsigned int* p_get;	// Pointer to get next buffer

};







ofstream logFile;
unsigned int EC;		// Event Counter
unsigned int FileSize;		// The Size of the file
cirQueue* pQue;			// Pointer to Data buffer







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
    pQue -> p_put = pQue -> buf[0].p_data;
    pQue -> p_get = pQue -> buf[0].p_data;




    // Open file for Decoding ==========
    for (int i = 0; i < file_names.size() ; i++)
    {
	logFile << "----- File --- " << i << " -----" << endl
		<< ">> File Name: " << file_names.at(i) << endl;
    	cout << endl 
	     << "--- Preparing for decoding file: " 
	     << file_names[i] << endl;

	// Decode current fule -----
	int temp = fileDecode( file_names.at(i) );

	cout << temp << "Events decoded in the file." << endl;
	logFile << ">> " << temp 
		<< " Events got in this file." << endl;

    }



















    // Deletion for data buffer ==========
    for (int i = 0; i < _BUF_NUM ; i++)
    {
	delete[] pQue->buf[i].p_data;
    } /* i */

    return 0;
}



/***************************************************
# Abstract:	
# Input:	string fileName
	Data File Name
# Output:	int x
	>0	x Events loaded, Succeed in loading the hole file
	0	Fail in decoding the file, NO Events Loaded
	<0	(-x) Events loaded, BUT interrupted inside the file

	    The reason who caused the interruption will be record
	 in the Log file.
****************************************************/
int fileDecode( string fileName )
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
    FileSize = file.tellg();		// The Size of the file
    file.seekg( ios::beg );		// Seek back
    

    // Get file header and the comments ==========
    char comment[1024];
    datafile.get( comment , 1024 );
    logFile << ">> File Comments: " << comment  << endl;

    datafile.seekg(1024 ,ios::beg);	// Skip the header 
    

    // Check the first Event Separator ==========
    unsigned int temp;
    file.read( (char*) (&temp) , sizeof(temp))
    if( _Event_Separator != temp )
    {
	logFile << ">> (E) First Event Separator Missing. "<< endl;
	return 0;
    }
    
    














}



/***************************************************
# Abstract:	
# Input:	ifstream* file
	Pointer to the position of the data file will be read
# Output:	unsigned int x
	Indicates how much data loaded in fect
****************************************************/
unsigned int LoadBuf( ifstream file)
{
    assert( (pQue->mNumb)>=0 && (pQue->mNumb)<=_BUF_NUM );

    // Wait for empty 
    while ( pQue->mNumb = _BUF_NUM ) 
    {
	// Just wait here if the buffer is full
    } 

    file.read( (char*) pQue->p_put , _BUF_SIZE );





}



// Scan for data files
int scanDir(vector<string>* file_names)
{
    int file_counter = 0;
    DIR *dp;
    struct dirent *dirp;
    char dirname[] = _DATA_DIRE ;

    if( ( dp=opendir(dirname) ) == NULL )
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



