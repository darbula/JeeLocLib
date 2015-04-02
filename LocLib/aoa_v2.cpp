/// @file
/// aoa_v2 library definitions.
// 2014-08-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php

#include <aoa_v2.h>
#include <avr/eeprom.h>
#include <util/atomic.h>


/**
 * Angle of arrival sensor uses two ports: @param num should be 1 or 2, it is
 * the port number where the phototransistor side is connected.
 */
AoAPlug::AoAPlug (uint8_t num)
  : Port(num), led((num==1) ? 4 : 3), threshold(AOA_NOISE_THRESHOLD) {

  led.mode(OUTPUT);

  /*
  // fixed from mp2
  P1I = 0.0714173;
  P1III = -0.046489;
  PII[5]  = {-0.000049933, -0.00018726, 0.021286, 0.2798932, 1.2691298};
  // calibrated mp1_polys2_mp2.pickle 16 bit
  P0I[AOA_INPUTS_NUM] = {2.1898,  1.8370,  1.9265,  2.0570,  1.9248,
        1.9618,  2.2114,  2.3106,  2.0218,  2.0032,
        1.8862,  1.9421};
  P0III[AOA_INPUTS_NUM] = {1.8466,  1.9086,  1.7957,  1.9228,  1.6183,
        1.5763,  1.6292,  1.6823,  1.8340,  1.8086,
        1.7718,  1.6616};
  */

  // get calibration params from EEPROM
  delayMicroseconds(10000);
  eeprom_read_block((void*)&P1I, (void*)P1I_EEPROM_ADDR, 4);
  eeprom_read_block((void*)&P1III, (void*)P1III_EEPROM_ADDR, 4);
  eeprom_read_block((void*)&PII, (void*)PII_EEPROM_ADDR, 20);
  eeprom_read_block((void*)&P0I, (void*)P0I_EEPROM_ADDR, 48);
  eeprom_read_block((void*)&P0III, (void*)P0III_EEPROM_ADDR, 48);
  MAX_VAL_THRESHOLD = eeprom_read_word(MAXVALTHR_EEPROM_ADDR);

  for (int i=-15; i<16; i++) { // dont change int to uint8_t
    CW_CCW[i+15] = PII[0]*pow(i,4) + PII[1]*pow(i,3) +
                     PII[2]*pow(i,2) + PII[3]*i + PII[4];
  }
  //TODO: get calibration params P0I and P0III from aoa's attiny45 EEPROM
}


/** Select the channel on the multiplexer.
 *  @param channel A number between 1 and AOA_INPUTS_NUM.
 */
void AoAPlug::select(uint8_t channel) {
    digiWrite(0);
    mode(OUTPUT);

    delayMicroseconds(50);
    byte data = 0x10 | (channel & 0x0F);
    byte mask = 1 << (portNum + 3); // digitalWrite is too slow

    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        for (byte i = 0; i < 5; ++i) {
            byte us = bitRead(data, 4 - i) ? 9 : 3;
#ifdef PORTD
            PORTD |= mask;
            delayMicroseconds(us);
            PORTD &= ~ mask;
#else
            //XXX TINY!
#endif
            delayMicroseconds(4);
        }
    }
}

void AoAPlug::read_inputs(uint16_t output_arr[]) {
  mode2(INPUT);
  for (uint8_t i=0; i < AOA_INPUTS_NUM; i++) {
    select(i);
    delay(1);
    select(i);
    delay(AOA_SWITCH_DELAY);
    output_arr[i] = anaRead();
  }
}

/**
 * Determinates IR light interference prior to aoa detection and sets
 * threshold for additional noise suppression.
 * Before LED light is turned on all photodetector values which represent
 * ambient light are read and stored so they can be subtracted later.
 * After that all photodetector values are read again and previously stored
 * values are substracted. Largest value that deviate from 0 is stored as
 * threshold.
 */
void AoAPlug::set_threshold() {
  int noise = 0;
  threshold = AOA_NOISE_THRESHOLD;

  read_inputs(interf);
  read_inputs(interf2);

  for (uint8_t i=0; i < AOA_INPUTS_NUM; i++) {
    noise = interf2[i] - interf[i];
    if (abs(noise) > threshold) {
      threshold = abs(noise) + AOA_NOISE_THRESHOLD;
    }
  }
}

/**
 * Sets values to be used in get_aoa methods
 */
void AoAPlug::set_values() {
  int diff = 0;

  read_inputs(values);
  for (uint8_t i=0; i<AOA_INPUTS_NUM; i++) {
    diff = values[i] - interf[i];
    // set to 0 all values that are above interf by less than threshold
    values[i] = (diff > threshold) ? (uint16_t)diff : 0;
  }
}

/**
 * Method for estimating aoa of incident infrared light. Returns value between
 * 0 and 360. If measurement is not reliable i.e. received light intensity was
 * not high enough, this method returns negative value.
 */
float AoAPlug::get_aoa_deg() {
  set_values();
  // if double_peak() is implemented change force version
  if (max_value()<MAX_VAL_THRESHOLD) {
    return AOA_DEG_FAIL;
  }
  return calculate_aoa();
}

float AoAPlug::get_aoa() {
  float aoa=get_aoa_deg();

  if (aoa==AOA_DEG_FAIL) {
    return AOA_FLOAT_FAIL;
  }
  return aoa*PI/180;
}

uint16_t AoAPlug::get_aoa_int() {
  float aoa=get_aoa();

  if (aoa==AOA_FLOAT_FAIL) {
    return AOA_INT_FAIL;
  }
  return (uint16_t)(aoa*10000);
}

uint16_t AoAPlug::get_aoa_int_force() {
  uint16_t tmp = MAX_VAL_THRESHOLD;

  MAX_VAL_THRESHOLD = 0; // temporarily remove threshold
  float aoa=get_aoa();

  MAX_VAL_THRESHOLD = tmp; // restore original threshold
  return (uint16_t)(aoa*10000);
}

float AoAPlug::calculate_aoa() {
  uint8_t m=0;    // index of values array element with maximum value
  uint8_t ccw=0;  // index of values array element ccw from m
  uint8_t cw=0;   // index of values array element cw from m
  uint8_t section;  // section used to calculate aoa: should be 1, 2 or 3
  float v_m, v_ccw, v_cw, v_cw_ccw, aoa;
  uint8_t i;

  for (i=1; i<AOA_INPUTS_NUM; i++) {
    if (values[i] > values[m]) m = i;
  }
  ccw = (m+AOA_INPUTS_NUM-1)%AOA_INPUTS_NUM;
  cw = (m+1)%AOA_INPUTS_NUM;
  v_m = values[m];
  v_cw = values[cw];
  v_ccw = values[ccw];
  v_cw_ccw = v_cw/v_ccw;

  // interpolate
  for (i=1; i<31; i++) {
    if (v_cw_ccw<CW_CCW[i])
      break;
  }
  // calculate aoa relative to m, based on chosen section
  if (i<AOA_I_MARGIN) {
    section = 1; //I
    aoa = (v_m/v_ccw-P0I[ccw])/P1I;
  } else if (i>(30-AOA_III_MARGIN)) {
    section = 3; //III
    aoa = (v_m/v_cw-P0III[ccw])/P1III;
  } else {
    section = 2; //II
    aoa = i-16+(v_cw_ccw-CW_CCW[i-1])/(CW_CCW[i]-CW_CCW[i-1]);
  }
  // get absolute aoa and put it in 0 to 360 range
  aoa += m*30.0;
  aoa = aoa<0 ? aoa + 360 : aoa;

  return aoa;
}

uint16_t AoAPlug::max_value() {
  uint16_t max=values[0];

  for (uint8_t i=1; i<AOA_INPUTS_NUM; ++i) {
    if (values[i]>max) {
      max = values[i];
    }
  }
  return max;
}

void AoAPlug::store_maxval_threshold(uint16_t maxval_threshold) {
  if (MAX_VAL_THRESHOLD==maxval_threshold) {
    return; // skip writing if it is the same value
  }
  MAX_VAL_THRESHOLD = maxval_threshold;
  eeprom_write_word(MAXVALTHR_EEPROM_ADDR, MAX_VAL_THRESHOLD);
}

/**
 * This method could serve to detect non line of sight aoa measurement.
 * Needs additional research to use it properly.
 */
/*
uint8_t AoAPlug::double_peak() {
  uint8_t direction1, direction2, peaks_num;
  uint16_t peaks[AOA_INPUTS_NUM/2];

  for (uint8_t i=0; i<AOA_INPUTS_NUM; ++i) {
    direction1 = values[(i+1)%AOA_INPUTS_NUM]-values[i];
    direction2 = values[(i+2)%AOA_INPUTS_NUM]-values[(i+1)%AOA_INPUTS_NUM];
    if (direction1>=0 && direction2<0) {
      peaks[peaks_num++] = values[(i+1)%AOA_INPUTS_NUM];
    }
  }
  //TODO: return diff of two highest peaks?
  return (peaks>1) ? 1 : 0;
}
*/

void AoAPlug::led_on() {
  led.digiWrite(HIGH);
}

void AoAPlug::led_off() {
  led.digiWrite(LOW);
}
