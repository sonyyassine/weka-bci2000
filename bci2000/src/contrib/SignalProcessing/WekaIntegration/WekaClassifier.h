////////////////////////////////////////////////////////////////////////////////
// $Id: WekaClassifier.h 2008-04-30 17:46:33Z villalon $
// Description: This filter sends the signal as a semicolon separated list of
//   numbers to a WekaServer running on a particular host.
//   The WekaServer loads trained classifiers from a folder in the server
//   filesystem and uses them to classify the signal sent from BCI, it then
//   answers with a code (200 if successful) and the predicted class for the
//   signal.
//   The WekaServer can also save the data for training purposes, basically
//   saves where the target is on the screen, the signal and the predicted
//   class.
//
// (C) 2007-2008, Learning Systems Group, University of Sydney
// http://www.weg.usyd.edu.au
////////////////////////////////////////////////////////////////////////////////
#ifndef MyClassFilterH
#define MyClassFilterH

#include "FileWriterBase.h"
#include "GenericFilter.h"
#include "SockStream.h"

#include <string>

using namespace std;

class WekaClassifier : public GenericFilter
{

 private:

  float UDthreshold;
  float LRthreshold;
  int isNominal;
  string attributeSelectionList;
  vector<int> attributesIndices;
  int classifierDisabled;
  
  // Stream to communicate to the WekaServer socket
  sockstream connection;
  // The TCP socket for communication
  client_tcpsocket socket;
  // Timeout for the socket
  int timeout;

  // Posts a message to the server
  void WekaClassifier::postMessage(const char* );
  // Reads the answer from the server
  string WekaClassifier::readAnswer();

  // Utilities to process the server answer

  // Gets the predicted class from a response to a SIGNAL message
  float WekaClassifier::getClassFromResponse(string);
  // Gets the code for the answer from the server
  int WekaClassifier::getCodeFromResponse(string);
  // Gets the message body from the server's answer
  string WekaClassifier::getMessageFromResponse(string);
  //
  void WekaClassifier::splitString(const string& , vector<int>& , const string& );
  //
  bool WekaClassifier::isAttributeIncluded(int );

  // All public methods are just to comply with the GenericFilter
  // interface
 public:
  WekaClassifier();
  ~WekaClassifier();
  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize( const SignalProperties&, const SignalProperties& );
  void Process(const GenericSignal& , GenericSignal& );
};
#endif
