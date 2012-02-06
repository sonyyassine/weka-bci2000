////////////////////////////////////////////////////////////////////////////////
// $Id: Stimulus.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The Stimulus class is a virtual base class which defines a
//   calling interface for stimuli.
//   A "Stimulus" is defined as "an object that can present and conceal itself."
//
//   Descendants of the Stimulus class may be members of one or more sets of
//   stimuli (class StimulusSet), which in turn are associated with stimulus
//   codes (class StimulusMap).
//   The StimulusTask framework class uses an AssociationMap to keep track of
//   the association between stimulus codes, stimuli, and targets.
//   Examples:
//   - For the P3Speller application, stimulus codes represent individual rows
//     or columns, and thus associations contain stimuli (matrix elements) from
//     rows and columns.
//   - For the StimulusPresentation application, stimulus codes are associated
//     with single stimuli, thus associations contain one stimulus each.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STIMULUS_H
#define STIMULUS_H

#include <set>
#include <map>

class Stimulus
{
 public:
  Stimulus()
    : mTag( 0 )
    {}
  virtual ~Stimulus()
    {}
  // Properties
  Stimulus& SetTag( int inTag )
    { mTag = inTag; return *this; }
  int Tag() const
    { return mTag; }
  // Event calling interface
  Stimulus& Present()
    { OnPresent(); return *this; }
  Stimulus& Conceal()
    { OnConceal(); return *this; }

 protected:
  // Event handling interface
  //  In its OnPresent event handler, a stimulus is supposed to present itself
  //  (e.g., to make itself visible, play itself if it is a sound or a movie,
  //  or highlight itself if it is a P300 matrix element).
  virtual void OnPresent() = 0;
  //  In its OnConceal event handler, a stimulus is supposed to conceal itself
  //  (e.g., make itself invisible, stop playback, or switch back to normal mode).
  //  This event is called Conceal rather than Hide because "Hide" is already
  //  used for making a graphic element invisible.
  virtual void OnConceal() = 0;

 private:
  int mTag;
};

class SetOfStimuli : public std::set<Stimulus*>
{
 public:
  SetOfStimuli()
    {}
  virtual ~SetOfStimuli()
    {}
  // Householding
  SetOfStimuli& Add( Stimulus* s )
    { insert( s ); return *this; }
  SetOfStimuli& Remove( Stimulus* s )
    { erase( s ); return *this; }
  SetOfStimuli& Clear()
    { clear(); return *this; }
  SetOfStimuli& DeleteObjects()
    { for( iterator i = begin(); i != end(); ++i ) delete *i; clear(); return *this; }

  bool Contains( Stimulus* s ) const
    { return find( s ) != end(); }
  bool Intersects( const SetOfStimuli& s ) const
    {
      for( const_iterator i = begin(); i != end(); ++i )
        if( s.Contains( *i ) ) return true;
      return false;
    }

  // Events
  void Present()
    { for( iterator i = begin(); i != end(); ++i ) ( *i )->Present(); }
  void Conceal()
    { for( iterator i = begin(); i != end(); ++i ) ( *i )->Conceal(); }
};

#endif // STIMULUS_H

