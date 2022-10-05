/*
   TRANSMISSOR LORA
  Module 1276 // Arduino NANO
    Vcc         ->   3.3V
    MISO        ->   D12
    MOSI        ->   D11
    SLCK        ->   D13
    Nss         ->   D10
    GND         ->   GND
*/
#include <SPI.h>
#include <LoRa.h>;
#define Qtd_Amostras 20  // Quantas Amostras para filtrar
#define Intervalo_Amostragem 1 // Definindo o intervalo de amostragem em ms

float fase1 = 0;  // Variável global que salva o dado lido da porta serial.
float fase2 = 0;// Variável global que salva o dado lido da porta serial.
float fase3 = 0; // Variável global que salva o dado lido da porta serial.
// A estratégia usada aqui é porque o comando analogRead possui um custo alto para o Arduino.
// Com isso salvamos na variável para essa leitura ser feita apenas uma vez a cada interação de loop.
unsigned long timer1 = 0; // A variável que irá contar o útimo


double NotifyFase1 = 1;
double NotifyFase2 = 1;
double NotifyFase3 = 1;


void setup() {
  Serial.begin(9600); // Inicio da comunicação serial
  Serial.println("MONITORAMENTO DE SISTEMA ENERGETICO FGA");
  if (!LoRa.begin(915E6)) { 
    Serial.println("FALHA NA CONFIGURAÇÃO DO MÓDULO LORA");
    while (1);
  } else {
    Serial.println("MÓDULO LORA CONFIGURADO COM SUCESSO");
  }
}

void loop() {

  fase1 = analogRead(A0) * (3.3 / 1024);
  fase2 = analogRead(A1) * (3.3 / 1024);
  fase3 = analogRead(A2) * (3.3 / 1024);
  Amostragem();

  //filtroMediaMovel(1);
  if (NotifyFase1 < 0.5) {
    String message = "FALHA ENERGETICA NA FASE 1";
    LoraSender(message);
    Serial.println(message);
    delay(500);

  }
  if (NotifyFase2 < 0.5) {
    String message = "FALHA ENERGETICA NA FASE 2";
    LoraSender(message);
   //Serial.println(message);
    delay(500);

  }
  if (NotifyFase3 < 0.5) {
    String message = "FALHA ENERGETICA NA FASE 3";
    LoraSender(message);
    //Serial.println(message);
    delay(500);

  }
  Serial.println("--END LOOP--");
}

void Amostragem() {
  if (millis() - timer1 > Intervalo_Amostragem) {
    filtroMediaMovel(1);
    timer1 = millis(); // atualiza para contar o tempo mais uma vez
  }
}

float filtroMediaMovel(bool atualiza_saida) {
  static  int Leituras_anteriores1[Qtd_Amostras];
  static  int Leituras_anteriores2[Qtd_Amostras];
  static  int Leituras_anteriores3[Qtd_Amostras];

  static  int Posicao = 0;
  static long Soma1 = 0;
  static long Soma2 = 0;
  static long Soma3 = 0;
  static float Media1 = 0;
  static float Media2 = 0;
  static float Media3 = 0;
  static bool zera_vetor = 1;

  if (zera_vetor) { // Zerando todo o buffer circular, para que as subtrações das sobrescrição não atrapalhe o filtro
    for (int i = 0; i <= Qtd_Amostras; i++) {
      Leituras_anteriores1[i] = 0;
      Leituras_anteriores2[i] = 0;
      Leituras_anteriores3[i] = 0;
    }
    zera_vetor = 0;
  }

  if (atualiza_saida == 0) return ((double)Media1, (double)Media2, (double)Media3);

  else {
    Soma1 = fase1 - Leituras_anteriores1[Posicao % Qtd_Amostras] + Soma1;
    Soma2 = fase2 - Leituras_anteriores2[Posicao % Qtd_Amostras] + Soma2;
    Soma3 = fase3 - Leituras_anteriores3[Posicao % Qtd_Amostras] + Soma3;
    Leituras_anteriores1[Posicao % Qtd_Amostras] = fase1;
    Leituras_anteriores2[Posicao % Qtd_Amostras] = fase2;
    Leituras_anteriores3[Posicao % Qtd_Amostras] = fase3;

    Media1 = (float)Soma1 / (float)(Qtd_Amostras);
    Media2 = (float)Soma2 / (float)(Qtd_Amostras);
    Media3 = (float)Soma3 / (float)(Qtd_Amostras);

    NotifyFase1 = Media1;
    NotifyFase2 = Media2;
    NotifyFase3 = Media3;

    Posicao = (Posicao + 1) % Qtd_Amostras;
    return ((double)Media1, (double)Media2, (double)Media3);
  }
}
void LoraSender(String message) {

  LoRa.beginPacket();
  LoRa.print(message);
  Serial.println(message);
  LoRa.endPacket();
}
