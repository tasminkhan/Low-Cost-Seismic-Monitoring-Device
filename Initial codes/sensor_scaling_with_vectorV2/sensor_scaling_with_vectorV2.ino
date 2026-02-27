// Basic demo for accelerometer readings from Adafruit MPU6050
// ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
// Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <vector>
#include <DS3231.h> 
#include <SD.h>
#include <SPI.h>

Adafruit_MPU6050 mpu;
DS3231 clocky;
RTCDateTime dt;

//time keepers here
const unsigned long pre_lim = 40000 ;
const unsigned long post_lim = 40000 ;
unsigned long startTime, postRec;  //time

//global sensor values
long ax, ay, az;
long ax_offset, ay_offset, az_offset;
sensors_event_t a, g, temp;

//threshold 
const unsigned long G = 9.81;
unsigned long xThreshold = 0.01*G;
unsigned long yThreshold = 0.01*G;
unsigned long zThreshold = 0.01*G;

//initial flags
bool Th = false;
bool Trig = false;
bool pre_rec = false;

//creating a prototype, don't ask questions 
void calibrateMPU6050(); 
void printVector(const std::vector<std::vector<float>>& vec);

// 2D Vector to store sensor values for each axis
std::vector<std::vector<float>> sensorValues;

//SD card file handling
String fileName;
File dataFile;

void setup() {
  Serial.begin(115200);
  while (!Serial){
    delay(10);
  } // will pause Zero, Leonardo, etc until serial console opens

  //RTC module initialization
  Wire.begin(27,14); //SDA. SCL
  clocky.begin();
  clocky.setDateTime(__DATE__, __TIME__);
  //String startup = timestring();

  //Serial.println("Setup starting at "+ startup);

  // mpu initialization
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  delay(100);
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  // Set the gyro full-scale range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // Set the bandwidth of the Digital Low-Pass Filter
  //260 HZ disables the filter
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  delay(100);
  calibrateMPU6050();
  Serial.println("Sensor values are calibrated sonny");

  //SD card initialization
  SD.begin();
  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized");

  startTime = millis();
  Serial.print("Set up Done \n"); //starting now babe
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
  if ((abs(ax) > xThreshold) || (abs(ay) > yThreshold) || (abs(az) > zThreshold)){
    Th = true; 
    } 
  else{
    Th = false;
    }

  //th and trig flag check babe

  if(Trig){ //post lim running
    Serial.print("RUNNING POST REC \n");
    //SDfile_Append();

    //checking post rec time
    if(millis() - postRec >= post_lim){
      Trig = false;}/*
      dataFile.close();
      Serial.println("File closed.");
    }*/
  }

  else if(Th){ //starting the post lim now
    Trig = true; //Trigger ON
    postRec = millis();
    Serial.print("OMYGOD TH \n");
    //generate file
    //SDfile_GenCopy();
  }

  else{ //pre lim running
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

void SDfile_GenCopy(){
  fileName = timestring() + ".csv"; //CSV format
  /*dataFile = SD.open(fileName, FILE_WRITE);

  // Check if the file opened successfully
  if (dataFile) {
    Serial.println("File created: " + fileName);
    // Write data to the file in CSV format
    for (const auto& values : sensorValues) {
      dataFile.print(values[0]); // X value
      dataFile.print(",");
      dataFile.print(values[1]); // Y value
      dataFile.print(",");
      dataFile.println(values[2]); // Z value
    }
  } 
  
  else {
    Serial.println("Error opening file: " + fileName);
  }*/
}

String timestring(){
  dt = clocky.getDateTime();
  String dtString = String(dt.year) + "-" +
                    String(dt.month) + "-" +
                    String(dt.day) + "-" +
                    String(dt.hour) + "-" +
                    String(dt.minute) + "-" +
                    String(dt.second);
  Serial.println("time string generated " + dtString);
  return dtString;
}

void SDfile_Append(){
  if (dataFile) {
        Serial.println("Appending file " + fileName);

        // Write new data to the file in CSV format
        dataFile.print(ax); // X value
        dataFile.print(",");
        dataFile.print(ay); // Y value
        dataFile.print(",");
        dataFile.println(az); // Z value
  }
  else {
    Serial.println("Error appending file: " + fileName);
  }
}



