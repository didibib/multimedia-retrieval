import os
import matplotlib.pyplot as plt
import numpy as np

rel_data_path = '../export/histogram/data/'
rel_img_path  = '../export/histogram/images/'

dirname = os.path.dirname(__file__) # Get current directory
root_data_path = os.path.join(dirname, rel_data_path)
full_img_path = os.path.join(dirname, rel_img_path)

files = os.listdir(root_data_path)
files = [f for f in files if os.path.isfile(root_data_path + '/' + f)]

def read_data(file):
    lines = []
    with open(file, 'r') as f:
        for i, line in enumerate(f):
            lines.append(line)
    
    entry = {
        'name' :                    lines[0].replace('\n', ''),
        'label' :                   lines[1].replace('\n', ''),
        'descriptor' :              lines[2].replace('\n', ''),
        'min_value' :         float(lines[3]),
        'max_value' :         float(lines[4]),
        'bin_width' :         float(lines[5]),
        'bins' : [float(i) for i in lines[6].split()],
        'data' : [float(i) for i in lines[7].split()]
    }
    return entry


print('Creating histogram images...')
for root, dirs, files in os.walk(root_data_path):
    for filename in files:
        path = os.path.join(root, filename)
        entry = read_data(path)

        plt.plot(entry['bins'], entry['data'])
        plt.title(entry['label'], fontsize = 24)
        plt.tick_params(labelsize=24)
        
        export_dir = full_img_path + entry['label'] + "/";
        if not os.path.exists(export_dir):
            os.makedirs(export_dir)
        plt.savefig(export_dir + entry['descriptor'] + '.png')
    plt.clf();

print('Finished!')
