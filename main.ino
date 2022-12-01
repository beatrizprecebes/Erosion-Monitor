/*Libraries*/
#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ThingSpeak.h"

/*WiFi connection data*/
char* ssid           = "ssid";
const char* password = "password";

String result;

unsigned long last_time = 0;
unsigned long timer_delay = 10000;

WiFiClient client;

/*OpenWeather API informations*/
String openWeatherMapApiKey = "OpenWeather api key";
String cityID               = "3467865";                           //Campinas ID
char servername[]           = "api.openweathermap.org";
bool id                     = false;

/*ThingSpeak channel informations*/
unsigned long channel_number = channel_number;
const char * write_API_key = "write_API_key";

/*Ultrasonic sensor informations*/
#define SOUND_SPEED 0.034
const int trigPin = 5;
const int echoPin = 18;
long duration;
float distance_cm;

void setup(){

  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  ThingSpeak.begin(client);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.println(".");
  }

  Serial.println("Connected to WiFi.");

  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

void loop(){

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  distance_cm = duration * SOUND_SPEED/2;
  
  if(client.connect(servername, 80)){
    client.println("GET /data/2.5/weather?id=" + cityID + "&units=metric&APPID=" + openWeatherMapApiKey);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }else{
    Serial.println("connection failed");
    Serial.println();
  }

  while(client.connected() && !client.available())
    delay(1);                                          
  while (client.connected() || client.available()){ 
    char c = client.read();                         
    result = result + c;
  }

  client.stop();                                      
  result.replace('[', ' ');
  result.replace(']', ' ');
  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> doc;
  DeserializationError  error = deserializeJson(doc, jsonArray);

  if (error){
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  String location = doc["name"];
  String country = doc["sys"]["country"];
  int temperature = doc["main"]["temp"];
  int humidity = doc["main"]["humidity"];
  float pressure = doc["main"]["pressure"];
  int id = doc["id"];
  float wind_speed = doc["wind"]["speed"];
  float wind_speed_kmh = wind_speed*3.6;
  int degree = doc["wind"]["deg"];
  float longitude = doc["coord"]["lon"];
  float latitude = doc["coord"]["lat"];

  Serial.println();
  Serial.print("Country: ");
  Serial.println(country);
  Serial.print("Location: ");
  Serial.println(location);
  Serial.print("Location ID: ");
  Serial.println(id);
  Serial.printf("Temperature: %d°C\r\n", temperature);
  Serial.printf("Humidity: %d %%\r\n", humidity);
  Serial.printf("Pressure: %.2f hpa\r\n", pressure);
  Serial.printf("Wind speed: %.1f m/s\r\n", wind_speed_kmh);
  Serial.printf("Wind degree: %d°\r\n", degree);
  Serial.printf("Longitude: %.2f\r\n", longitude);
  Serial.printf("Latitude: %.2f\r\n", latitude);

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, wind_speed_kmh);
  ThingSpeak.setField(5, distance_cm);

  ThingSpeak.writeFields(channel_number, write_API_key);
  
  // delay(1000);     //10 seconds delay
  delay(600000);      //10 minuts delay
  // delay(3600000);  //1 hour delay

}
