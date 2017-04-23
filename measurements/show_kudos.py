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

	x_task_list1=[]
	y_task_list1=[]
	x_task_list2=[]
	y_task_list2=[]
	x_task_list3=[]
	y_task_list3=[]

	verifying_key1 = sys.argv[1]
	verifying_key2 = sys.argv[2]
	verifying_key3 = sys.argv[3]

	kudos_final_value1,x_list1,y_list1 = kudos.getKudos(verifying_key1)
	kudos_final_value2,x_list2,y_list2 = kudos.getKudos(verifying_key2)
	kudos_final_value3,x_list3,y_list3 = kudos.getKudos(verifying_key3)

	task_final_value1,x_task_list1,y_task_list1 = kudos.getTasks(verifying_key1, verifying_key2, verifying_key3)
	task_final_value2,x_task_list2,y_task_list2 = kudos.getTasks(verifying_key2, verifying_key1, verifying_key3)
	task_final_value3,x_task_list3,y_task_list3 = kudos.getTasks(verifying_key3, verifying_key2, verifying_key1)

	start_time = min (x_list1[0], x_list2[0], x_list3[0])
	#start_time = min (x_task_list1[0], x_task_list2[0], x_task_list3[0])
	#start_time=0
	print(start_time)

	x_array1 = ((array(x_list1) - start_time) / 10000000)
	y_array1 = array(y_list1)
	x_array2 = ((array(x_list2) - start_time) / 10000000)
	y_array2 = array(y_list2)
	x_array3 = ((array(x_list3) - start_time) / 10000000)
	y_array3 = array(y_list3)

	x_task_array1 = ((array(x_task_list1) - start_time) / 10000000)
	y_task_array1 = array(y_task_list1)
	x_task_array2 = ((array(x_task_list2) - start_time) / 10000000)
	y_task_array2 = array(y_task_list2)
	x_task_array3 = ((array(x_task_list3) - start_time) / 10000000)
	y_task_array3 = array(y_task_list3)

	trace1 = go.Scatter(
	    x=x_array1,
	    y=y_list1,
	    mode = 'lines+markers',
	    name = 'Node 1', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(205, 12, 24)'))
	)

	trace12 = go.Scatter(
	    x=x_array1,
	    y=(y_array1 / 2),
	    mode = 'lines',
	    name = 'Node 1 acceptance border', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(205, 12, 24)'),
        	dash = 'dot')
	)

	trace13 = go.Scatter(
	    x=x_task_array1,
	    y=y_task_array1,
	    mode = 'markers',
	    name = 'Node 1 migrated tasks', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(205, 12, 24)'),
        	dash = 'dot'),
	    yaxis='y2'
	)

	trace2 = go.Scatter(
	    x=x_array2,
	    y=y_list2,
	    mode = 'lines+markers',
	    name = 'Node 2', # Style name/legend entry with html tags
    	line = dict(
        	color = ('rgb(22, 96, 167)'))
	)

	trace22 = go.Scatter(
	    x=x_array2,
	    y=(y_array2 / 2),
	    mode = 'lines',
	    name = 'Node 2 acceptance border', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(22, 96, 167)'),
        	dash = 'dot')
	)

	trace23 = go.Scatter(
	    x=x_task_array2,
	    y=y_task_array2,
	    mode = 'markers',
	    name = 'Node 2 migrated tasks', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(22, 96, 167)'),
        	dash = 'dot'),
	    yaxis='y2'
	)

	trace3 = go.Scatter(
	    x=x_array3,
	    y=y_list3,
	    mode = 'lines+markers',
	    name = 'Node 3', # Style name/legend entry with html tags
    	line = dict(
        	color = ('rgb(22, 205, 22)'))
	)

	trace32 = go.Scatter(
	    x=x_array3,
	    y=(y_array3 / 2),
	    mode = 'lines',
	    name = 'Node 3 acceptance border', # Style name/legend entry with html tags
    	line = dict(
        	color = ('rgb(22, 205, 22)'),
        	dash = 'dot')
	)

	trace33 = go.Scatter(
	    x=x_task_array3,
	    y=y_task_array3,
	    mode = 'markers',
	    name = 'Node 3 migrated tasks', # Style name/legend entry with html tags
	    line = dict(
	    	color = ('rgb(22, 205, 22)'),
        	dash = 'dot'),
	    yaxis='y2'
	)

	data = [trace1, trace12, trace13, trace2, trace22, trace23, trace3, trace32, trace33]
	#data = [trace13, trace23, trace33]

	layout = go.Layout(
    xaxis=dict(
        title='Time [s]',
    ),
    yaxis=dict(
        title='Kudos',
    ),
    yaxis2=dict(
        title='Migrated tasks',
        overlaying='y',
        side='right')
	)

	fig = dict(data=data, layout=layout)
	py.plot(fig, filename='simple-connectgaps')

if __name__ == "__main__":
    main()