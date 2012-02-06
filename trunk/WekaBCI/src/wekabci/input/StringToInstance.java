package wekabci.input;

import org.apache.log4j.Logger;

import weka.core.Instance;

/**
 * This class is responsable for transforming a text message
 * containing a brain signal into a Weka Instance
 * @author Jorge
 *
 */
public class StringToInstance {

	/** The logger */
	private static Logger logger = Logger.getLogger(StringToInstance.class);
	/** The separator we are going to use */
	private static String separator = ";";
	
	/**
	 * Gets the instance from the data
	 * @return
	 */
	public static Instance getInstanceFromSignal(String signal) {
		String[] dataString = signal.split(separator);
		double data[] = new double[dataString.length + 1];
		for(int i=0; i<dataString.length; i++) {
			double value = Double.NaN;
			try {
			  value = Double.parseDouble(dataString[i]);
  			  data[i] = value;
			} catch (Exception e) {
				logger.error("Couldn't parse a value:" + dataString[i]);
			}
		}
		data[data.length-1] = Double.NaN;
		Instance instance = new Instance(1.0, data);
		return instance;
	}

	/**
	 * The separator used by the class
	 * @return
	 */
	public static String getSeparator() {
		return separator;
	}

	/**
	 * Changes the separator that will be used to parse the brain signals
	 * @param separator
	 */
	public static void setSeparator(String separator) {
		if(separator.length() <= 0)
			return;
		
		logger.info("Changing separator to " + separator);
		StringToInstance.separator = separator;
	}	
}
