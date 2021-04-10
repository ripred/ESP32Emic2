/**
* @file   ESP32Emic2.h
*
* @brief  Emic 2 text to speech module class
*
* @author Trent M. Wyatt
*
*/
#ifndef ESP32_EMIC2_H
#define ESP32_EMIC2_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <HardwareSerial.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

/**
* @file
* @name
* @date
* @author
* @brief 
* @note 
* @param 
* @return 
* @returns 
* @deprecated
*/

/**
* @brief values for use in set_parser()
*
* @note  EPSON is the deault on power up
*/
enum {
  DECTALK = 0,
  EPSON   = 1
};

/**
* @brief return values
*/
enum {
  EMIC_OK    = 0,
  EMIC_FAIL  = 1,
  EMIC_TMOUT = 2
};

/**
* @brief value used to indicate we want to wait for the reponse byte
*/
enum {
  SYNC = 0
};

/**
* @brief values used as system defaults
*/
#if !defined(DEFAULT_VOICE)
#define  DEFAULT_VOICE   1
#endif

#if !defined(SETTLE_TIME)
#define  SETTLE_TIME     25
#endif

#if !defined(QUIET_VOLUME)
#define  QUIET_VOLUME    62
#endif

#if !defined(DEFAULT_VOLUME)
#define  DEFAULT_VOLUME  75
#endif

#if !defined(LOUD_VOLUME)
#define  LOUD_VOLUME     85
#endif

#if !defined(DEFAULT_WPM)
#define  DEFAULT_WPM     230
#endif

#if !defined(DEFAULT_PARSER)
#define  DEFAULT_PARSER  DECTALK
#endif

/**
* @brief data type for callback function
*/
typedef void (*Callback)(void);

/**
* @brief definition of the ESP32Emic2 class for use in controlling
* the Emic 2 Speech Synthesizer module
*/
class ESP32Emic2 {
private:
  HardwareSerial  uart;
  const uint8_t   tx_pin;
  const uint8_t   rx_pin;
  Callback        on_speak;
  bool            debug;

public:
  /**
  * Construct an ESP32Emic2 object
  *
  */
  ESP32Emic2(const int tx_gpio, const int rx_gpio);
  ~ESP32Emic2();

  /**
   * @brief set the debug flag and return the previous value
   *
   * @param dbg true := turn on debug output; false := turn off
   *
   * @returns the previous debug value
   */
  bool set_debug(bool dbg);

  /**
   * @brief get the transmit pin
   *
   * @returns the transmit pin
   */
  int get_tx_pin();

  /**
   * @brief get the receive pin
   *
   * @returns the receive pin
   */
  int get_rx_pin();

  /**
   * @brief Set a callback when we're about to speak
   *
   * @returns the previous callback or nullptr
   */
  Callback set_callback(Callback cb);

  /**
  * @brief Set up the baud rate, parity, stop bits, 
  * and the pins used
  *
  */
  bool setup();

  /**
  * Write a single byte to the Emic-2
  */
  int write(char c);

  /**
  * Write the specified bytes to the Emic-2
  * Note: If length is 0 or is not supplied
  * the data is assumed to be null (0x0)
  * terminated.
  *
  */
  int write(const char *data, bool sync=false, size_t length=0);

  /**
  * Wait for a response after issuing a command with optional timeout.
  * 
  * A timeout value of 0 (default) will not timeout
  *
  * Returns EMIC_OK if the next received byte matches
  * Returns EMIC_FAIL if the next received byte DOES NOT matche and ignore == false
  * Returns EMIC_TMOUT if the timeout is reached
  */
  int waitfor(int c, int timeout=0, bool ignore=true);

  /**
   * Read any received bytes and display them to the serial debugger.
   * Read any received bytes from the serial debugger and write them
   * to the Emic-2.
   */
  void read_and_show(int wait=0);

  /**
   * Speak the specified text.
   * 
   * if wait == 0 return immediately
   * if wait == SYNC wait for ':' and return
   * else delay(wait) and return
   */
  int say(const char *data, int wait=0);

  /**
   * Play one the built in demos
   * 
   * if wait == 0 return immediately
   * if wait == SYNC wait for ':' and return
   * else read_and_show(wait) and return
   */
  void demo(char which=0, int wait=0);
  
  /**
   * Stop playback (if any)
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void stop_playback(bool sync=false);

  /**
   * Toggle playback (if any)
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void toggle_pause(bool sync=false);

  /**
   * Select the specified built-in voice
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_voice(int which, bool sync=false);

  /**
   * Select the specified built-in voice
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_volume(float volume, bool sync=false);

  /**
   * Select the speech rate in Words Per Minute
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_words_per_minute(int wpm, bool sync=false);

  /**
   * Select the parser (EPSON or DECTALK)
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_parser(int parser, bool sync=false);

  /**
   * Select the language (US English, Castilian Spanish, or Latin Spanish)
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_language(int language, bool sync=false);

  /**
   * Revert to default text-to-speech settings
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void set_default(bool sync=false);

  /**
   * Print current text-to-speech settings
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void get_current(bool sync=false);

  /**
   * Print version information
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void get_info(bool sync=false);

  /**
   * Print list of available commands
   * 
   * if sync == true wait for ':'
   * else read_and_show(SETTLE_TIME) and then return
   */
  void get_help(bool sync=false);

  /**
   * Run test / demo
   * 
   */
  void test();

};

#endif  // #ifndef ESP32_EMIC2_H
