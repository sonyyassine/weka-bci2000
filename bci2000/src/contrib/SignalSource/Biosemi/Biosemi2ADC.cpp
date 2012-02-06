/**
 * Program:   Biosemi2ADC
 * Module:    Biosemi2ADC.CPP
 * Comment:   Acquires from a Biosemi Act2, tested with a MK1 may work
 *      on a MK2, Converts the signal to microVolts before sending it.
 * Version:   0.01
 * License:
 * Copyright (C) 2005 Samuel A. Inverso (samuel.inverso@gmail.com), Yang Zhen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 *
 * Revisions:
 *  $Log$
 *  Revision 1.2  2005/12/14 15:24:15  mellinger
 *  Fixed state name typo in Process().
 *
 *  Revision 1.1  2005/12/12 00:05:24  sinverso
 *  Initial Revision: Working and tested offline. Not tested in real experiments.
 *
 */

#include "PCHIncludes.h"
#pragma hdrstop

#include <vector>
#include <string>
#include <cassert>
#include "Biosemi2ADC.h"
#include "BCIError.h"
#include "GenericSignal.h"

#include "Biosemi2Client.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( Biosemi2ADC, 1 );

// **************************************************************************
// Function:   Biosemi2ADC
// Purpose:    The constructor for the Biosemi2ADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
Biosemi2ADC::Biosemi2ADC()
: _samplingRate( 0 ),
  _softwareCh(0),
  _sampleBlockSize(0),
  _postfixTriggers(false),
  _triggerScaleMultiplier(1)
{

 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh= 80 80 1 296 "
       "// number of digitized channels, includes triggers if postfix triggers is true.",
   "Source int SampleBlockSize= 120 5 1 128 "
       "// number of samples per block",
   "Source int SamplingRate=    512 128 1 40000 "
       "// the signal sampling rate",
   "Source int PostfixTriggers= 1 0 0 1 "
        "// Make the triggers 16 channels and place them after end of EEG channels"
            " 0: no (warnning they will not be saved at all),"
            " 1: yes, post fix them (enumeration)",
   "Source int TriggerScaleMultiplier= 3000 1 1 % "
        "//number to multiply triggers by to scale them to the visualization range",
 END_PARAMETER_DEFINITIONS


 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "BatteryLow 1 0 0 0",
   "Mode 8 0 0 0",
   "MK2 1 0 0 0",
 END_STATE_DEFINITIONS

}

Biosemi2ADC::~Biosemi2ADC()
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
void Biosemi2ADC::Preflight( const SignalProperties&,
                                   SignalProperties& outSignalProperties ) const
{

    // Required states
    State("BatteryLow");
    State("Mode");
    State("MK2");

    if( Parameter("SampleBlockSize") < 1 ){
        bcierr << "Sample block Size of " << Parameter("SampleBlockSize")
               << " is less than 1" << endl;
    }

    if( Parameter("TriggerScaleMultiplier") <= 0 ){
        bcierr << "Trigger scale multiplier is: "
            << Parameter("TriggerScaleMultiplier")
            << ", but it must be greater than 0."
            << endl;
    }
    try{
        int reqChannels = Parameter("SourceCh");

        // The number of Channels requested includes the triggers,
        // so if the user wants to append the triggers, we need NUM_TRIGGERS less
        // software channels
        if( 1 == Parameter("PostfixTriggers" )  ){
            reqChannels -= Biosemi2Client::NUM_TRIGGERS;
            if( reqChannels <= 0 ){
                bcierr << "Requested eeg channels is <= 0. "
                    << "You probably didn't mean this." << endl
                    << "Remeber if you want to postfix triggers, softwareCh "
                    << endl
                    << "should equal the number of EEG channels you want + "
                    << "the total number of Trigger channels ( "
                    << Biosemi2Client::NUM_TRIGGERS << ")." << endl;

            }
        }
        else{
            bciout << "Warning: you are not post-fixing triggers."
                      " Triggers will not be saved." << endl;
        }
        _biosemi.initialize(Parameter("SamplingRate"),
            Parameter("SampleBlockSize"), reqChannels, false );

    }
    catch(char * error ){
        bcierr << "Unable to initialize biosemi client.\n Error: " <<
            error << endl;
    }

    if( Parameter("SourceCh") > _biosemi.getNumChannels() ){
        bcierr << "Number of channels requested, "
            << Parameter("SourceCh")
            << " is greater than the number"
            << endl
            << " of channels the biosemi can send, "
            << _biosemi.getNumChannels()
            << ", at current mode: "
            << _biosemi.getMode() << endl;
    }


    if( 0 != (_biosemi.getSamplingRate() % (int)Parameter("SamplingRate")) ){
        bcierr << "Sampling rate requested: " << Parameter("SamplingRate")
            << " does not evenly divide biosemi sampling rate: "
            << _biosemi.getSamplingRate() << endl;

    }
    // Requested output signal properties.
    outSignalProperties = SignalProperties(
        Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ),
            SignalType::float32);
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the Biosemi2ADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:   N/A
//
// **************************************************************************
void Biosemi2ADC::Initialize( const SignalProperties&, const SignalProperties& )
{
    _dataBlock = &_biosemi.getDataBlock();

    // store the value of the needed parameters
    _samplingRate = Parameter( "SamplingRate" );
    _softwareCh = Parameter("SourceCh");
    _sampleBlockSize = Parameter("SampleBlockSize");
    _triggerScaleMultiplier = Parameter("TriggerScaleMultiplier");
    _postfixTriggers = Parameter("PostfixTriggers") != 0;

    if( _postfixTriggers ){
        _signalChannels = _softwareCh - Biosemi2Client::NUM_TRIGGERS;
    }
    else{
        _signalChannels = _softwareCh;
    }
    // Setup the State
    State("BatteryLow") = _biosemi.isBatteryLow();
    if( State("BatteryLow" ) ){
        bciout << "Warnning: Battery low " << endl;
    }
    State("Mode") =_biosemi.getMode();
    State("MK2") = _biosemi.isMK2();


}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:
// **************************************************************************
void Biosemi2ADC::Process( const GenericSignal&, GenericSignal& signal )
{
    // wait for data to become ready
    _biosemi.isDataReady();


    //
    // Make sure the block is valid.
    //
    if( !_dataBlock->isDataValid() ){
        if( _biosemi.isBatteryLow() && !State("BatteryLow")){
            bciout << "Warning: Battery Low" << endl;

            // we don't want to send messages to bicout everytime
            // Process is called, only send once and hope the
            // user is paying attention.
            State("BatteryLow") = BATTERY_LOW;
        }
        else{
            // TODO  make this more descriptive
            bcierr << "Data is invalid for unkown reason." << endl;
        }
    }

    //
    // place the data and triggers to the out signal
    //
    int triggerChan(0);
    for( int sample(0) ; sample < _sampleBlockSize ; ++sample ){

        for( int  channel(0); channel < _softwareCh ; ++channel ){
            triggerChan = 0;
            if( channel < _signalChannels ){
                // this is a signal channel
                signal(channel, sample)=
                    _dataBlock->getSignal(sample,channel)/8192.0;
            }
            else if(_postfixTriggers) {
                // this is a trigger channel
                signal(channel, sample)=
                    _dataBlock->getTrigger(sample, triggerChan );
                ++triggerChan;
            }

        }
    }

}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
//             In this special case, it does not do anything (since
//             the random number generator does not have to be turned off)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void Biosemi2ADC::Halt()
{
    _biosemi.halt();
}
