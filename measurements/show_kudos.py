import sys
sys.path.append('../userspace/blockchain')
import plotly.plotly as py
#import plotly.figure_factory as ff
import plotly.graph_objs as go
import plotly
from bigchaindb_driver import BigchainDB
from urllib.request import urlopen
import json
import kudos
from scipy import stats
import numpy as np
from numpy import arange,array,ones
import time
from scipy.optimize import curve_fit

#import numpy as np
plotly.tools.set_credentials_file(username='pepus', api_key='X9vAgXhDnpR8uTdtx7xN')

def main():
	api_endpoint, unspents_endpoint = kudos.initaliseKudos()
	bdb = BigchainDB(api_endpoint)

	x_list=[]
	y_list=[]

	verifying_key = sys.argv[1]

	kudos_final_value,x_list,y_list = kudos.getKudos(verifying_key)
	#x_list.append((time.time())*10000000)
	#y_list.append(y_list[-1]+10)
	x_array = array(x_list)
	y_array = array(y_list)
	#slope, intercept, r_value, p_value, std_err = stats.linregress(x_array,y_list)
	#line = slope*x_array+intercept
	#print ("slope: ", slope, "intercept: ", intercept)

	trace1 = go.Scatter(
	    x=x_list,
	    y=y_list,
	    mode = 'lines+markers',
	    name = 'Kudos', # Style name/legend entry with html tags
	)

	data = [trace1] #, trace1]

	fig = dict(data=data)
	py.plot(fig, filename='simple-connectgaps')

if __name__ == "__main__":
    main()