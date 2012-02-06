////////////////////////////////////////////////////////////////////////////////
// $Id: GenericFileWriter.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual class interface for output filters encapsulating
//       output file formats.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_FILE_WRITER_H
#define GENERIC_FILE_WRITER_H

#include "GenericFilter.h"

class GenericFileWriter : public GenericFilter
{
 protected: // No instantiation outside derived classes.
  GenericFileWriter() {}
 public:
  virtual ~GenericFileWriter() {}
  // A separate function for publishing parameters and states.
  virtual void Publish() const = 0;
  // GenericFilter inherited functions.
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const = 0;
  virtual void Initialize( const SignalProperties&,
                           const SignalProperties& ) = 0;
  virtual void StartRun() {}
  virtual void StopRun() {}
  // The write function takes as argument the state vector
  // that existed at the time of the signal argument's time stamp.
  virtual void Write( const GenericSignal&, const StateVector& ) = 0;
  virtual void Halt() {}

  virtual bool AllowsVisualization() const { return false; }

 private:
  // The Process() function should not be called for the
  // GenericFileWriter class.
  virtual void Process( const GenericSignal&,
                              GenericSignal& ) {};
};

#endif // GENERIC_FILE_WRITER_H

