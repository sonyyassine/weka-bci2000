package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This message shuts down the WekaServer.
 * 
 * SYNTAX:
 * 
 * {@code
 * HALT\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - Always returns 200, it can't fail.
 * 
 * @author Jorge
 *
 */
public class Halt extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.CLOSING;
	}

	@Override
	public String getResponse() {
		return "Shuting down the server";
	}

	@Override
	public String processMessage(String message) {
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.READY)
			return true;
		return false;
	}

}
