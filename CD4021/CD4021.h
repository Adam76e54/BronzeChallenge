#include <Arduino.h>

class CD4021 {
  private: 
    uint8_t clock_, data_, latch_; 

    uint8_t lastCounter_;
    unsigned long lastTime_;
    double rps_;

    static constexpr float COUNTS_PER_REV_ = 4.0f; // apparently there's 8 counts per revolution 

  public:

    CD4021(uint8_t clock, uint8_t data, uint8_t latch) : 
      clock_(clock)
      , data_(data)
      , latch_(latch)
      , lastCounter_(0)
      , lastTime_(0)
      , rps_(0) {}

    void begin() {
      pinMode(clock_, OUTPUT);
      pinMode(latch_, OUTPUT);
      pinMode(data_, INPUT);
      digitalWrite(clock_, LOW);
      digitalWrite(latch_, LOW);
      lastTime_ = micros();
    }

    double revsPerSecond() const{
      return rps_;
    }
    void update(uint8_t counter, unsigned long interval_microseconds){
      auto now = micros();
      unsigned long dt_micro = now - lastTime_;

      if(dt_micro < interval_microseconds) return;//only update if time interval is large enough
      lastTime_ = now;

      // We'll be computing dc/dt to get rpm 
      unsigned int dc = counter - lastCounter_;

      // Handle wraparound
      if(lastCounter_ > counter){
        dc = (counter + 255) - lastCounter_;
      }

      lastCounter_ = counter;

      if(dc == 0){
        rps_ = 0;
        return;
      }

      float revs = (float)dc / COUNTS_PER_REV_;
      double dt_s = dt_micro * 1e-6f;//convert micros to seconds;b
      rps_ = (revs/dt_s);
    }

    void read(unsigned long interval_microseconds){
      // This is the custom function the lecturer suggested using (although I've made it cleaner)
      
      // - Set up the CD4021 -
      // Get snapshot from CD4040. The clock speed is much faster than arduino so there's no need for using delays
      digitalWrite(latch_, 1);

      // Force the CD4021 to stop changing for a little bit
      digitalWrite(latch_, 0);
      
      // - Perform the actual shift in -
      uint8_t data = 0;
      for (int8_t i = 7; i >= 0; --i){
        digitalWrite(clock_, 0);
        // delayMicroseconds(1); // We probably don't need this because the CD4021 is much faster than sam anyway
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

      update(data, interval_microseconds);
    }
}