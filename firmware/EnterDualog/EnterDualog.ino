#include <time.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include "credentials.h"
#include "config.h"

#define BLYNK_PRINT Serial

bool hodor = false;
const int timezone = TIME_TIMEZONE, dst = TIME_DST;
Servo door;
unsigned long nextHodor = 0;
int hodorRemaining = 0;

// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);

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

  terminal.println("DOOR OPENER v0.1 started");
  terminal.flush();
}

void openDoor()
{
  terminal.println("Opening the door");
  terminal.flush();

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

  return (timeinfo->tm_hour >= TIME_HOUR_OPEN && timeinfo->tm_hour < TIME_HOUR_CLOSE && timeinfo->tm_wday >= 1 && timeinfo->tm_wday < 7);
}

void tryToOpenDoor()
{
  if (isDoorEnabled()) openDoor();
  else
  {
    hodor = false;
    terminal.println("Use the normal key to open the door");
    terminal.flush();

    Serial.println("Not opening the door.");
    printTime();
  }
}

BLYNK_WRITE(V0)
{
  if (param.asInt())
    tryToOpenDoor();
}

BLYNK_WRITE(V1)
{
  String cmd = param.asStr();
  if (String(HODOR_ENABLE) == cmd) {
    terminal.println("Walder is holding the door");
    hodorRemaining = HODOR_TIMES;
    hodor = true;
  }
  else if (String(HODOR_DISABLE) == cmd) {
    terminal.println("Hodor is defeated");
    hodor = false;
  } else {
    terminal.println("Command not supported");
  }

  terminal.flush();
}

BLYNK_WRITE(V2)
{
  int s = param.asInt();
  terminal.print(s ? "GPS: Enter" : "GPS: Exit");
  terminal.flush();
}

void loop()
{
  Blynk.run();

  if (hodor && millis() > nextHodor)
  {
    if (hodorRemaining-- > 0)
    {
      nextHodor = millis() + HODOR_REPEAT;
      tryToOpenDoor();
    }
    else
    {
      hodor = false;
      terminal.println("Walder will rest now");
      terminal.flush();
    }
  }
}

