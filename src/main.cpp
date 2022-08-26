#include <M5Stack.h>
#include "M5_ENV.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define JST 3600 * 9

SHT3X sht30;
QMP6988 qmp6988;

int count = 0;
int ok = 0;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;

const char *ssid = "hoge";
const char *pass = "fuga";
const char *published_url = "piyo";

void setup_wifi()
{
  WiFi.disconnect();
  delay(500);

  // 接続
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    M5.Lcd.print(".");
    delay(1000);
    count++;
    if (count > 30)
    {
      // WiFIに30秒以上接続できなかった場合はリセット
      ESP.restart();
    }
  }
  count = 0;
  M5.Lcd.fillScreen(BLACK);

  // IPアドレスの表示
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 170);
  M5.Lcd.print(WiFi.localIP());
}

void post_data(char *pubMessage)
{
  HTTPClient http;

  // 接続
  http.begin(published_url);
  int httpCode = http.POST(pubMessage);

  if (httpCode > 0)
  {
    M5.Lcd.setCursor(0, 190);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("HTTP Response:%d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      M5.Lcd.printf("Success: %d", ++ok);
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else
  {
    M5.Lcd.println("FAILED");
    Serial.printf("Failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void create_data(int hour)
{
  StaticJsonDocument<500> doc;
  char pubMessage[256];

  // JSONの作成
  JsonArray hourValues = doc.createNestedArray("hour");
  hourValues.add(String(hour));
  JsonArray tmpValues = doc.createNestedArray("temp");
  tmpValues.add(String(tmp));
  JsonArray humValues = doc.createNestedArray("humi");
  humValues.add(String(hum));
  JsonArray pressValues = doc.createNestedArray("pressure");
  pressValues.add(String(pressure));

  serializeJson(doc, pubMessage);

  post_data(pubMessage);
}

void get_time()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    M5.Lcd.println("Failed to obtain time");
    return;
  }

  // 日付・時刻の表示
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("%04d/%02d/%02d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon, timeinfo.tm_mday);
  M5.Lcd.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // スプレッドシートへの記入
  if (timeinfo.tm_min == 0 && timeinfo.tm_sec == 0)
  {
    create_data(timeinfo.tm_hour);
  }
}

void setup()
{
  M5.begin();
  M5.Power.begin();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2.5);
  Wire.begin();
  Serial.begin(115200);

  // 気圧センサーの初期化
  qmp6988.init();

  // ノイズ対策のスピーカーのミュート
  M5.Speaker.begin();
  M5.Speaker.mute();

  // WiFiの接続
  setup_wifi();

  // NTP
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void loop()
{
  // 日時表示
  M5.Lcd.setCursor(0, 0);
  get_time();

  // 気圧の測定
  pressure = qmp6988.calcPressure() * 0.01;

  // 気温・湿度の測定
  if (sht30.get() == 0)
  {
    tmp = sht30.cTemp;
    hum = sht30.humidity;
  }
  else
  {
    tmp = 0, hum = 0;
  }

  // ディスプレイに表示
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("Temp: %2.1f 'C\nHumi: %2.0f %%\nPressure: %2.0f hPa\n", tmp, hum, pressure);

  // 時刻補正
  count++;
  if (count > 600)
  {
    configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    count = 0;
  }
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("count: %02d", count);

  delay(1000);
}
