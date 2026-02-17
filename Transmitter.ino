#include <Wire.h>
#include "MPU6050.h"
#include <MadgwickAHRS.h>
#include <esp_now.h>
#include <WiFi.h>

// ================= USER CONFIGURATION =================
float FILTER_STRENGTH = 0.02; // Adjust (0.01 to 0.2) for fluidity/speed
uint8_t robotMAC[] = {0xC0, 0xCD, 0xD6, 0x85, 0x44, 0xDC}; 

// ================= HARDWARE & VARS =================
MPU6050 imu(0x68); 
Madgwick filter;
float off_yaw=0, off_pitch=0, off_roll=0, g_x=0, g_y=0, g_z=0;

// Variables initialized at 0 to prevent startup jumps
float sm_base=0, sm_shoulder=0, sm_elbow=0;

typedef struct struct_message {
  int base, shoulder, elbow, wrist, gripper;
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(400000);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
    esp_now_register_send_cb(OnDataSent);
    memcpy(peerInfo.peer_addr, robotMAC, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  imu.initialize();
  filter.begin(100); 

  // GYRO CALIBRATION
  Serial.println("STEP 1: Calibrating Gyro... DO NOT MOVE.");
  long wx=0, wy=0, wz=0;
  for (int i=0; i<200; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    wx+=gx; wy+=gy; wz+=gz;
    delay(2);
  }
  g_x=wx/200.0; g_y=wy/200.0; g_z=wz/200.0;

  // POSITION CALIBRATION
  Serial.println("STEP 2: Point hand toward the GROUND.");
  Serial.println("Zeroing in 3... 2... 1...");
  delay(3000); 
  
  for(int k=0; k<100; k++) { readSensors(); delay(5); }
  off_yaw = filter.getYaw(); off_pitch = filter.getPitch(); off_roll = filter.getRoll();
  Serial.println("Glove Ready!");
}

void readSensors() {
  int16_t ax, ay, az, gx, gy, gz;
  imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  filter.updateIMU((gx-g_x)*DEG_TO_RAD, (gy-g_y)*DEG_TO_RAD, (gz-g_z)*DEG_TO_RAD, ax, ay, az);
}

void loop() {
  readSensors();

  // Mapping current hand position relative to the Ground-Point
  float target_b = map(filter.getYaw() - off_yaw, -45, 45, 0, 90);
  float target_s = map(filter.getPitch() - off_pitch, 0, 90, 0, 90); 
  float target_e = map(filter.getRoll() - off_roll, -45, 45, 0, 90);

  // Fluid Exponential Filter Logic
  sm_base     = (sm_base * (1.0 - FILTER_STRENGTH)) + (constrain(target_b, 0, 90) * FILTER_STRENGTH);
  sm_shoulder = (sm_shoulder * (1.0 - FILTER_STRENGTH)) + (constrain(target_s, 0, 90) * FILTER_STRENGTH);
  sm_elbow    = (sm_elbow * (1.0 - FILTER_STRENGTH)) + (constrain(target_e, 0, 90) * FILTER_STRENGTH);

  myData.base = (int)sm_base;
  myData.shoulder = (int)sm_shoulder;
  myData.elbow = (int)sm_elbow;
  myData.wrist = 90; 
  myData.gripper = 90;

  esp_now_send(robotMAC, (uint8_t *) &myData, sizeof(myData));
  
  // Debug to Serial Monitor
  Serial.print("B:"); Serial.print(myData.base);
  Serial.print(" S:"); Serial.print(myData.shoulder);
  Serial.print(" E:"); Serial.println(myData.elbow);
  
  delay(10); 
}
