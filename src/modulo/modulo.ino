#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>

// Configuracao WIFI
const char* ssid      = "realponto";
const char* password  = "real#%ponto#rp177";

// Congigurar dhcp para não entrar no lugar do relogio.
IPAddress ip(177,177,0,152); 
IPAddress gateway(177,177,0,100); 
IPAddress subnet(255,255,255,0); 

// Porta do servidor para a requisicao de reset relogio
WiFiServer server(4000);

IPAddress google(8,8,8,8);

// Configuracao Reles
const int releRelogio = 12;  
const int releModulo  = 5;

//Configuracao Led
const int ledAzul = 13;  // acionamento invertido, low liga high desliga

// variaveis logicas
const int timeWithoutConnection   = 30;
int counterTimeWithoutConnection  = 0;

int counter = 0;

int counterLessConnection = 0;

bool beforeStateOnline = true;
bool resetRelogioFlag = false;
bool shouldResetRelogio = false;

// variaveis de tempo
unsigned long timeNow           = 0;
unsigned long timeLast          = 0;
unsigned long timeConnected     = 0;   //total
unsigned long timeDisconected   = 0; //total
unsigned long mediaConnected    = 0;
unsigned long mediaDisconnected = 0;

unsigned long amostragemConnected   = 1;
unsigned long amostragemDisconnect  = 0;


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

  timeLast = millis()/60000; 
  checkConnection();
}

void checkConnection() {

  Serial.println("verify connection");

  bool online = Ping.ping(google);
  
  if (online){
    
    timeNow = millis()/60000;

    if (beforeStateOnline){
      timeConnected += timeNow - timeLast;
    }
    
    else {
      timeDisconected += timeNow - timeLast;
      amostragemConnected += 1;
    }
    
    timeLast = millis()/60000;

    
    beforeStateOnline = true;
    
    Serial.println("online");
    counterTimeWithoutConnection = 0;
    digitalWrite(ledAzul, LOW);
    return;
  }
 
  else {
    
    timeNow = millis()/60000;
    
    if (beforeStateOnline){
      timeConnected += timeNow - timeLast;
      amostragemDisconnect += 1;
    }
    
    else {
      timeDisconected += timeNow - timeLast;
    }
    
    timeLast = millis()/60000;
    
    beforeStateOnline = false;
   
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
void checkClientConected (int counter) {

  if (counter % 50 == 0) {
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
  client.flush(); 
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
    
    resetRelogioFlag = true;

  } else if (request.indexOf("/getStatus") != -1)  { 

    timeNow = millis()/60000;

    if (beforeStateOnline){
      timeConnected += timeNow - timeLast;
    }
    
    else {
      timeDisconected += timeNow - timeLast;
      amostragemConnected += 1;
    }
    
    timeLast = millis()/60000;

    beforeStateOnline = true;

    
    Serial.print("workTime: ");
    Serial.print(String(timeNow, DEC));
    Serial.println(" minutes.");

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: application/json");
    client.println("Connection: close"); 
    
    client.println();
    
    String response = "";

    delay(0);
    
    response += "{\"Response\": ";

    response += "{\"workTime\":";
    response += String(timeNow, DEC);
    response += "},";

    response += "{\"connectedTime\":";
    response += String(timeConnected, DEC);
    response += "},";

    response += "{\"disconnectedTime\":";
    response += String(timeDisconected, DEC);
    response += "},";

    response += "{\"mediaConnection\":";
    response += String(timeConnected/amostragemConnected, DEC);
    response += "},";

    response += "{\"mediaDisconnection\":";
    response += String(timeDisconected/amostragemDisconnect, DEC);
    response += "},";

    response += "{\"timesLossConnection\":";
    response += String(amostragemDisconnect, DEC);
    response += "},";

    response += "}";
    
    client.println(response);
    client.flush();
    
    delay(1); //INTERVALO DE 1 MILISSEGUNDO
  } 
  else {
    client.println("HTTP/1.1 404 Not Found");
  }
  
}

// Desliga a energia do relógio por 7 segundos. 
void resetRelogio () {

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

  if (shouldResetRelogio){
    resetRelogio();
    resetRelogioFlag = false;
  }

  shouldResetRelogio = resetRelogioFlag;
  
  counter = counter + 1;
  if (counter > 600) {
     checkConnection();
     counter = 0;
  }

  checkClientConected(counter);
  
  delay(500); 
}





