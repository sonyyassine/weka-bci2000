package wekabci.server.protocol;

import wekabci.server.protocol.Protocol.States;

/**
 * This interface declare the minimum methods that every message must be able
 * to execute.
 * The protocol is thought as a finite state machine, starting from a NEW state and
 * then moving to other states according to different messages.
 * @author Jorge
 *
 */
public interface Message {

	/**
	 * Validates if a particular state is valid for the message execution
	 * @param state The state to validate
	 * @return
	 */
	public boolean validateState(States state);
	
	/**
	 * Processes the message from the client
	 * @param message the message from the client
	 * @return
	 */
	public String processMessage(String message);
	
	/**
	 * Returns the response to the last processed message.
	 * @return
	 */
	public String getResponse();
	
	/**
	 * Returns the new state for the protocol due to the result of 
	 * the processing of the message.
	 * @return
	 */
	public States getNewState();
	
	/**
	 * Returns the protocol to which the message belongs to.
	 * @return
	 */
	public Protocol getProtocol();
	
	/**
	 * Sets the protocol to which the message belongs to.
	 * @param protocol
	 */
	public void setProtocol(Protocol protocol);
}
