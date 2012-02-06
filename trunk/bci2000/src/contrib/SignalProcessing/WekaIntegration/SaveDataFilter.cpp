////////////////////////////////////////////////////////////////////////////////
//  $Id: SaveDataFilter.h 1723 2008-04-16 17:46:33Z villalon $
//  Author:      villalon@ee.usyd.edu.au
//  Description: A filter to save the signals after applying the BCI2000 filters
//
// (C) 2000-2008, Learning Systems Group, University of Sydney
// http://www.weg.usyd.edu.au
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "SaveDataFilter.h"

#include "MeasurementUnits.h"
#include "BCIError.h"
#include "defines.h"
#include <vector>
#include <cmath>
#include <limits>

using namespace std;

RegisterFilter( SaveDataFilter, 2.B1 );

SaveDataFilter::SaveDataFilter()
{
  BEGIN_PARAMETER_DEFINITIONS

	"Filtering string FilePrefix= NewFile NewFile a z "
	  "// A prefix for the file that will be written",

  END_PARAMETER_DEFINITIONS
}


SaveDataFilter::~SaveDataFilter()
{
    myFile.close();
}


void
SaveDataFilter::Preflight( const SignalProperties& Input,
                           SignalProperties& Output ) const
{
  Parameter( "FilePrefix" );
  Parameter( "SubjectName" );
  Parameter( "DataDirectory" );
  Parameter( "SubjectSession" );
  Parameter( "SubjectRun" );
  Parameter( "SavePrmFile" );

/*  if( LPTimeConstant < 0 )
	bcierr << "The LPTimeConstant parameter must be 0 or greater" << endl;*/

  // Request output signal properties:
  Output = Input;
}


void
SaveDataFilter::Initialize( const SignalProperties& Input,
                      const SignalProperties& /*Output*/ )
{
  string subjectName = Parameter( "SubjectName" );
  subjectName = "PostFilter" + subjectName;
  string baseFileName = BCIDirectory()
	.SetDataDirectory( Parameter( "DataDirectory" ) )
	.SetSubjectName( subjectName )
	.SetSessionNumber( Parameter( "SubjectSession" ) )
	.SetRunNumber( Parameter( "SubjectRun" ) )
	.SetFileExtension( ".dat" )
	.CreatePath()
	.FilePath();

  string prefix = Parameter( "FilePrefix" );
  prefix += ".dat";
  myFile.open(baseFileName.c_str() , ios::out | ios::app | ios::binary);
}


void
SaveDataFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
 char separator[8];
 for(int i=0; i<8; i++)
 separator[i] = 0;
  // This implements the second line for all channels:
  for( int channel = 0; channel < Input.Channels(); ++channel )
	for( int sample = 0; sample < Input.Elements(); ++sample )
	{
	  Input.WriteValueBinary(myFile, channel, sample);
	  myFile.write(separator, 8);
	}

  Output = Input;
}

