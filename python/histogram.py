import os
import matplotlib.pyplot as plt
import numpy as np

rel_data_path = '../export/histogram/data/'
rel_img_path = '../export/histogram/images/'

dirname = os.path.dirname(__file__) # Get current directory
full_data_path = os.path.join(dirname, rel_data_path)
full_img_path = os.path.join(dirname, rel_img_path)

files = os.listdir(full_data_path)
files = [f for f in files if os.path.isfile(full_data_path + '/' + f)]

def read_data(path):
    lines = []
    with open(path, 'r') as f:
        for i, line in enumerate(f):
            lines.append(line)
    
    dict = {
        'name' : lines[0].replace('\n', ''),
        'min_value' : float(lines[1]),
        'max_value' : float(lines[2]),
        'bin_width' : float(lines[3]),
        'bins' : [float(i) for i in lines[4].split()],
        'data' : [float(i) for i in lines[5].split()]
    }
    return dict

for f in files:
    path = os.path.join(full_data_path, f)
    dict = read_data(path)
    plt.plot(dict['bins'], dict['data'])

    # plt.hist(
    #     x=dict['bins'], 
    #     weights=dict['data'],
    #     label=dict['name'],
    #     rwidth=.9
    #     )
    plt.xlabel('bins')
    plt.ylabel('normalized count')
    plt.savefig(full_img_path + dict['name'] + '.png')
    plt.clf();

