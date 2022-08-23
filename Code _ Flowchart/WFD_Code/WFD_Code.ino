#include "LeptonFLiR.h"
#include "digitalWriteFast.h"

const byte flirCSPin = 23; //Chip select pin
float val = 0, avg = 0;
int a = 0, c = 0;
float temp_f = 0;
float b = 0; //Calibration factor adjusted based on initial testing 
int coldX, coldY; 

LeptonFLiR flirController(Wire, flirCSPin); // Using wire library for ESP32


static void fastEnableCS(byte pin) { digitalWriteFast(pin, LOW); }
static void fastDisableCS(byte pin) { digitalWriteFast(pin, HIGH); }

void setup() 
{
    Serial.begin(115200); //Higher baud rates req for FLIR data transfer
    Wire.begin();                      
    Wire.setClock(400000);  // Supported baud rates are 100kHz, 400kHz, and 1000kHz
    SPI.begin();                        
    flirController.init();
    flirController.setFastCSFuncs(fastEnableCS, fastDisableCS);
    flirController.sys_setTelemetryEnabled(ENABLED); // Enabling telemetry mode
}

void loop() 
{
    if (flirController.readNextFrame()) 
    {
        for (int y = 0; y < flirController.getImageHeight(); ++y) 
        {
            for (int x = 0; x < flirController.getImageWidth(); ++x) 
            {
                val = val + flirController.getImageDataRowCol(y, x); //adds all temp data from each pixel
                a = a+1; //counts no of pixels in captured frame
            }
        }
        avg = val/a; //Average temperature of captured frame
        temp_f = avg + b; //Average temperature after calibration factor added

        for (int y = 0; y < flirController.getImageHeight(); ++y) 
        {
            for (int x = 0; x < flirController.getImageWidth(); ++x) 
            {
                float temp = flirController.getImageDataRowCol(y, x);
                if (abs(temp - temp_f) < 3)
                {
                  coldX = x; coldY = y; //Water spill coordinates
                  c = c+1; //To check if water spill is sigificant - counts no of pixels 
                }
            }
        }
        if (c > 100)
        {
          Serial.println("Water Spill Detected");
          Serial.println("Last location of water spill: ");
          Serial.print("X Coordinate - ");
          Serial.println(coldX);
          Serial.print("Y Coordinate - ");
          Serial.println(coldY);
        }
    }
    c = 0; a = 0; val = 0; 
}
