# Demeter

Low power, wireless plant moisture detectors.

|Module|Description|
|--|--|
|`sensor`|Sensor built on ESP12E. Publishes data to MQTT. Low power usage, can last months|
|`aggregator`|Node server that collected sensor data and exposes it via API|
|`display`|ESP12E and eInk module. Pulls data from server and displays data at a slow cadence|