#include <SD.h>
String fileName;
File dataFile;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  delay(5000);
  Serial.println("Starting the setup babe");
  //SD card initialization
  if (!SD.begin(5)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card is initialized");

}

void loop() {
  // put your main code here, to run repeatedly:
  SDfile_GenCopy();
  while(1);

}

void SDfile_GenCopy(){
  
  fileName = "nomnom.csv"; //CSV format
  fileName = "/MPU6050 readings/" +fileName;
  dataFile = SD.open(fileName, FILE_WRITE);

  // Check if the file opened successfully
  if (dataFile) {
    Serial.println("File created: " + fileName);
  } 
  
  else {
    Serial.println("Error opening file: " + fileName);
  }
  dataFile.close();

}