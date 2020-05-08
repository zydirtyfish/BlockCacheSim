#encoding=utf-8
import numpy as np
from numpy import *
import matplotlib.pyplot as plt

def get_hit_ratio(name):
	f = np.loadtxt(name,delimiter=",")
	rst = [ 0 for i in range(0,3)]
	for i in f:
		idx = int(i[0]) -1
		rst[idx] = rst[idx] + float(i[4])

	for i in range(0,len(rst)):
		rst[i] = rst[i] / 5
	return rst

def get_total_traffic(name):
	f = np.loadtxt(name,delimiter=",")
	rst = [ 0 for i in range(0,3)]
	for i in f:
		idx = int(i[0]) -1
		rst[idx] = rst[idx] + int(i[5])

	for i in range(0,len(rst)):
		rst[i] = rst[i] / 5
	return rst


def main():
	name_list=["tencent_0","tencent_1","tencent_2","tencent_3","websearch","adsds","online","stg"]
	total_width , n = 0.6,3
	width = total_width / n
	fs = 24

	hit_ratio = [ [] for i in range(0,3)]
	traffic = [ [] for i in range(0,3)]

	for i in range(0,len(name_list)):
		hit_tmp = get_hit_ratio(name_list[i])
		traffic_tmp = get_total_traffic(name_list[i])
		for j in range(0,3):
			hit_ratio[j].append(hit_tmp[j] * 1.0 / 100)
			traffic[j].append(traffic_tmp[j])

	plt.figure(figsize=(12,6))
	x=list(range(len(hit_ratio[0])))
	plt.bar(x,hit_ratio[0],width=width,label='lru',ec='black',fc='white',hatch='x')

	for i in range(len(x)):
		x[i] = x[i] + width
	plt.bar(x,hit_ratio[1],width=width,label='cflru',tick_label=name_list,ec='black',fc='white',hatch='.')

	for i in range(len(x)):
		x[i] = x[i] + width
	plt.bar(x,hit_ratio[2],width=width,label='cflru+lea',ec='black',fc='white',hatch='/')

	plt.legend(fontsize=fs,ncol=3,loc="upper center")
	plt.ylabel('Hit ratio',fontsize=fs)
	plt.xlabel('IO trace',fontsize=fs)
	plt.xticks(fontsize=fs,rotation=20)
	plt.yticks(fontsize=fs)
	plt.ylim(0,0.9)
	plt.tight_layout()
	plt.savefig("hitratio.pdf")
	plt.close()


	plt.figure(figsize=(12,6))
	#normalize
	for i in range(2,-1,-1):
		for j in range(0,len(traffic[0])):
			traffic[i][j] = traffic[i][j] / traffic[0][j]

	x=list(range(len(traffic[0])))
	plt.bar(x,traffic[0],width=width,label='lru',ec='black',fc='white',hatch='x')

	for i in range(len(x)):
		x[i] = x[i] + width
	plt.bar(x,traffic[1],width=width,label='cflru',tick_label=name_list,ec='black',fc='white',hatch='.')

	for i in range(len(x)):
		x[i] = x[i] + width
	plt.bar(x,traffic[2],width=width,label='cflru+lea',ec='black',fc='white',hatch='/')
	
	plt.legend(fontsize=fs,ncol=3,loc="upper center")
	plt.ylabel('Normalized write traffic',fontsize=fs)
	plt.xlabel('IO trace',fontsize=fs)
	plt.xticks(fontsize=fs,rotation=20)
	plt.yticks(fontsize=fs)
	plt.ylim(0,1.4)
	plt.tight_layout()
	plt.savefig("traffic.pdf")
	plt.close()

	#print(hit_ratio)
	#print(traffic)

	hit_mean = [ mean(hit_ratio[i]) for i in range(0,3) ]
	traffic_mean = [ mean(traffic[i]) for i in range(0,3) ]
	print(hit_mean,traffic_mean)

if __name__ == "__main__":
	main()
