/**
 * Author: Samuel A. Inverso (samuel.inverso@gmail.com),  Yang Zhen
 * Contributors: Many thanks to the Biosemi.nl forum members
 * Description: Client to acquire from the USB BIOSEMI 2 Labview_DLL.
 * Note: This class is not thread safe
 * License:
 * Copyright (C) 2005  Samuel A. Inverso (samuel.inverso@gmail.com), Yang Zhen
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 * Revisions:
 *  $Log$
 *  Revision 1.1  2005/12/12 00:05:24  sinverso
 *  Initial Revision: Working and tested offline. Not tested in real experiments.
 *
 */

#include "stdafx.h"
#include <string>
#include <iostream>

#include <cassert>
#include <strstream>
#include "Biosemi2Client.h"

using namespace std;

Biosemi2Client::Biosemi2Client() :
    _usbdata(new char[USB_DATA_SIZE]),
    _data(new char[BUFFER_SIZE_IN_BYTES]),
    _bufferCursorPos(new DWORD),
    _oldBufferCursorPos(0),
    _intsAvailable(0),
    _numChannels(-1),
    _startPos(0),
    _nextPos(0),
    _isDataReadyCalledYet(false),
    _wasDriverSetup(true),
    _dataBlock(NULL)
{
    // all this code assumes 1 byte chars and 4 byte ints, For sanity sake
    // make sure that's what we are getting. All bets are off otherwise.
    assert(sizeof(int) == 4);
    assert(sizeof(char) == 1);

    _dataAsInt = reinterpret_cast<int *>(_data);

    try {
        //init
        OPEN_DRIVER_ASYNC = 0;
        USB_WRITE = 0;
        READ_MULTIPLE_SWEEPS = 0;
        READ_POINTER = 0;
        CLOSE_DRIVER_ASYNC = 0;
        //CLOSE_DRIVER = 0;

        HINSTANCE hLib;

        // To begin with - Load Library
        if(!(hLib = LoadLibrary("Labview_DLL.dll"))){  
            throw("Drivers are not present");
        }
        if ( !(OPEN_DRIVER_ASYNC = (dOPEN_DRIVER_ASYNC)GetProcAddress(
          hLib,"OPEN_DRIVER_ASYNC")) ) {
            throw("No OPEN_DRIVER_ASYNC");
        }
        if ( !(USB_WRITE = (dUSB_WRITE)GetProcAddress(hLib,"USB_WRITE")) ){
            throw("No USB_WRITE");
        }
        if ( !(READ_MULTIPLE_SWEEPS = (dREAD_MULTIPLE_SWEEPS)GetProcAddress(
          hLib,"READ_MULTIPLE_SWEEPS")) ){
            throw("No READ_MULTIPLE_SWEEPS");
        }
        if ( !(READ_POINTER = (dREAD_POINTER)GetProcAddress(
          hLib,"READ_POINTER")) ){
            throw("No READ_POINTER");
        }
        if ( !(CLOSE_DRIVER_ASYNC = (dCLOSE_DRIVER_ASYNC)GetProcAddress(
          hLib,"CLOSE_DRIVER_ASYNC")) ) {
          throw("No CLOSE_DRIVER_ASYNC");
        }
    }
    catch (char* cp) {
      throw(cp);
    }
}

Biosemi2Client::~Biosemi2Client(){
    halt();
    delete _dataBlock;
    delete _data;
    delete _usbdata;
    delete _bufferCursorPos;
}

void Biosemi2Client::setupDriver(){

/*******************************************************************************
To acquire data, there are 7 step we need to go.
*******************************************************************************/

/*******************************************************************************
Step 1:  "Initializing the USB2 interface"
Call function OPEN_DRIVER_ASYNC, the output provides a "handle" parameter,
used as an input for further function calls.
*******************************************************************************/

    _hdevice=OPEN_DRIVER_ASYNC();

/*******************************************************************************
Step 2:  "Start the acquisition with initializing the USB2 interface"
Call USB_WRITE, inputs are "handle" (from step 1) and "data",
use an array of 64 zero bytes for the "data" input.
*******************************************************************************/

      memset(_usbdata,0,USB_DATA_SIZE);

        USB_WRITE(_hdevice, _usbdata);

/*******************************************************************************
Step 3:  "Initialize the ringbuffer"
Call READ_MULTIPLE_SWEEPS, inputs are "handle" (from step 1),
"data" and "nBytes to read".
Use an initialized buffer of 32 Mbyte (recommended size) as input
for "data" (this is the ringbuffer),
and use the buffersize as input for "nBytes to read" (33554432 = 2^25).
Every thing is now set to start with the acquisition,
and the only thing to do is to enable the handshake.
*******************************************************************************/

        READ_MULTIPLE_SWEEPS(_hdevice, _data, BUFFER_SIZE_IN_BYTES);

/*******************************************************************************
Step 4:  "Enable the handshake"
Call again USB_WRITE, inputs are "handle" (from step 1)
and "data", this time use an array of 1 byte of 255 (FF) followed by 63 zero bytes
for the "data" input.
At this moment the handshake with the USB2 interface starts
(green "USB link" LED on receiver switches on),
and the ringbuffer is filled with incoming data from the ActiveTwo.
Your acquisition program should read the proper data for the ringbuffer.
*******************************************************************************/
                 _usbdata[0]=1;
          USB_WRITE(_hdevice, _usbdata);

    // tells halt it can actually stop the device
    // or bad things happen
    _wasDriverSetup = true;
}

void Biosemi2Client::initialize(const int &desiredSamplingRate,
    const int &desiredSampleBlockSize, const int &desiredNumChannels,
    const bool &throwBatteryLowException ){

    // Store the signal attributes the caller wants
    _desiredSamplingRate = desiredSamplingRate;
    _desiredNumChannels  =  desiredNumChannels;
    _desiredSampleBlockSize = desiredSampleBlockSize;
    _throwBatteryLowException =  throwBatteryLowException;

    // setup the biosemi driver
    setupDriver();

    // Get a few data points to check the status channel and get the
    // number of channels, sampling rate, etc.

    for( int i = 0; i < 10 ; ++i ){
        Sleep(10);
        READ_POINTER(_hdevice,_bufferCursorPos);
    }

    // If we haven't see two ints by this time either the power is probably
    // off
    if( *_bufferCursorPos < BYTES_PER_INT*2){
        throw "No cord or power";
    }

    // Decode the status channel and place the results in 
    // the member variables passed
    bool ignored;
    decodeStatus( _dataAsInt, 0, _mk2, _battLow,_mode, _numChannels,
        _samplingRate, ignored, ignored);

   
    // Check if the desired sampling rate evenly divides the
    // actual sampling rate.
    if( 0 != _samplingRate % desiredSamplingRate ){
        strstream error;
        error << "Sampling rate requested: " << desiredSamplingRate
            << " does not evenly divide biosemi sampling rate: "
            << _samplingRate << endl;
        throw(error.str());
    }

    // Check if the batter is low
    if( isBatteryLow() ){
        throw("ERROR: Battery is low" );
    }

    // Determine how much we have to decimate the sample to achieve
    // the requested sampling rate. (Basically how many samples 
    // we need to skip)
    // This also tells us the actual sampleblock size we need
    _decimationFactor = _samplingRate/desiredSamplingRate;
    _sampleBlockSize = _decimationFactor* desiredSampleBlockSize;

    // Deterimine how many ints need to be buffered before we can
    // start reading
    _numIntsToRead = _sampleBlockSize * _numChannels;

    // stop the driver or else data will be outof sync on the getData call
    halt();

     _intsBuffered =0;
    _oldBufferCursorPos = 0;
    *_bufferCursorPos = 0;
    _startPos = 0;
    _nextPos=0;
    _isDataReadyCalledYet = false;

}

/**
 * Returns a pointer to the data buffer
 */
const int * Biosemi2Client::getData() const{
    return _dataAsInt;
}

void Biosemi2Client::halt(){
    // only halt if we were initialized
    if(_wasDriverSetup){
    /*************************************************************************
    Step 6:  "Disable the handshake"
    Call USB_WRITE, inputs are "handle" (from step 1) and "data",
    use an array of 64 zero bytes for the "data" input.
    Finally, the interface drivers should be closed:
    **************************************************************************/

        _usbdata[0]=0;

        USB_WRITE(_hdevice, _usbdata);

    /*************************************************************************
    Step 7:  "Close the drivers"
    Call CLOSE_DRIVER_ASYNC, input is "handle" (from step 1).

    If all goes OK, you will receive the following data format from the 
    ActiveTwo:
    The ActiveTwo sends the following number of 32-bit words per sample:

    speedmode 0 and 4: 258
    speedmode 1 and 5: 130
    speedmode 2 and 6: 66
    speedmode 3 and 7: 34
    speedmode 8: 290

    The receiver converts every 24-bit word from the AD-box into a 
    32-bit Signed Integer,
    by adding an extra zero Least Significant Byte to the ADC data.
    The 24-bit ADC output has an LSB value of 1/32th uV.
    The 32-bit Integer received for the USB interface has an LSB value 
        of 1/32*1/256 = 1/8192th uV

    Two extra channels are leading the ADC data: before:
    channel 1 = sync (check for FFFFFF00) and channel 2 = Status (see
    http://www.biosemi.com/faq/trigger_signals.htm), 
    channels 3-258 are ADCs 1-256.
    *************************************************************************/

        CLOSE_DRIVER_ASYNC(_hdevice);

        _wasDriverSetup= false;
    }
}


int Biosemi2Client::getNumChannels() const{
    return _numChannels;
}

int Biosemi2Client::determineNumChannels(int mode, bool isMk2) const{
    int result = -1;
    if( isMk2 ){
        result = determineNumChannelsMk2(mode);
    }
    else{
        result = determineNumChannelsMk1(mode);
    }
    return result;
}

// http://www.biosemi.com/faq/adjust_samplerate.htm
int Biosemi2Client::determineNumChannelsMk1(int mode) const
{
    int result;
	switch(mode){
		case 0:
		case 4:
        	result = 258;
            break;
		case 1:
		case 5:
        	result =  130;
            break;
		case 2:
		case 6:
        	result = 66;
            break;
		case 3:
		case 7:
        	result = 34;
            break;
        case 8:
            result = 290; // 2 + 256 + 32
            break;
        default:
            throw("mode unacceptable");
            //result= -1;
    }
    return result;
}

// http://www.biosemi.com/faq/adjust_samplerate.htm
int Biosemi2Client::determineNumChannelsMk2(int mode) const
{
    int result;
	switch(mode){
		case 0:
		case 1:
		case 2:
		case 3:
        	result = 610; // 2+4*152
            break;
        case 4:
        	result = 282;
            break;
        case 5:
        	result = 154;
            break;
        case 6:
        	result = 90;
            break;
        case 7:
            result = 58;
            break;
        case 8:
            result = 314; // 2 + 280+32
            break;
        default:
            throw("mode unacceptable");
            //result= -1;
    }
    return result;
}


int Biosemi2Client::determineSamplingRate(int mode) const{
    int result;
	switch(mode){
		case 0:
		case 4:
        	result = 2048;  // 2 kHz
            break;
		case 1:
		case 5:
        	result =  4096; // 4 kHz
            break;
		case 2:
		case 6:
        	result = 8192;  // 8 kHz
            break;
		case 3:
		case 7:
        	result = 16384; // 16 kHz
            break;
        case 8:
            // AIB-mode, don't know how to deal wit this so throw exception
            throw("ERROR: Mode 8, AIB-mode, don't know how to deal with this");
            //break;
        default:
            throw("mode unacceptable");
            //result= -1;
    }
    return result;
}


void Biosemi2Client::decodeStatus(const int * data, int position,
    bool &mk2, bool &battLow, int &mode, int &numChannels, int &samplingRate,
    bool &cmsWithinRange, bool &newEpoch) const{
    int status = data[position+1];
    mk2 = isMK2(status);
    battLow = isBatteryLow(status);


    mode = statusToMode(status);


    numChannels = determineNumChannels(mode,mk2);

    samplingRate = determineSamplingRate(mode);
    cmsWithinRange = status & CMS_WITHIN_RANGE_MASK;

    newEpoch = status & NEW_EPOCH_MASK;

}

// Checks if the first sample  starting at startPos  in data
// is synced
bool  Biosemi2Client::isDataSynced(const int * data, const int &position) const{
    return isSampleSynced(data[position]);
}


bool Biosemi2Client::isSampleSynced( const int &sample ) const{
    return IS_SYNCED_MASK == sample;
}

int Biosemi2Client::statusToMode(const int *  data, int position) const{
    return statusToMode(data[position]);
}

// returns the mode corresponding to the one on the front of the
// biosemi box
int Biosemi2Client::statusToMode( int status) const{
    int result = ((status & MODE_MASK) >> MODE_SHIFT_TO_INT);
    // The status channel has speed bits laid out at
    // Speed bit 3
    // CMS
    // Speed bit 2
    // Speed bit 1
    // Speed bit 0
    // Therefore we need to move SpeedBit 3 next to Speed bit 2
    // to get a number that corresponds to the front of the biosemi
    // box
    if( result  & 0x10){
        result  |= 0x8; // set it true in the new position
        result &= 0xF; // clear it in the old poition
    }

    return result;
}

int Biosemi2Client::getMode() const{
    return _mode;
}
 // sampling rate in Hz
int Biosemi2Client::getSamplingRate() const{
    return _samplingRate;
}


/**
 * returns true if the data is valid, false if the battery is low
 * otherwise it throws an exception indicating what is wrong e.g.
 * data is not synced, changed in mode, (battery low if 
 *  _throwBatteryLowException is true
 */
bool Biosemi2Client::isDataValid(const int * data, const int &startPos ) const{
    int datum;
    bool result = true;
    for( int sample =0 ; sample < _sampleBlockSize ; ++sample ){
        for( int channel=0; channel < 2 ; ++channel ){
            datum =  data[(sample*_numChannels+channel+startPos)
                        % Biosemi2Client::BUFFER_SIZE_IN_INT];
            if( Biosemi2Client::SYNC_CHANNEL == channel ){
                result = isSampleSynced(datum);
            }
            else if( Biosemi2Client::STATUS_CHANNEL == channel ){
                result = isStatusValid(datum);
            }
        }
    }
    return result;
}

// vaidates the current data, it's up to the caller to make sure there's
// a full sampleblock
bool Biosemi2Client::isDataValid() const{
    return isDataValid(_dataAsInt,_startPos);
}

int Biosemi2Client::getSampleBlockSize() const{
    return _sampleBlockSize;
}

bool Biosemi2Client::isStatusValid( const int &sample ) const{
    bool result = true;
    if( _mode != statusToMode(sample)){
        // the mode has changed
        strstream error;
        error << "ERROR: Mode changed from: " << _mode << " to "
            << statusToMode(sample ) << endl
            << "startpos " << _startPos << endl
            << "ints buffered "   << _intsBuffered << endl
            << "_bufferCursorPos " << *_bufferCursorPos   << " - "
            << " _oldBufferCursorPos " << _oldBufferCursorPos
            << " = " << (*_bufferCursorPos - _oldBufferCursorPos) << endl;
        throw( error.str() );
    }
    else if( isBatteryLow(sample ) ){
        if( _throwBatteryLowException ){
            throw("The Battery is low" );
        }
        result = false;
    }

    return result;
}

/**
 * Blocks until blockSize number of elements are ready to read for each channel
 * Note: does not do any data validating you need to do that yourself
 * Parameters:
 * blockSize  int number of samples you want per channel
 * index      int the index to start reading from in the data buffer Note:
 *          it's a circular buffer and may rap around so you have to use
 *          modulus
 * numAvailable int the number of data samples available to read from the buffer
 */
void Biosemi2Client::isDataReady(){

    /*************************************************************************
    Step 5:  "generate a file pointer"
    Call READ_POINTER to generate a file pointer.
    Inputs are "handle" (from step 1), and "pointer (0).
    Output is a "pointer". Use this parameter as a file pointer.
    In our ActiVIEW software, we have two counters running continuously:
    one counter tracks the total of the buffered words,
    the other counts the total of words displayed on screen 
    (or written to file).

    Every time the difference in the counters exceeds a set amount of words,
    this amount of words is displayed on screen or written to file.
    The difference between the two counters is the "backlog" value,
    as soon as it exceeds the total ringbuffer size,
    an overflow error is generated and the program stops.
    To stop the acquisition you will have to disable the handshake.
    ************************************************************************/

    if( !_isDataReadyCalledYet ) {
        setupDriver();
        _isDataReadyCalledYet = true;
    }
    _startPos = _nextPos;

    //
    // wait until there is enough data in the buffer before returning
    //
    while( _intsAvailable < _numIntsToRead){
        Sleep(10);
        READ_POINTER(_hdevice,_bufferCursorPos);

        // Convert _bufferCursorPos to integer based vs byte based
        *_bufferCursorPos /= BYTES_PER_INT;

        if( *_bufferCursorPos >= _oldBufferCursorPos ){
            _intsAvailable += *_bufferCursorPos - _oldBufferCursorPos;
        }
        else {
            _intsAvailable += 0 +*_bufferCursorPos + BUFFER_SIZE_IN_INT -
                _oldBufferCursorPos;
        }

        _oldBufferCursorPos = *_bufferCursorPos;
    }

    // check if we have overrun the buffer
    if( _intsBuffered > BUFFER_SIZE_IN_INT ){
        throw ("ERROR: Buffer overrun error");
    }

    _nextPos =  (_startPos + _numIntsToRead) % BUFFER_SIZE_IN_INT;

    _intsAvailable  -= _numIntsToRead;
}

void Biosemi2Client::isDataReady(int &startPos){
    // Tell the caller where they should start reading from.
    startPos = _startPos;
    isDataReady();
}

bool Biosemi2Client::isTriggerHigh( const int & trigger, const int &datum ) const{
    bool result = false;
    switch( trigger ){
        case 0:
            result = datum & TRIGGER_0;
            break;
        case 1:
            result = datum & TRIGGER_1;
            break;
        case 2:
            result = datum & TRIGGER_2;
            break;
        case 3:
            result = datum & TRIGGER_3;
            break;
        case 4:
            result = datum & TRIGGER_4;
            break;
        case 5:
            result = datum & TRIGGER_5;
            break;
        case 6:
            result = datum & TRIGGER_6;
            break;
        case 7:
            result = datum & TRIGGER_7;
            break;
        case 8:
            result = datum & TRIGGER_8;
            break;
        case 9:
            result = datum & TRIGGER_9;
            break;
        case 10:
            result = datum & TRIGGER_10;
            break;
        case 11:
            result = datum & TRIGGER_11;
            break;
        case 12:
            result = datum & TRIGGER_12;
            break;
        case 13:
            result = datum & TRIGGER_13;
            break;
        case 14:
            result = datum & TRIGGER_14;
            break;
        case 15:
            result = datum & TRIGGER_15;
            break;
        default:
            strstream error;
            error << "Unkown trigger: " << trigger
                << ". Note triggers are numbered starting from 0.";
            throw(error.str());
    }
    return result;
}

bool Biosemi2Client::isBatteryLow(const int &datum) const{
    return datum & BATT_LOW_MASK;
}

bool Biosemi2Client::isBatteryLow() const{
    return isBatteryLow(calcIndex(_desiredSampleBlockSize-1,STATUS_CHANNEL));
}

bool Biosemi2Client::isMK2() const{
    return _mk2;
}
bool Biosemi2Client::isMK2(const int &status) const{
    return status & MK2_MASK;
}

Biosemi2Client::DataBlock& Biosemi2Client::getDataBlock(){
    if(NULL == _dataBlock){
        _dataBlock = new DataBlock(this);
    }
    return *_dataBlock;
}

int Biosemi2Client::calcIndex(const int &sample,
    const int &channel, const int &startPos) const{
    return ( startPos+sample
          * _numChannels
          * _decimationFactor
          + channel
          )
          % Biosemi2Client::BUFFER_SIZE_IN_INT;

}
int Biosemi2Client::calcIndex(const int &sample,
    const int &channel) const{
    return calcIndex(sample, channel, _startPos);
}



//
// D A T A B L O C K   I M P L E M E N T A T I O N
//

Biosemi2Client::DataBlock::DataBlock(  const Biosemi2Client *biosemi ):
    _biosemi(biosemi){
}

Biosemi2Client::DataBlock::~DataBlock(){
}


int Biosemi2Client::DataBlock::getTrigger(const int &sample, const int &trigger,
    const int &scaled ) const{
    return _biosemi->isTriggerHigh(trigger,
        _biosemi->_dataAsInt[
            _biosemi->calcIndex(sample,Biosemi2Client::STATUS_CHANNEL)]
        ) * scaled;
}

// loops through the signal channels starting at 0
// (so it pretends sync and status do not exist)
int Biosemi2Client::DataBlock::getSignal(const int &sample,
    const int &channel) const{
    if( sample > _biosemi->_desiredSampleBlockSize ){
        throw( "Sample out of bounds error." );
    }
    else if( channel > _biosemi->_desiredNumChannels ){
        throw( "Channel out of bounds error." );
    }
    // skip the sync and status channels
    return _biosemi->_dataAsInt[_biosemi->calcIndex(sample,channel+2)];
}

// Loop through the sync and status channels and make sure this data
// is valid
bool Biosemi2Client::DataBlock::isDataValid() const{
    return _biosemi->isDataValid();

}

const int &Biosemi2Client::DataBlock::getNumChannels(){
    return _biosemi->_desiredNumChannels;
}

const int &Biosemi2Client::DataBlock::getSampleBlockSize(){
    return  _biosemi->_desiredSampleBlockSize;
}

const int &Biosemi2Client::DataBlock::getSamplingRate(){
    return _biosemi->_desiredSamplingRate;
}
