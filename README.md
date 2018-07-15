# Sensebender-Lora

<picture>
  <img src="https://raw.githubusercontent.com/bvdbrule/Sensebender-Lora/master/Sensebender%2BRFM95.jpg" alt="Flowers" style="width:auto;">
</picture>

MySensors Sensebender Micro is normaly equipped with a stackable Nordic NRF24L01 module, see more information https://www.openhardware.io/view/1/Sensebender-Micro.


Hereby some files for the Sensebender to replace the NRF24L01 for a RFM95W Lora module.

A Fritzing file with a schema and PCB.<br>
Source code example communicate with The Things Network.<br>


<table>
<thead>
<tr>
<th align="center">RFM95W</th>
<th align="center">Sensebender</th>
</tr>
</thead>
<tbody>
<tr>
<td align="center">VCC</td>
<td align="center">3.3V</td>
</tr>
<tr>
<td align="center">GND</td>
<td align="center">GND</td>
</tr>
<tr>
<td align="center">SCK</td>
<td align="center">SCK</td>
</tr>
<tr>
<td align="center">MISO</td>
<td align="center">MISO</td>
</tr>
<tr>
<td align="center">MOSI</td>
<td align="center">MOSI</td>
</tr>
<tr>
<td align="center">NSS</td>
<td align="center">10</td>
</tr>
<tr>
<td align="center">DIO0</td>
<td align="center">2</td>
</tr>
<tr>
<td align="center">DIO1</td>
<td align="center">9</td>
</tr></tbody></table>
<br>
// LMIC Pin mapping<br>
const lmic_pinmap lmic_pins = {<br>
    .nss = 10,<br>
    .rxtx = LMIC_UNUSED_PIN,<br>
    .rst = LMIC_UNUSED_PIN,<br>
    .dio = {2, 9, LMIC_UNUSED_PIN},<br>
};<br>
<br>


# TTN Payload function

function Decoder(bytes, port) {<br>
  var decoded = {};<br>
<br>
  decoded.port = port;<br>
<br>
  if (port == 20)<br>
  {<br>
    bat = ((bytes[0]*256)+bytes[1])/100;<br> 
    decoded.battery = bat;<br>
  }<br>
  if (port == 21)<br>
  {<br>
    temp = ((bytes[0]*256)+bytes[1])/100;<br>  
    decoded.temperature = temp;<br>
  }<br>
  if (port == 22)<br>
  {<br>
    humi = ((bytes[0]*256)+bytes[1])/100;<br>  
    decoded.humidity = humi;<br>
  }<br>
  if (port == 23)<br>
  {<br>
    co = ((bytes[0]*256)+bytes[1]); <br> 
    decoded.co = co;    <br>
  }<br>
  if (port == 24)<br>
  {<br>
    light = ((bytes[0]*256)+bytes[1]);<br>  
    decoded.light = light;    <br>
  }<br>
  if (port == 25)<br>
  {<br>
    sw = ((bytes[0]*256)+bytes[1]);  <br>
    decoded.switch = sw;    <br>
  }  <br>
  if (port == 26)<br>
  {<br>
    temp = ((bytes[0]*256)+bytes[1])/100;  <br>
    decoded.temperature2 = temp;<br>
  }<br>
    if (port == 27)<br>
  {<br>
    temp = ((bytes[0]*256)+bytes[1])/100;  <br>
    decoded.current = temp;<br>
  }<br>
<br>
  return decoded;<br>
}<br>


