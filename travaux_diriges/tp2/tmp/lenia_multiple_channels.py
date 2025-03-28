import pygame  as pg
import numpy   as np
import scipy   as sp
from scipy import signal
from numpy import fft
import time

def gauss(x, mu, sigma):
    return np.exp(-0.5 * ((x-mu)/sigma)**2)

class LeniaFilter:
    def __init__(self, N = 128, ring_strengths = [0.5, 1, 0.667], bs = [[1,5/12,2/3],[1/12,1],[1]], ms = [0.156,0.193,0.342],
                 ss = [0.0118,0.049,0.0891], R = 10, mu = 0.5, sigma = 0.15):

        kernels = [
            {"b":[1],"m":0.272,"s":0.0595,"h":0.138,"r":0.91,"c0":0,"c1":0},
            {"b":[1],"m":0.349,"s":0.1585,"h":0.48,"r":0.62,"c0":0,"c1":0},
            {"b":[1,1/4],"m":0.2,"s":0.0332,"h":0.284,"r":0.5,"c0":0,"c1":0},
            {"b":[0,1],"m":0.114,"s":0.0528,"h":0.256,"r":0.97,"c0":1,"c1":1},
            {"b":[1],"m":0.447,"s":0.0777,"h":0.5,"r":0.72,"c0":1,"c1":1},
            {"b":[5/6,1],"m":0.247,"s":0.0342,"h":0.622,"r":0.8,"c0":1,"c1":1},
            {"b":[1],"m":0.21,"s":0.0617,"h":0.35,"r":0.96,"c0":2,"c1":2},
            {"b":[1],"m":0.462,"s":0.1192,"h":0.218,"r":0.56,"c0":2,"c1":2},
            {"b":[1],"m":0.446,"s":0.1793,"h":0.556,"r":0.78,"c0":2,"c1":2},
            {"b":[11/12,1],"m":0.327,"s":0.1408,"h":0.344,"r":0.79,"c0":0,"c1":1},
            {"b":[3/4,1],"m":0.476,"s":0.0995,"h":0.456,"r":0.5,"c0":0,"c1":2},
            {"b":[11/12,1],"m":0.379,"s":0.0697,"h":0.67,"r":0.72,"c0":1,"c1":0},
            {"b":[1],"m":0.262,"s":0.0877,"h":0.42,"r":0.68,"c0":1,"c1":2},
            {"b":[1/6,1,0],"m":0.412,"s":0.1101,"h":0.43,"r":0.82,"c0":2,"c1":0},
            {"b":[1],"m":0.201,"s":0.0786,"h":0.278,"r":0.82,"c0":2,"c1":1}]
        R = 12
        bs = [k["b"] for k in kernels]
        rs = [R * k["r"] for k in kernels]
        self.ms = [k["m"] for k in kernels]
        self.ss = [k["s"] for k in kernels]
        self.hs = [k["h"] for k in kernels]
        self.sources = [k["c0"] for k in kernels]
        self.destinations = [k["c1"] for k in kernels]

        M = int(np.ceil((16*N)/9)) 
        fhs_y    = N // 2
        fhs_x    = M // 2
        y, x = np.ogrid[-fhs_y:fhs_y, -fhs_x:fhs_x]

        Ks = []
        for b,r in zip(bs,rs):
            distance = np.sqrt(x**2 + y**2) / r * len(b)
            K = np.zeros_like(distance)
            for i in range(len(b)):
                mask = (distance.astype(int) == i)
                K += mask * b[i] * gauss(distance%1, mu, sigma)
            Ks.append(K/np.sum(K))

        self.fKs = []
        for K in Ks:
            fK = fft.fft2(fft.fftshift(K))
            self.fKs.append(fK)
    
    def evolve(self, cells, dt = 0.1):
        fXs = [np.fft.fft2(cells[:,:,i]) for i in range(3)]
        # For each kernel we compute the convolution with the corresponding source channel
        Us = [np.real(fft.ifft2(fK * fXs[source])) for fK,source in zip(self.fKs,self.sources)]
        # We compute the activation associated to each of those convolutions
        As = [2*gauss(U, self.ms[i], self.ss[i]) - 1 for i,U in enumerate(Us)]
        # Then we apply this activation to the corresponding destination channel with the corresponding strength
        Gs = np.zeros_like(cells)
        for destination, h, A in zip(self.destinations, self.hs, As):
            Gs[:,:,destination] += h * A
        # Finally we update the channels
        new_cells = np.clip(cells[:,:,:] + dt * Gs[:,:,:], 0, 1)
        return new_cells

class Drawing:
    def __init__(self, width = 800, height = 600):
        self.dimensions = (width, height)
        self.screen = pg.display.set_mode(self.dimensions)

    def draw(self, cells):
        indices = (255*cells).astype(dtype=np.int32)
        surface = pg.surfarray.make_surface(indices)
        surface = pg.transform.flip(surface, False, True)
        surface = pg.transform.scale(surface, self.dimensions)
        self.screen.blit(surface, (0,0))
        pg.display.update()
    
N = 128
M = int(np.ceil((16*N)/9))

aquarium = [[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.04,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0.49,1.0,0,0.03,0.49,0.49,0.28,0.16,0.03,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0.6,0.47,0.31,0.58,0.51,0.35,0.28,0.22,0,0,0,0,0], [0,0,0,0,0,0,0.15,0.32,0.17,0.61,0.97,0.29,0.67,0.59,0.88,1.0,0.92,0.8,0.61,0.42,0.19,0,0,0], [0,0,0,0,0,0,0,0.25,0.64,0.26,0.92,0.04,0.24,0.97,1.0,1.0,1.0,1.0,0.97,0.71,0.33,0.12,0,0], [0,0,0,0,0,0,0,0.38,0.84,0.99,0.78,0.67,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.95,0.62,0.37,0,0], [0,0,0,0,0.04,0.11,0,0.69,0.75,0.75,0.91,1.0,1.0,0.89,1.0,1.0,1.0,1.0,1.0,1.0,0.81,0.42,0.07,0], [0,0,0,0,0.44,0.63,0.04,0,0,0,0.11,0.14,0,0.05,0.64,1.0,1.0,1.0,1.0,1.0,0.92,0.56,0.23,0], [0,0,0,0,0.11,0.36,0.35,0.2,0,0,0,0,0,0,0.63,1.0,1.0,1.0,1.0,1.0,0.96,0.49,0.26,0], [0,0,0,0,0,0.4,0.37,0.18,0,0,0,0,0,0.04,0.41,0.52,0.67,0.82,1.0,1.0,0.91,0.4,0.23,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.04,0,0.05,0.45,0.89,1.0,0.66,0.35,0.09,0], [0,0,0.22,0,0,0,0.05,0.36,0.6,0.13,0.02,0.04,0.24,0.34,0.1,0,0.04,0.62,1.0,1.0,0.44,0.25,0,0], [0,0,0,0.43,0.53,0.58,0.78,0.9,0.96,1.0,1.0,1.0,1.0,0.71,0.46,0.51,0.81,1.0,1.0,0.93,0.19,0.06,0,0], [0,0,0,0,0.23,0.26,0.37,0.51,0.71,0.89,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.42,0.06,0,0,0], [0,0,0,0,0.03,0,0,0.11,0.35,0.62,0.81,0.93,1.0,1.0,1.0,1.0,1.0,0.64,0.15,0,0,0,0,0], [0,0,0,0,0,0,0.06,0.1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0.05,0.09,0.05,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]],
  [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.02,0.28,0.42,0.44,0.34,0.18,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.34,1.0,1.0,1.0,1.0,1.0,0.91,0.52,0.14,0], [0,0,0,0,0,0,0,0,0,0,0,0,0.01,0.17,0.75,1.0,1.0,1.0,1.0,1.0,1.0,0.93,0.35,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.22,0.92,1.0,1.0,1.0,1.0,1.0,1.0,0.59,0.09], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.75,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.71,0.16], [0,0,0,0,0,0,0,0,0,0,0,0,0.01,0.67,0.83,0.85,1.0,1.0,1.0,1.0,1.0,1.0,0.68,0.17], [0,0,0,0,0,0,0,0,0,0,0,0,0.21,0.04,0.12,0.58,0.95,1.0,1.0,1.0,1.0,1.0,0.57,0.13], [0,0,0,0,0,0,0,0,0,0,0,0.07,0,0,0,0.2,0.64,0.96,1.0,1.0,1.0,0.9,0.24,0.01], [0,0,0,0,0,0,0,0,0,0,0.13,0.29,0,0,0,0.25,0.9,1.0,1.0,1.0,1.0,0.45,0.05,0], [0,0,0,0,0,0,0,0,0,0,0.13,0.31,0.07,0,0.46,0.96,1.0,1.0,1.0,1.0,0.51,0.12,0,0], [0,0,0,0,0,0,0,0,0.26,0.82,1.0,0.95,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.3,0.05,0,0,0], [0,0,0,0,0,0,0,0,0.28,0.74,1.0,0.95,0.87,1.0,1.0,1.0,1.0,1.0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0.07,0.69,1.0,1.0,1.0,1.0,1.0,0.96,0.25,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0.4,0.72,0.9,0.83,0.7,0.56,0.43,0.14,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]],
  [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0,0,0,0,0.04,0.25,0.37,0.44,0.37,0.24,0.11,0.04,0,0,0,0], [0,0,0,0,0,0,0,0,0,0.19,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.75,0.4,0.15,0,0,0,0], [0,0,0,0,0,0,0,0,0.14,0.48,0.83,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.4,0,0,0,0], [0,0,0,0,0,0,0,0,0.62,0.78,0.94,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.64,0,0,0,0], [0,0,0,0,0,0,0,0.02,0.65,0.98,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.78,0,0,0,0], [0,0,0,0,0,0,0,0.15,0.48,0.93,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.79,0.05,0,0,0], [0,0,0,0,0,0,0.33,0.56,0.8,1.0,1.0,1.0,0.37,0.6,0.94,1.0,1.0,1.0,1.0,0.68,0.05,0,0,0], [0,0,0,0,0.35,0.51,0.76,0.89,1.0,1.0,0.72,0.15,0,0.29,0.57,0.69,0.86,1.0,0.92,0.49,0,0,0,0], [0,0,0,0,0,0.38,0.86,1.0,1.0,0.96,0.31,0,0,0,0,0.02,0.2,0.52,0.37,0.11,0,0,0,0], [0,0,0.01,0,0,0.07,0.75,1.0,1.0,1.0,0.48,0.03,0,0,0,0,0,0.18,0.07,0,0,0,0,0], [0,0.11,0.09,0.22,0.15,0.32,0.71,0.94,1.0,1.0,0.97,0.54,0.12,0.02,0,0,0,0,0,0,0,0,0,0], [0.06,0.33,0.47,0.51,0.58,0.77,0.95,1.0,1.0,1.0,1.0,0.62,0.12,0,0,0,0,0,0,0,0,0,0,0], [0.04,0.4,0.69,0.88,0.95,1.0,1.0,1.0,1.0,1.0,0.93,0.68,0.22,0.02,0,0,0.01,0,0,0,0,0,0,0], [0,0.39,0.69,0.91,1.0,1.0,1.0,1.0,1.0,0.85,0.52,0.35,0.24,0.17,0.07,0,0,0,0,0,0,0,0,0], [0,0,0.29,0.82,1.0,1.0,1.0,1.0,1.0,1.0,0.67,0.29,0.02,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0.2,0.51,0.77,0.96,0.93,0.71,0.4,0.16,0,0,0,0,0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0,0.08,0.07,0.03,0,0,0,0,0,0,0,0,0,0,0,0,0]]]
aquarium = [np.array(aquarium[c]) for c in range(3)]

cells = np.zeros((N,M,3))
pos_x,pos_y = N//2,M//2
for c in range(3):
    cells[pos_x:pos_x + aquarium[c].shape[0], pos_y:pos_y + aquarium[c].shape[1],c] = aquarium[c]

pos_x,pos_y = N//4,M//4
for c in range(3):
    cells[pos_x:pos_x + aquarium[c].shape[0], pos_y:pos_y + aquarium[c].shape[1],c] = aquarium[c]

pos_x,pos_y = N//2,M//4
for c in range(3):
    cells[pos_x:pos_x + aquarium[c].shape[0], pos_y:pos_y + aquarium[c].shape[1],c] = aquarium[c]

pos_x,pos_y = N//4,M//2
for c in range(3):
    cells[pos_x:pos_x + aquarium[c].shape[0], pos_y:pos_y + aquarium[c].shape[1],c] = aquarium[c]

filter = LeniaFilter(N)
drawing = Drawing(1024, 768)

pg.init()

loop = True
while loop:
    #time.sleep(0.1) # A régler ou commenter pour vitesse maxi
    t1 = time.time()
    cells = filter.evolve(cells,0.235)
    t2 = time.time()
    drawing.draw(cells)
    t3 = time.time()
    for event in pg.event.get():
        if event.type == pg.QUIT:
            loop = False
    print(f"Temps calcul prochaine generation : {t2-t1:2.2e} secondes, temps affichage : {t3-t2:2.2e} secondes\r", end='')

pg.quit()
