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

	x_list1=[]
	y_list1=[]
	x_list2=[]
	y_list2=[]
	x_list3=[]
	y_list3=[]

	verifying_key1 = sys.argv[1]
	verifying_key2 = sys.argv[2]
	verifying_key3 = sys.argv[3]

	kudos_final_value1,x_list1,y_list1 = kudos.getKudos(verifying_key1)
	kudos_final_value2,x_list2,y_list2 = kudos.getKudos(verifying_key2)
	kudos_final_value3,x_list3,y_list3 = kudos.getKudos(verifying_key3)
	#x_list.append((time.time())*10000000)
	#y_list.append(y_list[-1]+10)
	x_array1 = array(x_list1)
	y_array1 = array(y_list1)
	x_array2 = array(x_list2)
	y_array2 = array(y_list2)
	x_array3 = array(x_list3)
	y_array3 = array(y_list3)
	#slope, intercept, r_value, p_value, std_err = stats.linregress(x_array,y_list)
	#line = slope*x_array+intercept
	#print ("slope: ", slope, "intercept: ", intercept)

	trace1 = go.Scatter(
	    x=x_list1,
	    y=y_list1,
	    mode = 'lines+markers',
	    name = 'Node 1', # Style name/legend entry with html tags
	)

	trace2 = go.Scatter(
	    x=x_list2,
	    y=y_list2,
	    mode = 'lines+markers',
	    name = 'Node 2', # Style name/legend entry with html tags
	)

	trace3 = go.Scatter(
	    x=x_list3,
	    y=y_list3,
	    mode = 'lines+markers',
	    name = 'Node 3', # Style name/legend entry with html tags
	)

	data = [trace1, trace2, trace3]

	fig = dict(data=data)
	py.plot(fig, filename='simple-connectgaps')

if __name__ == "__main__":
    main()