// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html

#include <math.h>

#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

static const char* TAG                       = "main";

static void sleepMs(TickType_t durationInMs) {
   vTaskDelay( durationInMs / portTICK_PERIOD_MS);
}

#define D2                          GPIO_NUM_2
#define D13                         GPIO_NUM_13
#define IO_PIN_FOR_RELAIS           D2

#define LEDC_TIMER                  LEDC_TIMER_0
#define LEDC_MODE                   LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO              D13 
#define LEDC_CHANNEL                LEDC_CHANNEL_0
#define LEDC_DUTY_RES               LEDC_TIMER_20_BIT
#define LEDC_FREQUENCY              50
#define DUTY_PER_MICRO              (((uint32_t)powf(2, 20)) / (1000000 / 50))

#define DIRECTION_RANGE_MICROS      600
#define CENTER_POSITION_IN_MICROS   1500

#define CENTER_POSITION             0

#define ONE_SECOND                  1000000

#define PI                          (acosf(-1))

static void initRelaisPin() {
   ESP_LOGI(TAG, "initializing relais pin ...");
   gpio_config_t pinConfig;
   pinConfig.intr_type    = GPIO_INTR_DISABLE;
   pinConfig.mode         = GPIO_MODE_OUTPUT;
   pinConfig.pin_bit_mask = (1ULL << IO_PIN_FOR_RELAIS);
   pinConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
   pinConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
   ESP_ERROR_CHECK(gpio_config(&pinConfig));
   ESP_ERROR_CHECK(gpio_set_level(IO_PIN_FOR_RELAIS, 0));
}

static void activateRelais() {
   ESP_LOGI(TAG, "setting relais pin to high");
   ESP_ERROR_CHECK(gpio_set_level(IO_PIN_FOR_RELAIS, 1));
}

static void deactivateRelais() {
   ESP_LOGI(TAG, "setting relais pin to low");
   ESP_ERROR_CHECK(gpio_set_level(IO_PIN_FOR_RELAIS, 0));
}

// input range [-1, 1]
// -1 stands for left most position
//  0 stands for center position
//  1 stands for right most position
//
// return the duty channel duty corresponding to the provided position
uint32_t positionToDuty(float position) {
   uint32_t pulseLengthInMicros = (position * DIRECTION_RANGE_MICROS) + CENTER_POSITION_IN_MICROS;
   uint32_t duty = pulseLengthInMicros * DUTY_PER_MICRO;
   return duty;
}

static void initLEDController()
{
   ESP_LOGI(TAG, "initializing PWM controller ...");
   ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_MODE,
      .timer_num        = LEDC_TIMER,
      .duty_resolution  = LEDC_DUTY_RES,
      .freq_hz          = LEDC_FREQUENCY,
      .clk_cfg          = LEDC_AUTO_CLK
   };
   ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

   ledc_channel_config_t ledc_channel = {
      .speed_mode     = LEDC_MODE,
      .channel        = LEDC_CHANNEL,
      .timer_sel      = LEDC_TIMER,
      .intr_type      = LEDC_INTR_DISABLE,
      .gpio_num       = LEDC_OUTPUT_IO,
      .duty           = positionToDuty(CENTER_POSITION),
      .hpoint         = 0
   };
   ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// input range [-1, 1]
// -1 stands for left most position
//  0 stands for center position
//  1 stands for right most position
void moveToPosition(float position) {
   //ESP_LOGI(TAG, "moving to position %.2f ...", position);
   ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, positionToDuty(position)));
   ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

float toRad(float degrees) {
   return degrees / (180 / PI);
}

void app_main() {   
   initRelaisPin();
   initLEDController();
   moveToPosition(CENTER_POSITION);
   sleepMs(5000);
   activateRelais();
   sleepMs(1000);
   for(float angleInRad=-PI/2; angleInRad < 3 * PI / 2; angleInRad += toRad(0.5)) {
      float position = (sinf(angleInRad) + 1) / 2;
      sleepMs(10);
      moveToPosition(position);
      float distanceToPeak = 1.0 - position;
      ESP_LOGI(TAG, "pos = %.3f, distanceToPeak = %.3f", position, distanceToPeak);
      distanceToPeak *= ((distanceToPeak < 0) ? -1 : 1);
      if (distanceToPeak < 0.0000001) {
         ESP_LOGI(TAG, "-- PEAK --");
         sleepMs(3000);
      }
   }
   deactivateRelais();
   sleepMs(100);
   int deepSleepDurationInSec = 15 * 60;
   ESP_LOGI(TAG, "activating deep sleep for %d seconds ...", deepSleepDurationInSec);
   esp_deep_sleep(deepSleepDurationInSec * ONE_SECOND);
}