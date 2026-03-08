#include <Arduino.h>
#include <Servo.h>
#include "WiFiS3.h"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int keyIndex = 0;
int status = WL_IDLE_STATUS;
const int svBasePin = 3;
const int svShoulderPin = 5;
const int svElbowPin = 6;

WiFiServer server(80);
Servo svBase, svShoulder, svElbow;

void printWifiStatus();
void calibration();
String getCoordinates(String r, String position);

void setup()
{
  Serial.begin(9600);

  while (!Serial)
    ;

  Serial.println("Serial enabled!");

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  while (status != WL_CONNECTED)
  {
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  server.begin();
  printWifiStatus();

  svBase.attach(svBasePin);
  svShoulder.attach(svShoulderPin);
  svElbow.attach(svElbowPin);

  calibration();
}

void loop()
{
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("Client connected!!!");

    String request = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        request += c;
      }

      if (request.endsWith("\r\n\r\n"))
        break;
    }

    Serial.println("Getting positions");
    String positionX = getCoordinates(request, "x=");
    String positionY = getCoordinates(request, "y=");
    String positionZ = getCoordinates(request, "z=");

    svBase.write(positionX.toFloat());
    svShoulder.write(positionY.toFloat());
    svElbow.write(positionZ.toFloat());

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Hello");
    client.stop();
  }
}

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void calibration()
{
  svBase.write(90);
  svShoulder.write(90);
  svElbow.write(90);
}

String getCoordinates(String r, String position)
{
  int cmdIndex = r.indexOf(position);
  String cmd = "";

  if (cmdIndex != -1)
  {
    int cmdEnd = r.indexOf(",", cmdIndex);

    cmd = r.substring(cmdIndex + 2, cmdEnd);
  }

  return cmd;
}