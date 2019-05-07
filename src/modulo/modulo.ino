/* Este Firmware foi desenvolvido por Joannis Basile e Guilherme Stain.
Todos os direitos estão em posse da real ponto. 
qualquer duvida entrar em contato 11 4332-4040 */

#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>

// Configuracao WIFI
const char* ssid      = "realpontomodrp330";
const char* password  = "real#%ponto#rp177";

// Congigurar dhcp para não entrar no lugar do relogio.
IPAddress ip(177,177,0,152); 
IPAddress gateway(177,177,0,100); 
IPAddress subnet(255,255,255,0); 

// Porta do servidor para a requisicao de reset relogio
WiFiServer server(4560);

IPAddress google(8,8,8,8);

// Configuracao Reles
const int releRelogio = 12;  
const int releModulo  = 5;

//Configuracao Led
const int ledAzul = 13;  // acionamento invertido, low liga high desliga

// variaveis logicas
const int timeWithoutConnection   = 650;
int counterTimeWithoutConnection  = 0;
int counterWithoutConnection = 0;

int counter = 0;

int counterLessConnection = 0;

bool beforeStateOnline  = true;
bool resetRelogioFlag   = false;
bool shouldResetRelogio = false;

bool autoRebootFlag = false;
bool shouldAutoReboot = false;

// variaveis de tempo
unsigned long timeLast          = 0;
unsigned long timeConnected     = 0;   //total

unsigned long timeWorked = 0;
unsigned long timeWorkedAux = 0;

int ping_ms = 0;

int lossConnection = 0;

void setup() {

  Serial.begin(115200);

  Serial.println("\n\ninicializate");

  pinMode(releRelogio, OUTPUT);
  pinMode(releModulo, OUTPUT);

  pinMode(ledAzul, OUTPUT);

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
  conterTimeWorked();
  timeLast = timeWorked; 
  checkConnection();
}

void checkConnection() {
  Serial.println("verify connection");
  
  bool online = Ping.ping(google,1);
  ping_ms = Ping.averageTime();

  while(!online) {
    if (beforeStateOnline){
      conterTimeWorked();
      timeConnected += timeWorked - timeLast;
    }

    conterTimeWorked();
    timeLast = timeWorked;
    
    beforeStateOnline = false;
   
    Serial.println("offline");
    delay(700);

    int timeCalc = timeWithoutConnection + (counterWithoutConnection * 100);

    if (timeCalc > 1500) {
      timeCalc = 1500;
    }

    Serial.println(String((timeCalc), DEC));
    if (counterTimeWithoutConnection > timeCalc) {

      Serial.println("Reseting");

      resetModulo();
      counterWithoutConnection = counterWithoutConnection  + 1;
      
      Serial.print("counterWithoutConnection ");
      Serial.println(String((counterWithoutConnection), DEC));
      counterTimeWithoutConnection = 0;
      
      bool reconnect = false;
      
      int couterTryReconect = 0;
    
    }
    counterTimeWithoutConnection = counterTimeWithoutConnection + 1;

    digitalWrite(ledAzul, HIGH);
    delay(100);
    digitalWrite(ledAzul, LOW);
    delay(200);
    digitalWrite(ledAzul, HIGH);

    online = Ping.ping(google,1);
    ping_ms = Ping.averageTime();
  }

  if (beforeStateOnline){
    conterTimeWorked();
    timeConnected += timeWorked - timeLast;
  }

  conterTimeWorked();
  timeLast = timeWorked;

  beforeStateOnline = true;

  
  counterWithoutConnection = 0;
  Serial.println("online");

  counterTimeWithoutConnection = 0;
  digitalWrite(ledAzul, LOW);

  return;
}

// analisa a requisicao e executa a tarefa
void checkClientConected (int counter) {

  if (counter % 10 == 0) {
    Serial.println();
  }
  
  Serial.print(".");

  WiFiClient client = server.available();

  // se não tem cliente destrava o codigo
  if (!client) { 
    return; 
  }
  
  Serial.println("");
  Serial.println("client connected");

  String request = client.readStringUntil('\r'); 
 
  Serial.println(request);

  if (request.indexOf("/resetRelogio") != -1)  { 

    Serial.println("reseting Relogio");

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: application/json");
    client.println("Connection: close"); 
    
    client.println();
    
    String response = "{\n\"Response\": \"relogioReseted\" }";
    
    client.println(response);
    client.flush();
    delay(0);

    delay(1000);
    
    WiFiClient client = server.available();
    
    resetRelogioFlag = true;

  } else if (request.indexOf("/getStatus") != -1)  { 

    if (beforeStateOnline){
      conterTimeWorked();
      timeConnected += timeWorked - timeLast;
    }
    
    
    timeLast = millis()/60000;

    beforeStateOnline = true;

    conterTimeWorked();
    Serial.print("workTime: ");
    Serial.print(String(timeWorked, DEC));
    Serial.println(" minutes.");

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: application/json");
    client.println("Connection: close"); 
    
    client.println();
    
    String response = "";

    delay(0);
    
    conterTimeWorked();
    client.print(String(timeWorked, DEC));
    client.print(";");

    client.print(String(timeConnected, DEC));
    client.print(";");
    
    client.print(String(lossConnection, DEC));
    client.print(";");
    
    client.print(String(ping_ms, DEC));
    client.print(";");

    client.print("v1.02");
    client.print(";");

    client.println("");
  
    
    delay(1); //INTERVALO DE 1 MILISSEGUNDO
  }else if (request.indexOf("/masterReboot") != -1)  { 
    conterTimeWorked();
    Serial.print("workTime: ");
    Serial.print(String(timeWorked, DEC));
    Serial.println(" minutes.");

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: application/json");
    client.println("Connection: close"); 
    
    client.println();

    delay(0);
  
    String response = "{\n\"Response\": \"reboot\" }";
    
    client.println(response);
    client.flush();
  
    delay(1); //INTERVALO DE 1 MILISSEGUNDO

    autoRebootFlag = true;
  } 
}
void autoReboot() {
  ESP.restart();
}
// Desliga a energia do relógio por 15 segundos. 
void resetRelogio () {
  
  Serial.println("f: reseting Relogio");

  digitalWrite(releRelogio, LOW);
  delay(15000);
  digitalWrite(releRelogio, HIGH);
}

void resetModulo () {
  lossConnection += 1;
  Serial.println("f: reseting Modulo");

  digitalWrite(releModulo, LOW);
  delay(15000);
  digitalWrite(releModulo, HIGH);
}

void conterTimeWorked() {
  if ((millis()/60000) >= timeWorkedAux) {
    timeWorked += ((millis()/60000) - timeWorkedAux);
    timeWorkedAux = (millis()/60000);
  }else {
    timeWorkedAux = (millis()/60000);
    timeWorked ++;
  }
}


void loop() {

  if (shouldResetRelogio){
    resetRelogio();
    delay(0);
    resetRelogioFlag = false;
  }

  if (shouldAutoReboot){
    autoReboot();
    delay(0);
    autoRebootFlag = false;
  }

  shouldResetRelogio = resetRelogioFlag;
  shouldAutoReboot = autoRebootFlag;
  
  counter = counter + 1;
  Serial.print(counter);
  if (counter > 450) {
    conterTimeWorked();
    Serial.println(timeWorked);
    checkConnection();
    counter = 0;
  }

  delay(0);

  checkClientConected(counter);
  
  delay(1000); 
}


