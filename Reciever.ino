#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

// ================= CONFIGURATION =================
float FILTER_STRENGTH = 0.05; // Adjust (0.05 to 0.3) for physical motor fluid speed
int DEADBAND = 2;            // Stops hum/shake when hand is still

// ================= HARDWARE & VARS =================
Servo sBase, sShoulder, sElbow;

// Everything initialized at 0 for static start
float currBase=0, currShoulder=0, currElbow=0;
int targetBase=0, targetShoulder=0, targetElbow=0;

typedef struct struct_message {
  int base, shoulder, elbow, wrist, gripper;
} struct_message;

void OnDataRecv(const uint8_t * mac, const uint8_t *incoming, int len) {
  struct_message data;
  memcpy(&data, incoming, sizeof(data));

  // Deadband: Ignore micro-movements of human hand
  if (abs(data.base - targetBase) > DEADBAND) targetBase = data.base;
  if (abs(data.shoulder - targetShoulder) > DEADBAND) targetShoulder = data.shoulder;
  if (abs(data.elbow - targetElbow) > DEADBAND) targetElbow = data.elbow;
}

void setup() {
  Serial.begin(115200);

  // Attach and LOCK to 0 immediately
  sBase.attach(13, 500, 2400);
  sShoulder.attach(12, 500, 2400);
  sElbow.attach(14, 500, 2400);
  
  sBase.write(0);
  sShoulder.write(0);
  sElbow.write(0);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver Online. Motors Parked at 0.");
}

void loop() {
  // Fluid Movement Logic: Current slides toward Target
  currBase     = (currBase * (1.0 - FILTER_STRENGTH)) + (targetBase * FILTER_STRENGTH);
  currShoulder = (currShoulder * (1.0 - FILTER_STRENGTH)) + (targetShoulder * FILTER_STRENGTH);
  currElbow    = (currElbow * (1.0 - FILTER_STRENGTH)) + (targetElbow * FILTER_STRENGTH);

  // Write finalized positions to physical motors
  sBase.write((int)currBase);
  sShoulder.write((int)currShoulder);
  sElbow.write((int)currElbow);

  delay(10); 
}
