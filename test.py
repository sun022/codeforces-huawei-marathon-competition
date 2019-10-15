import cv2
import numpy as np

# image = cv2.imread('shuffled-images-data/data_test2_blank/32/3000.png')
# image = cv2.imread('shuffled-images-data/data_test2_blank/64/3302.png')
image = cv2.imread('shuffled-images-data/data_train/32-sources/0608.png')

p = 32
m = int(512/p)

result = np.zeros((1024, 1024, 3), dtype=np.uint8)
result_occ = np.zeros((2*m, 2*m))
result_from = np.zeros((2*m, 2*m, 2), dtype=np.uint8)
used = np.zeros((m, m))

print(str(len(image)) + "x" + str(len(image[0])) + "x" + str(len(image[0][0])))

# cv2.imshow('orig', image)
# cv2.waitKey(0)

def pixel_err(x, y):
    return (int(x[0])-y[0])**2 + (int(x[1])-y[1])**2 + (int(x[2])-y[2])**2

def test_edge(x1, y1, x2, y2, dir):
    xa = x1*p
    ya = y1*p
    xb = x2*p
    yb = y2*p
    if(dir == "DOWN"):
        xa += p-1    
        dx = 0
        dy = 1    
    elif(dir == "RIGHT"):
        ya += p-1    
        dx = 1
        dy = 0
    elif(dir == "LEFT"):
        return test_edge(x2, y2, x1, y1, "RIGHT")
    elif(dir == "UP"):
        return test_edge(x2, y2, x1, y1, "DOWN")
    else:
        exit(0)

    e = 0.0
    for k in range(p):
        e += pixel_err(image[xa,ya], image[xb, yb])/p

        xa += dx
        xb += dx
        ya += dy
        yb += dy   
    return e

def place_piece(src_x, src_y, dest_x, dest_y):
    result[dest_x*p:dest_x*p + p , dest_y*p:dest_y*p + p,:] = np.array(image[src_x*p:src_x*p + p , src_y*p:src_y*p + p,:])
    result_occ[dest_x, dest_y] = True
    result_from[dest_x, dest_y] = [src_x, src_y]
    used[src_x][src_y] = True

dir = [[0,1],[0,-1],[-1,0],[1,0]]

def find_best_fit_for_position(rx, ry):  
    num_edge = 0
    if ry + 1 != 2*m and result_occ[rx, ry+1]:
        num_edge += 1
    if ry != 0 and result_occ[rx, ry-1]:
        num_edge += 1
    if rx + 1 != 2*m and result_occ[rx+1, ry]:
        num_edge += 1
    if rx != 0 and result_occ[rx-1, ry]:
        num_edge += 1
    
    if(num_edge == 0):
        return [-1, -1, -1]

    bestScore = 999999
    bestX = -1
    bestY = -1
    for x in range(m):
        for y in range(m):
            if used[x][y]:
                continue
            score = 0
            # for each bounding edge
            if ry + 1 != 2*m and result_occ[rx, ry+1]:
                [sx, sy] = result_from[rx, ry+1]
                score += test_edge(x, y, sx, sy, "RIGHT")
            if ry != 0 and result_occ[rx, ry-1]:
                [sx, sy] = result_from[rx, ry-1]
                score += test_edge(x, y, sx, sy, "LEFT")
            if rx + 1 != 2*m and result_occ[rx+1, ry]:
                [sx, sy] = result_from[rx+1, ry]
                score += test_edge(x, y, sx, sy, "DOWN")
            if rx != 0 and result_occ[rx-1, ry]:
                [sx, sy] = result_from[rx-1, ry]
                score += test_edge(x, y, sx, sy, "UP")
            if(score < bestScore):
                bestScore = score
                bestX=x
                bestY=y
    # print(bestScore, bestX, bestY)
    # place_piece(bestX, bestY, rx, ry)

    bestScore *= 1.0/num_edge
    return [bestScore, bestX, bestY]



# start from some root piece
Ry = 8
Rx = 8


place_piece(Rx, Ry, m, m)
# cv2.imshow('result', result)
# cv2.waitKey(0)

for t in range(m*m-1):
    bestScore = 999999
    best = [-1, -1]
    bestR = [-1, -1]
    for rx in range(2*m):
        for ry in range(2*m):
            if(result_occ[rx, ry]):
                continue
            [s, x, y] = find_best_fit_for_position(rx, ry)
            if(s != -1 and s < bestScore):
                bestScore = s
                best = [x, y]
                bestR = [rx, ry]
    print(bestScore, bestR, best)
    place_piece(best[0], best[1], bestR[0], bestR[1])

# find_best_fit_for_position(m + 1, m)
# find_best_fit_for_position(m, m + 1)
# find_best_fit_for_position(m + 1, m + 1)

# find_best_fit_for_position(m + 2, m)
# find_best_fit_for_position(m + 2, m + 1)
# find_best_fit_for_position(m , m + 2)
# find_best_fit_for_position(m + 1, m + 2)
# find_best_fit_for_position(m + 2, m + 2)
cv2.imshow('result', result)
cv2.waitKey(0)


exit(0)
for x in range(m):
    for y in range(m):
        if used[x][y]:
            continue
        score = test_edge(0, 0, x, y, "DOWN")
        if(score < 100):
            print(x, y, score)
            # cv2.imshow('orig', image[x*p:(x+1)*p-1 ,  y*p:(y+1)*p-1])
            # cv2.waitKey(0)


# b = image.copy()
# # set green and red channels to 0
# b[:, :, 1] = 0
# b[:, :, 2] = 0


# g = image.copy()
# # set blue and red channels to 0
# g[:, :, 0] = 0
# g[:, :, 2] = 0

# r = image.copy()
# # set blue and green channels to 0
# r[:, :, 0] = 0
# r[:, :, 1] = 0

# cv2.imshow('orig', image)

# # RGB - Blue
# cv2.imshow('B-RGB', b)

# # RGB - Green
# cv2.imshow('G-RGB', g)

# # RGB - Red
# cv2.imshow('R-RGB', r)

# cv2.imwrite("b.png", b)
# cv2.imwrite("g.png", g)
# cv2.imwrite("r.png", r)

# cv2.waitKey(0)
