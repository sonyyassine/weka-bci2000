/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    ModularEEGADC.CPP                                             *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   0.01                                                            *
 * Author:    Gerwin Schalk, Christoph Veigl                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 *         05/23/2000 - completed first start and documented                  *
 * V0.02 - 05/26/2000 - changed **RawEEG to *GenericIntSignal                 *
 * V0.03 - 07/25/2000 - made it a waveform generator                          *
 * V0.04 - 03/23/2001 - made the waveform generator more powerful             *
 * V0.05 - 09/28/2001 - can modulate samples based on mouse                   *
 * V0.06 - 03/28/2003 - juergen.mellinger@uni-tuebingen.de:                   *
 *                      now derived from GenericFilter                        *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "ModularEEGADC.h"
#include "BCIError.h"
#include "GenericSignal.h"
#include <stdio.h>
#include <math.h>


using namespace std;

// Register the source class with the framework.
RegisterFilter( ModularEEGADC, 1 );

// **************************************************************************
// Function:   ModularEEGADC
// Purpose:    The constructor for the ModularEEGADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
ModularEEGADC::ModularEEGADC()
: samplerate( 0 ),
  comport( 0 ), protocol( 0 ),
  modulateamplitude( 0 ),
  mCount(0)

{
 devicehandle=INVALID_HANDLE_VALUE;
 PACKET.readstate=0;

 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh=      6 6 6 6"
       "// the number of digitized and stored channels",
   "Source int SampleBlockSize= 16 16 1 64"
       "// samples transmitted at a time (1-64)",
   "Source int SamplingRate= 256 256 256 256"
       "// the sampling rate (currently 256 Hz)",
   "Source int ComPort=     3 3 1 5"
       "// ModularEEG connected to: 1 COM1, 2 COM2, 3 COM3, 4 COM4, 5 COM5 (enumeration)",
   "Source int Protocol=    1 1 1 2"
       "// transmission protocol: 1 P2, 2 P3 (enumeration)",
   "Source int SimulateEEG= 0 0 0 1"
       "// generate sinewaves with mouse (boolean)",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)


 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
//   "SourceTime 16 2347 0 0",
 END_STATE_DEFINITIONS

 }

ModularEEGADC::~ModularEEGADC()
{
  Halt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void ModularEEGADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
  ser_t testhandle;
  char comname[5];

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  PreflightCondition( Parameter( "SamplingRate" ) == 256 );
  PreflightCondition( Parameter( "SourceCh" ) == 6 );
  PreflightCondition( Parameter( "Protocol" ) <= 2 );
  PreflightCondition( Parameter( "SampleBlockSize" ) >= 1 );
  PreflightCondition( Parameter( "SampleBlockSize" ) <= 64 );

  // Resource availability checks.
  strcpy(comname,"COM ");comname[3]=Parameter("ComPort")+'0';
  if ((testhandle=openSerialPort(comname))==INVALID_HANDLE_VALUE)
     bcierr<< "the selected Comport is not available" << endl;
  else  closeSerialPort (testhandle);

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::int16 );

}


// **************************************************************************
// Function:   ADInit
// Purpose:    This function parameterizes the RandomNumberADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void ModularEEGADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  char comname[5];

  // store the value of the needed parameters
  samplerate = Parameter( "SamplingRate" );
  comport = Parameter( "ComPort" );
  protocol = Parameter( "Protocol" );
  modulateamplitude = ( ( int )Parameter( "SimulateEEG" ) != 0 );
  mCount=0;

  PACKET.readstate=0;
  strcpy(comname,"COM ");comname[3]=comport+'0';

  if ((devicehandle=openSerialPort(comname))==INVALID_HANDLE_VALUE)
     bcierr<< "Could not open Comport" << endl;

}


// **************************************************************************
// Function:   ADReadDataBlock
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void ModularEEGADC::Process( const GenericSignal&, GenericSignal& signal )
{
int     time2wait;
int     value;
long    longvalue;
double  t;
double  cursorpos, cursorposx;
const   maxvalue = 1 << 15 - 1,
        minvalue = - 1 << 15;
float   sr = samplerate;

  // if simulation was chosen, generate sinewaves using cursorpos and wait for sampling period
  if (modulateamplitude)
  {
     cursorpos=Mouse->CursorPos.y/70.0+1.0;
     if ((int)Mouse->CursorPos.x >= Screen->Width) // this fixes a problem with dual monitors (desktop can be larger than screen and thus screenposx could be 0; 04/19/05 GS)
       cursorposx=1;
     else cursorposx=(Screen->Width-Mouse->CursorPos.x)/70.0+1.0;
     if( sr < 1.0 )  sr = 1.0;
     time2wait = 1e3 * signal.Elements() / sr - 5.0;
     ::Sleep(time2wait);
  }


  // generate the sine wave and write it into the signal
  for (int sample=0; sample<signal.Elements(); sample++)
  {
    if (!modulateamplitude) read_channels(devicehandle,protocol);
    for (int channel=0; channel<signal.Channels(); channel++)
    {
      if (modulateamplitude)
      {
        t=(double)(mCount+sample)/(double)samplerate*4.0*(double)cursorposx;
        value=(int)((sin(t*2*3.14159265))*512.0/cursorpos+512.0);
      }
      else value = PACKET.buffer[channel];

      if( value > maxvalue ) value = maxvalue;
      if( value < minvalue ) value = minvalue;
      signal(channel, sample) = (short)value;
    }
  }
  mCount=mCount+signal.Elements();
}


// **************************************************************************
// Function:   ADShutdown
// Purpose:    This routine shuts down data acquisition
//             In this special case, it does not do anything (since
//             the random number generator does not have to be turned off)
// Parameters: N/A
// Returns:    1 ... always
// **************************************************************************
void ModularEEGADC::Halt()
{
  PACKET.readstate=PACKET_FINISHED;
  if (devicehandle!=INVALID_HANDLE_VALUE)
    CloseHandle(devicehandle);
  devicehandle=INVALID_HANDLE_VALUE;
 // bciout << "shutdown"<<endl;
}



