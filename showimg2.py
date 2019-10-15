import cv2
import numpy as np
import csv

image = cv2.imread('shuffled-images-data/data_train/64-sources/1236.png')
p = 64

m = int(512/p)

result = np.zeros((2*512, 2*512, 3), dtype=np.uint8)

border= 0 
def copy_over(sr, sc, r, c):
    # print(sr, sc, r, c)
    result[r*(p+border) : r*(p+border) + p , c*(p+border) : c*(p+border) + p] = image[sr*p : sr*p + p , sc*p : sc*p + p]
    # result[sr*(p+1) : sr*(p+1) + p , sc*(p+1) : sc*(p+1) + p] = image[sr*p : sr*p + p , sc*p : sc*p + p]


def draw_border(t, k):
    r = t//m
    c = t%m
    print(r,c)
    if(k==2):
        k = 0
        c-=1
    if(k==3):
        k = 1
        r-=1
    if(k==0):
        image[r*p : (r+1)*p, (c+1)*p : (c+1)*p + 1] = (0, 0, 255)    
    if(k==1):
        image[(r+1)*p : (r+1)*p + 1, c*p : (c+1)*p] = (0, 0, 255)

with open("out.txt") as fd:
    rd = csv.reader(fd, delimiter="\t", quotechar='"')
    for row in rd:
        r = int(row[0])
        c = int(row[1])
        t = int(row[2])
        copy_over(t//m, t%m, r, c)

# draw_border(114, 0)
# draw_border(114, 1)
# draw_border(146, 0)
# draw_border(146, 1)
# draw_border(178, 0)

cv2.imshow('result', result)
cv2.waitKey(0)
