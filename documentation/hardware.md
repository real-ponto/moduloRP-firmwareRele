# instalação 
## install IDE
sudo dnf install arduino

## install lib
Installing with Boards Manager  
Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit). 
Install the current upstream Arduino IDE at the 1.8.7 level or later. The current version is at the Arduino website.  
Start Arduino and open Preferences window.  
Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.  
Open Boards Manager from Tools > board menu and install esp8266 platform (and don't forget to select your ESP8266 board from Tools > Board menu after installation).  

# Conexão

header j2 para gravação

placa | conversor
------|-----------
GND   | GND  
TX    | RX  
RX    | TX  
VCC   | 3V3  

conectar buton 0 do header j3 com o gnd

# Usual Pin

GPIO     | Funcionalidade
---------|-----------
GPIO00   | Button 1   
GPIO01   | Serial RXD and Optional sensor  
GPIO03   | Serial TXD and Optional sensor  
GPIO05   | Relay 2  
GPIO09   | Button 2
GPIO10   | Button 3 
GPIO12   | Relay 1
GPIO13   | Blue Led   
  
Pino   | Funcionalidade
-------|-----------
12     | Green Led
13     | Blue Led (inverted)
