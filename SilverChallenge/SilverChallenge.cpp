#include "x_14_buggy.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 
constexpr uint8_t N = 200;

// Networking objects
Buffer<N> inBuffer, outBuffer;
WiFiServer server(wifi::PORT);
WiFiClient GUI;

// Hardware objects
ROB12629 leftEncoder(2), rightEncoder(3);
HCSR04 ears(5, 4);
L293D driver(6,7,11,12,9,10);

// Declare Interrupt Service Routines
void leftISR(); void rightISR();

// Tasks (for scheduler to handle)
void read(void *);
void sense(void *);
void send(void *);

// Declare handles for each task, used to pass to task-analysing functions like uxTaskGetStackHighWaterMark()
TaskHandle_t readHandle, senseHandle, sendHandle;

// Set up semaphores (I think we just need a mutex?)
SemaphoreHandle_t GUIMutex;

// NOTE: We'll need to sort out the GUI.print() in random sections of code. We can use a mutex and lock/unlock but I think we'd
// prefer using an outBuffer to write into locally, then set up a mutex around that local buffer (local to the arduino, that is)

void setup() {
  // Set up serial for debugging
  Serial.begin(115200);

  // Set up network
  wifi::initialiseAccessPoint();
  server.begin();

  // Set up hardware
  driver.begin();
  ears.begin();
  // We pass ISRs to the encoders
  leftEncoder.begin(leftISR);
  rightEncoder.begin(rightISR);

  // Set up tasks
  xTaskCreate(read, "Read from GUI", 1024, nullptr, 2, readHandle);
  xTaskCreate(sense, "Sense and drive", 1024, nullptr, 3, senseHandle);
  xTaskCreate(send, "Send back to GUI", 1024, nullptr, 1, sendHandle);
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
void read(void *parameters){
  while(true){
    // Reconnect if disconnected
    keep(GUI, server);
    // Read a command if there is one
    read(GUI, inBuffer);
    // Handle a command if there is one
    handle(inBuffer, driver);

    // Don't repeat this task again for at least 20 ms
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// - TASK 2 -
void sense(void *parameters){

}

// - TASK 3 -
void send(void *parameters){
  while(true){
      float distance;

      if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
          distance = sensors.distance;
          leftIR = sensors.leftIR;
          rightIR = sensors.rightIR;
          xSemaphoreGive(sensorMutex);
      }

      // We've only got one thing to send right now
      sendDistance(GUI, distance);
      printStackUsage();

      clock = (clock + 1) % numberOfSends;

    // Don't repeat this task again for at least 300 ms
      vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void printStackUsage() {
  // This function just gives us stack-usage so we can know roughly how much memory each task needs
  // NOTE (1): I think we have 32-bit words (4 bytes)
  // NOTE (2): a high water mark is the highest point of stack usage so far (or something like that)
  // NOTE (3): a word is the fundamental unit of memory the MCU has (computers are generally layed out in 32 or 64 bit chunks)

    UBaseType_t readHW = uxTaskGetStackHighWaterMark(readHandle);
    sendEvent(GUI, "Read from GUI high water mark (words): ", readHW);
    // Serial.print("Read from GUI high water mark (words): ");
    // Serial.println(readHW);

    UBaseType_t senseHW = uxTaskGetStackHighWaterMark(senseHandle);
    sendEvent(GUI, "Sensor and drive high water mark (words): ", senseHW);
    // G.print("Sensor and drive high water mark (words): ");
    // Serial.println(senseHW);

    UBaseType_t sendHW = uxTaskGetStackHighWaterMark(sendHandle);
    sendEvent(GUI, "Sending to GUI high water mark (words): ", sendHW);
    // Serial.print("Sending to GUI high water mark (words): ");
    // Serial.println(sendHW);
}


