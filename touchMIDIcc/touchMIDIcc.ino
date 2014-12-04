/*******************************************************************************

 Bare Conductive Touch USB MIDI instrument
 -----------------------------------------
 
 Midi_interface.ino - USB MIDI touch instrument - based on a piano
 
 Requires Arduino 1.5.6+ or greater and ARCore Hardware Cores 
 https://github.com/rkistner/arcore - remember to select 
 Bare Conductive Touch Board (arcore, iPad compatible) in the Tools -> Board menu
 
 Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige. 
 
 Adapted by Andy Smith / Makernow.org
 
 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 
 Unported License (CC BY-SA 3.0) http://creativecommons.org/licenses/by-sa/3.0/
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*******************************************************************************/

#include <MPR121.h>
#include <Wire.h>
MIDIEvent e;

#define numElectrodes 12

int restingValues[12];

void setup() {
  MPR121.begin(0x5C);
  MPR121.setInterruptPin(4);
  MPR121.updateTouchData();
  e.type = 0x08;
  e.m3 = 127;  // maximum volume
  e.m1 = 0xB0;
  pinMode(LED_BUILTIN, OUTPUT);
  
  delay(1000);
  updateRestingValues();
}

void loop() {
    MPR121.updateAll();
    for(int i=0; i<numElectrodes; i++){
      
      // MIDI note mapping from electrode number to MIDI CC note
      e.m2 = i;
      bool rest = false;
      if(MPR121.isNewTouch(i)){
        // if we have a new touch, turn on the onboard LED and
        // send a "note on" message
        digitalWrite(LED_BUILTIN, HIGH);
        e.m3 = calcVelocity(restingValues[i],MPR121.getFilteredData(i),i);
        
      } else if(MPR121.isNewRelease(i)){
        // if we have a new release, turn off the onboard LED and
        // send a "note off" message
        digitalWrite(LED_BUILTIN, LOW);
        e.m3 = 0;
      } else if(MPR121.getTouchData(i)) {
        //if electrode is still being touched, send the latest value
        e.m3 = calcVelocity(restingValues[i],MPR121.getFilteredData(i),i);
      }else {
        // else set a flag to do nothing...
        rest = true;
      }
      // only send a USB MIDI message if we need to
      if(!rest){
        MIDIUSB.write(e);
      }
    }
    // flush USB buffer to ensure all notes are sent
    MIDIUSB.flush(); 
  delay(5);
}

int calcVelocity(int rest, int current, int channel){
  int vel;
  //double logVal = current^1.2;
  vel = map(rest-current, 0, 100, 127, 0);
  if(vel < 10)   vel = 10;
  
  if(channel == 11){
    vel = pow(vel,1.3);
  }else if(channel == 10){
    vel = pow(vel,1.2);
  }else{
    vel = pow(vel,1.2);
  }
  
  
  if(vel > 127)  vel = 127;
  return vel;
}
  
void updateRestingValues(){
  MPR121.updateAll();
  for(int i = 0; i < 12; i++){
      restingValues[i] = MPR121.getFilteredData(i);
  }
}
