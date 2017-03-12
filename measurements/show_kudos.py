import plotly.plotly as py
#import plotly.figure_factory as ff
import plotly.graph_objs as go
import plotly
from bigchaindb_driver import BigchainDB
from urllib.request import urlopen
import json

#import numpy as np
#plotly.tools.set_credentials_file(username='pepus', api_key='X9vAgXhDnpR8uTdtx7xN')

def main():
	api_endpoint = 'http://192.168.99.100:59984/api/v1'
	unspents_endpoint = 'http://192.168.99.100:59984/api/v1/unspents/?owner_after='
	bdb = BigchainDB(api_endpoint)

	x_list=[]
	y_list=[]

	verifying_key = "HfP8mrYEfPKLYU671WpAGzVfdxJg81Z4PivX6w7EbHRP"
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/transactions/" + tx
		response = urlopen(url)
		string = response.read().decode('utf-8')
		tx_obj = json.loads(string)
		if ((list(tx_obj['transaction']['asset']['data'])[0]) == "KUDOS"):
			kudos_value = tx_obj['transaction']['asset']['data']['KUDOS']['kudos_value']
			kudos_time = tx_obj['transaction']['asset']['data']['KUDOS']['time']
			x_list.append(kudos_time)
			y_list.append(kudos_value)

	#sort lists
	x_list, y_list = (list(t) for t in zip(*sorted(zip(x_list, y_list))))
	for index in range(len(y_list)):
		if (index>1):
   			y_list[index] = y_list[index] + y_list[index-1]
   		#print (x_list[index])

	print (x_list)
	print (y_list)

	trace1 = go.Scatter(
	    x=x_list,
	    y=y_list,
	    mode = 'lines+markers',
	    name = '<b>No</b> Gaps', # Style name/legend entry with html tags
	)

	data = [trace1] #, trace2]

	fig = dict(data=data)
	py.plot(fig, filename='simple-connectgaps')

if __name__ == "__main__":
    main()