import os

import cv2 as cv
import numpy as np

NOMES = ["coracao", "estrela", "oculos", "coroa", "balao", "gato"]
DETECTOR_ROSTO = cv.CascadeClassifier(cv.data.haarcascades + "haarcascade_frontalface_default.xml")


def carregar_stickers(pasta):
    return [cv.imread(os.path.join(pasta, nome + ".png"), cv.IMREAD_UNCHANGED) for nome in NOMES]


def colar(img, sticker, cx, cy):
    h, w = sticker.shape[:2]
    x0, y0 = cx - w // 2, cy - h // 2
    x1, y1 = max(x0, 0), max(y0, 0)
    x2, y2 = min(x0 + w, img.shape[1]), min(y0 + h, img.shape[0])
    if x1 >= x2 or y1 >= y2:
        return
    pedaco = sticker[y1 - y0:y2 - y0, x1 - x0:x2 - x0]
    alfa = pedaco[:, :, 3:] / 255.0
    fundo = img[y1:y2, x1:x2]
    # C = c1 * alfa + c2 * (1 - alfa)
    img[y1:y2, x1:x2] = (pedaco[:, :, :3] * alfa + fundo * (1 - alfa)).astype(np.uint8)


def stickers_nos_rostos(img, base, sticker, nome):
    cinza = cv.cvtColor(base, cv.COLOR_BGR2GRAY)
    rostos = DETECTOR_ROSTO.detectMultiScale(cinza, scaleFactor=1.1, minNeighbors=5, minSize=(60, 60))
    for x, y, w, h in rostos:
        escala = w / sticker.shape[1]
        ajustado = cv.resize(sticker, None, fx=escala, fy=escala)
        if nome == "oculos":
            centro_y = y + int(h * 0.38)
        else:
            centro_y = y - ajustado.shape[0] // 2 + h // 10
        colar(img, ajustado, x + w // 2, centro_y)
    return len(rostos)
