////////////////////////////////////////////////////////////////////////////////
// $Id: TransmissionFilter.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that returns a subset of input channels in its output
//              signal.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TRANSMISSION_FILTER_H
#define TRANSMISSION_FILTER_H

#include "GenericFilter.h"

#include <vector>

class TransmissionFilter: public GenericFilter
{
 public:
  TransmissionFilter();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output );
 private:
   typedef std::vector<size_t> mChannelList_type;
   mChannelList_type           mChannelList;
};

#endif // TRANSMISSION_FILTER_H
