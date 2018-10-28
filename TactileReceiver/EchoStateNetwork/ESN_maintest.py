import matplotlib.pyplot as plt
import numpy as np
import argparse

import ESN
import NARMA10


def TrainESN(dataPath, inSize, outSize, resSize, alpha):

		# Get Data
	    data, Y = NARMA10.getData(train_cycles+test_cycles)
	    data_train, Y_train = data[:train_cycles], Y[:train_cycles]
	    data, Y = NARMA10.getData(train_cycles+test_cycles)
	    data_test, Y_test = data[train_cycles:], Y[train_cycles:]

	    # Reservoir & Training
	        # setup reservoir
	    Echo = ESN.ESN(inSize, outSize, resSize, alpha)
	        # get reservoir activations for training data
	    RA_Train  = Echo.reservoir(data_train)
	        # caclulate output matrix via moore pensore pseudoinverse (linear reg)
	    Wout = np.dot(np.linalg.pinv(RA_Train), Y_train )
	        # get reservoir activation for test data
	    RA_Test = Echo.reservoir(data_test, new_start=True)
	        # calculate predictions using output matrix
	    Yhat = np.dot(RA_Test, Wout)

	    # Calculate Error
	        # we throw away the first 50 values, cause the system needs
	        # enough input to being able to predict the NARMA10, since it is a
	        # delayed differential equation
	    NRMSE = em.NRMSE(Y_test, Yhat, throw=50)
	    #print(NRMSE)

	    # Plotting & Saving
	    plot_name += '_NRMSE=' + str(NRMSE)
	    Echo.plot_reservoir(path=plot_path, name=plot_name, plot_show=plot_show)

	    Echo.save_dm(name=plot_name)

	    # Prediction Plot
	    plt.figure('Prediction', figsize=(20,10)).clear()
	    plt.yscale('log')
	    plt.plot(Y_test, color='k', linewidth=5, label='Y')
	    plt.plot(Yhat, color='r', linewidth=2, label='Y-Hat/Predictions of ESN')
	    plt.legend()
	    plt.savefig(plot_path + plot_name + '.png')
	    print('\t[+]Plot saved in', plot_path + plot_name + '.png')
	    if plot_show:
	        plt.show()

	    return 42 #NRMSE


TrainESN("/", 1, 1, 1000, 0.8)
