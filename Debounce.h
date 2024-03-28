/* Debounce
 *  This class debounces any input by periodicly reading the input.  It changing the debounced
 *  value after a specified time delay only if the value remains constant.  A user must call the
 *  heartbeat() function at least as fast as the delay time for this class to work.
 *  
 *  Version: 1.0.0
 *  Author: R. D. MacBride
 *  
 *  For questions, call (804) 426-3681
 */

#ifndef Debounce_h
#define Debounce_h
#include "Arduino.h"

class Debounce {
public:
  Debounce(int pin, long delay);
  bool heartbeat();
  int state();

private:
  int _pin;
  int _state;
  unsigned long _timeout;
  unsigned long _delay;
};
#endif
