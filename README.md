# Demeter

Low power, wireless plant moisture sensors.

|Module|Description|
|--|--|
|`sensor`|Sensor built on ESP12E. Publishes data to MQTT. Low power usage, can last months on a single battery charge|
|`aggregator`|Node server that collected sensor data and exposes it via API|
|`display`|ESP12E and eInk module. Pulls data from server and displays data at a slow cadence|

### TODOs

* Figure out how to interpret raw values from sensors for display
* Hook up pager duty