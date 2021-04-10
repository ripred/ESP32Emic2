/**
* @Brief Example showing the use of the ESP32Emic2 class
*        to control an Emic 2 Text-to-Speech Synthesizer
*        module.
*
* Written by Trent M. Wyatt
* April, 2021 version 1.0
*
*/

#include <Arduino.h>
#include <ESP32Emic2.h>

// Project pin assignments
// Change to match your project connection choices.
//
enum {
  LED_PIN = 4,
  TX_PIN = 26,
  RX_PIN = 27,
};

/**
* This example is written with the following circuit:
*
*  1.  Emic 2  GND pin to Gnd pin on the ESP32
*  2.  Emic 2   5v pin to  5v pin on the ESP32
*  3.  Emic 2 SOUT pin to GPIO pin 27 of the ESP32
*  4.  Emic 2  SIN pin to GPIO pin 26 of the ESP32
*  5.  Emic 2  SP- pin to - terminal of your speaker
*  6.  Emic 2  SP+ pin to - terminal of your speaker
*  7.  Cathode of a red LED to Gnd
*  8.  Anode of the red LED to one side of a 470 ohm resistor
*  9.  Other side of the resistor to GPIO pin 4 on the ESP32
*/

ESP32Emic2    speech(TX_PIN, RX_PIN);

void blink_led(int count=2, int duration=50) {
    while (count--) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(duration);
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    speech.setup();
    speech.stop_playback();
    speech.set_voice(DEFAULT_VOICE;
    speech.set_volume(QUIET_VOLUME;
    speech.set_words_per_minute(DEFAULT_WPM);
    speech.set_callback([]() { blink_led(); });
}

void loop() {
  static int pass = 0;
  char buffer[16];
  bool sync = true;
  bool dbg = false;
  
  bool old_debug = speech.set_debug(dbg);
  speech.stop_playback(sync);
  speech.set_voice(DEFAULT_VOICE, sync);
  speech.set_volume(QUIET_VOLUME, sync);
  speech.set_words_per_minute(DEFAULT_WPM, sync);
  speech.set_parser(DECTALK, sync);

  // stop after 3 passes
  if (pass >= 3) return;

  sprintf(buffer, "%d.\r", ++pass);

  speech.write((char*) "sStarting Speech Tests, pass number ");
  speech.write(buffer, true);

  strcpy(buffer, "Voice 0.");
  for (uint8_t voice=0; voice < 9; ++voice) {
    speech.set_voice(voice, true);
    buffer[6] = '0' + voice;
    speech.say(buffer, SYNC);
  }
  speech.read_and_show(1200);

  speech.say("[:rate 200][:n1][:dv ap 90 pr 0] All your base are belong to us.", SYNC);
  delay(300);
  speech.say("[:rate 200][:n1][:dv ap 90 pr 0] Intruder Alert, Intruder Alert, Movement in sector 5.", SYNC);
  delay(500);
  
  speech.set_voice(DEFAULT_VOICE, sync);
  speech.stop_playback(sync);
  speech.set_debug(old_debug);

  // demo 0
  //speech.demo(0, SYNC);

  // demo 1
  //speech.demo(1, SYNC);

  // demo 2
  //speech.demo(2, SYNC);

  speech.set_voice(DEFAULT_VOICE, sync);
  speech.set_volume(DEFAULT_VOLUME, sync);
  speech.say("The Speech Tests Have Now Completed.", SYNC);
  
  Serial.print("Finished speaking.\n\n");
}

