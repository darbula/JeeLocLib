// 2014-08-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php

#ifndef aoa_v2_h
#define aoa_v2_h

/// @file
/// aoa-v2 library definitions.

#include <JeeLib.h>
#include <avr/eeprom.h>

#define AOA_INPUTS_NUM 12 //number of phototransistors on aoa sensor
#define AOA_SWITCH_DELAY 5 // delay until next phototransistor is selected in ms
#define AOA_NOISE_THRESHOLD 2
#define AOA_I_MARGIN 8 // margin of section I when calculating aoa
#define AOA_III_MARGIN 8 // margin of section I when calculating aoa

/**
 * EEPROM usage: 126 bytes
 * EEPROM layout:
 * float 0x100   P1I
 * float 0x104   P1III
 * float 0x108   PII[5]
 * float 0x11C   P0I[AOA_INPUTS_NUM]
 * float 0x14C   P0III[AOA_INPUTS_NUM]
 * word  0x17C   MAX_VAL_THRESHOLD
 */
// TODO: move calibration data to sensor
// EEPROM layout to adresses compatible with RF12demo.ino
// http://jeelabs.net/projects/jeelib/wiki/RF12demo#Eeprom-Layout
#define CALIBRATION_EEPROM_ADDR    ((float*) 0x100)
#define P1I_EEPROM_ADDR       CALIBRATION_EEPROM_ADDR
#define P1III_EEPROM_ADDR     (CALIBRATION_EEPROM_ADDR + 1)
#define PII_EEPROM_ADDR       (CALIBRATION_EEPROM_ADDR + 2)
#define P0I_EEPROM_ADDR       (CALIBRATION_EEPROM_ADDR + 7)
#define P0III_EEPROM_ADDR     (CALIBRATION_EEPROM_ADDR + 19)
#define MAXVALTHR_EEPROM_ADDR (uint16_t*)(CALIBRATION_EEPROM_ADDR + 31)

#define AOA_DEG_FAIL -1
#define AOA_FLOAT_FAIL -1
#define AOA_INT_FAIL 63000

/// Interface for the AoA Plug
class AoAPlug : public Port {
    Port led;

public:
    AoAPlug (uint8_t num);

    // calibration params in polys2 format
    float P1I;
    float P1III;
    float PII[5];  // for convenient calculation of cw_ccw
    float CW_CCW[31];
    // bias for section I and III, these values can be reduced to int
    float P0I[AOA_INPUTS_NUM];
    float P0III[AOA_INPUTS_NUM];
    // minimum value of maximum incident light accepted as reliable mesurement
    uint16_t MAX_VAL_THRESHOLD;

    //values of each photodetector reading
    uint16_t values[AOA_INPUTS_NUM];
    //interference caused by exterior light on each photodetector
    uint16_t interf[AOA_INPUTS_NUM];
    uint16_t interf2[AOA_INPUTS_NUM];
    int threshold;

    void select(uint8_t channel);
    void read_inputs(uint16_t output_arr[]);
    void set_threshold();
    void set_values();
    float get_aoa_deg();
    float get_aoa();
    uint16_t get_aoa_int();
    uint16_t get_aoa_int_force();
    float calculate_aoa();
    uint16_t max_value();
    void store_maxval_threshold(uint16_t);
    //uint8_t double_peak();
    void led_on();
    void led_off();
};

#endif // aoa_v2_h
