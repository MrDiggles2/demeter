### Chip

Using ESP8266 Node MCU CP2102

Uses ESP12E module

https://www.aliexpress.com/item/1005001636474859.html?spm=a2g0s.9042311.0.0.6a644c4dI5g24P

### Power

* Battery
  * https://www.aliexpress.com/item/5Pcs-3-7V-1200mAh-Lipo-Battery-Charger-for-Syma-X5SW-X5SC-M18-H5P-RC-Drone/1005001450744187.html?spm=a2g0s.9042311.0.0.afc44c4d2v9m4p
    * Have to trim connector to fit on chip
  * Capacity: 1200 mAh
  * Volage: 4.2 max, dead at 3.4: https://learn.adafruit.com/li-ion-and-lipoly-batteries/voltages

* Load

  * Deep sleep
    * Supposed to use 20 uA: https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/
    * Currently, uses ~14mA.
    * Due to "dev" components on chip
      * Other people having issues: https://github.com/esp8266/Arduino/issues/4957#issuecomment-516771848
      * Cutting power to the bits: https://tinker.yeoman.com.au/2016/05/29/running-nodemcu-on-a-battery-esp8266-low-power-consumption-revisited/
        * **Botched a chip trying this, ordered a magnifying glass to see if that helps**
          * Marked with an X
          * Eating 35 mA constantly for some reason
    * Probably have to just to the underlying ESP12E chip
      * Need 3.3V regulator (have HT7333)
      * ~~Need to figure out how to program~~ - Made jigs by taking apart ESP8266 
      * **It's lasted several months without issue**
  * Active
    * Measured average of 70-80 mA while active
    * ~5 seconds to wake up, measure, and publish
    * 350 mAs per check

### Sensor

* Consistent reads with same sensor
  * Fluctuates even when reading within 5 minutes of each other
  * Potentially due to high impedance on sensor, started experiment with 100k and 220k resistors
    * Named as "pcb-test-w-220uF-new-resistors"

* ~~Normalizing reads between sensors~~ - **No longer necessary, fixed accuracy by increasing resistance on voltage dividers**
  * Usually within 20% of each other
  * Have to normalize somehow and map to 1-100 range
    * Manual?
    * Regression?
      * Both linear and logrithimic gets really weird.
      * Needs more experimentig