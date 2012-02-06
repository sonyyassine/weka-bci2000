/////////////////////////////////////////////////////////////////////////////
//
// File: ArtifactView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef ARTIFACT_VIEW_H
#define ARTIFACT_VIEW_H

#include "PresView.h"
#include "StateAccessor.h"
#include "GUIArtifactView.h"

class MidiPlayer;

class TArtifactView : public TPresView, private TGUIArtifactView
{
  public:
                        TArtifactView( ParamList *inParamList );
    virtual             ~TArtifactView();

    virtual TPresError  Initialize(         ParamList   *inParamList,
                                    const   TGUIRect    &inRect );
    
    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& );
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& );
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
            TStateAccessor  artifact;
            MidiPlayer      *midiPlayer;
            int             visInvalid,
                            audInvalid;
};

#endif // ARTIFACT_VIEW_H

 
