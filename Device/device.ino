// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include <WiFi.h>
#include "Esp32MQTTClient.h"
#include "AzureIotHub.h"

#define INTERVAL 5000

// Please input the SSID and password of WiFi
const char* ssid     = "";
const char* password = "";

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "";

static bool isConnected = false;
const char *twinProperties="{\"Protocol\": \"MQTT\", \"SupportedMethods\": \"LedColor\", \"Telemetry\": { \"%s\": {\"Interval\": \"%s\",\"MessageTemplate\": \"{\\\"temperature\\\":${temperature},\\\"temperature_unit\\\":\\\"${temperature_unit}\\\", \\\"humidity\\\":${humidity},\\\"humidity_unit\\\":\\\"${humidity_unit}\\\",\\\"pressure\\\":${pressure},\\\"pressure_unit\\\":\\\"${pressure_unit}\\\"}\",\"MessageSchema\": {\"Name\": \"%s\",\"Format\": \"JSON\",\"Fields\": {\"temperature\": \"Double\", \"temperature_unit\": \"Text\",\"humidity\": \"Double\",\"humidity_unit\": \"Text\",\"pressure\": \"Double\",\"pressure_unit\": \"Text\" } } } },\"Type\": \"%s\",\"Firmware\": \"%s\",\"Model\":\"M5STACK\",\"FirmwareUpdateStatus\": \"%s\",\"Location\": \"%s\",\"Latitude\": %f,\"Longitude\": %f}";

float temperature = 50;
char temperatureUnit = 'F';
float humidity = 50;
char humidityUnit = '%';
float pressure = 55;
const char *pressureUnit = "psig";

const char *roomSchema = "chiller-sensors;v1";
const char *interval = "00:00:05";
const char *deviceType = "Chiller";
const char *deviceFirmware = "1.0.0";
const char *deviceFirmwareUpdateStatus = "";
const char *deviceLocation = "Madrid";
const double deviceLatitude = 40.418371;
const double deviceLongitude =-3.797997;

static uint64_t send_interval_ms;
static uint64_t reset_interval_ms;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities

// Initialize WiFi
void InitWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  isConnected = true;
}

// Sending Device Info
bool sendDeviceInfo()
{
  char reportedProperties[2048];
  snprintf(reportedProperties,2048, twinProperties,roomSchema, interval ,roomSchema,deviceType,deviceFirmware,deviceFirmwareUpdateStatus,deviceLocation,deviceLatitude,deviceLongitude);
  Serial.println(reportedProperties);
  return Esp32MQTTClient_ReportState(reportedProperties);
}

// Device Twin Callback
void twinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  LogInfo("*** Twin State: %s",updateState?"Complete":"Partial");
}

// Device Method Callback
int device_method_callback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  LogInfo("Try to invoke method %s", methodName);
  int result = 200;

  if(strcmp(methodName,"LedColor")==0)
  {
    Serial.println("Method Invoked!");
    const char *ok="{\"result\":\"OK\"}";
    *response_size=strlen(ok);
    *response = (unsigned char*)malloc(*response_size);
    strncpy((char *)(*response), ok, *response_size);
    return 200;
  }

  return result;
}

// Sending sensor data
void sendData()
{
  temperature = (float)random(0,500)/10;
  humidity = (float)random(0, 1000)/10;
  pressure = (float)random(0, 10000)/10;
  char sensorData[200];
  sprintf_s(sensorData, sizeof(sensorData), "{\"temperature\":%f,\"temperature_unit\":\"%c\",\"humidity\":%f,\"humidity_unit\":\"%c\",\"pressure\":%f,\"pressure_unit\":\"%s\"}", temperature, temperatureUnit,humidity, humidityUnit,pressure, pressureUnit);
  
  time_t t = time(NULL);
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

  EVENT_INSTANCE* message = Esp32MQTTClient_Event_Generate(sensorData,MESSAGE);
  Esp32MQTTClient_Event_AddProp(message,"$$CreationTimeUtc", buf);
  Esp32MQTTClient_Event_AddProp(message, "$$MessageSchema", roomSchema);
  Esp32MQTTClient_Event_AddProp(message, "$$ContentType", "JSON");

  Esp32MQTTClient_SendEventInstance(message);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Device");
  Serial.println("Initializing...");

  // Initialize the WiFi module
  Serial.println(" > WiFi");
  isConnected = false;
  InitWifi();
  if (!isConnected)
  {
    return;
  }
  randomSeed(analogRead(0));

  // Setup the MQTT Client
  Serial.println(" > IoT Hub");
  Esp32MQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "RemoteMonitoring");
  Esp32MQTTClient_Init((const uint8_t*)connectionString, true);

  Esp32MQTTClient_SetDeviceTwinCallback(twinCallback);
  Esp32MQTTClient_SetDeviceMethodCallback(device_method_callback);

  send_interval_ms = millis();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    if((int)(millis() - send_interval_ms)>INTERVAL)
    {
      sendData();
      send_interval_ms = millis();
    }
    if((int)(millis() - reset_interval_ms)>INTERVAL)
    {
      reset_interval_ms = millis();
    }
  }
}
