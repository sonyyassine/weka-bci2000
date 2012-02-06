////////////////////////////////////////////////////////////////////////////////
// $Id: SerialStream.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::iostream interface for blocking serial communication.
//              serialstream: A std::iostream interface to the data stream on a
//                serial line. Will wait for flush or eof before sending data.
//                Send/receive is blocking; one can use rdbuf()->in_avail()
//                to check for data.
//              serialbuf: A helper class that does the actual send/receive
//                calls.
//              NOTE: Configuration of the serial interface is independent of
//                stream communication, and not provided here.
//                Configuration must take place before opening the interface
//                from the serialstream class.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SERIAL_STREAM_H
#define SERIAL_STREAM_H

#include <iostream>

class serialbuf : public std::streambuf
{
    enum
    {
      buf_size = 512,
    };

  public:
    enum
    {
      infiniteTimeout = -1,
      defaultTimeout = 500, // ms
    };

  public:
    serialbuf();
    virtual ~serialbuf();
    void set_timeout( int t );
    int  get_timeout() const        { return m_timeout; }
    bool is_open() const;
    serialbuf* open( const char* device );
    serialbuf* close();

  protected:
    virtual int showmanyc();        // Called from streambuf::in_avail().

  protected:
    virtual int underflow();        // Called from read operations if empty.
    virtual int overflow( int c );  // Called if write buffer is filled.
    virtual int sync();             // Called from iostream::flush().

  protected:
    void* m_handle;
    int   m_timeout;
};

class serialstream : public std::iostream
{
  public:
    serialstream();
    explicit serialstream( const char* device );
    virtual ~serialstream()         {}
    bool is_open() const            { return buf.is_open(); }
    void open( const char* device );
    void close();

  private:
    serialbuf buf;
};

#endif // SERIAL_STREAM_H
