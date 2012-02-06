package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * Simple message just to check if the communication with the server is all right.
 * 
 * SYNTAX:
 * 
 * {@code
 * PING\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - Always returns 200, it can't fail.
 * 
 * @author villalon
 *
 */
public class Ping extends AbstractMessage {

	private States previousState;
	
	@Override
	public States getNewState() {
		return this.previousState;
	}

	@Override
	public String getResponse() {
		return "";
	}

	@Override
	public String processMessage(String message) {
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.READY
				|| state == States.CLASSIFY) {
			this.previousState = state;
			return true;
		}
		return false;
	}
}
