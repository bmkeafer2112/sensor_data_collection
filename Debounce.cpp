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

#include "Arduino.h"
#include "Debounce.h"

//Class header
Debounce::Debounce(int pin, long delay) {
  _pin = pin;                 //Input pin
  _state = digitalRead(_pin); //Debounced state of pin
  _timeout = millis();        //Last time the debounced state matched the actual state
  _delay = delay;             //Holding time required to change debounced state
  
  pinMode(_pin, INPUT);       //Set up input as an input
}


/* Heartbeat
 *  This function checks for updates on an input.  It will return true if the state changes.
 *  This class must be called at least twice as fast as the debounce delay to prevent nuisance changes.
 *  It functions in a similar maner to a watchdog timer.
 */
bool Debounce::heartbeat(){
  //Obtain current input state
  int newState = digitalRead(_pin);

  //If the new and debounced state are the same, refresh the timeout and return false
  if(_state == newState){
    _timeout = millis();
    return false;
  
  //If the states are different, and the require delay has elapsed, update the state and return true
  }else if(millis() - _timeout > _delay){
    _state = newState;
    return true;
  
  //If the states do not match, but the delay condition is not met, than just return false
  }else{
    return false;
  }
}


/* State
 *  This function provides a safe way of obtaining the debounced input state
 */
int Debounce::state(){
  return _state;
}
