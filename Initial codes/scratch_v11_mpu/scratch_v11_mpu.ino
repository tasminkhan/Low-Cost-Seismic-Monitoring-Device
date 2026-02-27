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

//time keepers here
const unsigned long pre_lim = 30000 ;
const unsigned long post_lim = 90000 ;
//DateTime startTime, postRec_Time;  //time
long int startTime, postRec_Time; 

//global sensor values
float ax_mpu, ay_mpu, az_mpu;
//float ax_adxl, ay_adxl, az_adxl;
float ax_mpuScal, ay_mpuScal, az_mpuScal;

//calib values
float x_mpuMin = -9.75, x_mpuMax = 10.02; 
float y_mpuMin = -9.52, y_mpuMax = 10.09;
float z_mpuMin = -2.78, z_mpuMax = 17.13;

//base offset values
float x_mpuBase, y_mpuBase, z_mpuBase;

const int RED = 26, YEL = 32, GRE = 25, CAL = 16; 

//button pins
const int calButton = 4;

//threshold 
float xThreshold = 15;
float yThreshold = 15;
float zThreshold = 15;

//initial flags
bool Th = false, Trig = false, 
     pre_rec = false, post_rec = false, 
     calibrate = true, start=true;
int warn = 0;

//SD card file handling
String nameStamp, mpuFileName; //adxlFileName;
File mpuFile; //adxlFile;

void vecAppend(float a_x, float a_y, float a_z,std::vector<std::vector<float>>& vecc){
  vecc.push_back({a_x, a_y, a_z});
  
  if (!pre_rec){
    //DateTime current = rtc.now();
    long int current = millis();
    //TimeSpan elapsed = current - startTime;
    //if ( (1000*(elapsed) >= pre_lim) {
    if ( (current - startTime) >= pre_lim) {
      pre_rec = true;
    }
  }
  if(pre_rec && (!post_rec)){
    if (!vecc.empty()) {
      vecc.erase(vecc.begin());
    }
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
      mpuFile.print(values[0]); 
      mpuFile.print(",");
      mpuFile.print(values[1]); 
      mpuFile.print(",");
      mpuFile.println(values[2]); 
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


void data_read_all(){

  int sample = 3, i;
  float sum_xmpu = 0, sum_ympu = 0, sum_zmpu = 0;

  for(i=0; i<sample; ++i){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    ax_mpu= -(a.acceleration.y);
    ay_mpu= a.acceleration.x;
    az_mpu= -(a.acceleration.z);

    sum_xmpu += ax_mpu;
    sum_ympu += ay_mpu;
    sum_zmpu += az_mpu;
    
    delayMicroseconds(20);
  }
  ax_mpu = sum_xmpu / sample;
  ay_mpu = sum_ympu / sample;
  az_mpu = sum_zmpu / sample;

}


// Custom lerp function that extends the range dynamically
float lerpExtend(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
    float inputRange = inputMax - inputMin;
    float t = (value - inputMin) / inputRange;
    float result = outputMin + t * (outputMax - outputMin);
    return result;
}


void set_base(int numCalib = 1000){
  int i;
  double ax_mpuSum = 0, ay_mpuSum = 0, az_mpuSum = 0;


  Serial.println("...Please wait for base offset set up...");
  delay(3000);
  for(i=0; i<numCalib; ++i){
    data_read_all();
    data_scale();

    ax_mpuSum += ax_mpuScal; 
    ay_mpuSum += ay_mpuScal;
    az_mpuSum += az_mpuScal;

    delay(5);
  }

  x_mpuBase = ax_mpuSum / numCalib;
  y_mpuBase = ay_mpuSum / numCalib;
  z_mpuBase = az_mpuSum / numCalib;

  Serial.println("Base values for mpu are x_mpuBase: " + String(x_mpuBase) + ", y_mpuBase: " + String(y_mpuBase) + ", z_mpuBase: " + String(z_mpuBase));

}

void data_scale(){
  //value scaling:: values are expressed in milli-G unit
  //using map for mpu reduces its senstivity because map deals in integer long
  //using lerpextend for adxl makes the value stuck at a high range
  ax_mpuScal = lerpExtend(ax_mpu, x_mpuMin, x_mpuMax, -1000, 1000);
  ay_mpuScal = lerpExtend(ay_mpu, y_mpuMin, y_mpuMax, -1000, 1000);    
  az_mpuScal = lerpExtend(az_mpu, z_mpuMin, z_mpuMax, -1000, 1000);
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
  // Set the gyro full-scale range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  //260 HZ disables the filter
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

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
  
    //saving in the running vector
    vecAppend(ax_mpuScal, ay_mpuScal, az_mpuScal, vector_mpu);

    //comparing with threshold values
    if ((abs(ax_mpuScal) > xThreshold) || (abs(ay_mpuScal) > yThreshold) || (abs(az_mpuScal) > zThreshold)){
      Th = true; 
    }
    else{
      Th = false;
    }

    if(Trig) {
      //DateTime current = rtc.now();
      long int current = millis();
      //TimeSpan elapsed = current - postRec_Time;
        if ( (current - postRec_Time) > post_lim){
        Trig = false;
        post_rec = false;
        //SD card cant save names with ":"
        nameStamp.replace(":", "_");
        nameStamp.replace("-", "_"); 
        SDfile_mpu();
        //clear vectors
        vector_mpu.clear();
        vector_mpu.shrink_to_fit();
        start = true;
        pre_rec = false;
        warn = 0;
        set_base(100);
      }
    }
    else if(Th){ 
      
      nameStamp = String(rtc.now().timestamp()); 
      warn +=1;
      if(warn == 2){
        Trig = true;
        post_rec = true;
        //postRec_Time = rtc.now();
        postRec_Time = millis();
      }
    }
    else{ 
      warn = 0;
    }
  
  }
  digitalWrite(CAL,LOW);
  if(warn == 2){
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
  delay(24);
}

  
  