#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <vector>
#include <SD.h>
#include <SPI.h>
#include <RTClib.h>

//object creation
DateTime dt;
RTC_DS3231 rtc;
TwoWire rtcWire(1); //second I2C bus
Adafruit_MPU6050 mpu;

// 2D Vector to store sensor values for each axis
std::vector<std::vector<float>> vector_mpu;
std::vector<std::vector<float>> vector_adxl;

//time keepers here
const unsigned long pre_lim = 20000  ;
const unsigned long post_lim = 40000 ;
//DateTime startTime, postRec_Time;  //time
long int startTime, postRec_Time; 

//global sensor values
float ax_mpu, ay_mpu, az_mpu;
float ax_adxl, ay_adxl, az_adxl;
float ax_mpuScal, ay_mpuScal, az_mpuScal,
      ax_adxlScal, ay_adxlScal, az_adxlScal;

//calib values
float x_mpuMin = -9.75, x_mpuMax = 10.02; 
float y_mpuMin = -9.52, y_mpuMax = 10.09;
float z_mpuMin = -2.78, z_mpuMax = 17.13;

float x_adxlMin = 1535, x_adxlMax = 2368;
float y_adxlMin = 1626, y_adxlMax = 2347;
float z_adxlMin = 1499, z_adxlMax = 2339;

//base offset values
float x_mpuBase, y_mpuBase, z_mpuBase;
float x_adxlBase, y_adxlBase, z_adxlBase;

//ADXL analog pins
const int xInput = A0;
const int yInput = A3;
const int zInput = A6;
const int RED = 26, YEL = 32, GRE = 25, CAL = 16; 

//button pins
const int calButton = 4;

// Take multiple samples to reduce noise
const int sampleSize = 10;

//threshold 
float xThreshold = 20;
float yThreshold = 20;
float zThreshold = 20;

//initial flags
bool Th = false, Trig = false, 
     pre_rec = false, post_rec = false, 
     calibrate = true, start=true;
int warn = 0;

//SD card file handling
String nameStamp, mpuFileName, adxlFileName;
File mpuFile, adxlFile;



void vecAppend(float a_x, float a_y, float a_z,std::vector<std::vector<float>>& vecc){
  vecc.push_back({a_x, a_y, a_z});
  
  //Check if one minute has elapsed
  if (!pre_rec){
    //DateTime current = rtc.now();
    long int current = millis();
    //TimeSpan elapsed = current - startTime;
    //if ( (1000*(elapsed) >= pre_lim) {
    if ( (current - startTime) >= pre_lim) {
      //pre rec limit has been filled
      pre_rec = true;
    }
  }
  //Shift and delete the first entry if filled
  if(pre_rec && (!post_rec)){
    if (!vecc.empty()) {
      vecc.erase(vecc.begin());
    }
  }
}



void printVector(const std::vector<std::vector<float>>& vec) {
  for (const auto& values : vec) {
    Serial.print("X: "); Serial.print(values[0]); Serial.print(" ");
    Serial.print("Y: "); Serial.print(values[1]); Serial.print(" ");
    Serial.print("Z: "); Serial.println(values[2]);
  }
}



void SDfile_mpu(){

  //creating directory if ti doesnt exist
  if (!SD.exists("/MPU6050 readings")) {
    SD.mkdir("/MPU6050 readings");
  }
  //mpu file opening
  mpuFileName = String("/MPU6050 readings/") + nameStamp + ".csv";
  mpuFile = SD.open(mpuFileName, FILE_WRITE);
  // Check if the file opened successfully
  if (mpuFile) {
    Serial.println("File created: " + mpuFileName);
    // Write data to the file in CSV format
    for (const auto& values : vector_mpu) {
      mpuFile.print(values[0]); // X value
      mpuFile.print(",");
      mpuFile.print(values[1]); // Y value
      mpuFile.print(",");
      mpuFile.println(values[2]); // Z value
    }
    // Close the file 
    mpuFile.flush();
    mpuFile.close();
    if (!mpuFile.available()) {
      Serial.println(mpuFileName + " File is closed");
    }
  } 
  else {
    Serial.println("Error opening file: " + mpuFileName);
  }
}

void SDfile_adxl(){

  //opening adxl file
  if (!SD.exists("/ADXL345 readings")) {
    SD.mkdir("/ADXL345 readings");
  }
  // Initialize adxlFileName
  adxlFileName = String("/ADXL345 readings/") + nameStamp + ".csv";
  adxlFile = SD.open(adxlFileName, FILE_WRITE);
  // Check if the file opened successfully
  if (adxlFile) {
    Serial.println("File created: " + adxlFileName);
    // Write data to the file in CSV format
    for (const auto& values : vector_adxl) {
      adxlFile.print(values[0]); // X value
      adxlFile.print(",");
      adxlFile.print(values[1]); // Y value
      adxlFile.print(",");
      adxlFile.println(values[2]); // Z value
    }
    // Close the file 
    adxlFile.flush();
    adxlFile.close();
    if (!adxlFile.available()) {
      Serial.println(adxlFileName + " File is closed");
    }
  } 
  else {
    Serial.println("Error opening file: " + adxlFileName);
  }

}


//calibrate the axes
void data_read_all(){

  int sample = 1, i;
  float sum_xmpu = 0, sum_ympu = 0, sum_zmpu = 0, 
        sum_xadxl = 0, sum_yadxl = 0, sum_zadxl = 0;

  for(i=0; i<sample; ++i){
    // Get mpu readings 
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    //swapping the variables to match that of adxl x.y axis physically
    ax_mpu= -(a.acceleration.y);
    ay_mpu= a.acceleration.x;
    az_mpu= -(a.acceleration.z);

    //Get adxl readings
    ax_adxl = analogRead(xInput);
    ay_adxl = analogRead(yInput);
    az_adxl = analogRead(zInput);
    
    // Sum the readings for MPU
    sum_xmpu += ax_mpu;
    sum_ympu += ay_mpu;
    sum_zmpu += az_mpu;

    // Sum the readings for ADXL
    sum_xadxl += ax_adxl;
    sum_yadxl += ay_adxl;
    sum_zadxl += az_adxl;
    
    delayMicroseconds(20);
  }

  // Calculate the average for MPU readings
  ax_mpu = sum_xmpu / sample;
  ay_mpu = sum_ympu / sample;
  az_mpu = sum_zmpu / sample;

  // Calculate the average for ADXL readings
  ax_adxl = sum_xadxl / sample;
  ay_adxl = sum_yadxl / sample;
  az_adxl = sum_zadxl / sample;

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


void set_base(int numCalib = 1000){
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



void setup() {

  Serial.begin(115200);
  while (!Serial);
  delay(5000);
  Serial.println("\n\n\nStarting the setup babe");



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
  // Set the pin mode to INPUT for analog sensor pins
  pinMode(xInput, INPUT);
  pinMode(yInput, INPUT);
  pinMode(zInput, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(YEL, OUTPUT);
  pinMode(GRE, OUTPUT);
  pinMode(CAL, OUTPUT);

  //setting rtc
  rtcWire.begin(14,27); //SDA, SCL
  if (!rtc.begin(&rtcWire)) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("RTC module is set at "+ rtc.now().timestamp());
  nameStamp = String(rtc.now().timestamp()); 

  //SD card initialization
  if (!SD.begin(5)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card is initialized");

  //input pins
  pinMode(calButton, INPUT);

  //starting now babe
  Serial.println("Set up is done");
  //startTime = rtc.now();

}



void loop() {
  //DateTime test1 = rtc.now();

  if(calibrate || (!digitalRead(calButton))){
    //Calibration 
    digitalWrite(CAL,HIGH);
    digitalWrite (RED, LOW);
    digitalWrite(YEL, LOW);
    digitalWrite (GRE, LOW);
    set_base();
    Serial.println("Sensor offset calibration is done");
    calibrate = false;
    vector_mpu.clear();
    vector_adxl.clear();
    start = true;
    Trig = false;
    post_rec = false;
  }

  else{
    if(start){
      //startTime = rtc.now();
      startTime = millis();
      start = false;
    }
    data_read_all();
    data_scale();
     
    ax_mpuScal -= x_mpuBase;
    ay_mpuScal -= y_mpuBase;
    az_mpuScal -= z_mpuBase;
    ax_adxlScal -= x_adxlBase;
    ay_adxlScal -= y_adxlBase;
    az_adxlScal -= z_adxlBase;

    //saving in the running vector
    vecAppend(ax_mpuScal, ay_mpuScal, az_mpuScal, vector_mpu);
    vecAppend(ax_adxlScal, ay_adxlScal, az_adxlScal, vector_adxl);

    //comparing with threshold values
    if ((abs(ax_mpuScal) > xThreshold) || (abs(ay_mpuScal) > yThreshold) || (abs(az_mpuScal) > zThreshold) ||
        (abs(ax_adxlScal) > xThreshold) || (abs(ay_adxlScal) > yThreshold) || (abs(az_adxlScal) > zThreshold)){
      Th = true; 
    }
    else{
      Th = false;
    }

    //th and trig flag check babe
    ////////
    if(Trig) { //post lim running
      //Serial.println("RUNNING POST REC");

      //checking post rec time
      //DateTime current = rtc.now();
      long int current = millis();
      //TimeSpan elapsed = current - postRec_Time;
    
      //end of post lim if true
      if ( (current - postRec_Time) > post_lim){
        Trig = false;
        post_rec = false;

        //SD card cant save names with ":"
        nameStamp.replace(":", "_");
        nameStamp.replace("-", "_"); 

        //SD files to store data
        SDfile_mpu();
        SDfile_adxl();

        //clear vectors
        vector_mpu.clear();
        vector_adxl.clear();
        vector_mpu.shrink_to_fit();
        vector_adxl.shrink_to_fit();
        start = true;
        pre_rec = false;
        warn = 0;
        set_base(1000);
      }
    }
    ///////////
    else if(Th){ //starting the post lim now
      //namestamp creation
      nameStamp = String(rtc.now().timestamp()); 
      //Serial.println("OMYGOD TH");
      warn +=1;
      if(warn == 3){
        Trig = true; //Trigger ON
        post_rec = true;
        //postRec_Time = rtc.now();
        postRec_Time = millis();
      }
    }
    /////
    else{ //pre lim running
      //Serial.println("RUNNING PRE REC");  
      warn = 0;
    }
    //Serial.println("warn value:" + String(warn));
  }
  digitalWrite(CAL,LOW);
  if(warn == 3){
    digitalWrite (RED, HIGH);
    digitalWrite(YEL, LOW);
    digitalWrite (GRE, LOW);
  }
  else if(warn>0){
    digitalWrite (RED, LOW);
    digitalWrite(YEL, HIGH);
    digitalWrite (GRE, LOW);
  }
  else{
    digitalWrite (RED, LOW);
    digitalWrite(YEL, LOW);
    digitalWrite (GRE, HIGH);
  }
  //DateTime test2 = rtc.now();
  //Serial.print("Time for one loop run in S: ");
  //Serial.println((test2-test1).totalseconds());
  delay(30);
  //delay(3000);
}

  
  