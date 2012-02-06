////////////////////////////////////////////////////////////////////////////////
// $Id: NullFileWriter.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file writer class that does _not_ write out any data.
//   Useful when no data file output is desired.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef NULL_FILE_WRITER_H
#define NULL_FILE_WRITER_H

#include "GenericFileWriter.h"

class NullFileWriter: public GenericFileWriter
{
 public:
          NullFileWriter();
  virtual ~NullFileWriter();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );
};

#endif // NULL_FILE_WRITER_H
