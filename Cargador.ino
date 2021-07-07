/*
  Potenciometro neceario para fuente de 120A --> 1KOhms
  Potenciometro neceario para fuente de 60A ---> 5Kohms
*/
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include <Wire.h>

/*********TFT**********/
TFT_eSPI tft = TFT_eSPI();//VCC 3.3V, GND, CS = 17, RESET = EN, DC = 16, MOSI = 23, SCK = 18, LED = 3.3V, MISO = 19 , T-CS= 27

int x_touch_Max;
int x_touch_Min;

int y_touch_Max;
int y_touch_Min;

uint16_t x, y, z;    //Cordenadas de la pulsacion, Z, presion

int xTitulo = 100;
int yTitulo =  25;
bool Flag_Tactil_Carga = 0;
bool Flag_Tactil_Descarga = 0;
bool Presion = 0;
/*****************/

/*******************************Variables ADS*****************************************************/
float r_shunt = 0.100;
float v_shunt;
float intensidad;
float v_celda1;
float v_fuente;

TaskHandle_t pantalla, sensado;

xSemaphoreHandle xMutex;

void setup()
{
  Serial.begin(115200);
  xMutex = xSemaphoreCreateMutex();
  /**********************Creacion de las Tareas del nucleo 0****************/
  xTaskCreatePinnedToCore(loop_pantalla, "loop_pantalla", 10048, NULL, 1, &pantalla, 1);
  xTaskCreatePinnedToCore(loop_sensado, "loop_sensado", 1024, NULL, 1, &sensado, 1);

}

void loop() {
  vTaskSuspend(NULL);
}

void touch_Calibracion()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.print("Calibracion de pantalla");
  tft.setCursor(xTitulo + 80, yTitulo + 30);
  tft.setTextColor(ILI9341_ORANGE);
  z = tft.getTouchRawZ();
  
  while(z<500)  //Calibracion del punto arriba a la izquierda
  {   
    tft.drawCircle(0,0,10,ILI9341_GREEN);
    tft.drawCircle(0,0,8,ILI9341_GREEN);
    tft.drawCircle(0,0,5,ILI9341_GREEN);
    tft.getTouchRaw(&x, &y);
    z = tft.getTouchRawZ();
    
    x_touch_Max = x;
    y_touch_Max = y;
  }
  tft.fillScreen(ILI9341_BLACK);

  
  while(z>100)  //Espero a que suelte el dedo
  {
    z = tft.getTouchRawZ();
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(1);
    tft.print("Levante el dedo");
    tft.setCursor(xTitulo + 80, yTitulo + 30);
    tft.setTextColor(ILI9341_ORANGE);
  }
  tft.fillScreen(ILI9341_BLACK);
    
  while(z<500)  //Calibracion del punto arriba a la izquierda
  {   
    tft.drawCircle(319,239,10,ILI9341_GREEN);
    tft.drawCircle(319,239,8,ILI9341_GREEN);
    tft.drawCircle(319,239,5,ILI9341_GREEN);
    tft.setTextSize(1);
    tft.print("Calibracion de pantalla");
    tft.setCursor(xTitulo + 80, yTitulo + 30);
    tft.setTextColor(ILI9341_ORANGE);
    tft.getTouchRaw(&x, &y);
    z = tft.getTouchRawZ();
    
    x_touch_Min = x;
    y_touch_Min = y;
  }
  
}

void touch()
{
  tft.getTouchRaw(&x, &y);
  z = tft.getTouchRawZ();
  Serial.printf("x: %i     ", x );

  Serial.print("\t");
  Serial.printf("y: %i     ", y);
  if (z > 100) Presion = 1;
  else Presion = 0;

  if (Presion == 1)
  {
    if (y > 1500)
    {
      tft.fillScreen(ILI9341_RED);
      Flag_Tactil_Descarga = 1;
      Flag_Tactil_Carga = 0;
      Serial.print("Flag_Carga: "); Serial.print(Flag_Tactil_Carga);
      Serial.print("\t");
      Serial.print("Flag_Descarga: "); Serial.println(Flag_Tactil_Descarga);
    }
    else
    {
      tft.fillScreen(ILI9341_BLUE);
      Flag_Tactil_Descarga = 0;
      Flag_Tactil_Carga = 1;
      Serial.print("Flag_Carga: "); Serial.print(Flag_Tactil_Carga);
      Serial.print("\t");
      Serial.print("Flag_Descarga: "); Serial.println(Flag_Tactil_Descarga);
    }
  }
  else
  {
    tft.fillScreen(ILI9341_BLACK);
  }


}


void loop_pantalla(void *Pvparameters)
{
  
  Serial.println("Declaracion: Pantalla");
  tft.init();//VCC=3.3, CS=17, RESET=EN, DC=16, MOSI=23, SCK=18, LED=3.3, MISO=19
  tft.setRotation(1);

  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(0X0000CE);                                     //Pantalla Azul Endurance
  yield();
  //tft.setTextSize(1);
  tft.setCursor(80, 25);
  tft.setTextColor(ILI9341_WHITE);
  //tft.print("endurance");
  tft.drawString("endurance", xTitulo, yTitulo, 4);
  tft.setTextSize(2);
  tft.setCursor(xTitulo + 20, yTitulo + 30);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("MOVE");
  tft.setCursor(xTitulo + 80, yTitulo + 30);
  tft.setTextColor(ILI9341_ORANGE);
  tft.print("ON");
  tft.setTextSize(1);
  tft.setCursor(xTitulo + 5, yTitulo + 155);
  tft.setTextColor(ILI9341_WHITE);
  delay(300);
  touch_Calibracion();

  while (1)
  {
    if (xMutex != NULL)
    {
      if (xSemaphoreTake( xMutex, portMAX_DELAY ) == pdTRUE)
      {
        //Serial.print("Tarea: Pantalla"); Serial.println("\t\t\tEn núcleo -> " +  String(xPortGetCoreID()));
        if (y > 1500)
        {
          tft.fillScreen(ILI9341_RED);
          Flag_Tactil_Descarga = 1;
          Flag_Tactil_Carga = 0;
          Serial.print("Flag_Carga: "); Serial.print(Flag_Tactil_Carga);
          Serial.print("\t");
          Serial.print("Flag_Descarga: "); Serial.println(Flag_Tactil_Descarga);
        }
        else
        {
          tft.fillScreen(ILI9341_BLUE);
          Flag_Tactil_Descarga = 0;
          Flag_Tactil_Carga = 1;
          Serial.print("Flag_Carga: "); Serial.print(Flag_Tactil_Carga);
          Serial.print("\t");
          Serial.print("Flag_Descarga: "); Serial.println(Flag_Tactil_Descarga);
        }
      }
      else
      {
        tft.fillScreen(ILI9341_BLACK);
      }

      vTaskDelay(100 / portTICK_PERIOD_MS);
      xSemaphoreGive(xMutex);
    }
  }

vTaskSuspend(NULL);
}

void loop_sensado (void *Pvparameters)
{
  //Serial.println("Declaracion: ADS");
  Adafruit_ADS1115 ads(0x48);
  const float multiplier = 0.1875;

  Adafruit_ADS1115 ads1(0x48);
  const float multiplier1 = 0.0078125;

  Adafruit_ADS1115 ads2(0x49);
  const float multiplier2 = 0.1875;

  ads.setGain(GAIN_TWOTHIRDS);
  ads1.setGain(GAIN_SIXTEEN);
  ads2.setGain(GAIN_TWOTHIRDS);

  // ads.setGain(GAIN_TWOTHIRDS);  +/- 6.144V  1 bit = 0.1875mV (default)
  // ads.setGain(GAIN_ONE);        +/- 4.096V  1 bit = 0.125mV
  // ads.setGain(GAIN_TWO);        +/- 2.048V  1 bit = 0.0625mV
  // ads.setGain(GAIN_FOUR);       +/- 1.024V  1 bit = 0.03125mV
  // ads.setGain(GAIN_EIGHT);      +/- 0.512V  1 bit = 0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    +/- 0.256V  1 bit = 0.0078125mV

  while (1)
  {
    if (xMutex != NULL)
    {
      if (xSemaphoreTake( xMutex, portMAX_DELAY ) == pdTRUE)
      {
        //Serial.print("Tarea: ADS"); Serial.println("\t\t\tEn núcleo -> " +  String(xPortGetCoreID()));
        v_shunt = ads1.readADC_Differential_2_3() * multiplier1;
        intensidad = v_shunt / r_shunt;

        v_fuente = ads2.readADC_Differential_0_1() * multiplier2 / 1000;
        v_celda1 = ads.readADC_Differential_0_1() * multiplier / 1000;

        vTaskDelay(100 / portTICK_PERIOD_MS);
        xSemaphoreGive(xMutex);
      }
    }
  }
  vTaskSuspend(NULL);
}
