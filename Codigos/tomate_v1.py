import cv2

# Função para analisar a cor média das imagens de tomate
def analisar_cor_tomate(imagem):
    # Convertendo a imagem de BGR para HSV
    hsv = cv2.cvtColor(imagem, cv2.COLOR_BGR2HSV)

    # Definindo o intervalo de cores para tomates não maduros (laranja, amarelo, verde)
    lower_nonripe = (10, 100, 100)
    upper_nonripe = (40, 255, 255)

    # Definindo o intervalo de cores para tomates maduros (vermelho)
    lower_ripe = (0, 100, 100)
    upper_ripe = (10, 255, 255)

    # Criando máscaras para os pixels que estão na faixa de cores dos tomates não maduros e maduros
    mascara_nonripe = cv2.inRange(hsv, lower_nonripe, upper_nonripe)
    mascara_ripe = cv2.inRange(hsv, lower_ripe, upper_ripe)

    # Calculando a área dos pixels na máscara para cada categoria
    area_nonripe = cv2.countNonZero(mascara_nonripe)
    area_ripe = cv2.countNonZero(mascara_ripe)
    total_area = area_nonripe + area_ripe

    # Verificando a porcentagem de pixels maduros em relação ao total
    porcentagem_vermelho = (area_ripe / total_area) * 100

    # Classificando o tomate com base na porcentagem de pixels vermelhos
    if porcentagem_vermelho >= 90:
        return "Maduro"
    else:
        return "Não Maduro"

# Carregando a imagem do tomate
imagem_tomate = cv2.imread("maduro1.jpeg")

# Analisando a cor do tomate
resultado = analisar_cor_tomate(imagem_tomate)

# Exibindo o resultado
print("O tomate está:", resultado)


