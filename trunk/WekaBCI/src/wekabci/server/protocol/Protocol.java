package wekabci.server.protocol;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;

import wekabci.server.WekaThread;

/**
 * This class implements the protocol of the server. Basically
 * processes all the messages and then calls the corresponding
 * class for processing.
 * It also manages the change of states.
 * @author Jorge
 *
 */
public class Protocol {

	/**
	 * States for the protocol
	 * @author Jorge
	 *
	 */
	public enum States {
		/** A new connection has just been created */
		NEW,
		/** The connection is ready to start processing */
		READY,
		/** The connection is closing so it doesn't accept more messages */
		CLOSING,
		/** The connection is in classifying mode, waiting for signals */
		CLASSIFY
	}

	private static Logger logger = Logger.getLogger(Protocol.class);

	/** The current state for the protocol */
	private States state = States.NEW;
	/** The thread that manages the current connection */
	private WekaThread clientThread = null;

	/**
	 * Constructor for the protocol. It requires a {@link WekaThread} object
	 * that represents the connection.
	 * @param thread The client connection.
	 */
	public Protocol(WekaThread thread) {
		logger.info("Starting WekaBCI protocol");
		this.clientThread = thread;
	}
	
	/**
	 * Processes a message from the client, parses it validate
	 * if the protocol can manage it and then uses the corresponding
	 * class to get a return code and response which is sent 
	 * back to the client.
	 * @param message The message from the client
	 * @return Return code followed by a space and then a detailed response
	 */
	public String processMessage(String message) {
		// The default return code and response, an error
		String returnCode = "300";
		String response = "Protocol not working properly";
		
		// First header and body are separated from the message
		String header = null;
		String body = null;
		Pattern pattern = Pattern.compile("^(\\w+)(\\s+)?(.*)?$");
		Matcher matcher = pattern.matcher(message);
		if(matcher.find()) {
			header = matcher.group(1);
			header = header.substring(0,1).toUpperCase() + header.substring(1).toLowerCase();
			body = matcher.group(3);
		}

		// We try to instantiate a message handler for that header, i.e.
		// the class that can process those messages
		Message messageHandler = null;
		try {
			messageHandler = (Message) Class.forName("wekabci.server.protocol." + header).newInstance();
		} catch(Exception e) {
			return "404 No message handler for " + header;
		}

		// First we validate that the previous state corresponds to a correct sequence
		if(!messageHandler.validateState(this.state)) {
			return "301 Invalid protocol sequence";
		}
		
		// If correct, we pass the protocol and message to the messageHandler and
		// get the return code, response and the new state
		messageHandler.setProtocol(this);
		returnCode = messageHandler.processMessage(body);
		response = messageHandler.getResponse();
		this.state = messageHandler.getNewState();

		// We compose the final response
		return returnCode + " " + response;
	}

	/**
	 * The client thread that the protocol is processing messages from
	 * @return
	 */
	WekaThread getClientThread() {
		return clientThread;
	}

	/**
	 * Gets the current state (see: {@link States}) of the protocol.
	 * @return
	 */
	public States getState() {
		return state;
	}
}
