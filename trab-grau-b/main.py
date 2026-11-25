import os
import sys
import time
import unicodedata

import cv2 as cv
import numpy as np

from efeitos import EFEITOS, aplicar, ajustar_parametro
from stickers import NOMES, carregar_stickers, colar, stickers_nos_rostos

PASTA = os.path.dirname(os.path.abspath(__file__))
JANELA = "Stories Lab"
CANAIS = [None, 2, 1, 0]
NOME_CANAL = ["RGB", "R", "G", "B"]

# códigos de cv.waitKeyEx no macOS, no Linux e no Windows
SETAS = {
    "esquerda": (63234, 65361, 2424832),
    "direita": (63235, 65363, 2555904),
    "cima": (63232, 65362, 2490368),
    "baixo": (63233, 65364, 2621440),
}

AJUDA = """
Teclas (com a janela da imagem em foco):
  ← / →      efeito anterior / próximo (mostra a descrição no console)
  ↑ / ↓      aumenta / diminui o parâmetro do efeito selecionado
  Enter      aplica o efeito selecionado (vários podem ser aplicados em sequência)
  c          troca o canal: RGB -> R -> G -> B
  z          desfaz o último efeito
  1 a 6      escolhe o sticker; clique esquerdo cola, clique direito remove o último
  d          liga/desliga sticker automático nos rostos
  f          abre outra foto (caminho digitado no console)
  i          troca a segunda imagem usada nas operações
  w          liga/desliga a webcam (modo vídeo)
  s          salva o resultado em saidas/
  r          reset: volta para a imagem original
  h          mostra esta ajuda
  q / Esc    sai
"""

estado = {
    "foto": None,
    "segunda": None,
    "camera": None,
    "aplicados": [],
    "colados": [],
    "efeito": 0,
    "canal": 0,
    "sticker": 0,
    "rostos": False,
}
stickers = carregar_stickers(os.path.join(PASTA, "assets", "stickers"))


def ajustar_tamanho(img, largura=960, altura=720):
    escala = min(largura / img.shape[1], altura / img.shape[0], 1.0)
    return cv.resize(img, None, fx=escala, fy=escala, interpolation=cv.INTER_AREA)


def ler_imagem(caminho):
    img = cv.imread(caminho)
    if img is None:
        print(f"Não foi possível abrir '{caminho}'.")
        return None
    return ajustar_tamanho(img)


def pedir_caminho(mensagem):
    return input(mensagem).strip().strip("'\"").replace("\\ ", " ")


def imagem_base():
    if estado["camera"] is None:
        return estado["foto"]
    ok, frame = estado["camera"].read()
    if not ok:
        return estado["foto"]
    return ajustar_tamanho(cv.flip(frame, 1))


def processar(base):
    altura, largura = base.shape[:2]
    outra = cv.resize(estado["segunda"], (largura, altura))
    img = base.copy()
    for e, valor, canal in estado["aplicados"]:
        img = aplicar(img, e, valor, canal, outra)
    for indice, x, y in estado["colados"]:
        colar(img, stickers[indice], x, y)
    if estado["rostos"]:
        stickers_nos_rostos(img, base, stickers[estado["sticker"]], NOMES[estado["sticker"]])
    canal = CANAIS[estado["canal"]]
    if canal is not None:
        so_um_canal = np.zeros_like(img)
        so_um_canal[:, :, canal] = img[:, :, canal]
        img = so_um_canal
    return img


def sem_acento(texto):
    return unicodedata.normalize("NFKD", texto).encode("ascii", "ignore").decode()


def desenhar_hud(img):
    tela = img.copy()
    e = EFEITOS[estado["efeito"]]
    modo = "VIDEO" if estado["camera"] is not None else "FOTO"
    linhas = [
        f"Modo {modo} | canal {NOME_CANAL[estado['canal']]} | efeitos aplicados: {len(estado['aplicados'])}",
        f"[{e['grupo']}] {e['nome']}" + (f" | {e['param']} = {e['valor']}" if e["param"] else ""),
        f"Sticker {estado['sticker'] + 1}: {NOMES[estado['sticker']]}" + (" | rostos ON" if estado["rostos"] else ""),
    ]
    for i, texto in enumerate(linhas):
        posicao = (10, 24 + 22 * i)
        cv.putText(tela, sem_acento(texto), posicao, cv.FONT_HERSHEY_SIMPLEX, 0.55, (0, 0, 0), 3, cv.LINE_AA)
        cv.putText(tela, sem_acento(texto), posicao, cv.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1, cv.LINE_AA)
    return tela


def mostrar_descricao():
    e = EFEITOS[estado["efeito"]]
    print(f"\n[{e['grupo']}] {e['nome']}: {e['descricao']}")
    if e["param"]:
        print(f"  Parâmetro: {e['param']} = {e['valor']} (ajuste com as setas ↑ e ↓). Enter aplica.")
    else:
        print("  Sem parâmetros. Enter aplica.")


def alternar_webcam():
    if estado["camera"] is not None:
        estado["camera"].release()
        estado["camera"] = None
        print("Webcam desligada, de volta ao modo foto.")
        return
    camera = cv.VideoCapture(0)
    if not camera.isOpened():
        print("Não foi possível abrir a webcam.")
        return
    estado["camera"] = camera
    print("Modo vídeo: webcam ligada.")


def salvar(img):
    pasta = os.path.join(PASTA, "saidas")
    os.makedirs(pasta, exist_ok=True)
    caminho = os.path.join(pasta, time.strftime("resultado_%Y%m%d_%H%M%S.png"))
    cv.imwrite(caminho, img)
    print(f"Salvo em {caminho}")


def resetar():
    estado["aplicados"].clear()
    estado["colados"].clear()
    estado["canal"] = 0
    estado["rostos"] = False
    print("Reset: imagem original.")


def mouse(evento, x, y, flags, param):
    if evento == cv.EVENT_LBUTTONDOWN:
        estado["colados"].append((estado["sticker"], x, y))
    elif evento == cv.EVENT_RBUTTONDOWN and estado["colados"]:
        estado["colados"].pop()


def tratar_tecla(tecla, img):
    e = EFEITOS[estado["efeito"]]
    if tecla in SETAS["esquerda"] + SETAS["direita"]:
        passo = 1 if tecla in SETAS["direita"] else -1
        estado["efeito"] = (estado["efeito"] + passo) % len(EFEITOS)
        mostrar_descricao()
        return True
    if tecla in SETAS["cima"] + SETAS["baixo"]:
        ajustar_parametro(e, 1 if tecla in SETAS["cima"] else -1)
        if e["param"]:
            print(f"  {e['param']} = {e['valor']}")
        return True

    tecla &= 0xFF
    if tecla in (27, ord("q")):
        return False
    if tecla in (13, 10, 32):
        estado["aplicados"].append((e, e["valor"], CANAIS[estado["canal"]]))
        print(f"Aplicado: {e['nome']} no canal {NOME_CANAL[estado['canal']]}")
    elif tecla == ord("c"):
        estado["canal"] = (estado["canal"] + 1) % len(CANAIS)
        print(f"Canal: {NOME_CANAL[estado['canal']]}")
    elif tecla == ord("z") and estado["aplicados"]:
        print(f"Desfeito: {estado['aplicados'].pop()[0]['nome']}")
    elif ord("1") <= tecla < ord("1") + len(NOMES):
        estado["sticker"] = tecla - ord("1")
    elif tecla == ord("d"):
        estado["rostos"] = not estado["rostos"]
    elif tecla == ord("f"):
        foto = ler_imagem(pedir_caminho("Caminho da foto: "))
        if foto is not None:
            if estado["camera"] is not None:
                alternar_webcam()
            estado["foto"] = foto
            estado["colados"].clear()
    elif tecla == ord("i"):
        segunda = ler_imagem(pedir_caminho("Caminho da segunda imagem: "))
        if segunda is not None:
            estado["segunda"] = segunda
    elif tecla == ord("w"):
        alternar_webcam()
    elif tecla == ord("s"):
        salvar(img)
    elif tecla == ord("r"):
        resetar()
    elif tecla == ord("h"):
        print(AJUDA)
    return True


def main():
    imagens = os.path.join(PASTA, "assets", "imagens")
    estado["foto"] = ler_imagem(sys.argv[1] if len(sys.argv) > 1 else os.path.join(imagens, "baboon.jpg"))
    estado["segunda"] = ler_imagem(os.path.join(imagens, "fruits.jpg"))
    if estado["foto"] is None or estado["segunda"] is None:
        return

    cv.namedWindow(JANELA)
    cv.setMouseCallback(JANELA, mouse)
    print(AJUDA)
    mostrar_descricao()

    rodando = True
    while rodando:
        resultado = processar(imagem_base())
        cv.imshow(JANELA, desenhar_hud(resultado))
        tecla = cv.waitKeyEx(30)
        if tecla != -1:
            rodando = tratar_tecla(tecla, resultado)
        if cv.getWindowProperty(JANELA, cv.WND_PROP_VISIBLE) < 1:
            rodando = False

    if estado["camera"] is not None:
        estado["camera"].release()
    cv.destroyAllWindows()


if __name__ == "__main__":
    main()
