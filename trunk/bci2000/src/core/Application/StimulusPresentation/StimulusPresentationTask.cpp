////////////////////////////////////////////////////////////////////////////////
// $Id: StimulusPresentationTask.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Authors: jhizver@wadsworth.org, schalk@wadsworth.org,
//   juergen.mellinger@uni-tuebingen.de
// Description: The task filter for a stimulus presentation task.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StimulusPresentationTask.h"

#include "TextStimulus.h"
#include "ImageStimulus.h"
#include "AudioStimulus.h"
#include "Localization.h"
#include "MeasurementUnits.h"

#include <algorithm>

using namespace std;

RegisterFilter( StimulusPresentationTask, 3 );


StimulusPresentationTask::StimulusPresentationTask()
: mNumberOfSequences( 0 ),
  mSequenceType( SequenceTypes::Deterministic ),
  mStimulusDuration( 0 ),
  mPreSequenceDuration( 0 ),
  mPostSequenceDuration( 0 ),
  mBlockCount( 0 ),
  mToBeCopiedPos( mToBeCopied.begin() ),
  mSequencePos( mSequence.begin() )
{
  // Sequencing
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Sequencing intlist Sequence= 4 1 3 4 2 1 1 % // "
      "Sequence in which stimuli are presented (deterministic mode)/"
      " Stimulus frequencies for each stimulus (random mode)",

    "Application:Sequencing int SequenceType= 0 0 0 1 // "
      "Sequence of stimuli is 0 deterministic, 1 random (enumeration)",

    "Application:Sequencing int NumberOfSequences= 3 1 0 % // "
      "number of sequence repetitions in a run",

    "Application:Sequencing intlist ToBeCopied= 3 1 2 3 1 1 % // "
      "Sequence in which stimuli need to be copied (only used in copy mode)",

    "Application:Sequencing string UserComment= % % % % // "
      "User comments for a specific run",
  END_PARAMETER_DEFINITIONS

  // Stimulus definition
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Stimuli matrix Stimuli= "
      "{ caption icon audio } " // row labels
      "{ stimulus1 stimulus2 stimulus3 stimulus4 stimulus5 stimulus6 } " // column labels
      " One Two Three Four Five Six "
      "images\\1.bmp images\\2.bmp images\\3.bmp images\\4.bmp images\\5.bmp images\\6.bmp "
      "sounds\\1.wav sounds\\2.wav sounds\\3.wav sounds\\4.wav sounds\\5.wav sounds\\6.wav  "
      " // captions and icons to be displayed, sounds to be played for different stimuli",

    "Application:Stimuli matrix FocusOn= "
      "{ caption icon audio } " // row labels
      "{ focuson } " // column labels
      "Please%20focus%20on "     // caption
      "images\\focuson.bmp " // video
      "sounds\\uh-uh.wav  "  // audio
      " // initial announcement what to focus on",

    "Application:Stimuli matrix Result= "
      "{ caption icon audio } " // row labels
      "{ result } " // column labels
      "The%20result%20was "     // caption
      "images\\result.bmp " // video
      "sounds\\uh-uh.wav "  // audio
      " // final result announcement ",
  END_PARAMETER_DEFINITIONS

  // Stimulus properties
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Stimuli int StimulusWidth= 5 0 0 100 // "
       "StimulusWidth in percent of screen width (zero for original pixel size)",
    "Application:Stimuli int CaptionHeight= 10 0 0 100 // "
       "Height of stimulus caption text in percent of screen height",
    "Application:Stimuli string CaptionColor= 0x00FFFFFF 0x00FFFFFF 0x00000000 0x00000000 // "
       "Color of stimulus caption text (color)",
    "Application:Stimuli string BackgroundColor= 0x00FFFF00 0x00FFFF00 0x00000000 0x00000000 // "
       "Color of stimulus background (color)",
    "Application:Stimuli int CaptionSwitch= 1 1 0 1 // "
       "Present captions (boolean)",
    "Application:Stimuli int IconSwitch= 1 1 0 1 // "
       "Present icon files (boolean)",
    "Application:Stimuli int AudioSwitch= 1 1 0 1 // "
       "Present audio files (boolean)",
    "Application:Stimuli float AudioVolume= 100 100 0 100 // "
       "Volume for audio playback in percent",
  END_PARAMETER_DEFINITIONS

  LANGUAGES "German",
  BEGIN_LOCALIZED_STRINGS
   "TIME OUT !!!",
           "Zeit abgelaufen!",
   "Waiting to start ...",
           "Warte ...",
  END_LOCALIZED_STRINGS

  BEGIN_STATE_DEFINITIONS
    "SelectedStimulus 16 0 0 0",
  END_STATE_DEFINITIONS
}

StimulusPresentationTask::~StimulusPresentationTask()
{
  mStimuli.DeleteObjects();
  mTargets.DeleteObjects();
}

void
StimulusPresentationTask::OnPreflight( const SignalProperties& /*Input*/ ) const
{
  GUI::GraphDisplay preflightDisplay;
  ImageStimulus* pImageStimulus = NULL;
  if( Parameter( "IconSwitch" ) == 1 )
    pImageStimulus = new ImageStimulus( preflightDisplay );
  AudioStimulus* pAudioStimulus = NULL;
  if( Parameter( "AudioSwitch" ) == 1 )
    pAudioStimulus = new AudioStimulus;

  bool presentFocusOn = false,
       presentResult = false;

  switch( int( Parameter( "InterpretMode" ) ) )
  {
    case InterpretModes::Copy:
      presentFocusOn = true;
      presentResult = ( Parameter( "DisplayResults" ) == 1 );
      Parameter( "ToBeCopied" );
      break;

    case InterpretModes::Free:
      presentFocusOn = false;
      presentResult = ( Parameter( "DisplayResults" ) == 1 );
      break;

    case InterpretModes::None:
    default:
      presentFocusOn = false;
      presentResult = false;
  }
  int focusDuration = MeasurementUnits::ReadAsTime(
        StimulusProperty( Parameter( "FocusOn" ), 0, "StimulusDuration" ) ),
      resultDuration = MeasurementUnits::ReadAsTime(
        StimulusProperty( Parameter( "Result" ), 0, "StimulusDuration" ) ),
      preSequenceDuration = MeasurementUnits::ReadAsTime( Parameter( "PreSequenceDuration" ) ),
      postSequenceDuration = MeasurementUnits::ReadAsTime( Parameter( "PostSequenceDuration" ) );

  if( presentFocusOn && preSequenceDuration < 2 * focusDuration )
    bcierr << "When FocusOn message and target stimulus are presented, "
           << "PreSequenceDuration must at least be twice the StimulusDuration "
           << "applying for the FocusOn message"
           << endl;
  if( presentResult && postSequenceDuration < 2 * resultDuration )
    bcierr << "When Result message and result stimulus are presented, "
           << "PostSequenceDuration must at least be twice the StimulusDuration "
           << "applying for the Result message"
           << endl;

  vector<string> stimParams;
  stimParams.push_back( "Stimuli" );
  if( presentFocusOn )
    stimParams.push_back( "FocusOn" );
  if( presentResult )
    stimParams.push_back( "Result" );

  // For parameters defining a time value, issue a warning if limited temporal
  // resolution leads to a discrepancy greater than 1ms.
  const char* timeParams[] =
  {
    "StimulusDuration",
    "ISIMinDuration",
    "ISIMaxDuration",
  };
  float oneMillisecond = MeasurementUnits::ReadAsTime( "1ms" );
  int minStimDuration = 0,
      minISIDuration = 0,
      epochLength = MeasurementUnits::ReadAsTime( OptionalParameter( "EpochLength", 0 ) );

  enum { caption, icon, audio };
  for( size_t i = 0; i < stimParams.size(); ++i )
    for( int j = 0; j < Parameter( stimParams[ i ] )->NumColumns(); ++j )
    {
      int stimDuration = MeasurementUnits::ReadAsTime(
        StimulusProperty( Parameter( stimParams[ i ] ), j, "StimulusDuration" ) ),
      isiDuration = MeasurementUnits::ReadAsTime(
        StimulusProperty( Parameter( stimParams[ i ] ), j, "ISIMinDuration" ) );
      if( minStimDuration > stimDuration )
        minStimDuration = stimDuration;
      if( minISIDuration > isiDuration )
        minISIDuration = isiDuration;
      for( size_t k = 0; k < sizeof( timeParams ) / sizeof( *timeParams ); ++k )
      { // Check individual stimulus durations.
        float value = MeasurementUnits::ReadAsTime(
          StimulusProperty( Parameter( stimParams[ i ] ), j, timeParams[ k ] )
        );
        if( value < 1.0f || ::fmod( value, 1.0f ) > oneMillisecond )
          bciout << "Due to a sample block duration of "
                 << 1.0f / oneMillisecond << "ms,"
                 << " the actual value of " << timeParams[ k ]
                 << " for stimulus " << j + 1 << " will be "
                 << ::floor( value ) / oneMillisecond << "ms"
                 << " rather than "
                 << value / oneMillisecond << "ms"
                 << endl;
      }
      // Test availability of icon and audio files.
      if( pImageStimulus != NULL )
        pImageStimulus->SetFile( Parameter( stimParams[ i ] )( icon, j ) );
      if( pAudioStimulus != NULL )
        pAudioStimulus->SetSound( Parameter( stimParams[ i ] )( audio, j ) );
    }

  int minStimToClassInterval = minStimDuration + minISIDuration + postSequenceDuration;
  if( presentResult && epochLength > minStimToClassInterval )
    bcierr << "EpochLength is " << epochLength / oneMillisecond << "ms, exceeds "
           << "allowable maximum of " << minStimToClassInterval / oneMillisecond << "ms. "
           << "This maximum corresponds to the condition that classification "
           << "must be finished before the next sequence of stimuli may begin. "
           << "Increase the sum of the "
           << "StimulusDuration, ISIMinDuration, and PostSequenceDuration "
           << "parameters, or decrease EpochLength, to fix this problem"
           << endl;

  delete pImageStimulus;
  delete pAudioStimulus;
}

void
StimulusPresentationTask::OnInitialize( const SignalProperties& /*Input*/ )
{
  // Read sequence parameters.
  mNumberOfSequences = Parameter( "NumberOfSequences" );
  mSequenceType = Parameter( "SequenceType" );
  mToBeCopied.clear();

  bool presentFocusOn = false,
       presentResult = false;
  switch( int( Parameter( "InterpretMode" ) ) )
  {
    case InterpretModes::Copy:
      presentFocusOn = true;
      presentResult = ( Parameter( "DisplayResults" ) == 1 );
      // Sequence of stimuli to attend.
      for( int i = 0; i < Parameter( "ToBeCopied" )->NumValues(); ++i )
        mToBeCopied.push_back( Parameter( "ToBeCopied" )( i ) );
      mToBeCopiedPos = mToBeCopied.begin();
      break;

    case InterpretModes::Free:
      presentFocusOn = false;
      presentResult = ( Parameter( "DisplayResults" ) == 1 );
      break;

    case InterpretModes::None:
    default:
      presentFocusOn = false;
      presentResult = false;
  }

  // This is used for announcement and results stimuli.
  mStimulusDuration = MeasurementUnits::ReadAsTime( Parameter( "StimulusDuration" ) );
  mPostSequenceDuration = MeasurementUnits::ReadAsTime( Parameter( "PostSequenceDuration" ) );
  mPreSequenceDuration = MeasurementUnits::ReadAsTime( Parameter( "PreSequenceDuration" ) );

  // Create stimuli and targets
  mStimuli.DeleteObjects();
  mTargets.DeleteObjects();
  Associations().clear();

  bool captionSwitch = ( Parameter( "CaptionSwitch" ) == 1 ),
       iconSwitch = ( Parameter( "IconSwitch" ) == 1 ),
       audioSwitch = ( Parameter( "AudioSwitch" ) == 1 );

  RGBColor backgroundColor = RGBColor::NullColor;
  if( !iconSwitch )
    backgroundColor = RGBColor( Parameter( "BackgroundColor" ) );

  enum { caption, icon, audio };
  ParamRef Stimuli = Parameter( "Stimuli" );
  for( int i = 0; i < Stimuli->NumColumns(); ++i )
  {
    Associations()[ i + 1 ].SetStimulusDuration(
      MeasurementUnits::ReadAsTime( StimulusProperty( Stimuli, i, "StimulusDuration" ) ) );
    Associations()[ i + 1 ].SetISIMinDuration(
      MeasurementUnits::ReadAsTime( StimulusProperty( Stimuli, i, "ISIMinDuration" ) ) );
    Associations()[ i + 1 ].SetISIMaxDuration(
      MeasurementUnits::ReadAsTime( StimulusProperty( Stimuli, i, "ISIMaxDuration" ) ) );

    float stimulusWidth = StimulusProperty( Stimuli, i, "StimulusWidth" ) / 100.0,
          captionHeight = StimulusProperty( Stimuli, i, "CaptionHeight" ) / 100.0,
          audioVolume = StimulusProperty( Stimuli, i, "AudioVolume" ) / 100.0;
    RGBColor captionColor = RGBColor( StimulusProperty( Stimuli, i, "CaptionColor" ) );

    if( captionSwitch )
    {
      GUI::Rect captionRect =
      {
        0.5, ( 1 - captionHeight ) / 2,
        0.5, ( 1 + captionHeight ) / 2
      };
      TextStimulus* pStimulus = new TextStimulus( Display() );
      pStimulus->SetText( Stimuli( caption, i ) )
                .SetTextHeight( 1.0 )
                .SetTextColor( captionColor )
                .SetColor( backgroundColor )
                .SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth )
                .SetDisplayRect( captionRect );
      pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
      mStimuli.Add( pStimulus );
      Associations()[ i + 1 ].Add( pStimulus );
    }
    if( iconSwitch )
    {
      int iconSizeMode = GUI::AspectRatioModes::AdjustHeight;
      if( stimulusWidth <= 0 )
        iconSizeMode = GUI::AspectRatioModes::AdjustBoth;
      GUI::Rect iconRect =
      {
        ( 1 - stimulusWidth ) / 2, 0.5,
        ( 1 + stimulusWidth ) / 2, 0.5
      };
      ImageStimulus* pStimulus = new ImageStimulus( Display() );
      pStimulus->SetFile( Stimuli( icon, i ) )
                .SetRenderingMode( GUI::RenderingMode::Opaque )
                .SetAspectRatioMode( iconSizeMode )
                .SetDisplayRect( iconRect );
      pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
      mStimuli.Add( pStimulus );
      Associations()[ i + 1 ].Add( pStimulus );
    }
    if( audioSwitch )
    {
      AudioStimulus* pStimulus = new AudioStimulus;
      pStimulus->SetSound( Stimuli( audio, i ) )
                .SetVolume( audioVolume );
      mStimuli.Add( pStimulus );
      Associations()[ i + 1 ].Add( pStimulus );
    }
    Target* pTarget = new Target;
    pTarget->SetTag( i + 1 );
    mTargets.Add( pTarget );
    Associations()[ i + 1 ].Add( pTarget );
  }

  // Create FocusOn stimuli
  mFocusAnnouncement.DeleteObjects();
  if( presentFocusOn )
  {
    ParamRef FocusOn = Parameter( "FocusOn" );
    mFocusAnnouncement.SetStimulusDuration(
      MeasurementUnits::ReadAsTime( StimulusProperty( FocusOn, 0, "StimulusDuration" ) ) );
    for( int i = 0; i < FocusOn->NumColumns(); ++i )
    {
      float stimulusWidth = StimulusProperty( FocusOn, i, "StimulusWidth" ) / 100.0,
            captionHeight = StimulusProperty( FocusOn, i, "CaptionHeight" ) / 100.0,
            audioVolume = StimulusProperty( FocusOn, i, "AudioVolume" ) / 100.0;
      RGBColor captionColor = RGBColor( StimulusProperty( FocusOn, i, "CaptionColor" ) );

      if( captionSwitch )
      {
        GUI::Rect captionRect =
        {
          0.5, ( 1 - captionHeight ) / 2,
          0.5, ( 1 + captionHeight ) / 2
        };
        TextStimulus* pStimulus = new TextStimulus( Display() );
        pStimulus->SetText( FocusOn( caption, i ) )
                  .SetTextHeight( 1.0 )
                  .SetTextColor( captionColor )
                  .SetColor( backgroundColor )
                  .SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth )
                  .SetDisplayRect( captionRect );
        pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
        mFocusAnnouncement.Add( pStimulus );
      }
      if( iconSwitch )
      {
        int iconSizeMode = GUI::AspectRatioModes::AdjustHeight;
        if( stimulusWidth <= 0 )
          iconSizeMode = GUI::AspectRatioModes::AdjustBoth;
        GUI::Rect iconRect =
        {
          ( 1 - stimulusWidth ) / 2, 0.5,
          ( 1 + stimulusWidth ) / 2, 0.5
        };
        ImageStimulus* pStimulus = new ImageStimulus( Display() );
        pStimulus->SetFile( FocusOn( icon, i ) )
                  .SetRenderingMode( GUI::RenderingMode::Opaque )
                  .SetAspectRatioMode( iconSizeMode )
                  .SetDisplayRect( iconRect );
        pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
        mFocusAnnouncement.Add( pStimulus );
      }
      if( audioSwitch )
      {
        AudioStimulus* pStimulus = new AudioStimulus;
        pStimulus->SetSound( FocusOn( audio, i ) )
                  .SetVolume( audioVolume );
        mFocusAnnouncement.Add( pStimulus );
      }
    }
  }
  // Create result stimuli
  mResultAnnouncement.DeleteObjects();
  if( presentResult )
  {
    ParamRef Result = Parameter( "Result" );
    mResultAnnouncement.SetStimulusDuration(
      MeasurementUnits::ReadAsTime( StimulusProperty( Result, 0, "StimulusDuration" ) ) );
    for( int i = 0; i < Result->NumColumns(); ++i )
    {
      float stimulusWidth = StimulusProperty( Result, i, "StimulusWidth" ) / 100.0,
            captionHeight = StimulusProperty( Result, i, "CaptionHeight" ) / 100.0,
            audioVolume = StimulusProperty( Result, i, "AudioVolume" ) / 100.0;
      RGBColor captionColor = RGBColor( StimulusProperty( Result, i, "CaptionColor" ) );

      if( captionSwitch )
      {
        GUI::Rect captionRect =
        {
          0.5, ( 1 - captionHeight ) / 2,
          0.5, ( 1 + captionHeight ) / 2
        };
        TextStimulus* pStimulus = new TextStimulus( Display() );
        pStimulus->SetText( Result( caption, i ) )
                  .SetTextHeight( 1.0 )
                  .SetTextColor( captionColor )
                  .SetColor( backgroundColor )
                  .SetAspectRatioMode( GUI::AspectRatioModes::AdjustWidth )
                  .SetDisplayRect( captionRect );
        pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
        mResultAnnouncement.Add( pStimulus );
      }
      if( iconSwitch )
      {
        int iconSizeMode = GUI::AspectRatioModes::AdjustHeight;
        if( stimulusWidth <= 0 )
          iconSizeMode = GUI::AspectRatioModes::AdjustBoth;
        GUI::Rect iconRect =
        {
          ( 1 - stimulusWidth ) / 2, 0.5,
          ( 1 + stimulusWidth ) / 2, 0.5
        };
        ImageStimulus* pStimulus = new ImageStimulus( Display() );
        pStimulus->SetFile( Result( icon, i ) )
                  .SetRenderingMode( GUI::RenderingMode::Opaque )
                  .SetAspectRatioMode( iconSizeMode )
                  .SetDisplayRect( iconRect );
        pStimulus->SetPresentationMode( VisualStimulus::ShowHide );
        mResultAnnouncement.Add( pStimulus );
      }
      if( audioSwitch )
      {
        AudioStimulus* pStimulus = new AudioStimulus;
        pStimulus->SetSound( Result( audio, i ) )
                  .SetVolume( audioVolume );
        mResultAnnouncement.Add( pStimulus );
      }
    }
  }
}

void
StimulusPresentationTask::OnStartRun()
{
  DisplayMessage( LocalizableString( "Waiting to start ..." ) );

  // Create a sequence for this run.
  mSequence.clear();
  int numSubSequences = mNumberOfSequences;
  if( Parameter( "InterpretMode" ) == InterpretModes::Copy )
    numSubSequences *= mToBeCopied.size();
  switch( mSequenceType )
  {
    case SequenceTypes::Deterministic:
      for( int i = 0; i < numSubSequences; ++i )
      {
        for( int j = 0; j < Parameter( "Sequence" )->NumValues(); ++j )
          mSequence.push_back( Parameter( "Sequence" )( j ) );
        mSequence.push_back( 0 );
      }
      break;

    case SequenceTypes::Random:
      {
        vector<int> seq;
        for( int i = 0; i < Parameter( "Sequence" )->NumValues(); ++i )
          for( int j = 0; j < Parameter( "Sequence" )( i ); ++j )
            seq.push_back( i + 1 );

        for( int i = 0; i < numSubSequences; ++i )
        {
          random_shuffle( seq.begin(), seq.end(), RandomNumberGenerator );
          mSequence.insert( mSequence.end(), seq.begin(), seq.end() );
          mSequence.push_back( 0 );
        }
      }
      break;
  }
  mSequencePos = mSequence.begin();
  mToBeCopiedPos = mToBeCopied.begin();
  DetermineAttendedTarget();
}

void
StimulusPresentationTask::OnStopRun()
{
  DisplayMessage( LocalizableString( "TIME OUT !!!" ) );
}

void
StimulusPresentationTask::OnPreSequence()
{
  DisplayMessage( "" );
  State( "SelectedStimulus" ) = 0;
  mFocusAnnouncement.Present();
  mBlockCount = 0;
}

void
StimulusPresentationTask::DoPreSequence( const GenericSignal&, bool& /*doProgress*/ )
{
  if( mBlockCount == mFocusAnnouncement.StimulusDuration() )
  {
    mFocusAnnouncement.Conceal();
    if( AttendedTarget() != NULL )
      Associations()[ AttendedTarget()->Tag() ].Present();
  }

  if( mBlockCount == 2 * mFocusAnnouncement.StimulusDuration() )
  {
    if( AttendedTarget() != NULL )
      Associations()[ AttendedTarget()->Tag() ].Conceal();
  }

  ++mBlockCount;
}

void
StimulusPresentationTask::OnSequenceBegin()
{
  mFocusAnnouncement.Conceal();
}

void
StimulusPresentationTask::OnPostRun()
{
  State( "SelectedStimulus" ) = 0;
}

Target*
StimulusPresentationTask::OnClassResult( const ClassResult& inResult )
{
  Target* pTarget = StimulusTask::OnClassResult( inResult );
  if( pTarget != NULL )
  {
    State( "SelectedStimulus" ) = pTarget->Tag();
    mResultAnnouncement.Present();
    mBlockCount = 0;
    DetermineAttendedTarget();
  }
  return pTarget;
}

void
StimulusPresentationTask::DoPostSequence( const GenericSignal&, bool& /*doProgress*/ )
{
  if( mBlockCount == mResultAnnouncement.StimulusDuration() )
  {
    mResultAnnouncement.Conceal();
    if( State( "SelectedStimulus" ) > 0 )
      Associations()[ State( "SelectedStimulus" ) ].Present();
  }

  if( mBlockCount == 2 * mResultAnnouncement.StimulusDuration() )
  {
    if( State( "SelectedStimulus" ) > 0 )
      Associations()[ State( "SelectedStimulus" ) ].Conceal();
  }

  ++mBlockCount;
}

int
StimulusPresentationTask::OnNextStimulusCode()
{
  return mSequencePos == mSequence.end() ? 0 : *mSequencePos++;
}


void
StimulusPresentationTask::DetermineAttendedTarget()
{
  Target* pTarget = NULL;

  if( mToBeCopiedPos == mToBeCopied.end() )
    mToBeCopiedPos = mToBeCopied.begin();

  if( mToBeCopiedPos != mToBeCopied.end() )
  {
    int code = *mToBeCopiedPos++;
    SetOfTargets::const_iterator i = mTargets.begin();
    while( pTarget == NULL && i != mTargets.end() )
    {
      if( ( *i )->Tag() == code )
        pTarget = *i;
      ++i;
    }
  }
  SetAttendedTarget( pTarget );
}

ParamRef
StimulusPresentationTask::StimulusProperty( const ParamRef& inMatrixParam,
                                            int inColIndex,
                                            const std::string& inProperty ) const
{
  return inMatrixParam->RowLabels().Exists( inProperty )
         ? inMatrixParam( inProperty, inColIndex )
         : Parameter( inProperty );
}



