#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/************ BLE UUIDs ************/
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_UUID      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_UUID      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/************ PIN DEFINITIONS ************/
#define TRIG_PIN     5
#define ECHO_PIN     18
#define FLAME_PIN    17
#define IR_LEFT_PIN  19
#define IR_RIGHT_PIN 21
#define MOTOR1_PIN   23   // LEFT MOTOR
#define MOTOR2_PIN   22   // RIGHT MOTOR
#define VOLT25_PIN   34
#define LED_PIN      2    // BLUE LED (ON when BLE connected)

/************ GLOBALS ************/
BLECharacteristic *txChar;
bool bleAlertActive = false;
long duration;
float distance;

/************ BLE SERVER CALLBACK (LED STATUS) ************/
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("🔵 BLE CONNECTED");
    digitalWrite(LED_PIN, HIGH);   // BLUE LED ON
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("⚪ BLE DISCONNECTED");
    digitalWrite(LED_PIN, LOW);    // LED OFF
    pServer->getAdvertising()->start(); // restart advertising
  }
};

/************ BLE RX CALLBACK ************/
class RXCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) {
    String cmd = c->getValue().c_str();
    Serial.println("📩 BLE CMD: " + cmd);

    if (cmd == "ALERT") {
      bleAlertActive = true;

      digitalWrite(MOTOR1_PIN, HIGH);
      digitalWrite(MOTOR2_PIN, HIGH);
      delay(3000);                 // 🔊 SOUND → 3 sec
      digitalWrite(MOTOR1_PIN, LOW);
      digitalWrite(MOTOR2_PIN, LOW);

      bleAlertActive = false;
    }

    else if (cmd == "STOP") {
      bleAlertActive = false;
      digitalWrite(MOTOR1_PIN, LOW);
      digitalWrite(MOTOR2_PIN, LOW);
    }
  }
};

/************ BATTERY ************/
float readBatteryPercent() {
  int adc = analogRead(VOLT25_PIN);
  float v = (adc / 4095.0) * 3.3 * 5.0;
  v = constrain(v, 0, 10);
  return (v / 10.0) * 100.0;
}

/************ BLE SEND ************/
void sendBLE(String data) {
  txChar->setValue(data.c_str());
  txChar->notify();
}

/************ SETUP ************/
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(IR_LEFT_PIN, INPUT);
  pinMode(IR_RIGHT_PIN, INPUT);
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(MOTOR1_PIN, LOW);
  digitalWrite(MOTOR2_PIN, LOW);
  digitalWrite(LED_PIN, LOW);  // LED OFF initially

  BLEDevice::init("SafeStepper");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *rxChar = service->createCharacteristic(
    RX_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  rxChar->setCallbacks(new RXCallbacks());

  txChar = service->createCharacteristic(
    TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  txChar->addDescriptor(new BLE2902());

  service->start();
  BLEDevice::getAdvertising()->start();

  Serial.println("✅ ESP32 BLE READY");
}

/************ LOOP ************/
void loop() {

  // ---------- ULTRASONIC ----------
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  distance = (duration > 0) ? (duration * 0.0343 / 2) : 0;

  // ---------- SENSOR READ ----------
  int flame = digitalRead(FLAME_PIN);     // LOW = fire
  int irL   = digitalRead(IR_LEFT_PIN);   // LOW = object
  int irR   = digitalRead(IR_RIGHT_PIN);  // LOW = object
  float battery = readBatteryPercent();

  // ---------- SEND BLE DATA ----------
  String payload =
    "DIST:" + String(distance, 1) +
    ",BAT:" + String(battery, 0) +
    ",FLAME:" + String(flame) +
    ",IRL:" + String(irL) +
    ",IRR:" + String(irR);

  sendBLE(payload);

  // ---------- VIBRATION LOGIC ----------
  if (!bleAlertActive) {

    // 🔥 FLAME → 5 sec
    if (flame == LOW) {
      digitalWrite(MOTOR1_PIN, HIGH);
      digitalWrite(MOTOR2_PIN, HIGH);
      delay(5000);
      digitalWrite(MOTOR1_PIN, LOW);
      digitalWrite(MOTOR2_PIN, LOW);
    }

    // 📏 ULTRASONIC → 5 sec
    else if (distance > 0 && distance <= 20) {
      digitalWrite(MOTOR1_PIN, HIGH);
      digitalWrite(MOTOR2_PIN, HIGH);
      delay(5000);
      digitalWrite(MOTOR1_PIN, LOW);
      digitalWrite(MOTOR2_PIN, LOW);
    }

    // ⬅ IR LEFT → 2 sec
    else if (irL == LOW) {
      digitalWrite(MOTOR1_PIN, HIGH);
      delay(2000);
      digitalWrite(MOTOR1_PIN, LOW);
    }

    // ➡ IR RIGHT → 2 sec
    else if (irR == LOW) {
      digitalWrite(MOTOR2_PIN, HIGH);
      delay(2000);
      digitalWrite(MOTOR2_PIN, LOW);
    }
  }

  delay(300);
}
