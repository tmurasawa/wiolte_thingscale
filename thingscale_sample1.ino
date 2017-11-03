#include <WioLTEforArduino.h>
#include <WioLTEClient.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <config.h>

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define MQTT_SERVER_HOST  "beam.soracom.io"
#define MQTT_SERVER_PORT  (1883)

#define ID                "WioLTE_"
// ThingScale
#define OUT_TOPIC         "<YOUR_DEVICE_TOKEN>/json"
#define IN_TOPIC          "<YOUR_DEVICE_TOKEN>/<YOUR_SORACOM_IMSI>/subscribe"

#define INTERVAL          (60000)

WioLTE Wio;
WioLTEClient WioClient(&Wio);
PubSubClient MqttClient;

String cmd_string;
long randnum;
String client;
char client_id[32];

void callback(char* topic, byte* payload, unsigned int length) {
  cmd_string = "";
  SerialUSB.print("Subscribe:");
  for (int i = 0; i < length; i++){
    cmd_string = String(cmd_string + (char)payload[i]); 
    //SerialUSB.print((char)payload[i]);
  }
  SerialUSB.println("");
  SerialUSB.print("cmd_string:");
  SerialUSB.print(cmd_string);
  if(cmd_string == "led:on"){
    SerialUSB.println("");
    SerialUSB.println("led:on!!");
    Wio.LedSetRGB(0, 128, 128);
  }
  if(cmd_string == "led:off"){
    SerialUSB.println("");
    SerialUSB.println("led:off");
    Wio.LedSetRGB(0, 0, 0);
  }
  
}

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(5000);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \""APN"\".");
  delay(5000);
  if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to MQTT server \""MQTT_SERVER_HOST"\"");
  MqttClient.setServer(MQTT_SERVER_HOST, MQTT_SERVER_PORT);
  MqttClient.setCallback(callback);
  MqttClient.setClient(WioClient);
  randnum = random(100000);
  client = String(ID + (String)randnum);
  SerialUSB.print("client ID:");
  SerialUSB.print(client);
  SerialUSB.println("");
  client.toCharArray(client_id, 32);
  if (!MqttClient.connect(client_id)) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  MqttClient.subscribe(IN_TOPIC);
}

void loop() {
  char data[100];
  sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
  SerialUSB.print("Publish:");
  SerialUSB.print(data);
  SerialUSB.println("");
  MqttClient.publish(OUT_TOPIC, data);
  
err:
  unsigned long next = millis();
  while (millis() < next + INTERVAL)
  {
    MqttClient.loop();
  }
}
