package wekabci.server;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;

import org.apache.log4j.Logger;

import weka.classifiers.Classifier;
import weka.core.Instances;
import weka.core.SerializationHelper;

/**
 * This class represents a Weka trained classifier and its corresponding DataSet structure.
 * It is created using two files, a ".classifier" file that contains the serialized
 * version of a model already trained, and the ".arff" file with no data that
 * explains the structure that the dataset has.
 * Given a name, the class tries to find the .model and .arff files in the
 * predefined folder in the settings file, open them and instantiate both a 
 * Classifier and a DataSet.
 * @author Jorge
 *
 */
public class WekaClassifier {

	/** The logger for debugging */
	private static Logger logger = Logger.getLogger(WekaClassifier.class);
	/** The server that uses the model */
	private WekaServer server = null;
	/** The name of the model */
	private String name = null;
	/** The classifier */
	private Classifier classifier = null;
	/** The dataset */
	private Instances dataSet = null;
	
	/**
	 * Creates a new classifier that belongs to the specified server
	 * @param server
	 */
	public WekaClassifier(WekaServer server) {
		assert(server != null);
		
		this.server = server;
	}
	
	/**
	 * Loads the model using a specific name, if anything fails it
	 * raises an exception.
	 * The convention is that looks for the files with the exact
	 * name and extensions ".classifier" and ".arff". Both should be in the 
	 * same folder, indicated by the settings file.
	 * @param name The name of the model to load
	 * @throws Exception If the files don't exist or are invalid models or arff files.
	 */
	public void LoadModel(String name) throws Exception {
		this.name = name;
		
		// Create the files for the model and dataset
		File modelFile = new File(server.getClassifiersFolderPath() + "/" + this.name + ".model");
		File modelArffFile = new File(server.getClassifiersFolderPath() + "/" + this.name + ".arff");
		
		// If both files exist, the classifier and dataset are created
		if(modelFile.exists() && modelArffFile.exists()) {
			this.classifier = (Classifier) SerializationHelper.read(
							modelFile.getAbsolutePath());
			this.dataSet = new Instances( new BufferedReader(
	                        new FileReader(modelArffFile.getAbsolutePath())));
			this.dataSet.setClassIndex(this.dataSet.numAttributes()-1);
			logger.info("Model " + this.name + " loaded. [" + modelFile.getAbsolutePath() + 
					"/" + modelArffFile.getAbsolutePath() + "]");
		} else {
			throw new FileNotFoundException("Couldn't find the " + this.name + " .model and .arff file in folder " + server.getClassifiersFolderPath());
		}
	}

	/**
	 * Gets the classifier that this model has loaded
	 * @return
	 */
	public Classifier getClassifier() {
		return classifier;
	}

	/**
	 * Gets the dataset that this model has loaded
	 * @return
	 */
	public Instances getDataSet() {
		return dataSet;
	}
	
	/**
	 * Gets the name of the WekaModel loaded in the sever
	 * @return the name of the model
	 */
	public String getName() {
		return this.name;
	}
}
