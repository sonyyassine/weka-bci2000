/**
 * This file is just an example on how to load a Weka already trained classifier
 * and use it to classify something.
 */
package wekabci.examples;

import java.io.BufferedReader;
import java.io.FileReader;

import weka.classifiers.trees.J48;
import weka.core.Instance;
import weka.core.Instances;

import org.apache.log4j.*;

public class LoadClassifierAndClassifySomething {

	private static final String MODELS_PATH = "/Users/Jorge/Documents/signals/WekaBCI/models/";
	static Logger logger = Logger.getLogger(LoadClassifierAndClassifySomething.class);
	private J48 tree;
	private String modelName = "Iris";
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		PropertyConfigurator.configure("wekaBci.log4j");
		new LoadClassifierAndClassifySomething();
	}

	public LoadClassifierAndClassifySomething() {
		try {
			logger.debug("Weka classifier: Loading model");
			tree = (J48) weka.core.SerializationHelper.read(MODELS_PATH + modelName + ".model");
			logger.debug("Weka classifier: Model loaded");
		} catch (Exception e2) {
			e2.printStackTrace();
			return;
		}
		
		
		try {
			Instances instances = this.getDataset();
			instances.setClassIndex(instances.numAttributes()-1);
			String signal = "6.7,2.5,5.8,1.8,?";
			String signal2 = "5.4,3.4,1.7,0.2,?";
			String signal3 = "6.3,2.5,4.9,1.5,?";
			instances.add(getInstanceFromSignal(signal, ","));
			instances.add(getInstanceFromSignal(signal2, ","));
			instances.add(getInstanceFromSignal(signal3, ","));
			logger.info(instances.toSummaryString());
			for(int i=0;i<instances.numAttributes();i++)
				logger.info(instances.attribute(i).toString());
			for(int i=0; i<instances.numInstances(); i++) {
				Instance instance = instances.instance(i);
				logger.info(instance.toString());
				double result = tree.classifyInstance(instance);
				instance.setClassValue(result);
				logger.info("Instance classified as " + result);
			}
		} catch (Exception e) {
			logger.fatal(e);
			e.printStackTrace();
		}
	}
	
	private Instances getDataset() throws Exception {
		Instances instancesFromFile = new Instances(
                new BufferedReader(
                        new FileReader(MODELS_PATH + modelName + ".arff")));
		
		return instancesFromFile;		
	}
	
	private Instance getInstanceFromSignal(String signal, String separator) {
		String[] dataString = signal.split(separator);
		double data[] = new double[dataString.length];
		for(int i=0; i<dataString.length; i++) {
			try {
				data[i] = Double.parseDouble(dataString[i]);
			} catch (Exception e) {
				data[i] = Double.NaN;
			}
		}
		Instance instance = new Instance(1.0, data);
		return instance;
	}
}
