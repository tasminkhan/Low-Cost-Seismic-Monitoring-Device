// Basic demo for accelerometer readings from Adafruit MPU6050

// ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
// Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <vector>

Adafruit_MPU6050 mpu;

const int sample_delay = 5000;  
const unsigned long pre_lim = 30000 ;
const unsigned long post_lim = 60000 ;
unsigned long startTime, postRec;  //time

long ax, ay, az;
long ax_offset, ay_offset, az_offset;
sensors_event_t a, g, temp;

//threshold 
float G = 9.81;
long xThreshold = 0.02*G;
long yThreshold = 0.02*G;
long zThreshold = 0.02*G;

//initial flags
bool Th = false;
bool Trig = false;
bool pre_rec = false;

//creating a prototype, don't ask questions 
void calibrateMPU6050(); 
void printVector(const std::vector<std::vector<float>>& vec);

// 2D Vector to store sensor values for each axis
std::vector<std::vector<float>> sensorValues;

void setup(void) {
  Serial.begin(115200);
  while (!Serial){
    delay(10);
  } // will pause Zero, Leonardo, etc until serial console opens
  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  // Set the gyro full-scale range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }
  // Set the bandwidth of the Digital Low-Pass Filter
  //260 HZ disables the filter
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  delay(100);
  calibrateMPU6050();

  Serial.println("");
  delay(100);

  startTime = millis();
  Serial.print("Set up done"); //starting now babe
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax= a.acceleration.x - ax_offset ;
  ay= a.acceleration.y - ay_offset ;
  az= a.acceleration.z - az_offset ;

  vecAppend(ax,ay,az);

  //comparing with threshold values
  if(!Trig){
    if ((abs(ax) > xThreshold) || (abs(ay) > yThreshold) || (abs(az) > zThreshold)){
      Th = true; 
    } 
  }

  //th and trig flag check
  if(Trig){ //post lim running
    Serial.print("RUNNING POST REC \n");
    //SDfile.Append();
    if(millis() - postRec >= post_lim){
      Trig = false;
      //SDfile_close();
    }
  }

  else if(Th){ //starting the post lim now
    //Trigger ON
    Trig = true;
    postRec = millis();
    Serial.print("OMYGOD TH \n");
    //generate file
    //SDfile_GenCopy();
  }

  else{ //pre lim running
    //append array till pre_lim
    Serial.print("RUNNING PRE REC \n");  
  }

  //sample print 2D vector in serial monitor
  printVector(sensorValues);
  delay(5000);
}

void calibrateMPU6050() {
  const int numSamples = 1000;

  // Variables to store cumulative values
  long ax_sum = 0, ay_sum = 0, az_sum = 0;

  for (int i = 0; i < numSamples; ++i) {
    // Read accelerometer and gyroscope data
    mpu.getEvent(&a, &g, &temp);
    
    ax= a.acceleration.x;
    ay= a.acceleration.y;
    az= a.acceleration.z;

    // Accumulate values
    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;

    delay(10);
  }

  // Calculate offsets
  ax_offset = ax_sum / numSamples;
  ay_offset = ay_sum / numSamples;
  az_offset = az_sum / numSamples;

}

void vecAppend(float a_x, float a_y, float a_z){
    sensorValues.push_back({a_x, a_y, a_z});

    //Check if one minute has elapsed
    if (!pre_rec){
     if (millis() - startTime >= pre_lim) {
        //pre rec limit has been filled
        pre_rec = true;
      }
    }

    //Shift and delete the first entry if filled
    if(pre_rec){
      if (!sensorValues.empty()) {
        sensorValues.erase(sensorValues.begin());
      }
    }
}
    

void printVector(const std::vector<std::vector<float>>& vec) {
  Serial.println("vector print check");
  for (const auto& values : vec) {
    Serial.print("X: "); Serial.print(values[0]); Serial.print(" ");
    Serial.print("Y: "); Serial.print(values[1]); Serial.print(" ");
    Serial.print("Z: "); Serial.println(values[2]);
  }
}