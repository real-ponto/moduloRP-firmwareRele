#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>

// Configuracao WIFI
const char* ssid = "realponto";
const char* password = "real#%ponto#rp177";

// Congigurar dhcp para não entrar no lugar do relogio.
IPAddress ip(177,177,0,152); 
IPAddress gateway(177,177,0,100); 
IPAddress subnet(255,255,255,0); 

// Porta do servidor para a requisicao de reset relogio
WiFiServer server(4000);

// Configuracao Reles
const int releRelogio = 12;  
const int releModulo = 5;

//Configuracao Led
const int ledAzul = 13;  // acionamento invertido, low liga high desliga

IPAddress google(8,8,8,8);

const int timeWithoutConnection = 30;
int counterTimeWithoutConnection = 0;

int counter = 0;

// times
unsigned long timeNow = 0;
unsigned long timeLast = 0;
unsigned long timeConnected = 0;
unsigned long timeDisconected = 0;

void setup() {
Serial.begin(115200);

  Serial.println("realponto");

  pinMode(releRelogio, OUTPUT);
  pinMode(releModulo, OUTPUT);

  pinMode(ledAzul, OUTPUT);

  digitalWrite(ledAzul, LOW);
  delay(200);

  digitalWrite(releModulo, HIGH);
  digitalWrite(releRelogio, HIGH);

  digitalWrite(ledAzul, HIGH);

  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); 

  
  Serial.println("try connect to wifi");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledAzul, LOW);
    delay(100);    
    digitalWrite(ledAzul, HIGH);
    delay(100);
    
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("wifi connected");

  digitalWrite(ledAzul, LOW); 

  server.begin(); 
  
  Serial.println("server inicialized");

}

void checkConnection() {

 Serial.println("verify connection");

 bool online = Ping.ping(google);
 
 if (online){
   Serial.println("online");
   counterTimeWithoutConnection = 0;
   digitalWrite(ledAzul, LOW);
   return;
 }
 
 else {
   
   Serial.println("offline");

   if (counterTimeWithoutConnection > timeWithoutConnection) {
     reseting:
     resetModulo();
     counterTimeWithoutConnection = 0;
     
     bool reconnect = false;
     int couterTryReconect = 0;

     while (!reconnect) {
       
       Serial.println("try reconnect");

       reconnect = Ping.ping(google);
     
       delay (100);

       couterTryReconect = couterTryReconect + 1;
       if (couterTryReconect >= 9000) {
         goto reseting;
       }
     }
     
     Serial.println("reconnected");
     checkConnection();
   }

   counterTimeWithoutConnection = counterTimeWithoutConnection + 1;
   digitalWrite(ledAzul, HIGH);
   delay(100);
   digitalWrite(ledAzul, LOW);
   delay(200);
   digitalWrite(ledAzul, HIGH);
   checkConnection();
 }
}

// analisa a requisicao e executa a tarefa
void checkClientConected () {

  Serial.print(".");
  delay (10);
  WiFiClient client = server.available();

  delay (20);

  // se não tem cliente destrava o codigo
  if (!client) { 
    return; 
   }
  
  Serial.println("");
  Serial.println("client connected");
  String request = client.readStringUntil('\r'); 
  client.flush(); 
  
  if (request.indexOf("/resetRelogio") != -1)  { 
    
    Serial.println("reseting Relogio");
    client.println("OK");
    delay(5);
    client.stop();
    delay(2);
    resetRelogio();

  } else {
    client.stop();
  }
}

// Desliga a energia do relógio por 7 segundos. 
void resetRelogio () {
  timeNow = millis()/60000; 
  String times = String(timeNow, DEC);
  Serial.println(times);
  delay(50);
  Serial.println("aqui");
  digitalWrite(releRelogio, LOW);
  delay(7000);
  digitalWrite(releRelogio, HIGH);
}

void resetModulo () {

  digitalWrite(releModulo, LOW);
  delay(7000);
  digitalWrite(releModulo, HIGH);
}

void loop() {
  

  counter = counter + 1;
  if (counter > 600) {
     checkConnection();
     counter = 0;
  }

  checkClientConected();
  delay(500); 
}



