#include <WiFi.h>
#include <HTTPClient.h>

// ========================= DEFINIÇÕES DO HARDWARE =========================
#define LED_VERDE 2 // Pino utilizado para controle do led verda
#define LED_VERMELHO 40 // Pino utilizado para controle do led vermelho
#define LED_AMARELO 9 // Pino utilizado para controle do led azul
#define BUTTON_PIN 18  // O numero do pino do botao

// ========================= DECLARAÇÕES GLOBAIS E CONSTANTES =========================
int BUTTON_STATE = 0;  // Variavel para ler status do botao
const int LDR_PIN = 4;  // O numero do pino do sensor ldr
int THRESHOLD = 600;
const unsigned long tempoVermelho = 5000;      // 6 segundos
const unsigned long tempoAmarelo = 2000;   // 2 segundos
const unsigned long tempoVerde = 3000;    // 2 segundos
unsigned long previousMillis = 0;   // Marca o último momento de transição
int estadoSemaforo = 0;               // Armazena a fase atual do semáforo

// Gerenciamento de LEDs
class LEDManager {
private:
    int pinRed, pinGreen, pinYellow; // Pinos dos LEDs vermelho e verde e amarelo

public:
    LEDManager(int redPin, int greenPin, int yellowPin) : pinRed(redPin), pinGreen(greenPin), pinYellow(yellowPin) {
        pinMode(pinRed, OUTPUT); // Define o pino vermelho como saída
        pinMode(pinGreen, OUTPUT); // Define o pino verde como saída
        pinMode(pinYellow, OUTPUT); // Define o pino verde como saída
    }

    void setYellow() {
        digitalWrite(pinGreen, LOW); // Liga o LED verde
        digitalWrite(pinRed, LOW); // Desliga o LED vermelho
        digitalWrite(pinYellow, HIGH); // Define o pino verde como saída
    }

    void setGreen() {
        digitalWrite(pinGreen, HIGH); // Liga o LED verde
        digitalWrite(pinRed, LOW); // Desliga o LED vermelho
        digitalWrite(pinYellow, LOW); // Define o pino verde como saída
    }

    void setRed() {
        digitalWrite(pinGreen, LOW); // Liga o LED verde
        digitalWrite(pinRed, HIGH); // Desliga o LED vermelho
        digitalWrite(pinYellow, LOW); // Define o pino verde como saída
    }

    void turnOff() {
        digitalWrite(pinGreen, LOW); // Desliga o LED verde
        digitalWrite(pinRed, LOW); // Desliga o LED vermelho
        digitalWrite(pinYellow, LOW); // Define o pino verde como saída
    }
};

LEDManager leds(LED_VERMELHO, LED_VERDE, LED_AMARELO); 

void setup() {

  leds.turnOff();

  // Inicialização das entradas
  pinMode(BUTTON_PIN, INPUT); // Inicializa o pin do botao como uma entrada

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

  if(WiFi.status() == WL_CONNECTED){ // Se o ESP32 estiver conectado à Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

    if (httpResponseCode>0) {
      Serial.print("HTTP codigo de resposta: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      }
    else {
      Serial.print("Error codigo: ");
      Serial.println(httpResponseCode);
      }
      http.end();
    }
  else {
    Serial.println("WiFi Desconectado");
  }
}

void loop() {
  int ldrstatus = analogRead(LDR_PIN);
  unsigned long currentMillis = millis();

  if(ldrstatus <= THRESHOLD){
    switch (estadoSemaforo) {
      case 0:  // Luz Verde
        if (currentMillis - previousMillis >= tempoVerde) {
          previousMillis = currentMillis;
          estadoSemaforo = 1;
        }
        leds.setGreen();
        break;

      case 1:  // Luz amarela após o verde
        if (currentMillis - previousMillis >= tempoAmarelo) {
          previousMillis = currentMillis;
          estadoSemaforo = 2;
        }
        leds.setYellow();
        break;

      case 2:  // Luz verde inicial
        // Quando vermelho le o botao pra ver se deve abrir no else if e coloca um delayzinho
        BUTTON_STATE = digitalRead(BUTTON_PIN);
        if ((currentMillis - previousMillis >= tempoVermelho) ) {
          previousMillis = currentMillis;
          estadoSemaforo = 0;
        } else if ( BUTTON_STATE == HIGH) {
          previousMillis = currentMillis;
          estadoSemaforo = 0;
          delay(1000);
        }
        leds.setRed();
        break;
    }

  }else{
    Serial.print("esta escuro: ");
    Serial.println(ldrstatus);
    leds.setYellow();
    delay(1000);
    leds.turnOff();
    delay(1000);
  }

  // Verifica estado do botão
  BUTTON_STATE = digitalRead(BUTTON_PIN);
  if (BUTTON_STATE == HIGH) {
    Serial.println("Botao pressionado!");
  } else {
    Serial.println("Botao nao pressionado!");
  }
}
