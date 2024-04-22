#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float temperature;
float beatsPerMinute;
int beatAvg;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

TaskHandle_t Core0Task;
TaskHandle_t Core1Task;

/* ============================================================= */

class Stats
{
  private:
  float bpm, temp, humid, gas;

  public:
  int GetBPM()
  {
    return (int)bpm;
  }

  void SetBPM(float bpm)
  {
    this->bpm = bpm;
  }

  int GetTemp()
  {
    return (int)bpm;
  }

  void SetTemp(float temp)
  {
    this->temp = temp;
  }

  int GetHumid()
  {
    return (int)bpm;
  }

  void SetHumid(float temp)
  {
    this->temp = temp;
  }

  int GetGas()
  {
    return (int)bpm;
  }

  void SetGas(float temp)
  {
    this->temp = temp;
  }
};

class LCD
{
  private:
  Stats stats;

  public:
  void ShowO2Stats()
  {
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("BPM: ");
    lcd.print(stats.GetBPM());
    
    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(stats.GetTemp());
    lcd.print(" C");

    delay(10);
  }

  void ShowEnvStats()
  {
    
  }

  void Init()
  {
    lcd.init();
  }

  void Update(Stats stats)
  {
    this->stats = stats;
  }
};

LCD display;

/* ============================================================= */

void StartMAX30102()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void StartTasks()
{
  xTaskCreatePinnedToCore(
    codeForCore0Task,
    "Core 0 task",
    10000,
    NULL,
    1,
    &Core0Task,
    0);


  // Set up Core 1 task handler
  xTaskCreatePinnedToCore(
    codeForCore1Task,
    "Core 1 task",
    10000,
    NULL,
    1,
    &Core1Task,
    1);
}

void setup()
{
  StartMAX30102();
  display.Init();
  StartTasks();
}

/* ============================================================================================== */

void RunMAX30102()
{
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  /*------------------------------*/
  Serial.print(particleSensor.getRed());
  Serial.print(", ");
  Serial.println(particleSensor.getIR());
}

float IIRFilter(float prev, float curr)
{
  float res = 0;
  int highpass;
  int lowpass;
  return res;
}

void codeForCore0Task(void *parameter)
{
    for (;;)
    {
        RunMAX30102();
    }
}


void codeForCore1Task(void *parameter)
{
    for (;;)
    {
        temperature = particleSensor.readTemperature();
    }
}

void loop()
{

}
