#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>


IPAddress server(192, 168, 0, 56); // IP de la raspberry Pi
const char* ssid     = "SSID"; // WIFI ssid
const char* pass = "PASWORD"; // WIFI Password
const char* host = "blind1"; // nombre del entorno
int pin_subir = 16;
int pin_bajar = 12;
int pulsador_subir = 0;
int pulsador_bajar =14;
int pulsador_reset =13;
int pin_sensor_hall =2;
boolean estado_pulsador_subir=HIGH;
boolean estado_pulsador_bajar=HIGH;
boolean estado_pulsador_reset=HIGH;
//boolean estado_sensor_hall=LOW;
boolean calibracion=LOW;
boolean subiendo=LOW;
boolean bajando=LOW;
int pasos_totales=0;
int pasos=0;
float valor_paso=0;
int posicion_actual = 0;
int posicion_destino = 0;
int contador=0;
//int pasos_mapeados=0;
boolean sinprog=LOW;


WiFiClient wclient;
PubSubClient client(wclient, server);

void callback(const MQTT::Publish& pub) {
  Serial.println (pub.payload_string());
  posicion_destino = (pub.payload_string().toFloat());
  posicion_destino= map(posicion_destino,0,100,0,pasos_totales);
  Serial.print ("dato recibido - destino :");
  Serial.println(posicion_destino);
  delay(500);
  ajustar_persiana();

}


void setup()
{
  pinMode(pulsador_subir,INPUT);
  pinMode(pulsador_bajar,INPUT);
  pinMode(pulsador_reset,INPUT);
  pinMode(pin_sensor_hall,INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_sensor_hall), cuentapasos, CHANGE);
  pinMode(pin_subir, OUTPUT);
  pinMode(pin_bajar, OUTPUT);
  digitalWrite(pin_subir,HIGH);
  digitalWrite(pin_bajar,HIGH);
  delay(2000);
  estado_pulsador_reset=digitalRead(pulsador_reset);
  Serial.begin(115200);
  EEPROM.begin(512);
  
  if (estado_pulsador_reset==LOW)
  {
    Serial.println("Entrando en calibracion");
    calibracion=HIGH;
    pasos=0;
    calibracion_persiana();
  } 
  delay(10);
  client.set_callback(callback);
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("WiFi not connected");
    }
    else
    {
      Serial.println("WiFi connected");
    }
  }
  
  Serial.println("INICIANDO CONFIGURACION DE LA PERSIANA");
  delay(2000);
  
  posicion_actual= EEPROM.read(0);
  pasos_totales=EEPROM.read(1); 
  Serial.print("dato eeprom 0 : ");
  Serial.println(EEPROM.read(0));
  Serial.print("dato eeprom 1 : ");
  Serial.println(EEPROM.read(1));
  
  if (pasos_totales==0)
  {
    sinprog=HIGH;
    funcionamiento_libre();
  }

  
  Serial.print("Pasos guardados en memoria = ");
  Serial.println(pasos_totales);
  Serial.print("La posicion actual en pasos es : ");
  Serial.println(posicion_actual);
  int pos = map(posicion_actual,0,pasos_totales,0,100);
  Serial.print("La persiana esta abierta al ");
  Serial.print(pos);
  Serial.println(" %");
  client.publish("blind1posicion", String(pos));



}


void ajustar_persiana()
{
  Serial.println("AJUSTAR PERSIANA");
  Serial.print("La posicion actual es: ");
  Serial.println(posicion_actual);
  Serial.print("La posicion destino :");
  Serial.println(posicion_destino);
  
  if (posicion_destino != posicion_actual)
  {
  while (posicion_destino > posicion_actual)
  {
    subiendo=HIGH;
    digitalWrite(pin_subir, LOW);
    Serial.println(posicion_actual);
    int pos =map(posicion_actual,0,pasos_totales,0,100);    
    client.publish("blind1posicion", String(pos));
    //client.publish("blind1posicion", String(pos+200));
    delay(100);  
  }
  subiendo=LOW;
  digitalWrite(pin_subir, HIGH);

  while (posicion_destino < posicion_actual)
  {
    bajando=HIGH;
    digitalWrite(pin_bajar, LOW);
    Serial.println(posicion_actual);
    int pos =map(posicion_actual,0,pasos_totales,0,100);  
    //client.publish("blind1posicion", "301");
    client.publish("blind1posicion", String(pos));
    //client.publish("blind1posicion", String(pos+200));
    delay(100);    
  }
  bajando=LOW;
  digitalWrite(pin_bajar, HIGH);
  int pos =map(posicion_actual,0,pasos_totales,0,100);
  EEPROM.write(0, posicion_actual);
  EEPROM.commit();
 Serial.println("SALIENDO DE AJUSTAR PERSIANA");
 //client.publish("blind1posicion", "303");

  }
}


void loop()
{ 
estado_pulsador_subir=digitalRead(pulsador_subir);
estado_pulsador_bajar=digitalRead(pulsador_bajar);
estado_pulsador_reset=digitalRead(pulsador_reset);

if (estado_pulsador_reset==LOW)
{
  reseteando();
}

if (estado_pulsador_subir==LOW)
{
   while (estado_pulsador_subir==LOW && posicion_actual<pasos_totales)
  {
   estado_pulsador_subir=digitalRead(pulsador_subir);
   digitalWrite(pin_subir,LOW);
   delay(10);
   subiendo=HIGH;
   int pos = map(posicion_actual,0,pasos_totales,0,100);
   client.publish("blind1posicion", String(pos));
   client.publish("blind1posicion", String(pos+200));
  } 
digitalWrite(pin_subir,HIGH); 
subiendo=LOW;
EEPROM.write(0,posicion_actual);
EEPROM.commit();
delay(100);
client.publish("blind1posicion", "303");
}

if (estado_pulsador_bajar==LOW)
{
   while (estado_pulsador_bajar==LOW && posicion_actual>0)
  {
   estado_pulsador_bajar=digitalRead(pulsador_bajar);
   digitalWrite(pin_bajar,LOW);
   delay(10);
   bajando=HIGH;
   int pos = map(posicion_actual,0,pasos_totales,0,100);
   client.publish("blind1posicion", String(pos));
   client.publish("blind1posicion", String(pos+200));
  }
digitalWrite(pin_bajar,HIGH); 
bajando=LOW; 
EEPROM.write(0,posicion_actual);
EEPROM.commit();
delay(100);
client.publish("blind1posicion", "303"); 
}

    if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect("ESP8266: blind1")) {
        client.publish("outTopic",(String)"hello world, I'm "+host);
        client.subscribe(host+(String)"/#");
      }
    }
    if (client.connected())
      client.loop();
  }
if (contador>=1500)
{
int pos = map(posicion_actual,0,pasos_totales,0,100);
client.publish("blind1posicion", String(pos)); 
client.publish("blind1posicion", String(pos+200)); 
//client.publish("blind1posicion", "303");
contador=0;
}
contador++;
delay(10);

}


void calibracion_persiana()
{
  if (calibracion==HIGH)
  {
  Serial.println("Borrando memoria EEPROM");
  EEPROM.write(0,0);
  EEPROM.write(1,0);
  EEPROM.commit();
  delay(2000);
  Serial.println("CALIBRANDO");
  digitalWrite(pin_bajar,LOW);
  while ( calibracion == HIGH)
  {
    estado_pulsador_subir=digitalRead(pulsador_subir);
    if (estado_pulsador_subir==LOW)
    {
      Serial.println("PULSADOR ACCIONADO");
      digitalWrite(pin_bajar,HIGH);
      calibracion=LOW;
      delay(500);
    }
  delay(100);
  }
  EEPROM.write(1, pasos);
  EEPROM.write(0,0);
  EEPROM.commit();
  delay(100);
  Serial.print("Pasos guardados: ");
  Serial.println(pasos);
  pasos_totales=EEPROM.read(1);
  posicion_actual=EEPROM.read(0);
  pasos=0;
  Serial.print("dato eeprom 0 : ");
  Serial.println(EEPROM.read(0));
  Serial.print("dato eeprom 1 : ");
  Serial.println(EEPROM.read(1));
  Serial.println("PERSIANA CALIBRADA");
  delay(2000);
  }
}

void cuentapasos()
{
  
  if (calibracion==HIGH)
  {
    pasos++;
    Serial.print("PASOS CONTADOS: "); 
    Serial.println(pasos);
  }

  if (subiendo==HIGH)
  {
   if (posicion_actual<pasos_totales)
   {
   posicion_actual++;
   }
   Serial.print("La posicion actual en pasos es : ");
   Serial.println(posicion_actual);
   Serial.print("La posicion actual es: ");
   int pos =map(posicion_actual,0,pasos_totales,0,100);
   Serial.println(pos);
  }
  
  if (bajando==HIGH)
  {
  if (posicion_actual>0)
  {
   posicion_actual--;
  }
   Serial.print("La posicion actual en pasos es : ");
   Serial.println(posicion_actual);   
   Serial.print("La posicion actual es: ");
   int pos =map(posicion_actual,0,pasos_totales,0,100);
   Serial.println(pos);
  }   
}

void reseteando()
{
  EEPROM.write(0,0);
  EEPROM.write(1,0);
  EEPROM.commit();
  sinprog=HIGH;
  funcionamiento_libre();
}


void funcionamiento_libre()
{
  Serial.println("FUNCIONAMIENTO LIBRE");
  while (sinprog==HIGH)
  {
    estado_pulsador_subir=digitalRead(pulsador_subir);
    estado_pulsador_bajar=digitalRead(pulsador_bajar);

    if (estado_pulsador_subir==LOW)
    {
      while (estado_pulsador_subir==LOW)
      {
        estado_pulsador_subir=digitalRead(pulsador_subir);
        digitalWrite(pin_subir,LOW);
        delay(50);
      }
      digitalWrite(pin_subir,HIGH);
    }

    if (estado_pulsador_bajar==LOW)
    {
      while (estado_pulsador_bajar==LOW)
      {
        estado_pulsador_bajar=digitalRead(pulsador_bajar);
        digitalWrite(pin_bajar,LOW);
        delay(50);
      }
      digitalWrite(pin_bajar,HIGH);
    }
    delay(50);  
  }
}

