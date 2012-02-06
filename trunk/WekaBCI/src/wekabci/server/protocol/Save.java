package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This message indicates the thread to save all the data
 * coming from the client into a file. The file and name for the 
 * file are part of the message that comes from the client.
 * 
 * SYNTAX:
 * 
 * {@code
 * BYE\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - It can't fail, so it always returns this.
 * 501 - An exception was found trying to assign the name of the file and the list of attributes.
 * 502 - Syntax invalid for the message.
 * 
 * 
 * @author villalon
 *
 */
public class Save extends AbstractMessage {

	@Override
	public States getNewState() {
		return States.CLASSIFY;
	}

	@Override
	public String getResponse() {
		return "";
	}

	@Override
	public String processMessage(String message) {
		String[] messageParts = message.split(" ");
		
		if(messageParts.length != 2)
			return "502";
		
		String filename = messageParts[0];
		String attributeList = messageParts[1];
		try {
			this.protocol.getClientThread().setTrainingFileName(filename, attributeList);
			this.protocol.getClientThread().setSaveTrainingFile(true);
		} catch (Exception ex) {
			logger.error(ex.toString());
			return "501";
		}
		return "200";
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.CLASSIFY)
			return true;
		return false;
	}

}
