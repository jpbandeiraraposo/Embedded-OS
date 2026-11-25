/*
Pinagem: 
   Infravermelho:	 GPIO17 (Pin 11)
   LED 1 (verde):	 GPIO5 (Pin 29 - Pin 21 no WiringPi)
   LED 2 (vermelho):	 GPIO6 (Pin 31 - Pin 22 no WiringPi)
   Camera: 		 USB (GPIO17 - Pin 11)
   Servo motor: 	 GPIO18 (Pin 12 - Pin 1 no WiringPi)
*/

#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>

#define LED_VERDE 21
#define LED_VERMELHO 22

using namespace std;
using namespace cv;

const int irSensorPin = 0; // Pino GPIO17 conectado ao sensor IR
const int servoPin = 1; // Pino GPIO18 conectado ao servo motor

// Função para inicializar o sensor IR
void inicializarSensorIR() {
    wiringPiSetup(); // Inicializar WiringPi
    pinMode(irSensorPin, INPUT); // Configurar pino do sensor IR como entrada
}

// Função para detectar a presença de um objeto usando o sensor IR
bool detectarObjeto() {
    return digitalRead(irSensorPin) == LOW; // LOW significa que o objeto está presente
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
        return "Maduro";
    } else {
        return "Não Maduro";
    }
}

// Função para acender o LED verde ou vermelho com base no resultado
void acenderLED(string resultado) {
    if (resultado == "Maduro") {
        // LED verde acende
        digitalWrite(LED_VERDE, HIGH);
        delay(3000);
        digitalWrite(LED_VERDE, LOW);
    } else {
        // LED vermelho acende
        digitalWrite(LED_VERMELHO, HIGH);
        delay(3000);
        digitalWrite(LED_VERMELHO, LOW);
    }
}

// Função para inicializar o servo motor
void inicializarServo() {
   if(wiringPiSetup() == -1) {
      cerr << "Erro ao inicializar o wiringPi." << endl;
      exit(EXIT_FAILURE);
   }
   if(softPwmCreate(servoPin, 0, 200) != 0) {
      cerr << "Erro ao inicializar o PWM suave no pino " << servoPin << "." << endl;
      exit(EXIT_FAILURE);
   }
   softPwmWrite(servoPin, 15); // Posição neutra (90°)
   delay(1000); // Esperar 1 segundo para estabilizar
}

// Função para mover o servo motor de acordo com o resultado
void moverServoMotor(string resultado) {
    if (resultado == "Maduro") {
        // Mover o servo motor para 45° (direita)
        softPwmWrite(servoPin, 20); // Aproximadamente 45°
    } else {
        // Mover o servo motor para -45° (esquerda)
        softPwmWrite(servoPin, 10); // Aproximadamente -45°
    }
    delay(3000); // Aguardar o servo motor se mover
    softPwmWrite(servoPin, 15); // Retornar para a posição neutra (0°)
    delay(1000); // Aguardar 1 segundo para estabilizar
}

int main() {
    // Inicializar WiringPi
    inicializarServo();
    inicializarSensorIR();
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);

    // Laço principal
    while (true) {
        // Detectar objeto usando o sensor IR
        bool objetoDetectado = detectarObjeto();
        
        if (objetoDetectado) {
            cout << "Objeto detectado!" << endl;
            
            // Capturar imagem da webcam
            Mat imagem_tomate = capturarImagemWebcam(640, 480);
    
            // Analisar a cor do tomate
            string resultado = analisarCorTomate(imagem_tomate);
    
            // Exibir o resultado
            cout << "O tomate está: " << resultado << endl;

            // Criar threads para realizar múltiplas tarefas simultaneamente
            thread threadLED(acenderLED, resultado);
            thread threadServo(moverServoMotor, resultado);

            // Esperar que as threads terminem
            threadLED.join();
            threadServo.join();
          
        } else {
            cout << "Nenhum objeto detectado." << endl;
        }

        delay(1000); // Espera por 1 segundo antes de verificar novamente
    }

    return 0;
}
