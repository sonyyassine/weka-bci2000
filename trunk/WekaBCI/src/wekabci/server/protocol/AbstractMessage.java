package wekabci.server.protocol;

import org.apache.log4j.Logger;

import wekabci.server.protocol.Protocol.States;

/**
 * Abstract class for all the valid messages in the {@link Protocol}. This class
 * implements the {@link Message} interface, which defines the minimum behavior for
 * every message in the {@link Protocol}.
 * Most of the methods are abstract and they have to be implemented by each message.
 * @author villalon
 *
 */
public abstract class AbstractMessage implements Message {

	protected static Logger logger = Logger.getLogger(AbstractMessage.class);
	
	/** The protocol */
	protected Protocol protocol = null;
	
	/**
	 * Processes a message, returning a code
	 */
	public abstract String processMessage(String message);
	
	/**
	 * Gets the new state for the protocol as a result of processing the message.
	 */
	public abstract States getNewState();
	
	/**
	 * Validates if the current state in the protocol is valid for the processing
	 * of the message.
	 */
	public abstract boolean validateState(States state);
	
	/**
	 * Gets the response after processing a message
	 */
	public abstract String getResponse();
	
	/**
	 * The protocol to which the message belongs to
	 */
	public Protocol getProtocol() {
		return protocol;
	}
	
	/**
	 * Sets the protocol to which the message will report.
	 */
	public void setProtocol(Protocol protocol) {
		this.protocol = protocol;
	}
}
