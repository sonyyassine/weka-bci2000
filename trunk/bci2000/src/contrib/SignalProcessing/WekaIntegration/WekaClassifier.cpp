////////////////////////////////////////////////////////////////////////////////
// $Id: WekaClassifier.cpp 2008-04-30 17:46:33Z villalon $
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
#include "PCHIncludes.h"
#pragma hdrstop
//------------------------------------------------------------------------------

#include "WekaClassifier.h"
#include "BCIError.h"

using namespace std;

// This is useful only when the pipe definition is defined without compiling
RegisterFilter( WekaClassifier, 2.E4 );

// Constructor of the class, reserverd only for parameter definitions
WekaClassifier::WekaClassifier()
{
   // Parameter declaration for the WekaClassifier filter the syntax is:
   // [Window where it appears] [type] [Parameter name]= [Default value]
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering int WekaDisableClassifier= 0 1 0 1 "
	  "// disables the classifying task, allowing only to save (0=no, 1=yes) (boolean)",
   "Filtering string WekaClassifier= Sample "
	 "Sample a z // File with the trained classifier (without extension)",
   "Filtering int ClassifierIsNominal= 0 1 0 1 "
	  "// the weka classifier is nominal (0=no, 1=yes) (boolean)",
   "Filtering float NumericThresholdLR= 1.5 1.5 % % "
	  "// threshold for the numeric classifier moving Left and Right",
   "Filtering float NumericThresholdUD= 3.5 3.5 % % "
	  "// threshold for the numeric classifier moving Up and Down",
   "Filtering string ClassifierAttributeList= all "
	 "all % % // space separated list of attribute indices (e.g. 1 2 4)",
   "Filtering int WekaSaveTrainingFile= 0 1 0 1 "
	  "// save weka file for training (0=no, 1=yes) (boolean)",
   "Filtering string WekaFilePrefix= WekaFile "
	 "WekaFile a z // file prefix to save the Weka data",
   "Filtering string WekaServerIP= 127.0.0.1 "
	 "% % % // IP address of the Weka server",
   "Filtering string WekaServerPort= 4444 "
	 "4444 1000 9999 // TCP port of the Weka server",
   "Filtering string WekaTimeout= 100 "
	 "100 0 9999 // Timeout for TCP requests (in milliseconds)",
 END_PARAMETER_DEFINITIONS
}

// Destructor of the class, called when the program is closing
WekaClassifier::~WekaClassifier()
{
  // Checks if the socket to the WekaServer is still open, in which case
  // we send a BYE message and close the connection.
  if(connection.is_open()) {
	  postMessage("BYE\n");
	  connection.close();
  }
}

// Preflight is executed when configuration is set, to validate
// if all the required conditions for the filter are met
void
WekaClassifier::Preflight( const SignalProperties& inSignalProperties,
						   SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SampleBlockSize" );
  Parameter( "WekaServerIP" );
  Parameter( "WekaServerPort" );
  Parameter( "WekaTimeout" );
  Parameter( "WekaDisableClassifier" );
  Parameter( "WekaClassifier" );
  Parameter( "ClassifierIsNominal" );
  Parameter( "NumericThresholdLR" );
  Parameter( "NumericThresholdUD" );
  Parameter( "ClassifierAttributeList" );
  // Checking parameters for writing a File
  Parameter( "WekaSaveTrainingFile" );
  Parameter( "WekaFilePrefix" );
  Parameter( "SubjectName" );
  Parameter( "DataDirectory" );
  Parameter( "SubjectSession" );
  Parameter( "SubjectRun" );
  Parameter( "SavePrmFile" );
  // Checking state to read
  State( "Running" );
  State( "SourceTime" );
  State( "Recording" );
  State( "TargetCode" );
  State( "ResultCode" );
  State( "StimulusTime" );
  State( "Feedback" );

  int isClassifierDisabled = Parameter("WekaDisableClassifier");

  if(isClassifierDisabled) {
    outSignalProperties = inSignalProperties;
  } else {
  // Requested output signal properties. We change the output to
  // 2 channels with only one element each. Each channel should be
  // up - down and left - right.
  outSignalProperties = SignalProperties( 2, 1, inSignalProperties.Type() );

  // Output description.
  outSignalProperties.ChannelUnit() = inSignalProperties.ChannelUnit();
  outSignalProperties.ValueUnit().SetRawMin( inSignalProperties.ValueUnit().RawMin() )
					.SetRawMax( inSignalProperties.ValueUnit().RawMax() );
  }
}

// This method is called when we set the configuration
void
WekaClassifier::Initialize( const SignalProperties& Input,
					  const SignalProperties& /*Output*/ )
{
  // Reading the parameters from the user configuration
  string host = Parameter("WekaServerIP");
  string port = Parameter("WekaServerPort");
  string classifier = Parameter("WekaClassifier");
  classifierDisabled = Parameter("WekaDisableClassifier");
  timeout =  Parameter("WekaTimeout");

  isNominal = Parameter("ClassifierIsNominal");
  attributeSelectionList = Parameter("ClassifierAttributeList");
  UDthreshold = Parameter("NumericThresholdUD");
  LRthreshold = Parameter("NumericThresholdLR");

  PreflightCondition(UDthreshold >= 0);
  PreflightCondition(LRthreshold >= 0);

  if(attributeSelectionList == "all") {
	  bciout << "Using all attributes"<< endl;
  } else {
	attributesIndices.clear();
	splitString(attributeSelectionList, attributesIndices, " ");
	for(unsigned int attIndex=0; attIndex < attributesIndices.size(); attIndex++) {
	  bciout << attIndex << ":" << attributesIndices[attIndex] << endl;
	}
	PreflightCondition( Input.Elements() * Input.Channels() >= attributesIndices.size());
  }

  // Based on the labels for the channels and elements we create a list
  // of attributes that the signal has. The list is semi colon separated.
  string attributeList = "";
  int attributeIndex = 0;
  LabelIndex lblElements = Input.ElementLabels();
  LabelIndex lblChannels = Input.ChannelLabels();
  for(int channel=0; channel < Input.Channels(); channel++) {
	string channelLabel = lblChannels.TrivialLabel(channel);
	for(int element=0; element < Input.Elements(); element++) {
	  attributeIndex++;
	  if(isAttributeIncluded(attributeIndex)) {
		string elementLabel = lblElements.TrivialLabel(element);
		attributeList += "Channel" + channelLabel + "-Element" + elementLabel + ";";
	  }
	}
  }


  // Creating the URL needed by the socket library
  host = host + ":" + port;

  // Opening the socket or dying because the server is down
  try {
	socket.open(host.c_str());
	connection.open(socket);
	if(socket.is_open() && connection.is_open()) {
	  bciout << "Connection to Weka server established" << endl;
	  readAnswer();
	}
  } catch (char * str) {
	PreflightCondition(socket.is_open() && connection.is_open());
	bcierr << "Couldn't connect to the Weka server" << endl;
	bcierr << "ERROR:" << str;
  }

  // Handshaking, we send a HELLO message, we should get a 200 in response
  postMessage("HELLO\n");
  string handShakeResponse = readAnswer();
  int handShakeResponseCode = getCodeFromResponse(handShakeResponse);
  bcidbg << "HandShakeResponse:" << handShakeResponse << endl;
  PreflightCondition(handShakeResponseCode == 200);

  // Tell the server to load the classifier indicated by the model file
  postMessage(("CLASSIFIER " + classifier + "\n").c_str());
  string loadClassifierResponse = readAnswer();
  int loadClassifierResponseCode = getCodeFromResponse(loadClassifierResponse);
  bcidbg << "LoadClassifierResponse:" << loadClassifierResponse << endl;
  if(loadClassifierResponseCode != 200) {
    if(loadClassifierResponseCode == 301) {
        bcierr << "Classifier not found"<< endl; 
		postMessage("SHOW\n");
		string response = readAnswer();
		response = getMessageFromResponse(response);
		bcierr << "Available classifiers:" << response << endl;
	} else {
		bcierr << "Couldn't load the weka classifier:" << classifier << endl;
	}
	PreflightCondition(loadClassifierResponseCode == 200);
  }

  // If the filter is in training mode, it will save a file with all
  // the signals and the target position (class)
  int saveTrainingFile = Parameter("WekaSaveTrainingFile");
  if(saveTrainingFile == 1) {
	  string subjectName = Parameter( "SubjectName" );
	  string subjectSession = Parameter( "SubjectSession" );
	  string subjectRun = Parameter( "SubjectRun" );

	  // The filename is created based on the subject, session and run
	  string filename = "SAVE " + subjectName + subjectSession + subjectRun + " " + attributeList + "\n";

	  postMessage(filename.c_str());
	  string response = readAnswer();
	  string saveFileResponse = getMessageFromResponse(response);
	  int saveFileResponseCode = getCodeFromResponse(response);
	  if(saveFileResponseCode != 200) {
		bcierr << "Can't save training file with name " << filename << endl << response << endl;
      }
  }
}

// Called every time we get a new block of data from the previous filter
// This method creates a SIGNAL message per each block, send the info
// to the classifier and then passes the response to the application
// layer.
void
WekaClassifier::Process(const GenericSignal& input,
		GenericSignal& output)
{
  // We create a message, and per each channel and element, extract
  // the value, put it as a string and concatenate with the previous
  // values using a semicolon as separator
  string message = "";
  int attributeIndex = 0;
  for(int channel = 0; channel < input.Channels(); channel++) {
	for(int element = 0; element < input.Elements(); element++) {
	  attributeIndex++;
	  if(isAttributeIncluded(attributeIndex)) {
		double value = input.Value(channel, element);
		stringstream out;
		out << value;
		message += out.str();
	    message += ";";
	  }
	}
   }

  // Get the target code from the state vector and add it to the message
  // as the first value, separated with a pipe
  unsigned int targetCode = State("TargetCode");
  stringstream outtc;
  outtc << targetCode;
  message = outtc.str() + "|" + message;

  // Create the SIGNAL message to send to the server
  message =   "SIGNAL " + message + "\n";

  // We send the message and wait for the response, getting the body
  // and predicted class
  postMessage(message.c_str());
  string resp = readAnswer();

  int code = getCodeFromResponse(resp);
  string responseBody = getMessageFromResponse(resp);
  float predictedClass = getClassFromResponse(resp);

  if(classifierDisabled) {
    output = input;
  } else {
  // Reset the values of the output channel so it's clean
  for( int ch = 0; ch < output.Channels(); ++ch )
	for( int el = 0; el < output.Elements(); ++el )
	  output( ch, el ) = 0.0;

  if(isNominal != 0) {

	// Writing the output with the values from the classifier
	// First channel is for left and right, second channel for
	// up and down
	int nominalClass = predictedClass;
	switch(nominalClass) {
	  case 1:
		output( 0, 0 ) = -1;
		break;
	  case 2:
		output( 0, 0 ) = 1;
		break;
/*	  case 3:
		output( 1, 0 ) = -1;
		break;
	  case 4:
		output( 1, 0 ) = 1;
		break;*/
	  default:
		bciout << "Invalid value returned from classifier:" << predictedClass << endl;
		break;
	}
  } else {
	if(predictedClass > LRthreshold) {
		output( 0, 0 ) = 1;
	}
	else {
		output( 0, 0 ) = -1;
	}
		/*if(predictedClass > UDthreshold)
		output( 1, 0 ) = 1;
	else
		output( 1, 0 ) = -1;*/
  }
  }
}

// This method reads the answer from the server, getting all the content
// from the TCP buffer.
string
WekaClassifier::readAnswer()
{
  // Waits for reading, with a timeout.
  socket.wait_for_read(timeout, true);
  string output = "";
  while(socket.can_read()) {
	char* buff = new char[256];
	int length = socket.read(buff, 256);
	if(length <= 0) {
	  bcierr << "Got zero length response from server" << endl;
	} else {
	  AnsiString response = buff;
	  output += response.SubString(0,length).c_str();
	}
  }
  return output;
}

// This method obtains the substring corresponding to the predicted
// class from a response to a SIGNAL message to the WekaServer
float
WekaClassifier::getClassFromResponse(string response)
{
  string body = getMessageFromResponse(response);
  float predictedClass = 0;
  try {
	predictedClass = atof(body.c_str());
  } catch (char * str) {
	bcierr << "Couldn't parse class" << endl;
  }
  return predictedClass;
}

// This method gets the return code from an answer from the WekaServer
int
WekaClassifier::getCodeFromResponse(string response)
{
  int code = 0;
  try {
	code = atoi(response.substr(0,3).c_str());
  } catch (char * str) {
	bcierr << str << " - Couldn't parse code:" << response << endl;
  }
  return code;
}

// Gets the message from a response from the WekaServer, everything
// that comes after the code.
string
WekaClassifier::getMessageFromResponse(string response) {
  string message = "";
  try {
	message = response.substr(4, response.length() - 4);
  } catch (char * str) {
	bcierr << str << " - Couldn't parse message:" << response << endl;
  }
  return message;
}

// Posts a message to the WekaServer, waits for timeout before and after
// for extra reliability.
void
WekaClassifier::postMessage(const char* msg)
{
  socket.wait_for_write(timeout, true);
  socket.write(const_cast<char *>( msg ), StrLen(msg));
  socket.wait_for_write(timeout, true);
}

void
WekaClassifier::splitString(const string& str,
		  vector<int>& tokens,
		  const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(atoi(str.substr(lastPos, pos - lastPos).c_str()));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

bool
WekaClassifier::isAttributeIncluded(int index) {
	if(attributeSelectionList == "all") {
	  return true;
	}

	for(unsigned int attIndex=0; attIndex < attributesIndices.size(); attIndex++) {
	  if(index == attributesIndices[attIndex])
		return true;
	}

	return false;
}
