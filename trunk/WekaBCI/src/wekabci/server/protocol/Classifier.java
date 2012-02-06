package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This message loads a particular classifier for the thread communication,
 * then gets into a new state to start classifying signals. 
 * 
 * SYNTAX:
 * 
 * {@code
 * CLASSIFIER classifierName\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - Always returns 200, it can't fail.
 * 301 - The classifier was not found.
 * 
 * @author villalon
 *
 */
public class Classifier extends AbstractMessage {

	private boolean isValidClassifier = false;
	private String classifier;
	
	@Override
	public States getNewState() {
		if(this.isValidClassifier)
			return States.CLASSIFY;
		else
			return States.READY;			
	}

	@Override
	public String getResponse() {
		if(this.isValidClassifier) {
			return "Using new classifier " + this.classifier + ", starting classify mode";
		} else {
			return "Invalid classifier";
		}
	}

	@Override
	public String processMessage(String message) {
		try {
			this.protocol.getClientThread().setClassiferFromName(message);
			this.isValidClassifier = true;
			this.classifier = message;
			return "200";
		} catch (Exception e) {
			return "301";
		}
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.READY)
			return true;
		return false;
	}

}
