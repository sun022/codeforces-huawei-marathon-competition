import cv2
import numpy as np
import csv

image = cv2.imread('shuffled-images-data/data_train/32-sources/0608.png')

border= 0 ;
p = 32
m = int(512/p)

result = np.zeros((2*512, 2*512, 3), dtype=np.uint8)

def copy_over(sr, sc, r, c):
    # print(sr, sc, r, c)
    result[r*(p+border) : r*(p+border) + p , c*(p+border) : c*(p+border) + p] = image[sr*p : sr*p + p , sc*p : sc*p + p]
    # result[sr*(p+1) : sr*(p+1) + p , sc*(p+1) : sc*(p+1) + p] = image[sr*p : sr*p + p , sc*p : sc*p + p]


with open("out.txt") as fd:
    rd = csv.reader(fd, delimiter="\t", quotechar='"')
    for row in rd:
        r = int(row[0])
        c = int(row[1])
        val = int(row[2])
        copy_over(val//m, val%m, r, c)

cv2.imshow('result', result)
cv2.waitKey(0)
