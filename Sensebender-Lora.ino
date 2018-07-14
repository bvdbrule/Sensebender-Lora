/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SI7021.h>
#include <LowPower.h>


// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

int sleepcycles = 75; // every sleepcycle will last 8 secs, total sleeptime will be sleepcycles * 8 sec
bool joined = false;
bool sleeping = false;

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;
static osjob_t initjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {2, 9, LMIC_UNUSED_PIN},
};

int send_msg = 20;
// 20 = battery status
// 21 = temperature
// 22 = humidity
// 23 = co
// 24 = current
// 25 = ...

//Sensebender Temp and Humi sensor
SI7021 humiditySensor;

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADcdMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
 
}

float getBatVoltage() {
  float measuredvbat = readVcc()/1000.0f;
  Serial.print("VBat: " ); Serial.println(measuredvbat);
  return measuredvbat;
}

float getTemperature() {
  si7021_env data = humiditySensor.getHumidityAndTemperature();
  float measuredTemp = data.celsiusHundredths / 100.0;
  
  Serial.print("Temp: " ); Serial.print(measuredTemp);
  return measuredTemp;
}

float getHumidity() {
  si7021_env data = humiditySensor.getHumidityAndTemperature();
  float measuredHumi = data.humidityPercent;

  Serial.print("Humi: " ); Serial.print(measuredHumi);
  return measuredHumi;
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);

            joined = true;  // Low Power functionality
            
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:

            sleeping = true;  // Low Power functionality
          
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
  byte buffer[22];
  float fval;
  int ival;
  // It is a bad idea to send ASCII for real, only use this when developing!!
  buffer[0]='H';
  buffer[1]='i';
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.

    switch (send_msg) {
      case 20:
         fval = getBatVoltage();
         ival = (int)(fval*100);
         Serial.print("vbatint: " ); Serial.println(ival);
         buffer[0] = (uint8_t)highByte(ival);
         buffer[1] = (uint8_t)lowByte(ival);
      break;
      case 21: 
         fval = getTemperature(); 
         Serial.print("temp: "); Serial.println(fval);
         ival = (int)(fval*100);
         buffer[0] = (uint8_t)highByte(ival);
         buffer[1] = (uint8_t)lowByte(ival);
      break;
      case 22:
         fval = getHumidity(); 
         Serial.print("humi: "); Serial.println(fval);
         ival = (int)(fval*100);
         buffer[0] = (uint8_t)highByte(ival);
         buffer[1] = (uint8_t)lowByte(ival);
      break;
      default:
      break;
    } // switch
         
    LMIC_setTxData2(send_msg, (uint8_t*) buffer, 2 , 0);
    Serial.print(F("Sending: ")); Serial.println(send_msg);
    send_msg++;
    if (send_msg == 23) { send_msg = 20; }
  
  } // else
} // end do_send function


static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked…
}

void setup() {
    pinMode(A2, OUTPUT);
    digitalWrite(A2, HIGH);  // LED 
    delay(100);
    digitalWrite(A2, LOW);  
    
    delay(10000);
    Serial.begin(9600);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(2000);
    #endif

    // LMIC init
    os_init();

    // Low power functionality
    os_setCallback(&initjob, initfunc);
    
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    // Low power functionality do_send is turned off here
    // do_send(&sendjob);
    
}

void loop() {
    // Low power functionality os_runloop_once is turned off here
    // os_runloop_once();

    // start OTAA JOIN
    if (joined==false)
    {
      os_runloop_once();
    }
    else
    {
      do_send(&sendjob);    // Sent sensor values
      while(sleeping == false)
      {
        os_runloop_once();
      }
      sleeping = false;
      for (int i=0;i<sleepcycles;i++)
      {
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);    //sleep 8 seconds
      }
    } // else
    digitalWrite(A2,((millis()/100) % 2) && (joined==false)); // only blinking when joining and not sleeping

}
