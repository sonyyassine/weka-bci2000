////////////////////////////////////////////////////////////////////////////////
// $Id: GDF.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A C++ representation of a BCI2000 relevant subset of the EDF
//              data format as defined in Kemp et al, 1992, and the
//              GDF 1.25 data format as defined in Schloegl et al, 1998.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GDF.h"

#include "BCIError.h"

#include <iomanip>
#include <string>
#include <ctime>
#include <limits>


using namespace std;

void
GDF::EncodedString::WriteToStream( std::ostream& os ) const
{
  if( empty() )
    os.put( 'X' );
  for( const_iterator i = begin(); i != end(); ++i )
    isspace( *i ) ? os.put( '_' ) : os.put( *i );
}

string
GDF::DateTimeToString( signed long long t )
{
  ostringstream oss;
  // Not all OSes interpret negative time_t values correctly as dates before
  // 1970. In this case (birthdays), we just report the corresponding year.
  if( t == cInvalidDate )
    oss << "XX-XXX-XXXX";
  else if( t < 0 || t > numeric_limits<time_t>::max() )
  {
    oss << "XX-XXX-" << 1970 + t / cSecondsPerYear;
  }
  else
  {
    static const char* monthNames[] =
    {
      "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
    };
    time_t timeVal = t;
    struct tm* time = localtime( &timeVal );
    oss << setw( 2 ) << setfill( '0' ) << time->tm_mday
        << '-' << monthNames[ time->tm_mon ]
        << '-' << 1900 + time->tm_year;
  }
  return oss.str();
}

