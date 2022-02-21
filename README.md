# Demeter

Low power, wireless plant moisture sensors.

|Module|Description|
|--|--|
|`sensor`|Sensor built on ESP12E. Publishes data to MQTT. Low power usage, can last months on a single battery charge|
|`aggregator`|Node server that collected sensor data and exposes it via API|
|`display`|ESP12E and eInk module. Pulls data from server and displays data at a slow cadence|

<table>
  <tr>
    <td><img src="https://user-images.githubusercontent.com/1780320/154992799-800c1053-1b74-4c65-8d9d-270619f6bad5.jpg" height=270></td>
    <td><img src="https://user-images.githubusercontent.com/1780320/154992803-2cdc40c7-4ff4-4f19-a88f-31ef4a9f04af.jpg" height=270></td>
  </tr>
</table>

### TODOs

* Figure out how to interpret raw values from sensors for display
* Hook up pager duty
