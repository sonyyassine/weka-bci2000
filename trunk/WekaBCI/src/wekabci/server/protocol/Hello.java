package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This is the basic hand shaking message, expected to move
 * from a new connection to a ready to process messages connection.
 * 
 * SYNTAX:
 * 
 * {@code
 * HELLO\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - Always returns 200, it can't fail.
 * 
 * @author villalon
 *
 */
public class Hello extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.READY;
	}

	@Override
	public String getResponse() {
		return "Welcome to WekaBCI server v1.0";
	}

	@Override
	public String processMessage(String message) {
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.NEW)
			return true;
		return false;
	}

}
