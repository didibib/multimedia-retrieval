import os
import numpy as np
import matplotlib.pyplot as plt

rel_folder = '../export/results/'

dirname = os.path.dirname(__file__) # Get current directory
folder = os.path.join(dirname, rel_folder)

def create_bar_plot(data, labels, title):
    fig, ax = plt.subplots()

    bar = ax.bar(labels, data)

    def gradientbars_sliced(bars):
        ax = bars[0].axes
        xmin, xmax = ax.get_xlim()
        ymin, ymax = ax.get_ylim()
        for bar in bars:
            bar.set_zorder(1)
            bar.set_facecolor("none")
            x, y = bar.get_xy()
            w, h = bar.get_width(), bar.get_height()
            grad = np.linspace(y, y + h, 256).reshape(256, 1)
            ax.imshow(grad, extent=[x, x + w, y, y + h], aspect="auto", zorder=0, origin='lower',
                    vmin=ymin, vmax=ymax, cmap='magma')
        ax.axis([xmin, xmax, ymin, ymax])

    gradientbars_sliced(bar)
    plt.xticks(rotation = 45)
    plt.title(title)
    plt.savefig(folder + title)

file_names = ["ANN_KNN", "ANN_RNN", "KNN_handmade"]

for name in file_names:
    path = os.path.join(folder, name + "_result_data.txt")
    data = np.loadtxt(path)

    path = os.path.join(folder, name + "_result_labels.txt")
    file = open(path, 'r')
    lines = file.readlines()
    labels = [l.strip() for l in lines]

    create_bar_plot(data, labels, name)
        

