package wekabci.server;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import org.apache.log4j.Logger;

import weka.classifiers.Classifier;
import weka.core.Attribute;
import weka.core.FastVector;
import weka.core.Instance;
import weka.core.Instances;
import wekabci.server.protocol.Protocol;


/**
 * This class manages each connection that is created by the Weka server.
 * One thread is created by each new client that connects to the server.
 * This thread has a {@link Protocol} that processess all the messages that
 * come from the client, and manages important state changes.
 * @author villalon
 *
 */
public class WekaThread extends Thread {

	private static Logger logger = Logger.getLogger(WekaThread.class);

	/**
	 * Communication stuff
	 */
	private volatile 
	
	boolean listening = true;
	private Socket clientSocket = null;
	private Protocol protocol = null;
	// Input and output buffers for the communication.
    private PrintWriter out = null;
    private BufferedReader in = null;
    

	/**
	 * WekaServer stuff
	 */
	/** Link to the Weka server from which we got the connection */
	private WekaServer server = null;
	/** The id of the thread in the server */
	private int threadId = -1;

	/**
	 * Classification stuff
	 */
	/** The classifier to use when classifying */
	private Classifier classifier = null;
	/** The instances used to classify new signals */
	private Instances instances = null;
	
	/**
	 * Saving file for training stuff
	 */
	/** The instances used when in training mode (saving the file) */
	private Instances trainingInstances = null;
	/** If we must save a file for training */
	private boolean saveTrainingFile = false;
	/** The writer to which we're going to write the training file */
	private BufferedWriter trainingFileWriter = null;
	
	
	/**
	 * Creates a new thread that will process messages
	 * @param server The server
	 * @param socket The socket
	 * @param thId An id for the thread
	 */
	public WekaThread(WekaServer server, Socket socket, int thId) {
		super("WekaServerThread");
		assert(thId > 0);
		this.clientSocket = socket;
		this.server = server;
		this.threadId = thId;
		
		logger.info("Connection started, thread id " + this.getThreadId());
	}

	/**
	 * Classifies an instance using the classifier that was previously set
	 * according to the protocol in this particular communication thread. 
	 * @param instance The instance to classify
	 * @return the output from the classifier (the predicted class)
	 * @throws Exception
	 */
	public double classifyInstance(Instance instance) throws Exception {
		double output = -1;
		output = this.classifier.classifyInstance(instance);
		if(output < 0)
			throw new Exception("Instance couldn't be classified");
		return output;
	}

	/**
	 * Sends the signal to the {@link WekaServer} to make it shutdown.
	 */
	public void closeServer() {
		this.server.setListening(false);
	}
	
	/**
	 * Gets the classifier set for this thread
	 * @return The Weka Classifier
	 */
	public Classifier getClassifier() {
		return classifier;
	}
	
	/**
	 * Gets the instances used for classification in this thread
	 * @return Weka Instances
	 */
	public Instances getInstances() {
		return instances;
	}
	
	/**
	 * Gets a structure for Instances from a list of attributes coming from 
	 * a semicolon separated list. This list and name should come as a result
	 * of a SAVE message from the client.
	 * It adds a final attribute called "class" which represents the value
	 * to predict. 
	 * @param name The name of the dataset (usually the subject, session and run)
	 * @param list A semicolon separated list of attribute names
	 * @return Weka Instances with the structure
	 */
	private Instances getInstancesFromAttributeList(String name, String list) {
		String[] attributes = list.split(";");
		// Here we create the vector with the attributes
		FastVector vector = new FastVector();
		for(String attribute : attributes) {
			vector.addElement(new Attribute(attribute));
		}
		// We add the class attribute at the end
		vector.addElement(new Attribute("Class"));
		// We create the Instances dataset and assign the class to the last element
		Instances instances = new Instances(name, vector, 0);
		instances.setClassIndex(vector.size()-1);
		
		return instances;
	}

	/**
	 * The {@link WekaServer} to which this thread belongs
	 * @return
	 */
	public WekaServer getServer() {
		return this.server;
	}

	/**
	 * The id of the thread in the {@link WekaServer} context
	 * @return
	 */
	public int getThreadId() {
		return threadId;
	}
	
	/**
	 * The instances dataset used for training (saving a file)
	 * @return
	 */
	public Instances getTrainingInstances() {
		return trainingInstances;
	}
	
	/**
	 * If the thread is listening for messages
	 * @return
	 */
	public boolean isListening() {
		return listening;
	}

	/**
	 * If the thread is in save file for training mode. If it is the case, the
	 * thread will save all the values coming from the client as instances
	 * of a Weka dataset. 
	 * @return
	 */
	public boolean isSaveTrainingFile() {
		return saveTrainingFile;
	}
	
	/**
	 * The running of the thread, this is called every time a new client connects
	 * to the {@link WekaServer}.
	 */
	public void run() {

		logger.info("Client socket initiated for " 
				+ clientSocket.getInetAddress().getCanonicalHostName() + ":" 
				+ clientSocket.getPort());
		
		// A new instance of the protocol is created.
		this.protocol = new Protocol(this);
		
        // Tries to create and initialise the buffers.
		try {
			out = new PrintWriter(clientSocket.getOutputStream(), true);
	        in = new BufferedReader(
					new InputStreamReader(
					clientSocket.getInputStream()));
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		// If everything goes well, we send the welcome message
        out.println("Weka TCP server - version 1.0");
        
		// For reading every line from the input
        String inputLine;

        try {
        	// Main loop, until the thread shouldn't be listening and there's any 
        	// message to process, we send the message to the protocol for processing.
        	// The result from the protocol is a response, that we send back to the client.
			while ((inputLine = in.readLine()) != null && this.isListening()) {
				
				// Process the message and get a response from the protocol.
				String response = this.protocol.processMessage(inputLine);
				
				// Send the response to the client
			    out.println(response);
			    
			    logger.debug(inputLine);
			    logger.debug(response);

			    // If the new state of the protocol is to close the thread, we finish
			    switch(this.protocol.getState()) {
			    	case CLOSING:
			    		this.setListening(false);
			    		break;
			    }
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		closeConnection();
	}
	
	private void closeConnection() {
		// Once the communication is finished, we close everything
		try {
			// Close the file writer if open
			if(this.trainingFileWriter != null) {
				this.trainingFileWriter.flush();
				this.trainingFileWriter.close();
			}
			// Close the buffers for communication
			out.close();
			in.close();
			// Closing the socket
			clientSocket.close();
			logger.info("Socket closed, thread id " + this.threadId);
		} catch (Exception e) {
			e.printStackTrace();
		}		
	}
	
	/**
	 * Saves a Weka Instance to the file that was indicated previously
	 * by the SAVE message.
	 * @param instance The instance to save
	 * @throws IOException
	 */
	public void saveInstanceToFile(Instance instance) throws IOException {
		if(!this.isSaveTrainingFile())
			return;
		
		this.trainingFileWriter.append(instance.toString());
		this.trainingFileWriter.append("\n");
	}

	/**
	 * Sets the classifier that this connection is going to use for classifying
	 * @param name
	 * @throws Exception
	 */
	public void setClassiferFromName(String name) throws Exception {
		if(this.server.getClassifiers().containsKey(name)) {
			this.classifier = this.server.getClassifiers().get(name).getClassifier();
			this.instances = this.server.getClassifiers().get(name).getDataSet();
		}
		else
			throw new Exception("Invalid classifier");
	}

	/**
	 * Sets if the thread will continue to process messages or close
	 * @param listening
	 */
	public void setListening(boolean listening) {
		this.listening = listening;
		
		if(!listening) {
			closeConnection();
		}
	}

	/**
	 * Sets if the thread should be saving the data to the
	 * training file.
	 * @param saveTrainingFile
	 */
	public void setSaveTrainingFile(boolean saveTrainingFile) {
		this.saveTrainingFile = saveTrainingFile;
	}
	
	/**
	 * Sets the name for the training file to be saved and the list
	 * of attributes to define the structure of the dataset.
	 * @param name The name of the dataset (subject, session, run)
	 * @param attributeList The list of attributes from BCI2000
	 * @throws Exception
	 */
	public void setTrainingFileName(String name, String attributeList) throws Exception {
		File file = new File("trainingData" + "/" + name + "-training.arff");
		if(file.isDirectory()) {
			throw new Exception("Invalid filename");
		}
		
		this.trainingInstances = getInstancesFromAttributeList(name, attributeList);
		this.trainingFileWriter = new BufferedWriter(new FileWriter(file));
		this.trainingFileWriter.append(this.trainingInstances.toString());
	}	
}