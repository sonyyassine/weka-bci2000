////////////////////////////////////////////////////////////////////////////////
// $Id: TrialStatistics.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates trial statistics for an application.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TRIAL_STATISTICS_H
#define TRIAL_STATISTICS_H

#include "Environment.h"
#include <vector>

class TrialStatistics: public EnvironmentExtension
{
  public:
    void  Publish() {}
    void  Preflight() const;
    void  Initialize();
    void  Process() {};

    void  Reset();
    void  Update( int inTargetCode, int inResultCode );
    void  UpdateInvalid();

    int   Hits()    const;
    int   Total()   const;
    int   Invalid() const;
    float Bits()    const;

  private:
    int                            mInvalidTrials;
    std::vector<std::vector<int> > mTargetsResultsMatrix;
};

#endif // TRIAL_STATISTICS_H
