// ----------------------------
// Bibliotecas Padrões
// ----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Bibliotecas adicionais - cada uma dessa deve ser instalada.
// ----------------------------

#include <ArduinoJson.h>
// Biblioteca usada para analisar Json das respostas da API
// Procure por "Arduino Json" no gerenciador de biblioteca do Arduino

//------- Substitua os campos! ------
char ssid[] = "YOUR_SSID";       // seu nome da rede (SSID)
char password[] = "YOUR_PASSWORD";  // sua senha de rede

// Para requisicoes HTTPS
WiFiClientSecure client;

// Apenas a base do URL ao qual você deseja se conectar
#define TEST_HOST "api.coingecko.com"

// OPCIONAL - A impressao digital(fingerprint) do site
#define TEST_HOST_FINGERPRINT "YOUR_FINGERPRINT_OF_API"
// A impressao digital muda a cada poucos meses.

int ledPin = LED_BUILTIN;

void setup() {

  Serial.begin(9600);

  // Conecta o WiFI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Define ledpin com saida e o deixa desligado
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); //inverso para ESP

  // Tente se conectar a rede Wifi:
  delay(500);
  Serial.print("Conectando ao Wifi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Servidor operando em:");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  //--------

  // Checa a impressao digital
  client.setFingerprint(TEST_HOST_FINGERPRINT);
}

void makeHTTPRequest() {
  
  // Abrindo conexao com o servidor
  if (!client.connect(TEST_HOST, 443))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  // executa funcoes utilitarias em segundo plano para o ESP
  yield();

  // Envia requisicao HTTP
  client.print(F("GET "));
  // Esta é a segunda metade da requisicao (tudo o que vem depois do URL base)
  client.print("/api/v3/simple/price?ids=bitcoin&vs_currencies=usd%2Cbrl");
  client.println(F(" HTTP/1.1"));

  //Cabecalhos (Headers)
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Checa status HTTP
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Pula cabecalhos(headers) HTTP
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  // Isso provavelmente nao e necessario para a maioria, mas em algumas
  // API's ha caracteres voltando antes do corpo da resposta.
  // O peek () irá olhar para o caractere, mas nao o tirara da fila
  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
  }

  // Use o ArduinoJson Assistant para calcular o tamanho do JSON e nao estourar os bufferes:
  //https://arduinojson.org/v6/assistant/
  
  DynamicJsonDocument doc(192); //Para ESP32/ESP8266 voce usara dinamico

  DeserializationError error = deserializeJson(doc, client);

  if (!error) {

    long bitcoin_usd = doc["bitcoin"]["usd"];
    long bitcoin_brl = doc["bitcoin"]["brl"];
    
    Serial.print("-------------");
    Serial.println((String)"\n\n");
     
    if (bitcoin_brl < THE VALUE YOU WANT TO CHECK) { // Preço do dia configurado para verificacao
      Serial.print("O Bitcoin abaixou para:  ");
      Serial.println((String)bitcoin_brl + ",00 reais\n");
      
      digitalWrite(ledPin, LOW); // Ascende led se valor for menor
    }

    else if (bitcoin_brl > THE VALUE YOU WANT TO CHECK){ // Preço do dia configurado para verificacao
      Serial.print("O Bitcoin aumentou para:  ");
      Serial.println((String)bitcoin_brl + ",00 reais\n");
      
      digitalWrite(ledPin, LOW); // Ascende led se valor for maior
    }

    else{
      digitalWrite(ledPin, HIGH); // Desliga led se valor for entre os dois
    }

    Serial.print("bitcoin_usd: ");
      Serial.println((String)bitcoin_usd + ",00 dolares\n");
    Serial.print("bitcoin_brl: ");
      Serial.println((String)bitcoin_brl + ",00 reais\n\n");
    
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

}

void loop() {

  makeHTTPRequest();
  
  delay(10000); //10s
  
}
