import os

import cv2 as cv
import numpy as np

PASTA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "stickers")
AA = cv.LINE_AA


def tela(largura, altura):
    return np.zeros((altura, largura, 4), np.uint8)


def com_borda(sticker, espessura=5):
    kernel = cv.getStructuringElement(cv.MORPH_ELLIPSE, (2 * espessura + 1, 2 * espessura + 1))
    alfa = sticker[:, :, 3:] / 255.0
    saida = np.zeros_like(sticker)
    saida[:, :, :3] = (sticker[:, :, :3] + 255 * (1 - alfa)).astype(np.uint8)
    saida[:, :, 3] = cv.dilate(sticker[:, :, 3], kernel)
    return saida


def coracao():
    s = tela(170, 150)
    cor = (70, 40, 225, 255)
    cv.circle(s, (55, 55), 40, cor, -1, AA)
    cv.circle(s, (115, 55), 40, cor, -1, AA)
    cv.fillPoly(s, [np.array([[19, 73], [151, 73], [85, 140]])], cor, AA)
    cv.ellipse(s, (45, 42), (14, 9), -40, 0, 360, (190, 170, 255, 255), -1, AA)
    return s


def estrela():
    s = tela(160, 160)
    pontos = []
    for i in range(10):
        raio = 68 if i % 2 == 0 else 28
        angulo = np.radians(-90 + 36 * i)
        pontos.append([80 + raio * np.cos(angulo), 84 + raio * np.sin(angulo)])
    pontos = np.array(pontos, np.int32)
    cv.fillPoly(s, [pontos], (40, 210, 255, 255), AA)
    cv.polylines(s, [pontos], True, (0, 140, 255, 255), 4, AA)
    return s


def oculos():
    s = tela(220, 90)
    armacao = (25, 25, 25, 255)
    for cx in (60, 160):
        cv.ellipse(s, (cx, 46), (45, 31), 0, 0, 360, (80, 50, 30, 255), -1, AA)
        cv.ellipse(s, (cx, 46), (45, 31), 0, 0, 360, armacao, 6, AA)
        cv.ellipse(s, (cx - 18, 34), (13, 6), -25, 0, 360, (220, 200, 190, 255), -1, AA)
    cv.line(s, (104, 38), (116, 38), armacao, 6, AA)
    return s


def coroa():
    s = tela(180, 140)
    pontos = np.array([[20, 122], [20, 45], [55, 85], [90, 20], [125, 85], [160, 45], [160, 122]])
    cv.fillPoly(s, [pontos], (30, 195, 245, 255), AA)
    cv.polylines(s, [pontos], True, (20, 125, 195, 255), 4, AA)
    for x, y in [(20, 45), (90, 20), (160, 45)]:
        cv.circle(s, (x, y), 9, (60, 60, 230, 255), -1, AA)
    cv.circle(s, (90, 102), 11, (220, 120, 40, 255), -1, AA)
    return s


def balao():
    s = tela(210, 160)
    branco, preto = (255, 255, 255, 255), (20, 20, 20, 255)
    cauda = np.array([[62, 100], [100, 112], [45, 146]])
    cv.ellipse(s, (105, 66), (92, 57), 0, 0, 360, preto, -1, AA)
    cv.fillPoly(s, [cauda], preto, AA)
    cv.polylines(s, [cauda], True, preto, 8, AA)
    cv.ellipse(s, (105, 66), (87, 52), 0, 0, 360, branco, -1, AA)
    cv.fillPoly(s, [cauda], branco, AA)
    (largura, altura), _ = cv.getTextSize("UAU!", cv.FONT_HERSHEY_DUPLEX, 1.6, 3)
    cv.putText(s, "UAU!", (105 - largura // 2, 66 + altura // 2), cv.FONT_HERSHEY_DUPLEX, 1.6, preto, 3, AA)
    return s


def gato():
    s = tela(220, 100)
    for lado in (-1, 1):
        cx = 110 + lado * 62
        orelha = np.array([[cx - 38, 92], [cx + 38, 92], [cx + lado * 14, 10]], np.float32)
        centro = orelha.mean(axis=0)
        dentro = centro + (orelha - centro) * 0.55
        cv.fillPoly(s, [orelha.astype(np.int32)], (70, 70, 80, 255), AA)
        cv.fillPoly(s, [dentro.astype(np.int32)], (180, 150, 250, 255), AA)
    return s


if __name__ == "__main__":
    os.makedirs(PASTA, exist_ok=True)
    for desenhar in (coracao, estrela, oculos, coroa, balao, gato):
        caminho = os.path.join(PASTA, desenhar.__name__ + ".png")
        cv.imwrite(caminho, com_borda(desenhar()))
        print("Gerado", caminho)
