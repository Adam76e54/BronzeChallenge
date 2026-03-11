#include "x_14_buggy.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 

// Declaration of Interrupt Service Routines
void leftISR(); void rightISR();
ROB12629 leftEncoder(2);

// Tasks (for scheduler to handle)
void telemetry(void *);
void sense(void *);


// Declare handles for each task, used to pass to task-analysing functions like uxTaskGetStackHighWaterMark()
TaskHandle_t telemetryHandle, senseHandle;

// Set up semaphores (I think we just need a mutex?)
SemaphoreHandle_t statMutex;

void setup() {
  // Set up serial for debugging
  Serial.begin(115200);

  // Set up network
  wifi::initialiseAccessPoint();

  // Set up semaphore


  // Set up tasks
  xTaskCreate(telemetry, "Read from GUI", 1024, nullptr, 1, telemetryHandle);
  xTaskCreate(sense, "Sense and drive", 1024, nullptr, 2, senseHandle);

  // Start scheduler (does not return)
  vTaskStartScheduler();
}

void loop() {
  // This is the idle task if everything else is blocking, everything else is handled by the scheduler
}

// - ISRs -
void leftISR(){
  leftEncoder.increment();
}

void rightISR(){
  rightEncoder.increment();
}

// - TASK 1 -
void telemetry(void *parameters){
  // Networking objects
  Buffer<200> inBuffer, outBuffer;
  WiFiServer server(wifi::PORT);
  server.begin();

  WiFiClient GUI;

  while(true){
    // Reconnect if disconnected
    keep(GUI, server);
    // Read a command if there is one
    read(GUI, inBuffer);
    // Handle a command if there is one
    handle(inBuffer, driver);

    float distance;

    // We've only got one thing to send right now
    sendDistance(GUI, distance);
    printStackUsage();

    clock = (clock + 1) % numberOfSends;

    // Don't repeat this task again for at least 300 ms
      vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// - TASK 2 -
void sense(void *parameters){
  // NEED TO CONFIRM VALUES FOR THESE CONSTANTS
  // Set up pins
  constexpr uint8_t clockPin, dataPin, latchPin; 
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPTUT);

  // Declare physical constants
  constexpr float CIRCUMFERENCE = 20.4, DIAMETER = 6.5, AXLE = 14.5;

  // Hardware objects
  HCSR04 ears(8, 3); // PINS NEED TO BE CHANGED
  L293D driver(6,7,11,12,9,10);
  CD4021 shifter(4, 5, 13);

  // Set up hardware
  driver.begin();
  ears.begin();
  shifter.begin();

  // We pass ISRs to the encoders
  leftEncoder.begin(leftISR);
  rightEncoder.begin(rightISR);


  while(true){


  }
}

// - OTHER FUNCTIONS -


void printStackUsage() {
  // This function just gives us stack-usage so we can know roughly how much memory each task needs
  // NOTE (1): I think we have 32-bit words (4 bytes)
  // NOTE (2): a high water mark is the highest point of stack usage so far (or something like that)
  // NOTE (3): a word is the fundamental unit of memory the MCU has (computers are generally layed out in 32 or 64 bit chunks)

    UBaseType_t telemetryHW = uxTaskGetStackHighWaterMark(telemetryHandle);
    sendEvent(GUI, "Networking high water mark (words): ", telemetryHW);
    // Serial.print("Networking high water mark (words): ");
    // Serial.println(telemetryHW);

    UBaseType_t senseHW = uxTaskGetStackHighWaterMark(senseHandle);
    sendEvent(GUI, "Sensor and drive high water mark (words): ", senseHW);
    // G.print("Sensor and drive high water mark (words): ");
    // Serial.println(senseHW);

}


