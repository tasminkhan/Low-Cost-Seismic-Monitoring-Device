#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>

Adafruit_MPU6050 mpu;

//global sensor values
float ax_mpu, ay_mpu, az_mpu;
float ax_adxl, ay_adxl, az_adxl;

float ax_mpuScal, ay_mpuScal, az_mpuScal,
      ax_adxlScal, ay_adxlScal, az_adxlScal;

//calib values
/*float x_mpuMin, x_mpuMax, y_mpuMin, y_mpuMax, z_mpuMin, z_mpuMax;
float x_adxlMin = 2048, x_adxlMax = 2048; 
float y_adxlMin = 2048, y_adxlMax = 2048;
float z_adxlMin = 2048, z_adxlMax = 2048;*/

//calib values
float x_mpuMin = -9.52, x_mpuMax = 10.09; 
float y_mpuMin = -9.75, y_mpuMax = 10.02;
float z_mpuMin = -2.78, z_mpuMax = 17.13;
//float x_adxlMin = 1535, x_adxlMax = 2368;
//float y_adxlMin = 1626, y_adxlMax = 2347;
//float z_adxlMin = 1499, z_adxlMax = 2339;

//base offset values
float x_mpuBase, y_mpuBase, z_mpuBase;
float x_adxlBase, y_adxlBase, z_adxlBase;

//ADXL analog pins
const int xInput = A0;
const int yInput = A3;
const int zInput = A6;

//button pins
const int calButton = 4, forcedButton = 15;

//initial flags
bool calibrate = true;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(5000);
  Serial.println("Starting the setup for calibration");

  
  // mpu initialization
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1);
  }
  Serial.println("MPU6050 is Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  Serial.print("Accelerometer range is set to: ");
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
  Serial.print("Gyro range is set to: ");
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
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth is set to: ");
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
  Serial.println("Check connection to ADXL once again.");


}

void loop() {

  if(calibrate || (!digitalRead(calButton))){
    //Calibration 
    Serial.println("Starting the calibration\n...It might take a while ...");
    //autocalib();
    set_base();
    Serial.println("Sensor calibration is done");
    calibrate = false;
  }
  else{
    data_read_all();
    data_scale();

    ax_mpuScal -= x_mpuBase;
    ay_mpuScal -= y_mpuBase;
    az_mpuScal -= z_mpuBase;

    ax_adxlScal -= x_adxlBase;
    ay_adxlScal -= y_adxlBase;
    az_adxlScal -= z_adxlBase;

    Serial.print("\n\nSample values for mpu are ");
    Serial.println("x_mpu: " + String(ax_mpuScal) + ", y_mpu: " + String(ay_mpuScal) + ", z_mpu: " + String(az_mpuScal));
    Serial.print("Sample values for adxl are ");
    Serial.println("x_adxl: " + String(ax_adxlScal) + ", y_adxl: " + String(ay_adxlScal) + ", z_adxl: " + String(az_adxlScal));
    
    Serial.println("Raw MPU values: x: " + String(ax_mpu) + ", y: " + String(ay_mpu) + ", z: " + String(az_mpu));
    Serial.println("Raw ADXL values: x: " + String(ax_adxl) + ", y: " + String(ay_adxl) + ", z: " + String(az_adxl));
    delay(3000);
  }
}



void calib_all(){

  if (ax_mpu < x_mpuMin)  { x_mpuMin = ax_mpu; }
  if (ax_mpu > x_mpuMax)  { x_mpuMax = ax_mpu; }
  if (ay_mpu < y_mpuMin)  { y_mpuMin = ay_mpu; }
  if (ay_mpu > y_mpuMax)  { y_mpuMax = ay_mpu; }
  if (az_mpu < z_mpuMin)  { z_mpuMin = az_mpu; }
  if (az_mpu > z_mpuMax)  { z_mpuMax = az_mpu; }

  if (ax_adxl < x_adxlMin)  { x_adxlMin = ax_adxl; }
  if (ax_adxl > x_adxlMax)  { x_adxlMax = ax_adxl; }
  if (ay_adxl < y_adxlMin)  { y_adxlMin = ay_adxl; }
  if (ay_adxl > y_adxlMax)  { y_adxlMax = ay_adxl; }
  if (az_adxl < z_adxlMin)  { z_adxlMin = az_adxl; }
  if (az_adxl > z_adxlMax)  { z_adxlMax = az_adxl; }
 
}


void autocalib(){
  const int numCalib = 20;
  double ax_mpuSum = 0, ay_mpuSum = 0, az_mpuSum = 0,
         ax_adxlSum = 0, ay_adxlSum = 0, az_adxlSum = 0;
  int i;
  float ax_mpuScal, ay_mpuScal, az_mpuScal,
        ax_adxlScal, ay_adxlScal, az_adxlScal;


  Serial.print("Please turn the board sideways and press the button for pos x. ");
  while (digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }

  Serial.print("Please turn the board sideways and press the button for neg x. ");
  while (digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }

  Serial.print("Please turn the board sideways and press the button for pos y. ");
  while (digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }


  Serial.print("Please turn the board sideways and press the button for neg y. ");
  while (digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }
  
  Serial.print("Please turn the board over and press the button for neg z. ");
  while (digitalRead(calButton));
  delay(10);
  while (!digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }

  Serial.print("Please keep the board flat and press the button for pos z. ");
  while (digitalRead(calButton));
  Serial.println("Button high");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    calib_all();
    delay(100);
  }

  Serial.println("Maximum MPU values: x: " + String(x_mpuMax) + ", y: " + String(y_mpuMax) + ", z: " + String(z_mpuMax));
  Serial.println("Maximum ADXL values: x: " + String(x_adxlMax) + ", y: " + String(y_adxlMax) + ", z: " + String(z_adxlMax));
  Serial.println("Minimum MPU values: x: " + String(x_mpuMin) + ", y: " + String(y_mpuMin) + ", z: " + String(z_mpuMin));
  Serial.println("Minimum ADXL values: x: " + String(x_adxlMin) + ", y: " + String(y_adxlMin) + ", z: " + String(z_adxlMin));
  
}

void set_base(){
  const int numCalib = 100;
  int i;
  double ax_mpuSum = 0, ay_mpuSum = 0, az_mpuSum = 0,
         ax_adxlSum = 0, ay_adxlSum = 0, az_adxlSum = 0;

  Serial.println("...Please wait for base offset set up...");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    data_scale();

    ax_mpuSum += ax_mpuScal; 
    ay_mpuSum += ay_mpuScal;
    az_mpuSum += az_mpuScal;

    ax_adxlSum += ax_adxlScal; 
    ay_adxlSum += ay_adxlScal;
    az_adxlSum += az_adxlScal;

    delay(10);
  }

  x_mpuBase = ax_mpuSum / numCalib;
  y_mpuBase = ay_mpuSum / numCalib;
  z_mpuBase = az_mpuSum / numCalib;

  x_adxlBase = ax_adxlSum / numCalib;
  y_adxlBase = ay_adxlSum / numCalib;
  z_adxlBase = az_adxlSum / numCalib;

  Serial.print("Base values for mpu are ");
  Serial.println("x_mpuBase: " + String(x_mpuBase) + ", y_mpuBase: " + String(y_mpuBase) + ", z_mpuBase: " + String(z_mpuBase));
  Serial.print("Base values for adxl are ");
  Serial.println("x_adxlBase: " + String(x_adxlBase) + ", y_adxlBase: " + String(y_adxlBase) + ", z_adxlBase: " + String(z_adxlBase));
}

// Custom lerp function that extends the range dynamically
float lerpExtend(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
    // Determine the range of the input value
    float inputRange = inputMax - inputMin;
    // Map the input value to the range [0, 1] based on the extended range
    float t = (value - inputMin) / inputRange;
    
    // Perform linear interpolation within the extended range
    float result = outputMin + t * (outputMax - outputMin);
    return result;
}

//calibrate the axes
void data_read_all(){

  // Get mpu readings 
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  ax_mpu= a.acceleration.x;
  ay_mpu= a.acceleration.y;
  az_mpu= a.acceleration.z;

  //Get adxl readings
  ax_adxl = analogRead(xInput);
  ay_adxl = analogRead(yInput);
  az_adxl = analogRead(zInput);

}



void data_scale(){
  //value scaling:: values are expressed in milli-G unit
  //using map for mpu reduces its senstivity because map deals in integer long
  //using lerpextend for adxl makes the value stuck at a high range
  ax_mpuScal = lerpExtend(ax_mpu, x_mpuMin, x_mpuMax, -1000, 1000);
  ay_mpuScal = lerpExtend(ay_mpu, y_mpuMin, y_mpuMax, -1000, 1000);    
  az_mpuScal = lerpExtend(az_mpu, z_mpuMin, z_mpuMax, -1000, 1000);

  ax_adxlScal = map(ax_adxl, x_adxlMin, x_adxlMax, -1000, 1000);    
  ay_adxlScal = map(ay_adxl, y_adxlMin, y_adxlMax, -1000, 1000);
  az_adxlScal = map(az_adxl, z_adxlMin, z_adxlMax, -1000, 1000);

}