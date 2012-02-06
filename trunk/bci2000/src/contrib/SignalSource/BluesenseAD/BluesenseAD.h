#ifndef BLUESENSE_AD_H
#define BLUESENSE_AD_H

#include <string>
#include "GenericADC.h"

class BluesenseAD : public GenericADC
{
 public:
   BluesenseAD();
   ~BluesenseAD();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );
   void Halt();

 private:
   int  mSourceCh,
        mSampleBlockSize,
        mSamplingRate,
        mBaudRate;
   std::string mComPort;
};
#endif // BLUESENSE_AD_H

 