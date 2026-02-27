const int xInput = A0;
const int yInput = A3;
const int zInput = A6;
const int buttonPin = 4;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis
/*int xRawMin = 2048;
int xRawMax = 2048;

int yRawMin = 2048;
int yRawMax = 2048;

int zRawMin = 2048;
int zRawMax = 2048;*/

//calib values

float xRawMin = 1535, xRawMax = 2368;
float yRawMin = 1626, yRawMax = 2347;
float zRawMin = 1499, zRawMax = 2339;

float basex;
float basey;
float basez;

int ins=0;
// Take multiple samples to reduce noise
const int sampleSize = 10;

void setup() 
{
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
}

void loop() 
{
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);
  
  if (digitalRead(buttonPin) == LOW)
  {
    AutoCalibrate(xRaw, yRaw, zRaw);
    ins=1;
  }
  else
  {
    Serial.print("Raw Ranges: X: ");
    Serial.print(xRawMin);
    Serial.print("-");
    Serial.print(xRawMax);
    
    Serial.print(", Y: ");
    Serial.print(yRawMin);
    Serial.print("-");
    Serial.print(yRawMax);
    
    Serial.print(", Z: ");
    Serial.print(zRawMin);
    Serial.print("-");
    Serial.print(zRawMax);
    Serial.println();
    Serial.print(xRaw);
    Serial.print(", ");
    Serial.print(yRaw);
    Serial.print(", ");
    Serial.print(zRaw);
    
    // Convert raw values to 'milli-Gs"
    long xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
    long yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
    long zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);
  
    // re-scale to fractional Gs
    float xAccel = xScaled / 1000.0;
    float yAccel = yScaled / 1000.0;
    float zAccel = zScaled / 1000.0;

    Serial.print(" :: ");
    Serial.print(xAccel,3);
    Serial.print("G, ");
    Serial.print(yAccel,3);
    Serial.print("G, ");
    Serial.print(zAccel,3);
    Serial.println("G");
    
    if (ins==1)
    {
     basex = xAccel;
     basey = yAccel;
     basez = zAccel;
     ins=0;
     Serial.print("basex: ");
     Serial.print(basex);
     Serial.print("basey: ");
     Serial.print(basey);
     Serial.print("basez: ");
     Serial.print(basez);
     delay(1000);
    }
    
    
     if((xScaled-basex)>0.02 || (yScaled-basey)>0.02 || (yScaled-basey)>0.02)
     {
     Serial.println("Earthquake");
     delay(1000);
     }
  delay(50);
  }
}

//
// Read "sampleSize" samples and report the average
//
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

//
// Find the extreme raw readings from each axis
//
void AutoCalibrate(int xRaw, int yRaw, int zRaw)
{
  Serial.println("Calibrate");
  if (xRaw < xRawMin)
  {
    xRawMin = xRaw;
  }
  if (xRaw > xRawMax)
  {
    xRawMax = xRaw;
  }
  
  if (yRaw < yRawMin)
  {
    yRawMin = yRaw;
  }
  if (yRaw > yRawMax)
  {
    yRawMax = yRaw;
  }

  if (zRaw < zRawMin)
  {
    zRawMin = zRaw;
  }
  if (zRaw > zRawMax)
  {
    zRawMax = zRaw;
  }
}