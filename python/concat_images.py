import enum
import os
from PIL import Image
import numpy as np
import math

rel_img_path = '../export/histogram/images/'
dirname = os.path.dirname(__file__) # Get current directory
full_img_path = os.path.join(dirname, rel_img_path)

images = os.listdir(full_img_path)
images = [f for f in images if os.path.isfile(full_img_path + '/' + f)]

A3 = []
D1 = []
D2 = []
D3 = []
D4 = []

for root, dirs, files in os.walk(full_img_path):
    for filename in files:
        path = os.path.join(root, filename)
        match filename:
            case 'A3.png':
                A3.append(path)
            case 'D1.png':
                D1.append(path)
            case 'D2.png':
                D2.append(path)
            case 'D3.png':
                D3.append(path)
            case 'D4.png':
                D4.append(path)

A3 = np.array(A3)
D1 = np.array(D1)
D2 = np.array(D2)
D3 = np.array(D3)
D4 = np.array(D4)

p = np.random.permutation(len(A3))

A3[p]
D1[p]
D2[p]
D3[p]
D4[p]

def concat(descriptor, filename):
    img0 = Image.open(descriptor[0])
    (width, height) = img0.size
    # Create 4x4 images
    dst = Image.new('RGB', (width * 4, height * 4))
    i = 0
    n = 0
    for (index, path) in enumerate(descriptor):        
        img = Image.open(path)
        x = i % 4
        y = math.floor(i / 4)
        dst.paste(img, (x * width, y * height))
        i = i + 1
        if(i >= 16):
            dst.save(full_img_path + '/' + filename + '_' + str(n) + '.jpg')
            dst = Image.new('RGB', (width * 4, height * 4), 'white')
            i = 0
            n = n + 1
        if(index >= len(descriptor) - 1):    
            left = top = 0
            right = dst.width
            bottom = (y + 1) * height
            dst.crop((left, top, right, bottom))          
            dst.save(full_img_path + '/' + filename + '_' + str(n) + '.jpg')          

print('Concatting images...')
concat(A3, 'A3')
concat(D1, 'D1')
concat(D2, 'D2')
concat(D3, 'D3')
concat(D4, 'D4')
print('Finished!')