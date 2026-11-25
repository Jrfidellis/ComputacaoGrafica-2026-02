import cv2 as cv
import numpy as np


def em_cinza(img, funcao):
    if img.ndim == 2:
        return funcao(img)
    resultado = funcao(cv.cvtColor(img, cv.COLOR_BGR2GRAY))
    return cv.cvtColor(resultado, cv.COLOR_GRAY2BGR)


def cinza(img, valor, outra):
    return em_cinza(img, lambda g: g)


def negativo(img, valor, outra):
    return cv.bitwise_not(img)


def saturacao(img, valor, outra):
    if img.ndim == 2:
        return img
    h, s, v = cv.split(cv.cvtColor(img, cv.COLOR_BGR2HSV))
    s = cv.multiply(s, valor)
    return cv.cvtColor(cv.merge([h, s, v]), cv.COLOR_HSV2BGR)


def limiar(img, valor, outra):
    return em_cinza(img, lambda g: cv.threshold(g, valor, 255, cv.THRESH_BINARY)[1])


def media(img, valor, outra):
    return cv.blur(img, (valor, valor))


def gaussiano(img, valor, outra):
    return cv.GaussianBlur(img, (valor, valor), 0)


def mediana(img, valor, outra):
    return cv.medianBlur(img, valor)


def nitidez(img, valor, outra):
    kernel = np.array([[0, -valor, 0],
                       [-valor, 1 + 4 * valor, -valor],
                       [0, -valor, 0]], np.float32)
    return cv.filter2D(img, -1, kernel)


def laplaciano(img, valor, outra):
    return em_cinza(img, lambda g: cv.convertScaleAbs(cv.Laplacian(g, cv.CV_64F, ksize=valor)))


def sobel(img, valor, outra):
    def bordas(g):
        gx = cv.Sobel(g, cv.CV_64F, 1, 0, ksize=valor)
        gy = cv.Sobel(g, cv.CV_64F, 0, 1, ksize=valor)
        return cv.convertScaleAbs(cv.magnitude(gx, gy))
    return em_cinza(img, bordas)


def dilatacao(img, valor, outra):
    return cv.dilate(img, np.ones((valor, valor), np.uint8))


def erosao(img, valor, outra):
    return cv.erode(img, np.ones((valor, valor), np.uint8))


def soma(img, valor, outra):
    return cv.addWeighted(img, 1.0, outra, valor, 0)


def subtracao(img, valor, outra):
    return cv.addWeighted(img, 1.0, outra, -valor, 0)


def blending(img, valor, outra):
    return cv.addWeighted(img, valor, outra, 1 - valor, 0)


def multiplicacao(img, valor, outra):
    return cv.multiply(img, outra, scale=1 / 255)


def efeito(nome, grupo, funcao, descricao, param=None, valor=None, passo=None, minimo=None, maximo=None):
    return {"nome": nome, "grupo": grupo, "funcao": funcao, "descricao": descricao,
            "param": param, "valor": valor, "passo": passo, "minimo": minimo, "maximo": maximo}


EFEITOS = [
    efeito("Escala de cinza", "Cor", cinza,
           "Junta os três canais em um só, guardando apenas a luminosidade (cv.cvtColor BGR2GRAY)."),
    efeito("Negativo", "Cor", negativo,
           "Inverte cada canal: novo = 255 - valor, o mesmo que fazer XOR com 255."),
    efeito("Saturação (HSV)", "Cor", saturacao,
           "Converte para HSV e multiplica o canal S. Acima de 1 as cores ficam mais vivas, abaixo de 1 mais apagadas.",
           "fator", 1.5, 0.25, 0.0, 3.0),
    efeito("Limiarização", "Intensidade", limiar,
           "Binariza a imagem: pixels acima do limiar K viram branco e o resto vira preto.",
           "limiar K", 84, 10, 0, 255),
    efeito("Média (Box blur)", "Suavização", media,
           "Passa-baixa: cada pixel vira a média da vizinhança. Quanto maior o kernel, mais borrado.",
           "tamanho do kernel", 5, 2, 3, 31),
    efeito("Gaussiano", "Suavização", gaussiano,
           "Passa-baixa com pesos gaussianos: o centro pesa mais que as bordas, então borra de forma mais natural.",
           "tamanho do kernel", 5, 2, 3, 31),
    efeito("Mediana", "Estatístico", mediana,
           "Troca cada pixel pela mediana da vizinhança. Remove ruído sal e pimenta sem borrar tanto as bordas.",
           "tamanho do kernel", 5, 2, 3, 15),
    efeito("Nitidez (Sharpen)", "Realce", nitidez,
           "Passa-alta: kernel [0,-1,0; -1,5,-1; 0,-1,0] que reforça a diferença entre o pixel e os vizinhos.",
           "intensidade", 1.0, 0.5, 0.5, 5.0),
    efeito("Laplaciano", "Realce", laplaciano,
           "Segunda derivada da imagem: destaca bordas em todas as direções e apaga as regiões lisas.",
           "tamanho do kernel", 3, 2, 1, 7),
    efeito("Sobel", "Bordas", sobel,
           "Calcula o gradiente em x e em y e mostra a magnitude, ou seja, onde a intensidade muda mais.",
           "tamanho do kernel", 3, 2, 1, 7),
    efeito("Dilatação (máximo)", "Estatístico", dilatacao,
           "Filtro de máximo: cada pixel recebe o maior valor da vizinhança, expandindo as áreas claras.",
           "tamanho do kernel", 3, 2, 3, 21),
    efeito("Erosão (mínimo)", "Estatístico", erosao,
           "Filtro de mínimo: cada pixel recebe o menor valor da vizinhança, expandindo as áreas escuras.",
           "tamanho do kernel", 3, 2, 3, 21),
    efeito("Soma", "Operação com 2 imagens", soma,
           "g = f1 + peso * f2. Soma a segunda imagem por cima, clareando (satura em 255).",
           "peso da 2a imagem", 0.5, 0.1, 0.0, 1.0),
    efeito("Subtração ponderada", "Operação com 2 imagens", subtracao,
           "g = f1 - peso * f2. Tira a segunda imagem da atual, escurecendo (satura em 0).",
           "peso da 2a imagem", 0.5, 0.1, 0.0, 1.0),
    efeito("Blending", "Operação com 2 imagens", blending,
           "Mistura com transparência: g = alfa * f1 + (1 - alfa) * f2.",
           "alfa", 0.5, 0.1, 0.0, 1.0),
    efeito("Multiplicação", "Operação com 2 imagens", multiplicacao,
           "g = f1 * f2 / 255. Escurece onde qualquer uma das duas imagens for escura."),
]


def ajustar_parametro(e, direcao):
    if e["param"] is None:
        return
    novo = min(max(e["valor"] + direcao * e["passo"], e["minimo"]), e["maximo"])
    e["valor"] = round(novo, 2) if isinstance(novo, float) else novo


def aplicar(img, e, valor, canal, outra):
    if canal is None:
        return e["funcao"](img, valor, outra)
    canais = list(cv.split(img))
    canais[canal] = e["funcao"](canais[canal], valor, cv.split(outra)[canal])
    return cv.merge(canais)
