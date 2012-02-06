package wekabci.server;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.*;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import wekabci.server.protocol.Message;
import wekabci.server.protocol.Protocol;

/**
 * This is the main class for the WekaBCI server, it loads all the Weka models available
 * in the specified folder, and processes all incoming requests creating a {@link WekaThread}
 * per each connection.
 * The {@link WekaThread} uses then an instance of the {@link Protocol} to process all
 * the {@link Message}s coming from the client.
 * @author Jorge
 *
 */
public class WekaServer extends Thread {

	/**
	 * Log4j logger and properties configuration
	 */
	static Logger logger = Logger.getLogger(WekaServer.class);
	private static Properties configFile = new Properties();
	private static final String WEKA_SERVER_PROPERTIES = "WekaServer.properties";

	/**
	 * TCP communication
	 */
	ServerSocket serverSocket = null;
	Socket clientSocket = null;	
	private List<WekaThread> clients = null;
	volatile boolean listening = true;
	private int port = 0;
	private int threadCount = 0;

	/**
	 * Weka classifiers
	 */
	private Hashtable<String, WekaClassifier> classifiers = null;
	private File classifierFolder = null;

	/**
	 * Runs the server
	 */
	public void run() {		
		PropertyConfigurator.configure(WekaServer.class.getCanonicalName() + ".log4j");

		logger.info("Starting Weka server");

		/**
		 * Reading the configuration and validating it. Then loads all the classifiers
		 * in the folder.
		 * If no classifiers are found, the server fails.
		 */
		try {
			LoadConfiguration();
			LoadClassifiers();
		} catch (Exception e) {
			logger.fatal("Problems reading the configuration");
			logger.fatal(e.getMessage());
			return;
		}

		// We start with an empty array of threads
		this.clients = new ArrayList<WekaThread>();

		/**
		 * Once we have classifiers we open a socket to listen to TCP connections
		 * on the right port
		 */
		try {
			serverSocket = new ServerSocket(port);
			logger.info("Weka server listening on port " + port);
		} catch (IOException e) {
			logger.fatal("Couldn't start the TCP server");
			logger.fatal(e.getMessage());
			e.printStackTrace();
			return;
		}

		/**
		 * Main loop, waits for connections and starts a thread on each one
		 */
		while(listening) {
			logger.debug("-");
			try {
				WekaThread thread = new WekaThread(this, serverSocket.accept(), threadCount + 1);
				threadCount++;
				this.clients.add(thread);
				thread.start();
			} catch (IOException e1) {
				logger.error("Error creating a thread for a connection");
				logger.error(e1.getMessage());
				e1.printStackTrace();
			}
		}

		closeServer();
	}

	private void closeServer() {
		/**
		 * Closing the server when there's no more to listen
		 */
		try {
			serverSocket.close();
			logger.info("Weka server closing");
		} catch (IOException e) {
			e.printStackTrace();
		}

		logger.info("Closing Weka server");				
	}

	/**
	 * Loads all the classifiers from the folder
	 * @throws Exception
	 * @throws IOException
	 */
	private void LoadClassifiers() throws Exception, IOException {
		this.classifiers = new Hashtable<String, WekaClassifier>();

		// Loading every file with the extension .model as a Classifier
		for(File classifier : this.classifierFolder.listFiles()) {
			if(classifier.getName().endsWith(".model")) {
				String classifierName = classifier.getName().substring(0, classifier.getName().indexOf(".model"));
				try {
					WekaClassifier wm = new WekaClassifier(this);
					wm.LoadModel(classifierName);
					this.classifiers.put(classifierName, wm);
				} catch (Exception e) {
					logger.error("Couldn't load classifier " + classifier.getAbsolutePath() + " :" + e.getMessage());
				}
			}
		}

		// If there are no classifiers there's no point on going on
		if(this.classifiers.size() == 0) {
			throw new Exception("No loadable classifiers!");
		}
	}

	/**
	 * Loads the configuration from the file
	 * @throws Exception
	 */
	private void LoadConfiguration() throws Exception {
		// Loads configuration from properties file
		configFile.load(new FileReader(WEKA_SERVER_PROPERTIES));

		// Reads the TCP port and validate it
		this.port = Integer.parseInt(configFile.getProperty("TCP_PORT"));
		if(this.port < 1000 || this.port > 60000)
			throw new IOException("Invalid port number");

		// Reads the classifier folder and validates that it exists
		this.classifierFolder = new File(configFile.getProperty("CLASSIFIERS_FOLDER"));
		if(!this.classifierFolder.exists())
			throw new IOException("Invalid classifier folder");		
	}

	/**
	 * The list of classifiers that the WekaServer has available
	 * @return
	 */
	public Hashtable<String, WekaClassifier> getClassifiers() {
		return classifiers;
	}

	/**
	 * If the WekaServer is listening to incoming messages
	 * @return
	 */
	public boolean isListening() {
		return listening;
	}

	/**
	 * Sets if the WekaServer should continue to listen
	 * @param listening
	 */
	public void setListening(boolean listening) {
		this.listening = listening;

		if(!listening) {
			logger.info("Stopping WekaBCI server");
			for(WekaThread thread : this.clients) {
				thread.setListening(false);
			}
			closeServer();
		}
	}

	/**
	 * Returns the path of the folder where the classifiers are read from
	 * @return
	 */
	public String getClassifiersFolderPath() {
		return this.classifierFolder.getAbsolutePath();
	}
}
