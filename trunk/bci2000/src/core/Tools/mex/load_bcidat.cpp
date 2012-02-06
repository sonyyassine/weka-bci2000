///////////////////////////////////////////////////////////////////////////////
// $Id: load_bcidat.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A Matlab (mex) subroutine that reads BCI2000 .dat files into
//  Matlab workspace variables.
//
//  [ signal, states, parameters, total_samples ] = load_bcidat( 'filename1', 'filename2', ... )
//
//  loads signal, state, and parameter data from the files whose names are given
//  as function arguments.
//
//  Examples for loading multiple files:
//    files = dir( '*.dat' );
//    [ signal, states, parameters ] = load_bcidat( files.name );
//
//    files = struct( 'name', uigetfile( 'MultiSelect', 'on' ) );
//    [ signal, states, parameters ] = load_bcidat( files.name );
//
//
//  For multiple files, number of channels, states, and signal type must be
//  consistent.
//
//  By default, signal data will be in raw A/D units, and will be represented by the
//  smallest Matlab data type that accommodates them.
//  To obtain signal data calibrated into physical units (microvolts),
//  specify "-calibrated" as an additional option anywhere in the argument list.
//
//  The 'states' output variable will be a Matlab struct with BCI2000 state
//  names as struct member names, and the number of state value entries matching
//  the first dimension of the 'signal' output variable.
//
//  The 'parameters' output variable will be a Matlab struct with BCI2000
//  parameter names as struct member names.
//  Individual parameters are represented as cell arrays of strings, and may
//  be converted into numeric matrices by Matlab's str2double function.
//  If multiple files are given, parameter values will match the parameters
//  contained in the first file.
//
//  The 'total_samples' output variable reports the total number of samples
//  present in all files.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"
#include "mexutils.h"
#include "BCI2000FileReader.h"
#include "BCIError.h"
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

typedef unsigned char      uint8;
typedef signed short       int16;
typedef unsigned short     uint16;
typedef signed int         int32;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef float              float32;

struct StateInfo
{
  int       location,
            length;
  union
  {
    uint8*  data8;
    uint16* data16;
    uint32* data32;
    uint64* data64;
  };
  mxClassID classID;
};

struct FileInfo
{
  int begin, // range of samples to read
      end;
  BCI2000FileReader* data;
};

struct FileContainer : public vector<FileInfo>
{
  public:
    FileContainer() {}
    ~FileContainer()
    { for( iterator i = begin(); i != end(); ++i ) delete i->data; }
  private:
    FileContainer( const FileContainer& );
    const FileContainer& operator=( const FileContainer& );
};

template<typename T, bool Raw>
void
ReadSignal( FileContainer& inFiles, mxArray* ioSignal )
{
  T* data = reinterpret_cast<T*>( mxGetData( ioSignal ) );
  int sampleOffset = 0,
      totalSamples = 0;
  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
    totalSamples += i->end - i->begin;

  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
  {
    BCI2000FileReader* file = i->data;
    int numSamples = i->end - i->begin,
        numChannels = file->SignalProperties().Channels();
    for( int sample = 0; sample < numSamples; ++sample )
      for( int channel = 0; channel < numChannels; ++channel )
        data[ totalSamples * channel + sample + sampleOffset ]
          = Raw ? file->RawValue( channel, sample + i->begin )
                : file->CalibratedValue( channel, sample + i->begin );
    sampleOffset += numSamples;
  }
}

void
mexFunction( int nargout, mxArray* varargout[],
             int nargin,  const mxArray* varargin[] )
{
  std::ios_base::Init();

  mxAssert(
    sizeof( int16 ) == 2 && sizeof( int32 ) == 4 && sizeof( float32 ) == 4
    && sizeof( uint8 ) == 1 && sizeof( uint16 ) == 2 && sizeof( uint32 ) == 4
    && sizeof( uint64 ) == 8,
    "Numeric types don't agree with this function's assumptions."
  );

  if( nargin < 1 )
    mexErrMsgTxt( "No file name given." );

  // Parse arguments and open files.
  bool rawData = true;
  FileContainer files;
  int i = 0;
  while( i < nargin )
  {
    if( mxGetClassID( varargin[ i ] ) != mxCHAR_CLASS )
    {
      ostringstream oss;
      oss << "File name or option expected in argument " << i + 1 << ".";
      mexErrMsgTxt( oss.str().c_str() );
    }
    char* stringArg = mxArrayToString( varargin[ i ] );
    if( stringArg == NULL )
      mexErrMsgTxt( "Out of memory when reading string argument." );
    else if( *stringArg == '-' )
    { // Parse options
      if( string( "-raw" ) == stringArg )
        rawData = true;
      else if( string( "-calibrated" ) == stringArg )
        rawData = false;
      else
      {
        ostringstream oss;
        oss << "Unknown option: " << stringArg;
        mexErrMsgTxt( oss.str().c_str() );
      }
    }
    else
    {
      BCI2000FileReader* file = new BCI2000FileReader;
      FileInfo fileInfo =
      {
        0, numeric_limits<int>::max(),
        file
      };
      files.push_back( fileInfo );
      file->Open( stringArg );
      if( !file->IsOpen() )
        file->Open( ( string( stringArg ) + ".dat" ).c_str() );
      if( !file->IsOpen() )
      {
        ostringstream oss;
        oss << "Could not open \"" << stringArg << "\" as a BCI2000 data file.";
        mexErrMsgTxt( oss.str().c_str() );
      }

      int samplesInFile = file->NumSamples(),
          begin = 0,
          end = numeric_limits<int>::max();

      if( ( i + 1 < nargin )
           && ( mxGetClassID( varargin[ i + 1 ] ) == mxDOUBLE_CLASS ) )
      {
        // If a file name is followed by a numeric argument, we interpret it as a sample range.
        // [1 0] and [0 0] are considered an empty range.
        ++i;
        const int* dim = mxGetDimensions( varargin[ i ] );
        int numEntries = dim[ 0 ] * dim[ 1 ];
        double* range = mxGetPr( varargin[ i ] );
        for( int j = 0; j < numEntries; ++j )
          if( floor( abs( range[ j ] ) ) != range[ j ] )
            mexErrMsgTxt( "Nonnegative integers expected in range vector." );

        if( numEntries > 0 )
          begin = range[ 0 ] - 1;
        if( numEntries > 1 )
          end = range[ 1 ];
      }
      if( begin == -1 && end == 0 ) // The [0 0] case.
        begin = 0;
      if( end > samplesInFile )
        end = samplesInFile;
      if( begin >= samplesInFile )
        begin = end;
      if( begin < 0 || ( end - begin ) < 0 )
      {
        ostringstream oss;
        oss << "Invalid sample range specified for file \"" << stringArg << "\".";
        mexErrMsgTxt( oss.str().c_str() );
      }
      if( begin > end )
        end = begin;
      files.rbegin()->begin = begin;
      files.rbegin()->end = end;
    }
    mxFree( stringArg );
    bcierr__.clear();
    ++i;
  }

  size_t totalSamples = files[ 0 ].end - files[ 0 ].begin;
  int numChannels = files[ 0 ].data->SignalProperties().Channels();
  SignalType dataType = files[ 0 ].data->SignalProperties().Type();
  mxClassID classID = mxDOUBLE_CLASS;
  if( rawData )
  {
    switch( dataType )
    {
      case SignalType::int16:
        classID = mxINT16_CLASS;
        break;

      case SignalType::int32:
        classID = mxINT32_CLASS;
        break;

      case SignalType::float32:
        classID = mxSINGLE_CLASS;
        break;

      default:
        classID = mxDOUBLE_CLASS;
    }
  }
  // Check whether files are compatible.
  const StateList* statelist = files[ 0 ].data->States();
  for( size_t i = 1; i < files.size(); ++i )
  {
    totalSamples += files[ i ].end - files[ i ].begin;
    if( files[ i ].data->SignalProperties().Channels() != numChannels )
      mexErrMsgTxt( "All input files must have identical numbers of channels." );

    if( files[ i ].data->SignalProperties().Type() != dataType )
      classID = mxDOUBLE_CLASS;

    if( nargout > 1 ) // The caller wants us to read state information.
      for( int j = 0; j < statelist->Size(); ++j )
        if( !files[ i ].data->States()->Exists( ( *statelist )[ j ].Name() ) )
          mexErrMsgTxt( "Incompatible state information across input files." );
  }

  // Read EEG data into the first output argument.
  int dim[] = { totalSamples, numChannels };
  mxArray* signal = mxCreateNumericArray( 2, dim, classID, mxREAL );
  if( signal == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for the signal variable." );
  switch( classID )
  {
    case mxINT16_CLASS:
      ReadSignal<int16, true>( files, signal );
      break;

    case mxINT32_CLASS:
      ReadSignal<int32, true>( files, signal );
      break;

    case mxSINGLE_CLASS:
      ReadSignal<float32, true>( files, signal );
      break;

    case mxDOUBLE_CLASS:
      if( rawData )
        ReadSignal<double, true>( files, signal );
      else
        ReadSignal<double, false>( files, signal );
      break;

    default:
      mexErrMsgTxt( "Unsupported class ID" );
  }
  bcierr__.clear();
  varargout[ 0 ] = signal;

  // Read state data if appropriate.
  if( nargout > 1 )
  {
    const StateList* mainStatelist = files[ 0 ].data->States();
    int numStates = mainStatelist->Size();
    char** stateNames = MexAlloc<char*>( numStates );
    StateInfo* stateInfo = MexAlloc<StateInfo>( numStates );
    for( int i = 0; i < numStates; ++i )
    {
      const State& s = ( *mainStatelist )[ i ];
      stateNames[ i ] = reinterpret_cast<char*>( mxMalloc( s.Name().length() + 1 ) );
      strcpy( stateNames[ i ], s.Name().c_str() );
    }
    mxArray* states = mxCreateStructMatrix(
      1, 1, numStates, const_cast<const char**>( stateNames )
    );
    for( int i = 0; i < numStates; ++i )
    {
      int maxLength = 0;
      for( FileContainer::iterator file = files.begin(); file != files.end(); ++file )
      {
        const State& s = file->data->StateVector()->StateList()[ stateNames[ i ] ];
        if( maxLength < s.Length() )
          maxLength = s.Length();
      }
      mxArray* stateArray = NULL;
      if( maxLength <= 8 )
      { // Calling mxCreateNumericMatrix with a mxClassID type variable rather than a
        // constant appears unreliable.
        stateInfo[ i ].classID = mxUINT8_CLASS;
        stateArray = mxCreateNumericMatrix( totalSamples, 1, mxUINT8_CLASS, mxREAL );
      }
      else if( maxLength <= 16 )
      {
        stateInfo[ i ].classID = mxUINT16_CLASS;
        stateArray = mxCreateNumericMatrix( totalSamples, 1, mxUINT16_CLASS, mxREAL );
      }
      else if( maxLength <= 32 )
      {
        stateInfo[ i ].classID = mxUINT32_CLASS;
        stateArray = mxCreateNumericMatrix( totalSamples, 1, mxUINT32_CLASS, mxREAL );
      }
      else
      {
        stateInfo[ i ].classID = mxUINT64_CLASS;
        stateArray = mxCreateNumericMatrix( totalSamples, 1, mxUINT64_CLASS, mxREAL );
      }
      if( stateArray == NULL )
        mexErrMsgTxt( "Out of memory when allocating space for state variables." );
      mxSetFieldByNumber( states, 0, i, stateArray );
      stateInfo[ i ].data8 = reinterpret_cast<uint8*>( mxGetData( stateArray ) );
    }
    for( FileContainer::iterator file = files.begin(); file != files.end(); ++file )
    { // Locations and lengths are not necessarily compatible across files, so we must
      // create StateInfos for each file individually.
      const StateVector* statevector = file->data->StateVector();
      const StateList& curStatelist = statevector->StateList();
      for( int i = 0; i < numStates; ++i )
      {
        const State& s = curStatelist[ stateNames[ i ] ];
        stateInfo[ i ].location = s.Location();
        stateInfo[ i ].length = s.Length();
      }
      for( int sample = file->begin; sample < file->end; ++sample )
      { // Iterating over samples in the outer loop will avoid scanning
        // the file multiple times.
        file->data->ReadStateVector( sample );
        for( int i = 0; i < numStates; ++i )
        {
          State::ValueType value
            = statevector->StateValue( stateInfo[ i ].location, stateInfo[ i ].length );
          switch( stateInfo[ i ].classID )
          {
            case mxUINT8_CLASS:
              *stateInfo[ i ].data8++ = value;
              break;

            case mxUINT16_CLASS:
              *stateInfo[ i ].data16++ = value;
              break;

            case mxUINT32_CLASS:
              *stateInfo[ i ].data32++ = value;
              break;

            case mxUINT64_CLASS:
              *stateInfo[ i ].data64++ = value;
              break;

            default:
              ::mexErrMsgTxt( "Unexpected data type." );
          }
        }
      }
    }
    bcierr__.clear();
    varargout[ 1 ] = states;
  }

  // Read parameters if appropriate.
  if( nargout > 2 )
  {
    varargout[ 2 ] = ParamlistToStruct( *files[ 0 ].data->Parameters() );
    bcierr__.clear();
  }
  // Return the total number of samples if requested.
  if( nargout > 3 )
  {
    long numSamples = 0;
    for( FileContainer::const_iterator i = files.begin(); i != files.end(); ++i )
      numSamples += i->data->NumSamples();
    varargout[ 3 ] = mxCreateDoubleScalar( numSamples );
  }
}

