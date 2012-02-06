////////////////////////////////////////////////////////////////////////////////
//  $Id: SaveDataFilter.h 1723 2008-04-16 17:46:33Z villalon $
//  Author:      villalon@ee.usyd.edu.au
//  Description: A filter to save the signals after applying the BCI2000 filters
//
// (C) 2000-2008, Learning Systems Group, University of Sydney
// http://www.weg.usyd.edu.au
////////////////////////////////////////////////////////////////////////////////
#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include "GenericFilter.h"
#include "FileWriterBase.h"
#include "BCIDirectory.h"
#include <iostream>
#include <fstream>

class SaveDataFilter : public FileWriterBase
{
 public:
   SaveDataFilter();
   ~SaveDataFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
  const char* DataFileExtension() const { return ".dat"; }

   ofstream				myFile;
};
#endif // SAVE_DATA_H
