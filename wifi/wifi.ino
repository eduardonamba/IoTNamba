#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

//Programa : Wifi Shield CC3000 Keyes
//Biblioteca : Adafruit_CC3000
//Alteracoes e adaptacoes : FILIPEFLOP
      
#include <Adafruit_CC3000.h>
//#include <ccspi.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
  
//Pinos
#define ADAFRUIT_CC3000_IRQ   2  //Pino de interrupcao
#define ADAFRUIT_CC3000_VBAT  5 
#define ADAFRUIT_CC3000_CS    10 //Preferencialmente pino 10 do Arduino Uno
  
//O clock pode ser alterado
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, 
                                         ADAFRUIT_CC3000_VBAT,SPI_CLOCK_DIVIDER); 
  
#define WLAN_SSID       "nGuest"   //Nome da rede - Ate 32 caractereres
#define WLAN_PASS       "nambalindo007"     //Senha da rede
//Tipo de seguranca: WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2  //Tipo de seguranca
  
#define LISTEN_PORT           80     
#define MAX_ACTION            10     
#define MAX_PATH              64     
#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20 
#define TIMEOUT_MS            500   
  
Adafruit_CC3000_Server httpServer(LISTEN_PORT);
  
uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];
  
void setup(void)
{
//  pinMode(A2, INPUT); //Define o pino para o potenciometro
  Serial.begin(9600);
  
  Serial.println("\nInicializando...: ");
  if (!cc3000.begin())
  {
    Serial.println("Shield nao encontrado. Verifique as conexoes !");
    while(1);
  }
    
  Serial.print(F("\nTentando conectar-se a rede ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) 
  {
    Serial.println(F("Falha !"));
    while(1);
  }
  Serial.println(F("Conectado!"));
    
  Serial.println(F("Requisitando endereco DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); 
  }  
  //Exibe as informacoes da conexao
  while (! displayConnectionDetails()) 
  {
    delay(1000);
  }
  //Aguarda conexao do browser
  httpServer.begin();
    
  Serial.println(F("Aguardando conexao..."));
}
  
void loop(void)
{
  //Armazena o valor lido do potenciometro
  int valor_potenciometro = 1; //analogRead(A2);
  
  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client) 
  {
    Serial.println(F("Client connected."));
    bufindex = 0;
    memset(&buffer, 0, sizeof(buffer));
    memset(&action, 0, sizeof(action));
    memset(&path,   0, sizeof(path));
    unsigned long endtime = millis() + TIMEOUT_MS;
      
    bool parsed = false;
    while (!parsed && (millis() < endtime) && (bufindex < BUFFER_SIZE)) 
    {
      if (client.available()) {
      buffer[bufindex++] = client.read();
    }
      parsed = parseRequest(buffer, bufindex, action, path);
   }
     
   if (parsed) 
   {
     Serial.println(F("Processing request"));
     Serial.print(F("Action: ")); Serial.println(action);
     Serial.print(F("Path: ")); Serial.println(path);
      if (strcmp(action, "GET") == 0) {
     client.fastrprintln(F("HTTP/1.1 200 OK"));
     client.fastrprintln(F("Content-Type: text/plain"));
     client.fastrprintln(F("Connection: close"));
     client.fastrprintln(F("Refresh: 0"));
     client.fastrprintln(F("Server: Adafruit CC3000"));
     client.fastrprintln(F(""));
     //Inicio do envio de dados para o browser
     client.fastrprintln(F("-----------------------------"));
     client.fastrprintln(F("         FILIPEFLOP          "));
     client.fastrprintln(F("-----------------------------"));
     client.fastrprintln(F(" "));
     client.fastrprint(F("Valor do potenciometro : "));
     client.println(valor_potenciometro);
     //client.fastrprintln((valor_potenciometro));
     client.fastrprintln(F(""));
   }
   else
   {
     client.fastrprintln(F("HTTP/1.1 405 Method Not Allowed"));
     client.fastrprintln(F(""));
   }
 }
  
    //Delay antes de desconectar
    delay(1000);
    Serial.println(F("Client disconnected"));
    client.close();
  }
  delay(500);
}
  
bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) {
  //Check if the request ends with rn to signal end of first line.
  if (bufSize < 2)
    return false;
  if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') {
    parseFirstLine((char*)buf, action, path);
    return true;
  }
  return false;
}
  
void parseFirstLine(char* line, char* action, char* path) {
  //Parse first word up to whitespace as action.
  char* lineaction = strtok(line, " ");
  if (lineaction != NULL)
    strncpy(action, lineaction, MAX_ACTION);
  //Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, " ");
  if (linepath != NULL)
    strncpy(path, linepath, MAX_PATH);
}
  
//Leitura do endereco IP e outras informacoes de conexao
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
    
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Nao foi possivel ler o endereco IP!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

