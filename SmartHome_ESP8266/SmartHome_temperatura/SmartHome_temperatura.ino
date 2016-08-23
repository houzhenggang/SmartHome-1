/*
  SmartHome temperatura V 1.0

  Comandi da inviare al topic "Temperatura_Topic":
  man           -> Imposta il termostato in "manuale"
  auto          -> Imposta il termostato in "automatico"
  t=XX o T=XX   -> Imposta il termostato alla temperatura XX
  1on           -> comando ON 1
  1off          -> comando OFF 1
  2on           -> comando ON 2
  2off          -> comando OFF 2
  stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
  reset         -> pulisce la memoria EEPROM e resetta l'ESC
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TPush.h>
#include <EEPROM.h>
#include "DHT.h"
#include "SSD1306.h"

// MQTT server
const char* ssid          = "wifi_ssid";      // WIFI SSID
const char* password      = "wifi_password";  // WIFI password
const char* mqtt_server   = "mqtt_server";    // MQTT server
const char* mqtt_user     = "mqtt_user";      // MQTT user
const char* mqtt_password = "mqtt_password";  // MQTT password
const int   mqtt_port     = mqtt_port;        // MQTT port

// DEBUG
#define DEBUG                                     // Commentare questa riga per disabilitare il SERIAL DEBUG

// HARDWARE
//#define ESP01                                     // Commentare l'hardware non corrente
//#define NODEMCU                                   // Commentare l'hardware non corrente
#define ELECTRODRAGON                             // Commentare l'hardware non corrente
#define TIPO_NODO         "TEM"                   // "TAP"->tapparella "TEM"->temperatura "INT"->interruttore
#define DHTTYPE DHT22                             // DHT 22  (AM2302)

// MQTT topic
#define ACK_Topic         "ack"
#define Temperatura_Topic  "temperatura_prototipo"

// TEMPI
#define MAX_RET_WIFI                20            // Indica per quante volte ritenta di connettersi al WIFI
#define MAX_RET_MQTT                3             // Indica per quante volte ritenta di connettersi al server MQTT
#define TEMPO_REFRESH_CONNESSIONE   60000         // Indica il timeout di refresh della connessione (60000=1 min.)
#define TEMPO_RELE                  200           // Indica il tempo tra una commutazione RELE e la successiva
#define TEMPO_TERMOSTATO            5000          // Indica il tempo 

// GPIO
#if defined(ESP01)
#define Flag_inversione_RELE        1             // Inversione del segnale di uscita RELE       (0=normale - 1=invertito)  
#define Flag_inversione_Status_LED  1             // Inversione del segnale di uscita Status_LED (0=normale - 1=invertito)
#define Status_LED BUILTIN_LED    //BUILTIN_LED -> GPIO16 (nodemcu) , GPIO1 (ESP01 TX)
#define RELE1_temperatura 14      // GPIO14
#define RELE2_temperatura 12      // GPIO12
#define DHTPIN 2                  // GPIO2
#endif
#if defined(NODEMCU)
#define Flag_inversione_RELE        1             // Inversione del segnale di uscita RELE       (0=normale - 1=invertito)  
#define Flag_inversione_Status_LED  1             // Inversione del segnale di uscita Status_LED (0=normale - 1=invertito)
#define Status_LED BUILTIN_LED    //BUILTIN_LED -> GPIO16 (nodemcu) , GPIO1 (ESP01 TX)
#define RELE1_temperatura 14      // GPIO14
#define RELE2_temperatura 12      // GPIO12
#define DHTPIN 2                  // GPIO2
#endif
#if defined(ELECTRODRAGON)       // OK
#define Flag_inversione_RELE        0             // Inversione del segnale di uscita RELE       (0=normale - 1=invertito)  
#define Flag_inversione_Status_LED  1             // Inversione del segnale di uscita Status_LED (0=normale - 1=invertito)
#define Status_LED                  BUILTIN_LED   //BUILTIN_LED -> GPIO16 (nodemcu) , GPIO1 (ESP01 TX)
#define RELE1_temperatura           13            // GPIO13
#define RELE2_temperatura           12            // GPIO12
#define DHTPIN                      14            // GPIO14
#endif

// VARIABILI
uint8_t     mac[6];
float       t;                              // Temperature Celsius
float       h;                              // Humidity
float       f;                              // Temperature Fahrenheit
float       hi;                             // Heat index in Celsius (isFahreheit = false)
float       soglia = 0.25;                  // +/- 0.25
float       termostato = 18.00;
boolean     AUTO = false;







// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D2;
const int SDC_PIN = D1;
//const int SDA_PIN = 2;
//const int SDC_PIN = 1;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 15);
TPush TIMER1, TIMER2;
SSD1306 display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);

void setup() {
#if defined(DEBUG)
  Serial.begin(115200);
#endif
  Serial.println();
  Serial.println(" ***** SmartHome temperatura *****");
  Serial.println("Status_LED      = GPIO" + String(Status_LED));
  Serial.println("RELE1           = GPIO" + String(RELE1_temperatura));
  Serial.println("RELE2           = GPIO" + String(RELE2_temperatura));
  Serial.println("DTH22           = GPIO" + String(DHTPIN));
  Serial.println("SSD1306 ADDRESS = " + String(I2C_DISPLAY_ADDRESS));

  // Inizializza EEPROM
  EEPROM.begin(512);
  delay(10);

  // Initialize the Status_LED
  pinMode(Status_LED, OUTPUT);
  digitalWrite(Status_LED, 1 ^ Flag_inversione_Status_LED);

  // Initialize GPIO
  pinMode(RELE1_temperatura, OUTPUT);
  digitalWrite(RELE1_temperatura, 0 ^ Flag_inversione_RELE);
  pinMode(RELE2_temperatura, OUTPUT);
  digitalWrite(RELE2_temperatura, 0 ^ Flag_inversione_RELE);

  // Initialize the display
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Smart home");
  display.drawString(0, 25, Temperatura_Topic);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "Firmware v1.0");
  display.display();
  delay(2500);

  // Inizializza connessioni
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  setup_wifi();
  reconnect();

  dht.begin();

  EEPROM_read();

}

void setup_wifi() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "SETUP WIFI");
  display.drawString(0, 25, "Connecting to:");
  display.drawString(0, 45, ssid);
  display.display();

  int i = 0;
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print(" ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while ((WiFi.status() != WL_CONNECTED) and i < MAX_RET_WIFI) {
    ++i;
    Serial.print(".");
    digitalWrite(Status_LED, 0 ^ Flag_inversione_Status_LED);
    delay(250);
    digitalWrite(Status_LED, 1 ^ Flag_inversione_Status_LED);
    delay(250);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("  NOT connected!");
  }
  else {
    Serial.println("  connected!");
  }
  Serial.println();
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  //Serial.print("SoftAP IP: ");
  //Serial.println(WiFi.softAPIP());
  Serial.print("MAC address : ");
  WiFi.macAddress(mac);
  Serial.println(macToStr(mac));
  digitalWrite(Status_LED, 1 ^ Flag_inversione_Status_LED);
}


void reconnect() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "SETUP MQTT");
  display.drawString(0, 25, "ESP8266Client-");
  display.drawString(0, 45, macToStr(mac));
  display.display();

  int i = 0;
  String clientName = "ESP8266Client-";
  clientName += macToStr(mac);
  while (!client.connected() and i < MAX_RET_MQTT) {
    ++i;
    Serial.print("MQTT Client: ");
    Serial.println(clientName);
    Serial.print("Attempting MQTT connection... ");
    if (client.connect(string2char(clientName), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      String payload = macToStr(mac);
      payload += " start at ";
      payload += getTime();
      client.publish(ACK_Topic, (char*) payload.c_str());  // Once connected, publish an announcement...
      delay(50);
      Send_ACK();
      client.subscribe(ACK_Topic); // ... and resubscribe
      client.subscribe(Temperatura_Topic); // ... and resubscribe
    }
    else {
      Serial.print("failed, rc = ");
      Serial.println(client.state());
    }
  }
}

void Send_ACK() {
  String payload = macToStr(mac);
  payload += " alive! ";
  payload += TIPO_NODO;
  payload += " ";
  payload += Temperatura_Topic;
  client.publish(ACK_Topic, (char*) payload.c_str());     // Pubblica su ACK_Topic -> MAC + " alive! " TIPO_NODO + " " + Temperatura_Topic
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
  digitalWrite(Status_LED, 0 ^ Flag_inversione_Status_LED);
  String payload;
  if (String(topic) == Temperatura_Topic) {                                                                             // se arriva il comando sul topic "temperatura"
    if ((char)message[0] == 'r' & (char)message[1] == 'e' & (char)message[2] == 'a' & (char)message[3] == 'd' ) {       // Topic "temperatura" = "read"
      delay(50);
      sendTemperature();                                                                                                // sendTemperature()
      delay(50);
    }
    if ((char)message[0] == '1' & (char)message[1] == 'o' & (char)message[2] == 'n' ) {                                 // Topic "temperatura" = "1on"
      digitalWrite(RELE1_temperatura, 1 ^ Flag_inversione_RELE);                                                             // Accendo il RELE1
      delay(TEMPO_RELE);                                                                                                // Aspetto
    }
    if ((char)message[0] == '1' & (char)message[1] == 'o' & (char)message[2] == 'f' & (char)message[3] == 'f' ) {       // Topic "temperatura" = "1off"
      digitalWrite(RELE1_temperatura, 0 ^ Flag_inversione_RELE);                                                             // Spengo il RELE1
      delay(TEMPO_RELE);                                                                                                // Aspetto
    }
    if ((char)message[0] == '2' & (char)message[1] == 'o' & (char)message[2] == 'n' ) {                                 // Topic "temperatura" = "2on"
      digitalWrite(RELE2_temperatura, 1 ^ Flag_inversione_RELE);                                                             // Accendo il RELE2
      delay(TEMPO_RELE);                                                                                                // Aspetto
    }
    if ((char)message[0] == '2' & (char)message[1] == 'o' & (char)message[2] == 'f' & (char)message[3] == 'f' ) {       // Topic "temperatura" = "2off"
      digitalWrite(RELE2_temperatura, 0 ^ Flag_inversione_RELE);                                                             // Spengo il RELE2
      delay(TEMPO_RELE);                                                                                                // Aspetto
    }
    if ((char)message[0] == 'T' & (char)message[1] == '=' ) {                                                           // Topic "temperatura" = "T="
      String stringtmp = "";
      for (int i = 2; i < length; i++)  {
        stringtmp = stringtmp + (char)message[i];
      }
      stringtmp = stringtmp + ".0";
      char buf[stringtmp.length()];
      stringtmp.toCharArray(buf, stringtmp.length());
      termostato = atof(buf);
      EEPROM_write();
    }
    if ((char)message[0] == 's' & (char)message[1] == 't' & (char)message[2] == 'a' & (char)message[3] == 't' & (char)message[4] == 'o' ) {       // Topic "temperatura" = "stato"
      payload = macToStr(mac);
      payload += " T=";
      char tmp[4];              // string buffer
      String stringtmp = "";      //data on buff is copied to this string
      dtostrf(termostato, 4, 1, tmp);
      for (int i = 0; i < sizeof(tmp); i++)  {
        stringtmp += tmp[i];
      }
      payload += stringtmp;
      if (AUTO == true) payload += " AUTO";
      else payload += " MAN";
      payload += " RELE1=";
      if (digitalRead(RELE1_temperatura) == true ^ Flag_inversione_RELE) payload += "ON";
      if (digitalRead(RELE1_temperatura) == false ^ Flag_inversione_RELE) payload += "OFF";
      payload += " RELE2=";
      if (digitalRead(RELE2_temperatura) == true ^ Flag_inversione_RELE) payload += "ON";
      if (digitalRead(RELE2_temperatura) == false ^ Flag_inversione_RELE) payload += "OFF";
      client.publish(ACK_Topic, (char*) payload.c_str());
      delay(100);
    }
    if ((char)message[0] == 'a' & (char)message[1] == 'u' & (char)message[2] == 't' & (char)message[3] == 'o' ) {       // Topic "temperatura" = "auto"
      AUTO = true;
      EEPROM_write();
    }
    if ((char)message[0] == 'm' & (char)message[1] == 'a' & (char)message[2] == 'n') {                                  // Topic "temperatura" = "man"
      AUTO = false;
      EEPROM_write();
    }
    if ((char)message[0] == 'r' & (char)message[1] == 'e' & (char)message[2] == 's' & (char)message[3] == 'e' & (char)message[4] == 't' ) {  // Topic "temperatura" = "reset"
      EEPROM_clear_reset();
    }
  }

  if (String(topic) == ACK_Topic) {                                                                                   // se arriva il comando sul topic "ACK"
    if ((char)message[0] == 'a' & (char)message[1] == 'c' & (char)message[2] == 'k' ) {                               // Topic "temperatura" = "ack"
      Send_ACK();
      delay(100);
    }
  }
  digitalWrite(Status_LED, 1 ^ Flag_inversione_Status_LED);
}

void sendTemperature() {
  h = dht.readHumidity();
  delay(50);
  t = dht.readTemperature(false);                     // Read temperature as Celsius (the default)
  delay(50);
  f = dht.readTemperature(true);                      // Read temperature as Fahrenheit (isFahrenheit = true)

  if (isnan(h) || isnan(t) || isnan(f)) {                   // Check if any reads failed and exit early (to try again).
    String payload = macToStr(mac);
    payload += " Failed to read from DHT sensor!";
    Serial.println("Failed to read from DHT sensor!");
    client.publish(ACK_Topic, (char*) payload.c_str());
    return;
  }
  hi = dht.computeHeatIndex(t, h, false);             // Compute heat index in Celsius (isFahreheit = false) float hi
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" % ");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *C");
  String payload = macToStr(mac);
  payload += " T:";
  payload += t;
  payload += "°C H:";
  payload += h;
  payload += "% HI:";
  payload += hi;
  payload += "°C";
  client.publish(ACK_Topic, (char*) payload.c_str());
}

void loop() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  if (digitalRead(RELE1_temperatura) == true ^ Flag_inversione_RELE) {
    display.drawString(128, 0, "ON");
  }
  if (digitalRead(RELE1_temperatura) == false ^ Flag_inversione_RELE) {
    display.drawString(128, 0, "OFF");
  }
  if (client.connected()) {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, String(termostato));

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    if (AUTO) {
      display.drawString(68, 0, "AUTO");
    }
    else {
      display.drawString(68, 0,  "MAN");
    }
  }
  else {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 5, "No connection!");
  }
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 25, "Temp:");
  display.drawString(0, 45, String(t) + " °C");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 25, "Umidità:");
  display.drawString(128, 45, String(h) + " %");
  display.display();

  if (TIMER1.Wait(TEMPO_REFRESH_CONNESSIONE)) {
    Serial.println("Timer -> check connections");
    if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    }
    if (!client.connected()) {
      reconnect();
    }
  }

  if (TIMER2.Wait(TEMPO_TERMOSTATO)) {
    h = dht.readHumidity();
    delay(50);
    t = dht.readTemperature(false);                                                   // Read temperature as Celsius (the default)
    if (isnan(t)) {                                                                         // Check if any reads failed and exit early (to try again).
      String payload = macToStr(mac);
      payload += " Failed to read from DHT sensor!";
      Serial.println("Failed to read from DHT sensor!");
      client.publish(ACK_Topic, (char*) payload.c_str());
      return;
    }
    if (AUTO) {
      if (t < (termostato - soglia)) {
        digitalWrite(RELE1_temperatura, 1 ^ Flag_inversione_RELE);                                                             // Accendo il RELE1
        delay(TEMPO_RELE);                                                                                                // Aspetto
      }
      if (t > (termostato + soglia)) {
        digitalWrite(RELE1_temperatura, 0 ^ Flag_inversione_RELE);                                                             // Spengo il RELE1
        delay(TEMPO_RELE);                                                                                                // Aspetto
      }
    }
  }
  client.loop();
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < char(16)) result += "0";
    result += String(mac[i], 16);
    //    if (i < 5)
    //      result += ':';
  }
  return result;
}

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

String getTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }
  client.print("HEAD / HTTP/1.1\r\n\r\n");
  while (!!!client.available()) {
    yield();
  }
  while (client.available()) {
    if (client.read() == '\n') {
      if (client.read() == 'D') {
        if (client.read() == 'a') {
          if (client.read() == 't') {
            if (client.read() == 'e') {
              if (client.read() == ':') {
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}

void EEPROM_clear_reset() {
  EEPROM.begin(512);
  // write a 0 to all 512 bytes of the EEPROM
  Serial.print("EEPROM clear:");
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("EEPROM CLEARED!");
  EEPROM.end();
  ESP.restart();
}

void EEPROM_read() {
  Serial.print("Reading EEPROM termostato: ");
  String stringtmp = "";
  float termostatotmp;
  for (int i = 0; i < 5; ++i)
  {
    stringtmp += char(EEPROM.read(i));
  }
  char buf[stringtmp.length()];
  stringtmp.toCharArray(buf, stringtmp.length());
  termostatotmp = atof(buf);
  Serial.println(termostatotmp);
  Serial.print("Reading EEPROM MAN/AUTO: ");
  stringtmp = "";
  for (int i = 5; i < 9; ++i)
  {
    stringtmp += char(EEPROM.read(i));
  }
  Serial.println(stringtmp);
  if (stringtmp == "AUTO") {
    AUTO = true;
  }
  else {
    AUTO = false ;
  }
  if (termostatotmp > 5)  {
    termostato = termostatotmp;
  }
}

void EEPROM_write() {
  char tmp[4];                                                        // string buffer
  String stringtmp = "";                                              //data on buff is copied to this string
  dtostrf(termostato, 4, 1, tmp);
  for (int i = 0; i < sizeof(tmp); i++)  {
    stringtmp += tmp[i];
  }
  Serial.println("writing EEPROM termostato:");
  for (int i = 0; i < 5; ++i)
  {
    EEPROM.write(i, stringtmp[i]);
    Serial.print("Wrote: ");
    Serial.println(stringtmp[i]);
  }
  Serial.println("writing EEPROM MAN/AUTO:");
  if (AUTO == true) stringtmp = "AUTO";
  else stringtmp = "MAN ";
  for (int i = 0; i < 4; ++i)
  {
    EEPROM.write(i + 5, stringtmp[i]);
    Serial.print("Wrote: ");
    Serial.println(stringtmp[i]);
  }
  EEPROM.commit();
}
