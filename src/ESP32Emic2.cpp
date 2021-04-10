/**
* @file   emic2.h
*
* @brief  Emic 2 text to speech module class
*
* @author Trent M. Wyatt
*
*/
#include "ESP32Emic2.h"

/**
* Construct an ESP32Emic2 object
*
*/
ESP32Emic2::ESP32Emic2(const int tx_gpio, const int rx_gpio) : 
    uart(HardwareSerial(2)),
    tx_pin(tx_gpio & 0xFF),
    rx_pin(rx_gpio & 0xFF),
    on_speak(nullptr),
    debug(false)
{
    // put additional construction code here
}

ESP32Emic2::~ESP32Emic2() {
  uart.end();
}

/**
 * @brief set the debug flag and return the previous value
 *
 * @param dbg true := turn on debug output; false := turn off
 *
 * @returns the previous debug value
 */
bool ESP32Emic2::set_debug(bool dbg) {
  std::swap(dbg, debug);
  return dbg;
}

/**
 * @brief get the transmit pin
 *
 * @returns the transmit pin
 */
int ESP32Emic2::get_tx_pin() { return tx_pin; }

/**
 * @brief get the receive pin
 *
 * @returns the receive pin
 */
int ESP32Emic2::get_rx_pin() { return rx_pin; }

/**
 * @brief Set a callback when we're about to speak
 *
 * @returns the previous callback or nullptr
 */
Callback ESP32Emic2::set_callback(Callback cb) {
  std::swap(cb, on_speak);
  return cb;
}

/**
* @brief Set up the baud rate, parity, stop bits, 
* and the pins used
*
*/
bool ESP32Emic2::setup() {
  uart.end();
  uart.begin(9600, SERIAL_8N1, get_rx_pin(), get_tx_pin(), false, 20UL);
  uart.setRxBufferSize(1024);
  uart.flush();
  stop_playback(true);
  set_parser(DECTALK, true);
  return true;
}

/**
* Write a single byte to the Emic-2
*/
int ESP32Emic2::write(char c) {
  if (on_speak) on_speak();
  return uart.write(c);
}

/**
* Write the specified bytes to the Emic-2
* Note: If length is 0 or is not supplied
* the data is assumed to be null (0x0)
* terminated.
*
*/
int ESP32Emic2::write(const char *data, bool sync, size_t length) {
  if (on_speak) on_speak();
  if (0 == length) { length = strlen(data); }
  int result = uart.write(data, length);
  if (sync) { waitfor(':'); }

  return result;
}

/**
* Wait for a response after issuing a command with optional timeout.
* 
* A timeout value of 0 (default) will not timeout
*
* Returns EMIC_OK if the next received byte matches
* Returns EMIC_FAIL if the next received byte DOES NOT matche and ignore == false
* Returns EMIC_TMOUT if the timeout is reached
*/
int ESP32Emic2::waitfor(int c, int timeout, bool ignore) {
  unsigned long timer = millis() + timeout;
  do {
    if (uart.available() > 0) {
      if (uart.read() == c) { return EMIC_OK; }
      if (!ignore) { return EMIC_FAIL; }
    }
  } while ((0 == timeout) || (millis() < timer));

  return EMIC_TMOUT;
}

/**
 * Read any received bytes and display them to the serial debugger.
 * Read any received bytes from the serial debugger and write them
 * to the Emic-2.
 */
void ESP32Emic2::read_and_show(int wait) {
  unsigned long timer = millis() + wait;
  do {
    bool recvd = false;
    size_t len = 0;
    do {
      if ((len = uart.available()) > 0) {
        recvd = true;
        char buffer[len + 1];
        buffer[len] = 0;
        if (uart.read(buffer, len) > 0) {
          printf("%s", buffer);
          delay(20);
        }
      }
    } while (len > 0);

    if (recvd) {
      recvd = false;        
      printf("\n");
    }

    if ((len = Serial.available()) > 0) {
      char buffer[len + 1];
      buffer[len] = 0;
      if (Serial.read(buffer, len) > 0) {
        uart.write(buffer, len);
        printf("%s", buffer);
      }
    }
  } while (millis() < timer);
}

/**
 * Speak the specified text.
 * 
 * if wait == 0 return immediately
 * if wait == SYNC wait for ':' and return
 * else delay(wait) and return
 */
int ESP32Emic2::say(const char *data, int wait) {
  if (on_speak) on_speak();
  if (debug) { printf("Writing 'Say \"%s\"' to Emic-2\n", data); }
  int sent = uart.write("S") + uart.write(data) + uart.write("\r");
  if (SYNC == wait) { waitfor(':'); } else { read_and_show(wait); }
  return sent;
}

/**
 * Play one the built in demos
 * 
 * if wait == 0 return immediately
 * if wait == SYNC wait for ':' and return
 * else read_and_show(wait) and return
 */
void ESP32Emic2::demo(char which, int wait) {
  if (debug) { printf("Writing 'Play Demo %d' to Emic-2\n", which); }
  char cmd[4] = { 'D', (char) ('0' + which), '\r', 0 };
  uart.write(cmd, 3);
  if (SYNC == wait) { waitfor(':'); } else { read_and_show(wait); }
}

/**
 * Stop playback (if any)
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::stop_playback(bool sync) {
  if (debug) { printf("Writing 'Stop Playing' to Emic-2\n"); }
  uart.write("X", 1);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Toggle playback (if any)
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::toggle_pause(bool sync) {
  if (debug) { printf("Writing 'Toggle Pause' to Emic-2\n"); }
  uart.write("Z", 1);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Select the specified built-in voice
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_voice(int which, bool sync) {
  if (debug) { printf("Writing 'Select Voice %d' to Emic-2\n", which); }
  char cmd[4] = { 'N', char('0' + which), '\r', 0 };
  uart.write(cmd, 3);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Select the specified built-in voice
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_volume(float volume, bool sync) {
  int vol = int(0.66f * volume - 48.0f);
  if (debug) { printf("Writing 'Volume = %.2f (%d)' to Emic-2\n", volume, vol); }
  char cmd[8];
  uart.write(cmd, sprintf(cmd, "V%d\r", vol));
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Select the speech rate in Words Per Minute
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_words_per_minute(int wpm, bool sync) {
  if (debug) { printf("Writing '%d Words/Minute' to Emic-2\n", wpm); }
  char cmd[8];
  uart.write(cmd, sprintf(cmd, "W%d\r", wpm));
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Select the parser (EPSON or DECTALK)
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_parser(int parser, bool sync) {
  if (debug) {
    printf("Writing 'Parser = %d (%s)' to Emic-2\n", parser, EPSON == parser ? "Epson" : EPSON == parser ? "DECtalk" : "unknown");
  }
  char cmd[4] { 'P', char('0' + parser), '\r', 0 };
  uart.write(cmd, 3);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Select the language (US English, Castilian Spanish, or Latin Spanish)
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_language(int language, bool sync) {
  if (debug) {
    printf("Writing 'Language = %d (%s)' to Emic-2\n", language, 0 == language ? "US English" : 1 == language ? "Castilian Spanish" : "Latin Spanish");
  }
  char cmd[4] { 'L', char('0' + language), '\r', 0 };
  uart.write(cmd, 3);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Revert to default text-to-speech settings
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::set_default(bool sync) {
  if (debug) { printf("Writing 'Revert to default text-to-speech settings' to Emic-2\n"); }
  uart.write("R\r", 2);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Print current text-to-speech settings
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::get_current(bool sync) {
  if (debug) { printf("Writing 'Print current text-to-speech settings' to Emic-2\n"); }
  uart.write("C\r", 2);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Print version information
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::get_info(bool sync) {
  if (debug) { printf("Writing 'Print version information' to Emic-2\n"); }
  uart.write("I\r", 2);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

/**
 * Print list of available commands
 * 
 * if sync == true wait for ':'
 * else read_and_show(SETTLE_TIME) and then return
 */
void ESP32Emic2::get_help(bool sync) {
  if (debug) { printf("Writing 'Print list of available commands' to Emic-2\n"); }
  uart.write("H\r", 2);
  if (sync) { waitfor(':', 1000, true); } else { read_and_show(SETTLE_TIME); }
}

void ESP32Emic2::test() {
  static int pass = 0;
  char buffer[16];
  bool sync = true;
  bool dbg = false;
  
  bool old_debug = set_debug(dbg);
  stop_playback(sync);
  set_voice(DEFAULT_VOICE, sync);
  set_volume(QUIET_VOLUME, sync);
  set_words_per_minute(DEFAULT_WPM, sync);
  set_parser(DECTALK, sync);

  // stop after 3 passes
  if (pass >= 3) return;

  sprintf(buffer, "%d.\r", ++pass);

  write((char*) "sStarting Speech Tests, pass number ");
  write(buffer, true);

  strcpy(buffer, "Voice 0.");
  for (uint8_t voice=0; voice < 9; ++voice) {
    set_voice(voice, true);
    buffer[6] = '0' + voice;
    say(buffer, SYNC);
  }
  read_and_show(1200);

  say("[:rate 200][:n1][:dv ap 90 pr 0] All your base are belong to us.", SYNC);
  delay(300);
  say("[:rate 200][:n1][:dv ap 90 pr 0] Intruder Alert, Intruder Alert, Movement in sector 5.", SYNC);
  delay(500);
  
  set_voice(DEFAULT_VOICE, sync);
  stop_playback(sync);
  set_debug(old_debug);

  // demo 0
  //demo(0, SYNC);

  // demo 1
  //demo(1, SYNC);

  // demo 2
  //demo(2, SYNC);

  set_voice(DEFAULT_VOICE, sync);
  set_volume(DEFAULT_VOLUME, sync);
  say("The Speech Tests Have Now Completed.", SYNC);
  
  Serial.print("Finished speaking.\n\n");
}
