////////////////////////////////////////////////////////////////////////////////
//
// File: SetBaseline.cpp
//
// Description: Slow Wave Class Definition
//           written by Dr. Thilo Hinterberger 2000-2001
//           Copyright University of Tuebingen, Germany
//
// Changes:  2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//           Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//           minor reformulations, reformatting.
//           Feb 24, 2004, jm: Moved the TSetBaseline class into separate files.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SetBaseline.h"

#include "BCIError.h"

#define SECTION "Filtering"

using namespace std;

const char* baselineStateName = "Baseline";

RegisterFilter( TSetBaseline, 2.D1 );

TSetBaseline::TSetBaseline()
: mLastBaselineState( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " intlist BaseChList= 2 1 1"
      " 0 0 1 // 1 to mark that BL is subtracted",
  END_PARAMETER_DEFINITIONS
}

void
TSetBaseline::Preflight( const SignalProperties& inSignalProperties,
                               SignalProperties& outSignalProperties ) const
{
  if( Parameter( "BaseChList" )->NumValues() != inSignalProperties.Channels() )
    bcierr << "The number of BaseChList entries "
           << "(currently " << Parameter( "BaseChList" )->NumValues() << ") "
           << "must match the number of input channels to the SetBaseline filter "
           << "(currently " << inSignalProperties.Channels() << ")"
           << endl;

  State( baselineStateName );
  outSignalProperties = inSignalProperties;
  outSignalProperties.SetName( "Baseline Filtered" );
}

void
TSetBaseline::Initialize( const SignalProperties&, const SignalProperties& )
{
  // allocating BL variables
  int numChan = Parameter( "BaseChList" )->NumValues();
  mBLSamples.resize( numChan, 0 );
  mBaseChList.resize( numChan, false );
  for( int i = 0; i < numChan; ++i )
    mBaseChList[ i ] = ( Parameter( "BaseChList" )( i ) > 0 );
  mBLSignal = GenericSignal( numChan, 1 );
  mLastBaselineState = 0;
}

void
TSetBaseline::Process( const GenericSignal& inSignal, GenericSignal& outSignal )
{
  outSignal = inSignal;
  GenericSignal& ioSignal = outSignal;

  if( State( baselineStateName ) - mLastBaselineState == 1 )
  {
    for( int i = 0; i < mBLSignal.Channels(); ++i )
      mBLSignal( i, 0 ) = 0;
    for( size_t i = 0; i < mBLSamples.size(); ++i )
      mBLSamples[ i ] = 0;
  }

  if( State( baselineStateName ) )
    for( int i = 0; i < ioSignal.Channels(); ++i )
    {
      float sum = 0;
      for( int j = 0; j < ioSignal.Elements(); ++j )
        sum += ioSignal( i, j );
      mBLSignal( i, 0 ) = mBLSignal( i, 0 ) * mBLSamples[ i ] + sum;
      mBLSamples[ i ] += ioSignal.Elements();
      mBLSignal( i, 0 ) /= mBLSamples[ i ];
    }

  for( size_t i = 0; i < mBaseChList.size(); ++i )
    if( mBaseChList[ i ] )
      for( int j = 0; j < ioSignal.Elements(); ++j )
        ioSignal( i, j ) -= mBLSignal( i, 0 );

  mLastBaselineState = State( baselineStateName );
}
