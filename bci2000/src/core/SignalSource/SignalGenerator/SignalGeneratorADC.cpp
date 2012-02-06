////////////////////////////////////////////////////////////////////////////////
// $Id: SignalGeneratorADC.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An ADC class for testing purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SignalGeneratorADC.h"
#include "BCIError.h"
#include "GenericSignal.h"
#include "MeasurementUnits.h"

#include <cmath>
#include <windows.h>
#ifndef _WIN32
# include <sys/socket.h>
#endif

using namespace std;

// Register the source class with the framework.
RegisterFilter( SignalGeneratorADC, 1 );

SignalGeneratorADC::SignalGeneratorADC()
: mSamplingRate( 1 ),
  mSineFrequency( 0 ),
  mSineAmplitude( 0 ),
  mNoiseAmplitude( 0 ),
  mDCOffset( 0 ),
  mSineChannelX( 0 ),
  mSineChannelY( 0 ),
  mSineChannelZ( 0 ),
  mModulateAmplitude( 1 ),
  mSinePhase( 0 ),
  mLasttime( 0 ),
  mAmplitudeX( 1 ),
  mAmplitudeY( 1 ),
  mAmplitudeZ( 1 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Signal%20Properties int SourceCh= 16 "
       "16 1 % // number of digitized and stored channels",
    "Source:Signal%20Properties int SampleBlockSize= 32 "
       "32 1 % // number of samples transmitted at a time",
    "Source:Signal%20Properties int SamplingRate= 256Hz "
       "256Hz 1 % // sample rate",
#ifdef _WIN32
    "Source int ModulateAmplitude= 0 0 0 1 "
      "// Modulate the amplitude with the mouse (0=no, 1=yes) (boolean)",
#endif // _WIN32
    "Source int SineChannelX= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse x position",
    "Source int SineChannelY= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse y position (0 for all)",
    "Source int SineChannelZ= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse key state",
    "Source float SineFrequency= 10Hz 10Hz % % "
      "// Frequency of sine wave",
    "Source int SineAmplitude= 100muV 100muV % % "
      "// Amplitude of sine wave",
    "Source int NoiseAmplitude= 30muV 30muV % % "
      "// Amplitude of white noise (common to all channels)",
    "Source int DCOffset= 0muV 0muV % % "
      "// DC offset (common to all channels)",
    "Source string OffsetMultiplier= % StimulusType % % "
      "// Expression to multiply offset by",
    "Source int RandomSeed= 0 0 0 1 "
      "// Initialization value for random number generator (initialize from time if 0)",
    "Source int SignalType= 0 0 0 2 "
      "// numeric type of output signal: "
        " 0: int16,"
        " 1: float32,"
        " 2: int32 "
        "(enumeration)",
  END_PARAMETER_DEFINITIONS
}


SignalGeneratorADC::~SignalGeneratorADC()
{
}


void
SignalGeneratorADC::Preflight( const SignalProperties&,
                                  SignalProperties& Output ) const
{
  PreflightCondition( Parameter( "SamplingRate" ) > 0 );
  MeasurementUnits::ReadAsFreq( Parameter( "SineFrequency" ) );
  MeasurementUnits::ReadAsVoltage( Parameter( "SineAmplitude" ) );
  MeasurementUnits::ReadAsVoltage( Parameter( "NoiseAmplitude" ) );
  if( MeasurementUnits::ReadAsVoltage( Parameter( "DCOffset" ) ) != 0 )
    Expression( Parameter( "OffsetMultiplier" ) ).Evaluate();
  Parameter( "RandomSeed" );

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  SignalType signalType;
  switch( int( Parameter( "SignalType" ) ) )
  {
    case 0:
      signalType = SignalType::int16;
      break;
    case 1:
      signalType = SignalType::float32;
      break;
    case 2:
      signalType = SignalType::int32;
      break;
    default:
      bcierr << "Unknown SignalType value" << endl;
  }
  Output = SignalProperties(
    Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), signalType );
}


void
SignalGeneratorADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  mSamplingRate = Parameter( "SamplingRate" );
  mSineFrequency = MeasurementUnits::ReadAsFreq( Parameter( "SineFrequency" ) );
  mSineAmplitude = MeasurementUnits::ReadAsVoltage( Parameter( "SineAmplitude" ) );
  mSinePhase = M_PI / 2;
  mNoiseAmplitude = MeasurementUnits::ReadAsVoltage( Parameter( "NoiseAmplitude" ) );
  mDCOffset = MeasurementUnits::ReadAsVoltage( Parameter( "DCOffset" ) );
  if( mDCOffset == 0 )
    mOffsetMultiplier = Expression( "" );
  else
    mOffsetMultiplier = Expression( Parameter( "OffsetMultiplier" ) );
  mSineChannelX = Parameter( "SineChannelX" );
  mSineChannelY = Parameter( "SineChannelY" );
  mSineChannelZ = Parameter( "SineChannelZ" );

#ifdef _WIN32
  mModulateAmplitude = ( Parameter( "ModulateAmplitude" ) != 0 );
#endif // _WIN32
}


void
SignalGeneratorADC::StartRun()
{
  if( Parameter( "RandomSeed" ) != 0 )
    mSinePhase = 0;
}


void
SignalGeneratorADC::Process( const GenericSignal&, GenericSignal& Output )
{
#ifdef _WIN32
  if( mModulateAmplitude )
  {
    POINT p = { 0, 0 };
    if( ::GetCursorPos( &p ) )
    {
      int width = ::GetSystemMetrics( SM_CXVIRTUALSCREEN ),
          height = ::GetSystemMetrics( SM_CYVIRTUALSCREEN );
      mAmplitudeX = float( p.x ) / width;
      mAmplitudeY = 1.0 - float( p.y ) / height;
    }
    enum { isPressed = 0x8000 };
    bool leftButton = ::GetAsyncKeyState( VK_LBUTTON ) & isPressed,
         rightButton = ::GetAsyncKeyState( VK_RBUTTON ) & isPressed;
    mAmplitudeZ = 0.5 + ( leftButton ? -0.5 : 0 ) + ( rightButton ? 0.5 : 0 );
  }
#endif // _WIN32

  float maxVal = Output.Type().Max(),
        minVal = Output.Type().Min();

  for( int sample = 0; sample < Output.Elements(); ++sample )
  {
    mSinePhase += 2 * M_PI * mSineFrequency;
    mSinePhase = ::fmod( mSinePhase, float( 2 * M_PI ) );
    float sineValue = ::sin( mSinePhase ) * mSineAmplitude;

    float offset = mDCOffset;
    if( offset != 0 )
      offset *= mOffsetMultiplier.Evaluate();
    for( int ch = 0; ch < Output.Channels(); ++ch )
    {
      float value = offset;
      value += ( mRandomGenerator.Random() * mNoiseAmplitude / mRandomGenerator.RandMax() - mNoiseAmplitude / 2 );
      if( mSineChannelX == ch + 1 )
        value += sineValue * mAmplitudeX;
      if( mSineChannelY == 0 || mSineChannelY == ch + 1 )
        value += sineValue * mAmplitudeY;
      if( mSineChannelZ == ch + 1 )
        value += sineValue * mAmplitudeZ;

      value = max( value, minVal );
      value = min( value, maxVal );

      Output( ch, sample ) = value;
    }
  }

  // Wait for the amount of time that corresponds to the length of a data block.
  PrecisionTime now = PrecisionTime::Now();
  float blockDuration = 1e3 * Output.Elements() / mSamplingRate,
        time2wait = blockDuration - ( now - mLasttime );
  if( time2wait < 0 )
    time2wait = 0;
#ifdef _WIN32
  const float timeJitter = 5;
  ::Sleep( ::floor( time2wait / timeJitter ) * timeJitter );
  while( PrecisionTime::Now() - mLasttime < blockDuration - 1 )
    ::Sleep( 0 );
#else
  struct timeval tv = { 0, 1e3 * time2wait };
  ::select( 0, NULL, NULL, NULL, &tv );
#endif
  mLasttime = PrecisionTime::Now();
}


void
SignalGeneratorADC::Halt()
{
}



