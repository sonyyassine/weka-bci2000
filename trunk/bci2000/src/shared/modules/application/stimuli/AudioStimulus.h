////////////////////////////////////////////////////////////////////////////////
// $Id: AudioStimulus.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus that plays a sound, or speaks a text, when it is
//   presented.
//   When a sound string is enclosed in single quotes ('text'), it is rendered
//   using the OS's text-to-speech engine.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef AUDIO_STIMULUS_H
#define AUDIO_STIMULUS_H

#include "Stimulus.h"
#include <string>

class AudioStimulus : public Stimulus
{
 public:
  static const char cSpeechQuote = '\'';

  AudioStimulus();
  virtual ~AudioStimulus();
  // A sound is a file name or a text to speak when single-quoted (enclosed with '').
  AudioStimulus& SetSound( const std::string& );
  const std::string& Sound() const;
  AudioStimulus& SetVolume( float );
  float Volume() const;

 protected:
  virtual void OnPresent();
  virtual void OnConceal() {}

 private:
  Stimulus*   mpStimulus;
  std::string mSound;
  float       mVolume;
};

#endif // AUDIO_STIMULUS_H

