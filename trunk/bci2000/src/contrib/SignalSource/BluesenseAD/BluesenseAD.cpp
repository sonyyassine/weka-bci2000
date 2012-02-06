#ifdef __BORLANDC__
#pragma hdrstop             // borland specific
#include <condefs.h>
#pragma argsused
#endif


#include "BluesenseAD.h"
#include "BCIError.h"
#include "conio.h"
#include "Tserial_event.h"
#include "BSController.h"




using namespace std;

short buffer[MAX_CHANNEL][MAX_BUFFER_SIZE];

RegisterFilter( BluesenseAD, 1 );


BluesenseAD::BluesenseAD()
: mSourceCh( 0 ),
  mSampleBlockSize( 0 ),
  mSamplingRate( 0 ) ,
  mComPort( "COM1" ),
  mBaudRate( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source string ComPort=  COM1 COM1 COM1 COM100"
        "// This is the virtual serial port has been setup for Bluesense",
    "Source int BaudRate=  2 2 0 3"
        "// This is the serial Baud Rate: 0 '9600', 1 '38400', 2 '57600', 3 '115200' (enumeration)",
    "Source int SourceCh=  1 4 1 8"
        "// This is the number of digitized channels",
    "Source int SampleBlockSize= 1 16 1 256"
        "// This is the number of samples transmitted at a time",
    "Source int SamplingRate=    1 250 1 4000"
        "// This is the sample rate",
  END_PARAMETER_DEFINITIONS
}

BluesenseAD::~BluesenseAD()
{
      stop_bluesense_sampling();  
      disconnect_bluesense();
}


void BluesenseAD::Preflight( const SignalProperties&,
                            SignalProperties& outputProperties ) const
{

  string  comPort = "\\\\.\\";
  comPort = comPort.append(Parameter( "ComPort"));
  int tBaudRate = Parameter( "BaudRate");
  if ( tBaudRate == 0 ) tBaudRate =  9600;
  else if ( tBaudRate == 1 )  tBaudRate = 38400;
  else if ( tBaudRate == 2 )  tBaudRate = 57600;
  else if ( tBaudRate == 3 )  tBaudRate = 115200;
  else  tBaudRate =  57600;


  if (connect_bluesense( comPort.c_str() ,tBaudRate ,Parameter( "SamplingRate" ),Parameter( "SourceCh" ), SERIAL_PARITY_NONE, 8, false)){
   bcierr << "Connection could not be established, please check the COM PORT "
           << Parameter( "ComPort")
           << endl;
  }

  outputProperties = SignalProperties( Parameter( "SourceCh" ),
                          Parameter( "SampleBlockSize" ),
                          SignalType::int16 );
}


void BluesenseAD::Initialize( const SignalProperties&, const SignalProperties& )
{
  mSourceCh = Parameter( "SourceCh" );
  mSampleBlockSize = Parameter( "SampleBlockSize" );
  mSamplingRate = Parameter( "SamplingRate" );
  mComPort =  Parameter( "ComPort" );
  mBaudRate = Parameter( "BaudRate" );

  start_bluesense_sampling();
}

void BluesenseAD::Halt()
{
 stop_bluesense_sampling();
}


void BluesenseAD::Process( const GenericSignal&, GenericSignal& outputSignal )
{
  wait_for_data_block( buffer, mSampleBlockSize) ;
  for( int channel = 0; channel < mSourceCh; ++channel )
    for( int sample = 0; sample < mSampleBlockSize; ++sample )
       outputSignal( channel, sample ) = buffer[channel][ sample ];

}





