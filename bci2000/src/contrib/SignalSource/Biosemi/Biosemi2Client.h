/**
 * Author: Samuel A. Inverso (samuel.inverso@gmail.com) and Yang Zhen
 * Contributors:
 * Description: Client to acquire from the USB BIOSEMI 2 Labview_DLL.
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

#ifndef BIOSEMI2CLIENTH
#define BIOSEMI2CLIENTH

#include <iterator>
#include "LABVIEW_DLL.h"

class Biosemi2Client{

public:
    // forward declaration 
    class DataBlock;

    // 
    // C O N S T A N T S
    // 
    static const long BUFFER_SIZE_IN_BYTES = 1024*1024*32; // 33,554,432 (32MB)
    static const long BYTES_PER_INT = 4; // the number of chars that make up an
                                        // int. 1 byte per char, 4 bytes per int
                                        // so 4 chars per int
    // size of the int data buffer
    static const long BUFFER_SIZE_IN_INT = BUFFER_SIZE_IN_BYTES / BYTES_PER_INT;
    static const int SYNC_CHANNEL = 0;
    static const int STATUS_CHANNEL = 1;
    static const int NUM_TRIGGERS = 16; // the number of triggers


    Biosemi2Client();

    virtual ~Biosemi2Client();

    /**
     * Call initialize before calling anything else to setup everything
     *
     * @param desiredSamplingRate the sampling rate you want in Hz
     *      (as in samples per channel)
     *      Must be evenly divisable into the biosemi's sampling rate
     *      e.g. Biosemi Mode 5 in Mk1 = 4096 Hz, 
     *      so 4096 % desiredSamplingrate must equal 0 
     *
     * @param desiredSampleBlockSize the number of samples wait for  
     *      per channel before isDataReady becomes true.
     *      e.g. 32 would wait for 32 samples per channel
     *      at your desired sampling rate 
     *
     * @param desiredNumChannels the number of channels you want. This
     *      does not include the sync or status channel
     *      you must ask for a number of channels less than or equal to
     *      the number of signal channels the biosemi transmits on the
     *      current mode
     *      e.g. mode 5 transmits 128 signal channels (130 with status and sync)
     *      so you can ask for 1 to 128 channels
     *
     * @param throwBatteryLowException if true we throw an exception
     *     when checking valid status if the battery is low, otherwise
     *     we just return true. (Basically how bad to care if the battery
     *     is low during your experiment, use false if you don't want
     *      the experiment to crash to a halt when the battery becomes
     *      low) Default = false
     */
    virtual void  initialize(const int &desiredSamplingRate,
        const int &desiredSampleBlockSize, const int &desiredNumChannels,
        const bool &throwBatteryLowException=false);

    /**
     * Data block allows you to access the data as if it was actually
     * sampled at your desred sampling rate with your desired number of 
     * channels, and not have to worry about the offset into the data buffer
     *
     * Note: Call isDataReady automatically makes datablock access the
     *  new data just acquired. 
     *
     * @return Datablock to access the data
     * @see DataBlock
     */
    virtual DataBlock& getDataBlock();

    /**
     * Allows access to the data buffer. You should normally use
     * getDataBlock instead.
     *
     * @return pointer to the data buffer in ints 
     */
    virtual const int * getData() const;

    /**
     * Returns when there is enough data in the buffer to fulfill your
     * sample block size request.
     */
    virtual void isDataReady();

    /**
     * Returns when there is enough data in the buffer to fulfill your
     * sample block size request.
     *
     * @param startPos is an out variable, it contains starting position
     *  of the new data in the data buffer
     */
    virtual void isDataReady(int &startPos);

    /**
     * @return the actual number of channels the biosemi is sending
     * including sync and status channels
     */
    virtual int getNumChannels() const;


    virtual int getMode() const;

    /**
     * @return biosei's actual sampling rate in Hz
     */
    virtual int getSamplingRate() const;


    /**
     * @return the actual sampleBlockSize we are waiting for, which
     *   differs from the one you desired based on the difference
     *   in sampling rate
     */
    virtual int getSampleBlockSize() const;

    /**
     * Halts data acquisition
     */
    virtual void halt();

    virtual bool isDataValid(const int * data, const int &position ) const;
    virtual bool Biosemi2Client::isDataValid() const;

    // Checks if the first sample  starting at startPos  in data
    // is synced
    virtual bool  isDataSynced(const int * data, const int &position) const;

    virtual bool isSampleSynced(const int &sample ) const;

    virtual bool isStatusValid(const int &sample ) const;

    virtual bool isTriggerHigh( const int & trigger, const int &datum) const;

    /** 
     * If the battery based on the last status sample int he current 
     * data block (only call this after isDataReady
     */
    virtual bool isBatteryLow() const;
   
    /**
     * @return true if the battery is low in this status sample 
     */
    virtual bool isBatteryLow( const int &status ) const;

    /**
     * True if this is an MK2 system, only works after initialization
     */
    virtual bool isMK2() const;

    /**
     * @return true if the status sample says this is an MK2
     */
    virtual bool isMK2(const int &status) const;

protected:


    virtual int statusToMode(const int * data, int position) const;
    virtual int statusToMode(int status) const;

    virtual int determineNumChannels(int mode, bool isMk2) const;
    virtual int determineNumChannelsMk1(int mode) const;
    virtual int determineNumChannelsMk2(int mode) const;

    virtual int determineSamplingRate(int mode)const ;

    virtual void decodeStatus(const int * data, int position,
        bool &mk2, bool &battLow, int &mode, int &numChannels,
        int &samplingRate, bool &cmsWithinRange, bool &newEpoch) const;

    virtual void setupDriver();

    /** 
     * Calculate the real index into the data buffer
     * based on sample and channel, the start position of the data,
     * number of actual channels, and decimation factor
     *
     * Note: channel 0 is the sync, 1 is status, and 2 starts
     *      the EEG signal
     */
    inline virtual int calcIndex(const int &sample,
        const int &channel) const;

    inline virtual int calcIndex( const int &sample,
        const int &channel, const int &startPos ) const;


protected:

    //
    // C O N S T A N T S 
    // 

    static const int USB_DATA_SIZE = 64;

    static const int MK2_MASK = 0x80000000; // Status mask MK2 bit
    static const int BATT_LOW_MASK = 0x40000000; // Status mask battery low bit
    static const int CMS_WITHIN_RANGE_MASK=0x10000000; // Status mask if CMS
                                            // is within range
    static const int MODE_MASK = 0x2E000000; // Status mask for mode
    static const int MODE_SHIFT_TO_INT = 25; // How bits to shift the masked
                                             // mode bits, to get a mode int
    static const int NEW_EPOCH_MASK = 0x01000000; // status mask if this
                                // is a new epoch
    static const int IS_SYNCED_MASK = 0xFFFFFF00; // mask if this sample is
                                // synced


    // Mask if trigger is high using 32bits
    static const int TRIGGER_0  = 0x00000010;
    static const int TRIGGER_1  = 0x00000020;
    static const int TRIGGER_2  = 0x00000040;
    static const int TRIGGER_3  = 0x00000080;
    static const int TRIGGER_4  = 0x00000100;
    static const int TRIGGER_5  = 0x00000200;
    static const int TRIGGER_6  = 0x00000400;
    static const int TRIGGER_7  = 0x00000800;
    static const int TRIGGER_8  = 0x00001000;
    static const int TRIGGER_9  = 0x00002000;
    static const int TRIGGER_10 = 0x00004000;
    static const int TRIGGER_11 = 0x00008000;
    static const int TRIGGER_12 = 0x00010000;
    static const int TRIGGER_13 = 0x00020000;
    static const int TRIGGER_14 = 0x00080000;
    static const int TRIGGER_15 = 0x00100000;

    //
    // M E M B E R  V A R I A B L E S
    //

    /** to send start and stop codes to the usb driver*/
	PCHAR _usbdata;   

    /** Data buffer in bytes */
	PCHAR _data;      

    /** Data buffer as ints */
    int *_dataAsInt;

    /** True if this is an ActiveTwo Mk2 */
    bool _mk2; 

    /** 
     * True if the battery is low, Note this is only updated
     * at initialization and on calles to isBatteryLow
     */
    bool _battLow; 

    /**  sampling rate of biosemi in Hz */
    int _samplingRate; 

    /** The actual sample block size we are using based */
    int _sampleBlockSize;

    /** Sampling rate the user desires */
    int _desiredSamplingRate;

    /** Number of channels the user desires. */
    int _desiredNumChannels;

    /** Sample block size the user desires */
    int _desiredSampleBlockSize;

    /** 
     * Number of channels the biosemi is transmitting, including status 
     * and sync  
     */
    int _numChannels;

    /**
     * Biosemi's mode, only updated on initialization
     */
    int _mode;

    /**
     * handle to the biosemi device 
     */
     HANDLE _hdevice;

    /** the position of BIOSEMI's buffer cursor */
    PDWORD _bufferCursorPos;    

    /** 
     * Number of ints that were buffered between sucessive calls
     * to GET_POINTER
     */
    long _intsBuffered;

    /** The previous position of the buffer cursor */
    DWORD  _oldBufferCursorPos;

    /**
     * The position to start reading from the data buffer
     * as an int
     */
    long _startPos; 

    /** the next position to start reading in the data buffer */
    long _nextPos; 

    /** Number of ints read to read from the buffer */ 
    long _intsAvailable;         

    /** The number of ints that are ready to be ready, these are accumulated */
    int _numIntsToRead;

    /** true if the get data function has been called */
    bool _isDataReadyCalledYet;    

    /** if initialized was called */
    bool _wasDriverSetup;       

    /**
     * how many samples you need to skip to achieve
     * the desired sampling rate
     */
    int _decimationFactor;          
   
    /** 
     * Pointer to the datablock that helps the user travers the data
     * received on the last call to isDataReady
     */
    DataBlock *_dataBlock;

    /** 
     *true if we should throw an exception if the battery is low
     * when checking if the status is valid.
     */
    bool _throwBatteryLowException;

    /***********************************************************************
    To interface your own acquisition software directly with our USB2 interface,
    your acquisition software should make the function calls described below.
    The USB2 driver functions are in the library file called "Labview_DLL.dll".
    (this library file is included with ActiView). The functions used in the
    USB2 handshake are:

    HANDLE OPEN_DRIVER_ASYNC(void);
    BOOL USB_WRITE(HANDLE hdevice, PCHAR data);
    BOOL READ_MULTIPLE_SWEEPS(HANDLE hdevice,PCHAR data,DWORD nBytesToRead);
    BOOL READ_POINTER(HANDLE hdevice,PDWORD pointer);
    BOOL CLOSE_DRIVER_ASYNC(HANDLE hdevice);
    *************************************************************************/

    dOPEN_DRIVER_ASYNC OPEN_DRIVER_ASYNC;
    dUSB_WRITE USB_WRITE;
    dREAD_MULTIPLE_SWEEPS READ_MULTIPLE_SWEEPS;
    dREAD_POINTER READ_POINTER;
    dCLOSE_DRIVER_ASYNC CLOSE_DRIVER_ASYNC;

    //
    // I N N E R  C L A S S E S 
    //
public:
    /** 
     * Datablock presents the data with the sampleblockSize, number of 
     * channels, and sampingRate the caller desires (i.e. what
     * they passed to Biosemi2Client::initilaize
     *
     * Warning: Calls to isDataReady will change what data block DataBlock
     * accesses. So make sure you're done traversing the block before
     * calling isDataReady again, the DataBlock class cannot check for this
     */
    class DataBlock{
      public:

        DataBlock(  const Biosemi2Client *biosemi );
        virtual ~DataBlock();

        /**
         * Get's the data for the sample and channel specified. Acceses
         * basically like a 2d matrix. It ignores sync and status channels
         * so channel 0 is the first EEG channel
         * Signal is returned with least significat bit as 1/8192 microVolts
         * see http://www.biosemi.com/faq/make_own_acquisition_software.htm
         */
        inline virtual int getSignal(const int &sample, 
            const int &channel) const;

        /**
         * Gets the value of the trigger specified, scaled if desired.
         * scaled defaults to 1. Triggers are 0 for low and 1 for high
         * so scalling by 1000 would be 0 low 1000 high.
         * Scalling can be used to bring the value in range with
         * the signal for display purposes
         */
        inline virtual int getTrigger(const int &sample, const int &trigger,
            const int &scaled=1 ) const;

        /**
         * Loop through the sync and status channels and make sure this data
         *  is valid
         * Throws an exception stating what was invalid
         * otherwise returns true
         * TODO should probably return a status
         */
        virtual bool isDataValid() const;
        
        virtual const int &getNumChannels();

        virtual const int &getSampleBlockSize();

        virtual const int &getSamplingRate();

      protected:
        const Biosemi2Client *_biosemi;
    };

    /**
     * Allow DataBlock access to Biosem2Client's protected and private 
     * member variables
     */
    friend DataBlock;

};
#endif
