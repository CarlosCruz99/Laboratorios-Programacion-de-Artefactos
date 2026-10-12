#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTTYPE DHT11


// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

const char* WIFI_SSID = "Test";
const char* WIFI_PASSWORD = "sapo1234";


// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================


const char* MQTT_SERVER = "10.157.198.244";
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";


// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// (Estudiante)
// Definir los pines y objetos necesarios para los sensores.
int pinLDR=32;
int pinDHT=4;
float valorLeidoLDR=0.0;
float valorLeidoDHT=0.0;
float porcentajeLDR=0.0;
float humedad=0;
float temperatura=0;

DHT dht(pinDHT, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {
    //(Estudiante)
      //1. Obtener la temperatura.
      temperatura=dht.readTemperature();

      //2. Obtener la humedad.
      humedad=dht.readHumidity();

      //3. Obtener la lectura del sensor de iluminación.
      valorLeidoLDR=analogRead(pinLDR);
      porcentajeLDR=map(valorLeidoLDR, 0, 4095, 0, 100);

      //4. Preparar los valores para poder publicarlos mediante MQTT.
      String payloadTemperatura=String(temperatura, 0);
      String payloadHumedad=String(humedad, 0);
      String payloadLuz=String(porcentajeLDR, 0);

      //5. Publicar cada dato en su topic correspondiente:  TOPIC_TEMPERATURA, TOPIC_HUMEDAD, TOPIC_LUZ
      mqttClient.publish(TOPIC_TEMPERATURA, payloadTemperatura.c_str());
      mqttClient.publish(TOPIC_HUMEDAD, payloadHumedad.c_str());
      mqttClient.publish(TOPIC_LUZ, payloadLuz.c_str());

      //6. Mostrar las mediciones en el Monitor Serial.
      Serial.print("Valor de luz: ");
      Serial.println(porcentajeLDR);
      Serial.print("Valor de humedad: ");
      Serial.println(humedad);
      Serial.print("Valor de temperatura: ");
      Serial.println(temperatura);
      delay(1000);

}

void setup() {

  Serial.begin(115200);

  // (Estudiante)
  // Inicializar los sensores y configurar los pines
  // necesarios.
  pinMode(pinLDR, INPUT);

  dht.begin();

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();


  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}