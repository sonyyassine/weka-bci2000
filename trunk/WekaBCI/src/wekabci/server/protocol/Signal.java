package wekabci.server.protocol;

import weka.core.Instance;
import weka.core.Instances;
import wekabci.input.StringToInstance;
import wekabci.server.protocol.Protocol.States;

/**
 * The most important message, used to receive a brain signal from the BCI program,
 * it is classified and then predicted class is returned. It also saves the data to 
 * a file if the thread is configured to do so (see: {@link Save}).
 * 
 * SYNTAX:
 * 
 * {@code
 * SIGNAL targetCode|signal0;signal1;signal2;....;signalN;\n
 * }
 * 
 * RETURN CODE:
 * 
 * 200 - If the signal is correctly classified.
 * 302 - An error occurred trying to classify the signal.
 * 303 - An error occurred trying to save the signal data to the training file.
 * 
 * @author villalon
 *
 */
public class Signal extends AbstractMessage {

	private double result = -1;

	@Override
	public String processMessage(String message) {
		
		// First we get the value of the target and separate it from the signal
		String[] messageParts = message.split("\\|");
		String target = messageParts[0];
		String signal = messageParts[1];

		String outputCode = "302";
		
		// We classify the signal and update the result of the prediction
		try {
			// Get a copy of the instances structure from the protocol
			Instances instanceSet = new Instances(this.protocol.getClientThread().getInstances());
			
			// We add the signal as an instance to the set
			instanceSet.add(StringToInstance.getInstanceFromSignal(signal));
			
			// We get the instance from the set 
			Instance instance = instanceSet.firstInstance();
			instance.setClassValue(Double.NaN);
			
			// We classify the instance
			this.result = this.protocol.getClientThread().classifyInstance(instance);
			
			// If the classifier is nominal, parse the class name
			if(instanceSet.classAttribute().isNominal() && this.result != Instance.missingValue()) {
				String classPredicted = instanceSet.classAttribute().value((int) this.result);
				this.result = Double.parseDouble(classPredicted);
			}
			outputCode = "200";
		} catch (Exception e) {
			logger.error("Error classifying " + e.getMessage());
		}

		double targetCode = Double.NaN;

		// If the data must be saved for future training, a new instance with the
		// class assigned to the targetCode is created.
		if(this.protocol.getClientThread().isSaveTrainingFile()) {
			try {
				targetCode = Double.parseDouble(target.trim());
			} catch (Exception e) {
				logger.error("Couldn't parse target code:" + target);
				logger.error(e.toString());
			}

			Instances instanceSet = new Instances(this.protocol.getClientThread().getTrainingInstances());
			instanceSet.add(StringToInstance.getInstanceFromSignal(signal));
			Instance instance = instanceSet.firstInstance();
			instance.setClassValue(targetCode);

			try {
				this.protocol.getClientThread().saveInstanceToFile(instance);
				outputCode = "200";
			} catch (Exception e) {
				e.printStackTrace();
				logger.error("Error writing training file " + e.getMessage());
				outputCode = "303";
			}
		}

		return outputCode;
	}

	@Override
	public String getResponse() {
		return Double.toString(result);
	}

	@Override
	public States getNewState() {
		return States.CLASSIFY;
	}

	@Override
	public boolean validateState(States state) {
		if(state == States.CLASSIFY)
			return true;
		return false;
	}

}
