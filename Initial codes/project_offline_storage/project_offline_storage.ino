#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <vector>

Adafruit_MPU6050 mpu;

const int sample_delay = 5000;  
const unsigned long pre_lim = 60000 ;
const unsigned long post_lim = 120000 ;
unsigned long startTime;  //start-time

float ax, ay, az;
float ax_offset, ay_offset, az_offset;
sensors_event_t a, g, temp;

//initial flags
bool Th = false;
bool Trig = false;
bool pre_rec = false;

//creating a prototype, don't ask questions
void calibrateMPU6050(); 
//void printVector(const std::vector<std::vector<float>>& vec);

// 2D Vector to store sensor values for each axis
std::vector<std::vector<float>> sensorValues;

void setup() {
  Serial.begin(115200); //does this rate matter?
  while (!Serial){
    delay(10); 
    // will pause Zero, Leonardo, etc until serial console opens
  }

  //initialize mpu sensor
  if (!mpu.begin()) {
  Serial.println("Failed to initialize MPU6050");
  //to avoid hazard
  while (1); {
    delay(10);
  } 

  //initialize SD 

  // Set the accelerometer full-scale range to ±8g
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  // Set the gyro full-scale rangE
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // Set the bandwidth of the Digital Low-Pass Filter
  //260 HZ disables the filter
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  delay(100);
  calibrateMPU6050();

  //threshold setup
  //xThreshold =
  //yThreshold =
  //zThreshold =

  startTime = millis(); //starting now babe
  }
}

void loop() {

  //first read the data from sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  ax= a.acceleration.x - ax_offset ;
  ay= a.acceleration.y - ay_offset ;
  az= a.acceleration.z - az_offset ;

  //comparing with threshold values
  /*if(!Trig){
    if ((abs(ax) > xThreshold) || (abs(ay) > yThreshold) || (abs(az) > zThreshold)){
      Th = true;
    } 
  }*/

  //th and trig flag check
  if (Trig){ //post lim running
    //append file and vector till post_lim
  }
  else if(Th){ //starting the post lim now
    //
  }
  else{ //pre lim running
    //append array till pre_lim
    arrayappend(ax,ay,az);
  }

  //sample print 2D vector in serial monitor
  //printVector(sensorValues);
  Serial.println("offset");
  Serial.print(ax_offset);
  Serial.print("  ");
  Serial.print(ay_offset);
  Serial.print("  ");
  Serial.println(az_offset);
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

  Serial.println("offset from calib func");
  Serial.print(ax_offset);
  Serial.print("  ");
  Serial.print(ay_offset);
  Serial.print("  ");
  Serial.println(az_offset);
  delay(5000);
 
}

void arrayappend(float a_x, float a_y, float a_z){
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





