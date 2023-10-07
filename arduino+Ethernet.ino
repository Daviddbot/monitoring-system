#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include "MQ7.h"

#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int mq7Pin = A3;
//MQ7 mq7(A3, 5.0);

const int rainPin = A2;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 189, 177);
EthernetServer server(80);

const int dataCapacity = 5; // Jumlah data yang akan disimpan
float temperatureData[dataCapacity] = {0};
float humidityData[dataCapacity] = {0};
float mq7ValueData[dataCapacity] = {0};
float rainValueData[dataCapacity] = {0};
int dataCounter = 0;

unsigned long previousMillis = 0;
const long interval = 6000; // Interval pengiriman data (5 detik)

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(mq7Pin, INPUT);

  Ethernet.begin(mac, ip);

  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  dht.begin();
}

void sendWebPage(EthernetClient client, float temperature, float humidity, float mq7Value, float rainValue) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println("Refresh: 6 ");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>");
  client.println("<style>body { font-family: Arial, Helvetica, sans-serif; text-align: center; }</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='center'><h1>Monitoring Suhu, Kelembaban, Polusi Udara, dan Curah Hujan</h1></div>");
//  client.println("<div class='center'><h1>Monitoring Suhu, Kelembaban, Polusi Udara, dan Curah Hujan</h1></div>");
  client.println("<div></div>");
  client.println("<div style='width: 650px; margin: 0 auto;'>");
  client.println("<canvas id='myChart' width='1000' height='850'></canvas>");
  client.println("</div>");
  client.println("<script>");
  client.println("var ctx = document.getElementById('myChart').getContext('2d');");
  client.println("var myChart = new Chart(ctx, {");
  client.println("type: 'line',"); // Ubah tipe grafik menjadi 'line'
  client.println("data: {");
  client.println("labels: ['Data 1', 'Data 2', 'Data 3', 'Data 4', 'Data 5'],"); // Label sumbu X (maksimal 5 data)
  client.println("datasets: [{");
  client.println("label: 'Temperature',"); // Label untuk data suhu
  client.println("data: [" + String(temperatureData[0]) + ", " + String(temperatureData[1]) + ", " + String(temperatureData[2]) + ", " + String(temperatureData[3]) + ", " + String(temperatureData[4]) + ",+ '%'],");
  client.println("borderColor: 'rgba(255, 99, 132, 1)',");
  client.println("fill: false"); // Tidak mengisi area bawah kurva (garis)
  client.println("}, {");
  client.println("label: 'Humidity',"); // Label untuk data kelembaban
  client.println("data: [" + String(humidityData[0]) + ", " + String(humidityData[1]) + ", " + String(humidityData[2]) + ", " + String(humidityData[3]) + ", " + String(humidityData[4]) + "],");
  client.println("borderColor: 'rgba(54, 162, 235, 1)',");
  client.println("fill: false"); // Tidak mengisi area bawah kurva (garis)
  client.println("}, {");
  client.println("label: 'Polusi Udara',"); // Label untuk data polusi udara
  client.println("data: [" + String(mq7ValueData[0]) + ", " + String(mq7ValueData[1]) + ", " + String(mq7ValueData[2]) + ", " + String(mq7ValueData[3]) + ", " + String(mq7ValueData[4]) + "],");
  client.println("borderColor: 'rgba(255, 206, 86, 1)',");
  client.println("fill: false"); // Tidak mengisi area bawah kurva (garis)
  client.println("}, {");
  client.println("label: 'Curah Hujan',"); // Label untuk data curah hujan
  client.println("data: [" + String(rainValueData[0]) + ", " + String(rainValueData[1]) + ", " + String(rainValueData[2]) + ", " + String(rainValueData[3]) + ", " + String(rainValueData[4]) + "],");
  client.println("borderColor: 'rgba(75, 192, 192, 1)',");
  client.println("fill: false"); // Tidak mengisi area bawah kurva (garis)
  client.println("}]");
  client.println("},");
  client.println("options: {");
  client.println("scales: {");
  client.println("y: {");
  client.println("beginAtZero: true");
  // Jika diperlukan, atur batas sumbu Y seperti berikut
  // client.println("max: 100,"); // Misalnya batas atasnya adalah 100
  // client.println("min: 0,");   // Misalnya batas bawahnya adalah 0
  client.println("}");
  client.println("}");
  client.println("}");
  client.println("});");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
}

float convertToPPM(float sensorValue) {
  float ppmCO = 0.2 * sensorValue;
  return ppmCO;
}

int convertToISPU(float ppmCO) {
  int mq7Value = ppmCO * 10; 
  return mq7Value;
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    float sensorValue = analogRead(mq7Pin);
    float ppmCO = convertToPPM(sensorValue);
    int mq7Value = convertToISPU(ppmCO);

    int rainValue = analogRead(rainPin);
    float rainLevel = map(rainValue, 0, 1023, 0, 100);

    // Simpan data dalam array
    temperatureData[dataCounter] = temperature;
    humidityData[dataCounter] = humidity;
    mq7ValueData[dataCounter] = mq7Value;
    rainValueData[dataCounter] = rainLevel;

    // Increment counter untuk data selanjutnya
    dataCounter++;
    if (dataCounter >= dataCapacity) {
      dataCounter = 0;
    }
          Serial.print(dht.readTemperature());
          Serial.print(" ");
          Serial.print(dht.readHumidity());
          Serial.print(" ");
          Serial.print(mq7Value);
          Serial.print(" ");
          Serial.print(rainLevel);
          Serial.print(" ");
          Serial.println();
          Serial.println();

    // Memulai koneksi dengan client dan mengirimkan halaman web
    EthernetClient client = server.available();
    if (client) {
      sendWebPage(client, temperature, humidity, mq7Value, rainLevel);
      client.stop();
    }
  }
}
