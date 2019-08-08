#include <SoftwareSerial.h>
 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
#define TOPICO_SUBSCRIBE "ventoinha"     
#define TOPICO_PUBLISH   "temperatura"
    
#define ID_MQTT  "AC326"
     
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1

// WIFI
const char* SSID = "WLL-Inatel";
const char* PASSWORD = "inatelsemfio";
  
// MQTT
const char* BROKER_MQTT = "m15.cloudmqtt.com";
int BROKER_PORT = 13190;
const char* mqttUser="wkeuusha";
const char* mqttpassword="OEPpT3Jp1AG3";
 
//Variáveis e objetos globais
WiFiClient espClient; 
PubSubClient MQTT(espClient);
String ventoinha = "0";
char Buf[50];

//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);

// D9 (RX) e D10 (TX)
SoftwareSerial minhaSerial(D9 , D10); 

void setup() 
{   
    initSerial();
    initWiFi();
    initMQTT();
}
  
void initSerial() 
{
    Serial.begin(9600);
}

void initWiFi() 
{
    delay(10);
    /*Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");*/
     
    reconectWiFi();
}

void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        /*Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);*/
        if (MQTT.connect(ID_MQTT, mqttUser, mqttpassword)) 
        {
            // Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            /*Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");*/
            delay(2000);
        }
    }
}

void reconectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        // Serial.print(".");
    }
   
    /*Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());*/
}

void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT();
     
     reconectWiFi();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    String msg;
    
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    // Serial.println(msg);
 
    if (msg.equals("{\"ventoinha\":1}"))
    {
        ventoinha = '1';
        Serial.println("1");
    }
    if (msg.equals("{\"ventoinha\":0}"))
    {
        ventoinha = '0';
        Serial.println("0");
    }
}

void EnviaEstadoOutputMQTT(void)
{
    MQTT.publish(TOPICO_PUBLISH,Buf);
}

String leStringSerial(){
  String conteudo = "";
  char caractere;
  
  while(Serial.available() > 0) {
    caractere = Serial.read();
    // Serial.println(caractere);
    if (caractere != '#'){
      conteudo.concat(caractere);
    }
    delay(10);
  }

   // Serial.println(conteudo);
   return conteudo;
}

void loop()
{   
    VerificaConexoesWiFIEMQTT();
    // EnviaEstadoOutputMQTT();
    MQTT.loop();
    if (Serial.available() > 0){
      String recebido = leStringSerial();
      recebido.toCharArray(Buf,50);
      EnviaEstadoOutputMQTT();
      // Serial.println(Buf);  
    }         
}
