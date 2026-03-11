#include <Arduino.h>

class CD4021 {
  private: 
    uint8_t clock_, data_, latch_; 

    unsigned long lastTime_;
    double rpm_;

    static constexpr float COUNTS_PER_REV_ = 8.0f; // apparently there's 8 counts per revolution 

  public:

  

  byte read(){
    // This is the custom function the lecturer suggested using (although I've made it cleaner)
    
    // - Set up the CD4021 -
    // Get snapshot from CD4040
    digitalWrite(latch_, 1);
    // Wait a microsecond
    delayMicroseconds(1);
    // Force the CD4021 to stop changing for a little bit
    digitalWrite(latch_, 0);
    
    // - Perform the actual shift in -
    byte data = 0;
    for (uint8_t i = 7; i >= 0; --i){
      digitalWrite(clock_, 0);
      delayMicroSeconds(0.2);
      uint8_t bit = digitalRead(this->data_);

      // Flip the appropriate bit
      data = data | (bit << i);

      digitalWrite(clock_, 1); 

      // Debugging
      char statement[64];
      sprintf(statement, "Bit %d should be %d: ", i, bit);
      Serial.print(statement);
      Serial.println(data, BIN);
    }

    return data;
  }
}