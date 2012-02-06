package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This message closes the connection to the client.
 * 
 * SYNTAX:
 * 
 * {@code
 * BYE\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - Always returns 200, it can't fail.
 * 
 * @author villalon
 *
 */
public class Bye extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.CLOSING;
	}

	@Override
	public String getResponse() {
		return "Closing";
	}

	@Override
	public String processMessage(String message) {
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.NEW
				|| state == States.READY)
			return true;
		return false;
	}

}
