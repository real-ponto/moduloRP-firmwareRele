#include <ESP8266WiFi.h>

int ledAzul = 13;// invertido
int ledVerde = 12;

int releRelogio = 13; //alterar pino

const char* ssid = "REALPONTO";
const char* password = "real#%ponto#rp199";

IPAddress ip(192,168,0,210); 
IPAddress gateway(192,168,0,254); 
IPAddress subnet(255,255,255,0); 
 
WiFiServer server(4000);

int status = 0;

void setup() {
  // leds
  pinMode(ledAzul, OUTPUT);
  digitalWrite(ledAzul, LOW);
  pinMode(ledVerde, OUTPUT);
  digitalWrite(ledVerde, LOW);
  
  WiFi.begin(ssid, password); 
  WiFi.config(ip, gateway, subnet); 
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
  }
  
  server.begin(); 

}

void loop() {
  digitalWrite(ledVerde, HIGH);  
  digitalWrite(ledAzul, HIGH);

  WiFiClient client = server.available(); 
 
  if (!client) { return; }
 
  while(!client.available()){ //ENQUANTO CLIENTE ESTIVER CONECTADO
    delay(1); //INTERVALO DE 1 MILISEGUNDO
  }
 
  String request = client.readStringUntil('\r'); //FAZ A LEITURA DA PRIMEIRA LINHA DA REQUISIÇÃO
  client.flush(); //AGUARDA ATÉ QUE TODOS OS DADOS DE SAÍDA SEJAM ENVIADOS AO CLIENTE
 
  if (request.indexOf("") != -1)  { 
    client.stop();
    resetRelogio();
    
  }
  
 
  delay(1); //INTERVALO DE 1 MILISSEGUNDO
  

}

void resetRelogio () {
  digitalWrite(releRelogio, LOW);
  delay(5000);
  digitalWrite(releRelogio, HIGH);
}




