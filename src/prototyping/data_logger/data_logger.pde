/**
 * Data Logger
 *
 * Read Derial Data transmitted from Arduino
 *
 * 's' and saves a file if buffer is incomplete
 * 'w' key will be sent over Serial to allow for easy restart
 */


import processing.serial.*;
import java.nio.*;

Serial arduino;  // Create object from Serial class
int val;      // Data received from the serial port
final int total_bytes = 25000 * 4 * 2; 
byte[] readings = new byte[total_bytes];      // Data received from the serial port
int wh = 0; // write head index;
int fileCount  = 0;
int startTime, endTime;
int pix=0;
void setup()
{
  size(1000, 200);
  String portName = Serial.list()[1];
  arduino = new Serial(this, portName, 230400);
  startTime = millis();




  printArray(Serial.list());
  background(0);
}

void draw()
{
  background(0);
  while (arduino.available() > 0)
  {
    readings[wh] = byte(arduino.read());

    wh++;

    if (wh == readings.length)
    {
      writeToFile();
      arduino.clear();
    }
  }
  textAlign(CENTER, CENTER);
  text("bytes read: " + str(wh) + "\nFiles Written: " + str(fileCount) + "\nArduino Available: " + str(arduino.available()), width/2, height/2);
}

void  writeToFile()
{
  File f;
  String filename;
  do {
    filename = "data/log-" + nf(fileCount, 2) + ".dat";
    f = new File(sketchPath(filename));
    fileCount++;
  } while (f.exists());

  saveBytes(filename, readings);
  
  wh = 0;
  println(filename);
}


void keyPressed()
{
  if (key == 's')
  {
    writeToFile();
  }
  else if(key == 'w')
  {
    arduino.write('w');
  }
}
