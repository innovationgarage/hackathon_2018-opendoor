#include <time.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include "credentials.h"
#include "config.h"

#define BLYNK_PRINT Serial

const int timezone = TIME_TIMEZONE, dst = TIME_DST;
Servo door;

void setup()
{
  Serial.begin(9600);
  Serial.println("Connecting to WIFI ...");
  Blynk.begin(BLYNK_HASH, WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED);
  Serial.println("Connected.");

  configTime(TIME_TIMEZONE * 3600, TIME_DST * 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time ...");
  while (!time(nullptr));
  Serial.println("Time received.");
}

void openDoor()
{
  Serial.println("Opening door ...");
  door.attach(DOOR_PIN);
  door.write(DOOR_ANGLE_OPEN);
  delay(DOOR_DELAY);
  door.write(DOOR_ANGLE_CLOSE);
  door.detach();
}

void printTime()
{
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  Serial.print(timeinfo->tm_hour);
  Serial.print(":");
  Serial.print(timeinfo->tm_min);
  Serial.print(" DOW:");
  Serial.print(timeinfo->tm_wday);
}

bool isDoorEnabled()
{
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);

  return (timeinfo->tm_hour >= TIME_HOUR_OPEN && timeinfo->tm_hour < TIME_HOUR_CLOSE && timeinfo->tm_wday > 1 && timeinfo->tm_wday < 7);
}

BLYNK_WRITE(V0)
{
  if (param.asInt())
    if (isDoorEnabled()) openDoor();
    else
    {
      Serial.println("Not opening the door.");
      printTime();
    }
}

void loop()
{
  Blynk.run();
}

