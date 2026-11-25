/* 
   Fluxograma:
   1) identificar tomate via infravermelho 	OK
   2) tirar foto com webcam 			OK
   3) processamento da imagem 			OK
   4) LED verde ou vermelho acende		OK
   5) acionamento do servo motor		OK
   
   Pinagem: 
   Infravermelho:	 GPIO4
   LED 1 (verde):	 GPIO5
   LED 2 (vermelho):	 GPIO6
   Camera: 		 USB (GPIO12)
   Servo motor: 	 GPIO18
   
*/

#include <iostream>
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <pigpio.h> // Incluir a biblioteca pigpio para controlar o servo motor
#define LED_VERDE 5
#define LED_VERMELHO 6

using namespace std;
using namespace cv;

const int irSensorPin = 4;
const int servoPin = 18; // Pino GPIO conectado ao servo motor

pinMode(LED_VERDE, OUTPUT);
pinMode(LED_VERMELHO, OUTPUT);

// Função para inicializar o sensor IR
void inicializarSensorIR() {
    wiringPiSetup(); // Inicializar WiringPi
    pinMode(irSensorPin, INPUT); // Configurar pino do sensor IR como entrada
}

// Função para detectar a presença de um objeto usando o sensor IR
bool detectarObjeto() {
    return digitalRead(irSensorPin) == HIGH; // HIGH significa que o objeto está presente
}

// Função para capturar imagem da webcam
Mat capturarImagemWebcam(int largura, int altura) {
    VideoCapture cap(0); // Abrir a câmera padrão (0) como dispositivo de captura de vídeo
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir a câmera." << endl;
        exit(EXIT_FAILURE);
    }

    cap.set(CAP_PROP_FRAME_WIDTH, largura); // Definir a largura do frame
    cap.set(CAP_PROP_FRAME_HEIGHT, altura); // Definir a altura do frame

    Mat imagem;
    cap >> imagem; // Capturar o frame da webcam

    return imagem;
}

// Função para analisar a cor média das imagens de tomate
string analisarCorTomate(Mat imagem) {
    // Convertendo a imagem de BGR para HSV
    Mat hsv;
    cvtColor(imagem, hsv, COLOR_BGR2HSV);

    // Definindo o intervalo de cores para tomates não maduros (laranja, amarelo, verde)
    Scalar lower_nonripe = Scalar(10, 100, 100);
    Scalar upper_nonripe = Scalar(40, 255, 255);

    // Definindo o intervalo de cores para tomates maduros (vermelho)
    Scalar lower_ripe = Scalar(0, 100, 100);
    Scalar upper_ripe = Scalar(10, 255, 255);

    // Criando máscaras para os pixels que estão na faixa de cores dos tomates não maduros e maduros
    Mat mascara_nonripe, mascara_ripe;
    inRange(hsv, lower_nonripe, upper_nonripe, mascara_nonripe);
    inRange(hsv, lower_ripe, upper_ripe, mascara_ripe);

    // Calculando a área dos pixels na máscara para cada categoria
    int area_nonripe = countNonZero(mascara_nonripe);
    int area_ripe = countNonZero(mascara_ripe);
    int total_area = area_nonripe + area_ripe;

    // Verificando a porcentagem de pixels maduros em relação ao total
    double porcentagem_vermelho = (area_ripe / static_cast<double>(total_area)) * 100;

    // Classificando o tomate com base na porcentagem de pixels vermelhos
    if (porcentagem_vermelho >= 90) {
        //LED verde acende
        digitalWrite(LED_VERDE, HIGH);
        delay(3000);
        digitalWrite(LED_VERDE, LOW);
        
        // Mover o servo motor para a direita (90°)
        gpioServo(servoPin, 1500); // 1500us para 90°
        
        return "Maduro";
    } else {
        //LED vermelho acende
        digitalWrite(LED_VERMELHO, HIGH);
        delay(3000);
        digitalWrite(LED_VERMELHO, LOW);
        
        // Mover o servo motor para a esquerda (90°)
        gpioServo(servoPin, 500); // 500us para -90°
        
        return "Não Maduro";
    }
}

int main() {
    //====== Sensor Infravermelho: ======
    inicializarSensorIR();

    // Inicializar pigpio
    if (gpioInitialise() < 0) {
        cerr << "Erro ao inicializar o pigpio." << endl;
        return -1;
    }

    // Laço principal
    while (true) {
        // Detectar objeto usando o sensor IR
        bool objetoDetectado = detectarObjeto();

        if (objetoDetectado) {
            cout << "Objeto detectado!" << endl;
        } else {
            cout << "Nenhum objeto detectado." << endl;
        }

        delay(1000); // Espera por 1 segundo antes de verificar novamente

        //====== Processamento da imagem: ======
        // Capturar imagem da webcam
        Mat imagem_tomate = capturarImagemWebcam(640, 480);

        // Analisar a cor do tomate
        string resultado = analisarCorTomate(imagem_tomate);

        // Exibir o resultado
        cout << "O tomate está: " << resultado << endl;
    }

    // Liberar recursos do pigpio
    gpioTerminate();

    return 0;
}


