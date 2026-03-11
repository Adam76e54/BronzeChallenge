#pragma once

#include <Arduino.h>

class ROB12629{
  private:
    uint8_t pin_;//one digital pin
    volatile unsigned long counter_;
    unsigned long lastCounter_;
    unsigned long lastTime_;
    double rpm_;
    static constexpr float COUNTS_PER_REV_ = 8.0f; // apparently there's 8 counts per revolution 
  public:
    ROB12629(uint8_t pin) : pin_(pin), counter_(0), lastCounter_(0), lastTime_(0), rpm_(0){
      //empty
    }

    //requires us to feed an ISR to it
    void begin(void (*ISR)()){
      if(pin_ != 2 && pin_ != 3){
        Serial.println("You're not using interrupt pins !!!");
      }
      pinMode(pin_, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(pin_), ISR, RISING);
    }
    //NOTE: the reason you can' have an ISR method is because C++ always implicitly passes *this. E.g. Class::func() calls will always be fed Class::func(this)
    //so it autogenerates a parameter. An ISR must have no parameters so needs to be a "free" function (not attached to an instance of a class)

    void increment(){
    //we'll use this inside of a globally defined ISR
      counter_++;
    }

    unsigned long count() const{
      return counter_;
    }

    unsigned long lastCount() const{
      return lastCounter_;
    }

    void update(unsigned long interval){
      auto now = micros();
      unsigned long dt_m = now - lastTime_;
      if(dt_m < interval) return;//only update if over the interval
      lastTime_ = now;

      noInterrupts();
      unsigned int dc = counter_ - lastCounter_;
      interrupts();

      lastCounter_ = counter_;

      if(dc == 0){
        rpm_ = 0;
        return;
      }

      float revs = (float)dc / COUNTS_PER_REV_;
      double dt_s = dt_m * 1e-6f;//convert mircos to seconds;
      rpm_ = (revs/dt_s) * 60.0f;
    }

    double rpm() const{
      return rpm_;
    }
};
