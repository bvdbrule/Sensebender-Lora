# Sensebender-Lora

MySensors Sensebender Micro is normaly equipped with a stackable Nordic NRF24L01 module, see more information https://www.openhardware.io/view/1/Sensebender-Micro


Hereby some files for the Sensebender to replace the NRF24L01 for a RFM95W Lora module.

A Fritzing file with a schema and PCB.
Source code example communicate with The Things Network.


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

