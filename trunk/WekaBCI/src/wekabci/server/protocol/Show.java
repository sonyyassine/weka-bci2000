package wekabci.server.protocol;

import wekabci.server.WekaClassifier;
import wekabci.server.protocol.Protocol.States;

/**
 * This message return a list of the available classifiers on the server.
 * 
 * SYNTAX:
 * 
 * {@code
 * SHOW\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - If it was possible to return the list of available classifiers.
 * 500 - If there are no classifiers available.
 * 
 * 
 * @author villalon
 *
 */
public class Show extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.READY;
	}

	@Override
	public String getResponse() {
		String models = "";
		for(WekaClassifier model : this.protocol.getClientThread().getServer().getClassifiers().values()) {
			models += "\n" + model.getName();
		}
		return models;
	}

	@Override
	public String processMessage(String message) {
		if(this.protocol.getClientThread().getServer().getClassifiers().values().size() <= 0) {
			return "500";
		} else {
			return "200";
		}
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.READY)
			return true;
		return false;
	}

}
