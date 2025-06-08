#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Arduino.h>
#include <credentials.h>

#define DHTTYPE DHT22 // DHT11

WebServer server(80);

const int blinkLed = 2;
const int requestLed = 16;
const int webLed = 18;

const int dhtPin = 15;

unsigned long previousMillis = 0;
const unsigned int loopDelay = 10;

bool ledState = false;

int deviceCount = 3;
float th[2];

DHT dht(dhtPin, DHTTYPE);

void handleData()
{
  String json = "{\"temperature\":" + String(th[0], 1) +
                ",\"humidity\":" + String(th[1], 1) + "}";
  server.send(200, "application/json", json);
}

const char *htmlPage = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>ESP32 LED & Sensor</title>
    <style>
      body { font-family: Arial; text-align: center; margin-top: 50px; }
      button { font-size: 24px; padding: 20px; margin: 10px; }
      .sensor { font-size: 20px; margin-top: 30px; }
    </style>
  </head>
  <body>
    <h1>ESP32 LED Control</h1>
    <p>LED is <span id="state">%STATE%</span></p>
    <button onclick="toggleLED()">Toggle LED</button>
  
    <div class="sensor">
      <p>Temperature: <span id="temp">--</span> °C</p>
      <p>Humidity: <span id="hum">--</span> %</p>
    </div>
  
    <script>
      function toggleLED() {
        fetch("/toggle")
          .then(response => response.text())
          .then(state => {
            document.getElementById("state").innerText = state;
          });
      }
  
      function updateSensorData() {
        fetch("/data")
          .then(response => response.json())
          .then(data => {
            document.getElementById("temp").innerText = data.temperature.toFixed(1);
            document.getElementById("hum").innerText = data.humidity.toFixed(1);
          });
      }
  
      setInterval(updateSensorData, 5000); // Fetch every 5 seconds
      updateSensorData(); // Initial fetch
    </script>
  </body>
  </html>
  )rawliteral";

// Helper to replace placeholder with LED state
String processor(const String &var)
{
  if (var == "STATE")
  {
    return ledState ? "ON" : "OFF";
  }
  return "";
}

void handleRoot()
{
  digitalWrite(requestLed, HIGH);
  String page = htmlPage;
  page.replace("%STATE%", ledState ? "ON" : "OFF");
  server.send(200, "text/html", page);
  digitalWrite(requestLed, LOW);
}

void handleToggle()
{
  ledState = !ledState;
  digitalWrite(webLed, ledState ? HIGH : LOW);
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void get_temps()
{
  // BlinkNTimes(LED_0, 2, 500);
  deviceCount = 3;

  th[0] = 0;
  th[1] = 0;

  try
  {
#ifdef DEBUG
    h = 55;
    t = 22;
#else
    int numberOfTries = 0;
    int numberRead = 0;
    int numberNotRead = 0;
    float temperature = 0;
    float humidity = 0;
    int countToRead = 3;
    do
    {
      temperature = dht.readTemperature();
      delay(1000);
      humidity = dht.readHumidity();
      delay(1000);
      numberOfTries++;

      if (isnan(temperature) || isnan(humidity))
      {
        numberNotRead++;
      }
      else
      {
        numberRead++;
        th[0] = th[0] + temperature;
        th[1] = th[1] + humidity;
      }

      Serial.print("Temperature");
      Serial.print(numberOfTries);
      Serial.println(": " + String(temperature, 1));

      if (numberNotRead > 6)
      {
        Serial.println("Beginning DHT again");
        dht.begin();
        delay(1000);
        numberNotRead = 0;
      }

    } while (numberOfTries < countToRead);

#endif

    if (numberRead == 0)
    {
      Serial.println("Failed to read from DHT sensor!");
      deviceCount = 0;
    }

    if (deviceCount == 0)
    {
      Serial.print("No Content");
    }
    else
    {
      Serial.print("numberRead: ");
      Serial.println(numberRead);

      th[0] = th[0] / numberRead;
      th[1] = th[1] / numberRead;

      float hic = dht.computeHeatIndex(th[0], th[1], false);
    }
  }
  catch (const std::exception &e)
  {
  }
}

void get_temps2()
{
  th[0] = 0;
  th[1] = 0;

  try
  {
    float temperature = 0;
    float humidity = 0;
    temperature = dht.readTemperature();
    delay(500);
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
    }
    else
    {
      th[0] = temperature;
      th[1] = humidity;
    }

    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("Failed to read from DHT sensor!");
      deviceCount = 0;
    }

    if (deviceCount == 0)
    {
      Serial.print("No Content");
    }
    else
    {
      float hic = dht.computeHeatIndex(th[0], th[1], false);
    }
  }
  catch (const std::exception &e)
  {
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(blinkLed, OUTPUT);
  digitalWrite(blinkLed, LOW);
  pinMode(requestLed, OUTPUT);
  digitalWrite(requestLed, LOW);
  pinMode(webLed, OUTPUT);
  digitalWrite(webLed, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  server.begin();
  Serial.println("HTTP server started");
  server.on("/data", handleData);

  dht.begin();
  delay(1000);

  get_temps2();
}

void loop()
{
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= loopDelay * 1000)
  {
    // get_temps();
    get_temps2();

    Serial.print("Temperature: ");
    Serial.print(th[0]);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(th[1]);
    Serial.println(" %");

    if (ledState)
    {
      digitalWrite(blinkLed, LOW);
      // Serial.println("LED OFF");
      ledState = false;
    }
    else
    {
      digitalWrite(blinkLed, HIGH);
      // Serial.println("LED ON");
      ledState = true;
    }

    previousMillis = currentMillis;
  }
}
