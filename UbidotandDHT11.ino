

/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
DHTesp dht;

/****************************************
 * Define Constants
 ****************************************/
 
#define DHTpin 4    //D15 of ESP32 DevKit
#define WIFISSID "iotworkshop" // Put your WifiSSID here
#define PASSWORD "123414321*" // Put your wifi password here
#define TOKEN "A1E-QICFYVfwAheWmTBjHF4WDnQHFA5e6w" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "123443212" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                     //it should be a random and unique ascii string and different from all other devices
#define VARIABLE_LABEL_TEMP "Temp" // Assing the variable label
#define VARIABLE_LABEL_HUMI "humidity" // Assing the variable label
#define DEVICE_LABEL "ESP32x_01" // Assig the device label

char mqttBroker[]  = "things.ubidots.com";
char payload[100];
char topic[150];
char topic2[150];
char topicSubscribe[100];
String getStatusDHT11,gethumidity,getTemp;

WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      client.subscribe(topicSubscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}



/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT 

  Serial.println();
  Serial.print("Wait for WiFi...");
   dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);    
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // ตัวแปรของเซ็นเซอร์ DHT11
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  getStatusDHT11 = String(dht.getStatusString());
  gethumidity = String(humidity);
  getTemp = String(getTemp);


  // ตั้งเงื่อนไขว่า ถ้า Status ของ Sensor DHT11 พร้่อมทำงาน คือคำว่า OK จะส่งข้อมูล
  if(getStatusDHT11 == "OK") {

      /* ************** พารามิเตอร์ส่งขึ้น Ubidots ตัวที่ 1 ************** */
      sprintf(topic, "%s",""); // อย่าลืมประกาศตัวแปร topic ใน Golbal Val ด้วย
      sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL); // ขื่อ Label_Device บน Ubidot
      sprintf(payload, "%s", ""); // Clean payload
      sprintf(payload, "{\"%s\":", VARIABLE_LABEL_TEMP); // เพิ่มชื่อ Label ตัวแปรที่เก็บบน Ubidot
      // เนื่องจากตัวแปรที่ได้มาเป็น float ต้อง แปลงให้เป็น Char เนื่องจาก Payload() รองรับตัวแปรชนิดนี้
     
     char getTempValue[10];
     dtostrf(temperature, 4, 2, getTempValue);

     // ฟังก์ชัน แรนดอม
     /*
     long ar = random(20, 27);
     ltoa(ar,getTempValue,9);
     */
  
     sprintf(payload, "%s {\"value\": %s", payload, getTempValue); // Add value
     sprintf(payload, "%s } }", payload); // Close bracket
     Serial.println("ส่ง Temp ข้อมูลไป Ubidots แล้ว");
     client.publish(topic, payload);
     
     /* ****************************** END *********************** */     

      /* ************** พารามิเตอร์ส่งขึ้น Ubidots ตัวที่ 2 ************** */
      sprintf(topic2, "%s",""); // อย่าลืมประกาศตัวแปร topic ใน Golbal Val ด้วย// อย่าลืมประกาศตัวแปร topic ใน Golbal Val ด้วย
      sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL); // ขื่อ Label_Device บน Ubidot
      sprintf(payload, "%s", ""); // Clean payload
      sprintf(payload, "{\"%s\":", VARIABLE_LABEL_HUMI); // เพิ่มชื่อ Label ตัวแปรที่เก็บบน Ubidot
      // เนื่องจากตัวแปรที่ได้มาเป็น float ต้อง แปลงให้เป็น Char เนื่องจาก Payload() รองรับตัวแปรชนิดนี้
      
      // แปลงค่าจาก float to char เพื่อส่งเข้าฟังก์ชั่น
      char getHumi[10];
      dtostrf(humidity, 4, 2, getHumi);
    
     // ฟังก์ชัน แรนดอม
     /*
     long ar1 = random(0, 100);
     ltoa(ar1,getHumi,9);
     */
  
     sprintf(payload, "%s {\"value\": %s", payload, getHumi); // Add value
     sprintf(payload, "%s } }", payload); // Close bracket
     Serial.println("ส่ง Humid ข้อมูลไป Ubidots แล้ว");
     client.publish(topic2, payload);
      /* ****************************** END *********************** */     
  }

  client.loop();
  delay(1000);
}
