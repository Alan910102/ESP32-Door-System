#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

// ====== WiFi 設定 ======
const char* ssid = "{WIFI_NAME}";
const char* password = "{WIFI_PASSWORD}";


// ====== MQTT Broker 設定 ======
const char* mqtt_server = "{MQTT_SERVER}";
const int mqtt_port = 1883;
const char* mqtt_user = "{MQTT_USER}";
const char* mqtt_pass = "{MQTT_PASSWORD}";

// ====== 遙控器名稱 ======
String gate_id = "gate2";  // ← 換裝置只改這行即可

WiFiClient espClient;
PubSubClient client(espClient);

// ====== GPIO 定義 ======
const int pin_up = 4;
const int pin_down = 3;
const int pin_stop = 20;
const int pin_led = 8;            // 板載 LED
const int pin_relay_power = 10;  // 控制繼電器的 GPIO

// ====== 狀態變數 ======
bool all_power = false;
String currentMessage = "";
Ticker resetTimer;
Ticker dailyResetTimer;

// ====== 封裝主題 ======
String topic_power()     { return gate_id + "/power"; }
String topic_door()      { return gate_id + "/door"; }
String topic_ledstatus() { return gate_id + "/door/led"; }
String topic_status()    { return "remote/controller/status/" + gate_id; }

// ====== WiFi 連線 ======
void setup_wifi() {
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retry > 20) ESP.restart();  // 連不上就重啟
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

// ====== GPIO 全部復位 ======
void resetAllPins() {
  digitalWrite(pin_up, HIGH);
  digitalWrite(pin_down, HIGH);
  digitalWrite(pin_stop, HIGH);
  //digitalWrite(pin_relay_power, LOW);  // 關閉繼電器電源
  client.publish(topic_ledstatus().c_str(), "0", true);
  currentMessage = "";
}

// ====== MQTT 回呼處理 ======
void callback(char* topic, byte* payload, unsigned int length) {
  if (length >= 50) return;
  payload[length] = '\0';
  String topicStr = String(topic);
  String message = String((char*)payload);

  Serial.println("[MQTT] Topic: " + topicStr + " Message: " + message);

  if (topicStr == topic_power()) {
    bool new_power = (message == "1");
    if (new_power != all_power) {  // 只有狀態改變才處理
      all_power = new_power;
      Serial.println(all_power ? "Power ON" : "Power OFF");
      if (!all_power) resetAllPins();
    }
    // 不重複發佈狀態，直接 return
    return;
  }

  if (!all_power) return;

  if (topicStr == topic_door()) {
    resetAllPins();
    delay(50);

    if (message == "V0") {
      digitalWrite(pin_up, LOW);
      currentMessage = "V0";
    } else if (message == "V1") {
      digitalWrite(pin_down, LOW);
      currentMessage = "V1";
    } else if (message == "V4") {
      digitalWrite(pin_stop, LOW);
      currentMessage = "V4";
    } else {
      return;
    }

    client.publish(topic_ledstatus().c_str(), "1", true);
    resetTimer.detach();
    resetTimer.once(0.5, resetAllPins);  // 自動復位
  }
}

// ====== MQTT 重連 ======
void reconnect() {
  digitalWrite(pin_led, LOW);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32_" + gate_id;

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass,
                       topic_status().c_str(), 0, true, "offline")) {
      Serial.println("Connected!");
      client.subscribe(topic_power().c_str());
      client.subscribe(topic_door().c_str());

      // 上線同步
      client.publish(topic_status().c_str(), "online", true);
      //client.publish(topic_power().c_str(), "0", true);
      client.publish(topic_ledstatus().c_str(), "0", true);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 5s");
      delay(5000);
    }
  }
  client.publish(topic_power().c_str(), "0", true);
  digitalWrite(pin_led, HIGH);
}

// ====== 每日自動重啟 ======
void dailyReset() {
  Serial.println("Daily reset...");
  ESP.restart();
}

// ====== 初始化 ======
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("esp32_for_gate2");
  pinMode(pin_up, OUTPUT);
  pinMode(pin_down, OUTPUT);
  pinMode(pin_stop, OUTPUT);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_relay_power, OUTPUT);

  digitalWrite(pin_relay_power, LOW);   // 一開始繼電器為 OFF
  resetAllPins();              // 一律復位
  digitalWrite(pin_led, LOW); // LED閃爍代表初始化中

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  digitalWrite(pin_led, HIGH);           // WiFi OK
  delay(500);

  

  dailyResetTimer.attach(86400, dailyReset);  // 每 24 小時自動重啟
  
  digitalWrite(pin_relay_power, HIGH);   // 初始化結束，開始工作
}

// ====== 主迴圈 ======
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
