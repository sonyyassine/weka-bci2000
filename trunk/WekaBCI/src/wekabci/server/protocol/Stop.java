package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * The Stop message stops the thread from classifying signals and returns to
 * the ready state, this is useful if a new classifier wants to be loaded
 * for this thread.
 * 
 * SYNTAX:
 * 
 * {@code
 * STOP\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - It can't fail, so it always returns this.
 * 
 * @author villalon
 *
 */
public class Stop extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.READY;
	}

	@Override
	public String getResponse() {
		return "Stoping";
	}

	@Override
	public String processMessage(String message) {
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.CLASSIFY)
			return true;
		return false;
	}

}
