/* Dialog NB-IOT, CAT-M fuctions are integrated. This is a redistribution of Adafruit Fona library
Both MQTT and HTTP protocols are supported.*/





/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Dialog_NBIOT.h"


Adafruit_FONA::Adafruit_FONA(int8_t rst)
{
  _rstpin = rst;

  apn = F("FONAnet");
  apnusername = 0;
  apnpassword = 0;
  mySerial = 0;
  httpsredirect = false;
  useragent = F("FONA");
  ok_reply = F("OK");
}

uint8_t Adafruit_FONA::type(void) {
  return _type;
}



/*********nb-IoT*************************************************************/
boolean Adafruit_FONA::connectDialogNBIoT() {
  if(! sendCheckReply(F("AT+CNMP=38"), ok_reply))
    return false;
  if(!sendCheckReply(F("AT+CMNB=2"), ok_reply))
    return false;
  if(!sendCheckReply(F("AT+NBSC=1"), ok_reply))
    return false;
  DEBUG_PRINTLN(F("\nNB-IOT configuration done...."));
  //if(!sendCheckReply(F("AT+CFUN=1,1"), ok_reply))
    //return false;
 return true;
}

boolean Adafruit_FONA::connectDialog_eMTC() {
  if(! sendCheckReply(F("AT+CNMP=38"), ok_reply))
    return false;
  if(!sendCheckReply(F("AT+CMNB=1"), ok_reply))
    return false;
  DEBUG_PRINTLN(F("\neMTC(CAT-M) configuration done...."));
  return true;
  }


uint16_t  Adafruit_FONA::getBattPercent(void) {
  uint16_t p;
     sendParseReply(F("AT+CBC"), F("+CBC: "), &p, ',', 1);
     return p;
}

/*****************full network status**************************************/
uint8_t Adafruit_FONA::getfullstatus(void) {
  uint16_t rssi;
  sendParseReply(F("AT+CPSI?"), F("+CPSI:"),&rssi,',',3);
  return rssi;
}
/*****************full network status**************************************/

/******mode selection LTE/GSM*******/
uint8_t Adafruit_FONA::getMode(void) {
  uint16_t mode;
  sendParseReply(F("AT+CNMP?"), F("+CNMP: "),&mode,',',0);
  return mode;
}
/******mode selection LTE/GSM*******/

/******mode selection CAT-M/nb-IoT*******/
uint8_t Adafruit_FONA::getBandMode(void) {
  uint16_t bmode;
  sendParseReply(F("AT+CMNB?"), F("+CMNB: "),&bmode,',',0);
  return bmode;
}
/******mode selection CAT-M/nb-IoT*******/


/******Nb iot status**********************************************************/

uint8_t Adafruit_FONA::getNB_IoT_status(void) {
  uint16_t state;
  sendParseReply(F("AT+COPS?"), F("+COPS:"),&state,',',3);
  return state;
}
/******Nb iot status*******/

/*********gsm **********/
boolean Adafruit_FONA::connectDialogGsm() {
  if(!sendCheckReply(F("AT+CNMP=13"), ok_reply))
    return false;
    DEBUG_PRINTLN(F("\nGSM setup complete.."));
  //  if(!sendCheckReply(F("AT+CFUN=1,1"), ok_reply))
    //  return false;
 return true;
}
/*********gsm **********/



/*********nb-IoT**********/



boolean Adafruit_FONA::begin(Stream &port) {
  mySerial = &port;

  if (_rstpin != 99) { // Pulse the reset pin only if it's not an LTE module
  	DEBUG_PRINTLN(F("Resetting the module..."));
    pinMode(_rstpin, OUTPUT);
    digitalWrite(_rstpin, HIGH);
    delay(10);
    digitalWrite(_rstpin, LOW);
    delay(100);
    digitalWrite(_rstpin, HIGH);
  }

  DEBUG_PRINTLN(F("Attempting to open comm with ATs"));
  // give 7 seconds to reboot
  int16_t timeout = 7000;

  while (timeout > 0) {
    while (mySerial->available()) mySerial->read();
    if (sendCheckReply(F("AT"), ok_reply))
      break;
    while (mySerial->available()) mySerial->read();
    if (sendCheckReply(F("AT"), F("AT")))
      break;
    delay(500);
    timeout-=500;
  }

  if (timeout <= 0) {
#ifdef ADAFRUIT_FONA_DEBUG
    DEBUG_PRINTLN(F("Timeout: No response to AT... last ditch attempt."));
#endif
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
  }

  // turn off Echo!
  sendCheckReply(F("ATE0"), ok_reply);
  delay(100);

  if (! sendCheckReply(F("ATE0"), ok_reply)) {
    return false;
  }

  // turn on hangupitude
  if (_rstpin != 99) sendCheckReply(F("AT+CVHU=0"), ok_reply);

  delay(100);
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("ATI");

  mySerial->println("ATI");
  readline(500, true);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);



  if (prog_char_strstr(replybuffer, (prog_char *)F("SIM808 R14")) != 0) {
    _type = FONA808_V2;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM808 R13")) != 0) {
    _type = FONA808_V1;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM800 R13")) != 0) {
    _type = FONA800L;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIMCOM_SIM5320A")) != 0) {
    _type = FONA3G_A;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIMCOM_SIM5320E")) != 0) {
    _type = FONA3G_E;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000A R13")) != 0) {
    _type = FONA_LTE_A;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000C R13")) != 0) {
    _type = FONA_LTE_C;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000E R13")) != 0) {
    _type = FONA_LTE_E;
  }


  if (_type == FONA800L) {
    // determine if L or H

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("AT+GMM");

    mySerial->println("AT+GMM");
    readline(500, true);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);


    if (prog_char_strstr(replybuffer, (prog_char *)F("SIM800H")) != 0) {
      _type = FONA800H;
    }
  }

  return true;
}


/********* Serial port ********************************************/
boolean Adafruit_FONA::setBaudrate(uint16_t baud) {
  return sendCheckReply(F("AT+IPREX="), baud, ok_reply);
}

boolean Adafruit_FONA_LTE::setBaudrate(uint16_t baud) {
  return sendCheckReply(F("AT+IPR="), baud, ok_reply);
}


/********* Real Time Clock ********************************************/

boolean Adafruit_FONA::readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec) {
  uint16_t v;
  sendParseReply(F("AT+CCLK?"), F("+CCLK: "), &v, '/', 0);
  *year = v;

  DEBUG_PRINTLN(*year);
}

boolean Adafruit_FONA::enableRTC(uint8_t i) {
  if (! sendCheckReply(F("AT+CLTS="), i, ok_reply))
    return false;
  return sendCheckReply(F("AT&W"), ok_reply);
}


/********* POWER, BATTERY & ADC ********************************************/

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA::getBattVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), v, ',', 2);
}


/* returns value in mV (uint16_t) */
boolean Adafruit_FONA_3G::getBattVoltage(uint16_t *v) {
  float f;
  boolean b = sendParseReply(F("AT+CBC"), F("+CBC: "), &f, ',', 2);
  *v = f*1000;
  return b;
}


/* powers down the SIM module */
boolean Adafruit_FONA::powerDown(void) {
  if (! sendCheckReply(F("AT+CPOWD=1"), F("NORMAL POWER DOWN"))) // Normal power off
    return false;

  return true;
}

/* powers down the SIM5320 */
boolean Adafruit_FONA_3G::powerDown(void) {
  if (! sendCheckReply(F("AT+CPOF"), ok_reply))
    return false;

  return true;
}


boolean Adafruit_FONA::getADCVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CADC?"), F("+CADC: 1,"), v);
}

/********* SIM ***********************************************************/

uint8_t Adafruit_FONA::unlockSIM(char *pin)
{
  char sendbuff[14] = "AT+CPIN=";
  sendbuff[8] = pin[0];
  sendbuff[9] = pin[1];
  sendbuff[10] = pin[2];
  sendbuff[11] = pin[3];
  sendbuff[12] = '\0';

  return sendCheckReply(sendbuff, ok_reply);
}

uint8_t Adafruit_FONA::getSIMCCID(char *ccid) {
  getReply(F("AT+CCID"));
  // up to 28 chars for reply, 20 char total ccid
  if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(ccid, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(ccid, replybuffer, 20);
  }
  ccid[20] = 0;

  readline(); // eat 'OK'

  return strlen(ccid);
}

/********* IMEI **********************************************************/

uint8_t Adafruit_FONA::getIMEI(char *imei) {
  getReply(F("AT+GSN"));

  // up to 15 chars
  strncpy(imei, replybuffer, 15);
  imei[15] = 0;

  readline(); // eat 'OK'

  return strlen(imei);
}

/********* NETWORK *******************************************************/

uint8_t Adafruit_FONA::getNetworkStatus(void) {
  uint16_t status;

  if (! sendParseReply(F("AT+CREG?"), F("+CREG: "), &status, ',', 1)) return 0;

  return status;
}


uint8_t Adafruit_FONA::getRSSI(void) {
  uint16_t reply;

  if (! sendParseReply(F("AT+CSQ"), F("+CSQ: "), &reply) ) return 0;

  return reply;
}



/********* TIME **********************************************************/

boolean Adafruit_FONA::enableNetworkTimeSync(boolean onoff) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CLTS=1"), ok_reply))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CLTS=0"), ok_reply))
      return false;
  }

  flushInput(); // eat any 'Unsolicted Result Code'

  return true;
}

boolean Adafruit_FONA::enableNTPTimeSync(boolean onoff, FONAFlashStringPtr ntpserver) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CNTPCID=1"), ok_reply))
      return false;

    mySerial->print(F("AT+CNTP=\""));
    if (ntpserver != 0) {
      mySerial->print(ntpserver);
    } else {
      mySerial->print(F("pool.ntp.org"));
    }
    mySerial->println(F("\",0"));
    readline(FONA_DEFAULT_TIMEOUT_MS);
    if (strcmp(replybuffer, "OK") != 0)
      return false;

    if (! sendCheckReply(F("AT+CNTP"), ok_reply, 10000))
      return false;

    uint16_t status;
    readline(10000);
    if (! parseReply(F("+CNTP:"), &status))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CNTPCID=0"), ok_reply))
      return false;
  }

  return true;
}

boolean Adafruit_FONA::getTime(char *buff, uint16_t maxlen) {
  getReply(F("AT+CCLK?"), (uint16_t) 10000);
  if (strncmp(replybuffer, "+CCLK: ", 7) != 0)
    return false;

  char *p = replybuffer+7;
  uint16_t lentocopy = min(maxlen-1, strlen(p));
  strncpy(buff, p, lentocopy+1);
  buff[lentocopy] = 0;

  readline(); // eat OK

  return true;
}

/********* GPS **********************************************************/


boolean Adafruit_FONA::enableGPS(boolean onoff) {
  uint16_t state;

  // First check if its already on or off

  if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
    if (! sendParseReply(F("AT+CGNSPWR?"), F("+CGNSPWR: "), &state) )
      return false;
  } else {
    if (! sendParseReply(F("AT+CGPSPWR?"), F("+CGPSPWR: "), &state))
      return false;
  }

  if (onoff && !state) {
    if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
      if (! sendCheckReply(F("AT+CGNSPWR=1"), ok_reply))  // try GNS command
	return false;
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=1"), ok_reply))
	return false;
    }
  } else if (!onoff && state) {
    if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
      if (! sendCheckReply(F("AT+CGNSPWR=0"), ok_reply)) // try GNS command
	return false;
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=0"), ok_reply))
	return false;
    }
  }
  return true;
}


boolean Adafruit_FONA_3G::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off
  if (! Adafruit_FONA::sendParseReply(F("AT+CGPS?"), F("+CGPS: "), &state) )
    return false;

  if (onoff && !state) {
    if (! sendCheckReply(F("AT+CGPS=1"), ok_reply))
      return false;
  } else if (!onoff && state) {
    if (! sendCheckReply(F("AT+CGPS=0"), ok_reply))
      return false;
    // this takes a little time
    readline(2000); // eat '+CGPS: 0'
  }
  return true;
}

int8_t Adafruit_FONA::GPSstatus(void) {
  if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGNSINF"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGNSINF: "));
    if (p == 0) return -1;
    p+=10;
    readline(); // eat 'OK'
    if (p[0] == '0') return 0; // GPS is not even on!

    p+=2; // Skip to second value, fix status.
    //DEBUG_PRINTLN(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 1;
  }
  if (_type == FONA3G_A || _type == FONA3G_E) {
    // FONA 3G doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGPSINFO"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGPSINFO:"));
    if (p == 0) return -1;
    if (p[10] != ',') return 3; // if you get anything, its 3D fix
    return 0;
  }
  else {
    // 808 V1 looks for specific 2D or 3D fix state.
    getReply(F("AT+CGPSSTATUS?"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("SSTATUS: Location "));
    if (p == 0) return -1;
    p+=18;
    readline(); // eat 'OK'
    //DEBUG_PRINTLN(p);
    if (p[0] == 'U') return 0;
    if (p[0] == 'N') return 1;
    if (p[0] == '2') return 2;
    if (p[0] == '3') return 3;
  }
  // else
  return 0;
}

uint8_t Adafruit_FONA::getGPS(uint8_t arg, char *buffer, uint8_t maxbuff) {
  int32_t x = arg;

  if ( (_type == FONA3G_A) || (_type == FONA3G_E) ) {
    getReply(F("AT+CGPSINFO"));
  } else if (_type == FONA808_V1) {
    getReply(F("AT+CGPSINF="), x);
  } else {
    getReply(F("AT+CGNSINF"));
  }

  char *p = prog_char_strstr(replybuffer, (prog_char*)F("SINF"));
  if (p == 0) {
    buffer[0] = 0;
    return 0;
  }

  p+=6;

  uint8_t len = max(maxbuff-1, strlen(p));
  strncpy(buffer, p, len);
  buffer[len] = 0;

  readline(); // eat 'OK'
  return len;
}

boolean Adafruit_FONA::getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude) {

  char gpsbuffer[120];

  // we need at least a 2D fix
  if (GPSstatus() < 2)
    return false;

  // grab the mode 2^5 gps csv from the sim808
  uint8_t res_len = getGPS(32, gpsbuffer, 120);

  // make sure we have a response
  if (res_len == 0)
    return false;

  if (_type == FONA3G_A || _type == FONA3G_E) {
    // Parse 3G respose
    // +CGPSINFO:4043.000000,N,07400.000000,W,151015,203802.1,-12.0,0.0,0
    // skip beginning
    char *tok;

   // grab the latitude
    char *latp = strtok(gpsbuffer, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    // skip date & time
    tok = strtok(NULL, ",");
    tok = strtok(NULL, ",");

   // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;
      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

  } else if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
    // Parse 808 V2 response.  See table 2-3 from here for format:
    // http://www.adafruit.com/datasheets/SIM800%20Series_GNSS_Application%20Note%20V1.00.pdf

    // skip GPS run status
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip fix status
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    *lat = atof(latp);
    *lon = atof(longp);

    // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;

      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }
  }
  else {
    // Parse 808 V1 response.

    // skip mode
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip fix
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

    // only grab speed if needed
    if (speed_kph != NULL) {

      // grab the speed in knots
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      // convert to kph
      *speed_kph = atof(speedp) * 1.852;

    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);

    }

    // no need to continue
    if (altitude == NULL)
      return true;

    // we need at least a 3D fix for altitude
    if (GPSstatus() < 3)
      return false;

    // grab the mode 0 gps csv from the sim808
    res_len = getGPS(0, gpsbuffer, 120);

    // make sure we have a response
    if (res_len == 0)
      return false;

    // skip mode
    tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip lat
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip long
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab altitude
    char *altp = strtok(NULL, ",");
    if (! altp) return false;

    *altitude = atof(altp);
  }

  return true;

}

boolean Adafruit_FONA::enableGPSNMEA(uint8_t i) {

  char sendbuff[15] = "AT+CGPSOUT=000";
  sendbuff[11] = (i / 100) + '0';
  i %= 100;
  sendbuff[12] = (i / 10) + '0';
  i %= 10;
  sendbuff[13] = i + '0';

  if (_type == FONA808_V2 || _type == FONA_LTE_A || _type == FONA_LTE_C || _type == FONA_LTE_E) {
    if (i) {
    	sendCheckReply(F("AT+CGNSCFG=1"), ok_reply);
      sendCheckReply(F("AT+CGNSTST=1"), ok_reply);
      return true;
    }
    else
      return sendCheckReply(F("AT+CGNSTST=0"), ok_reply);
  } else {
    return sendCheckReply(sendbuff, ok_reply, 2000);
  }
}


/********* GPRS **********************************************************/


boolean Adafruit_FONA::enableGPRS(boolean onoff) {

  if (onoff) {
  	// if (_type < FONA_LTE_A) { // UNCOMMENT FOR 4G ONLY!
	    // disconnect all sockets
	    sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000);

	    if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
	      return false;

		// set bearer profile! connection type GPRS
		if (! sendCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), ok_reply, 10000))
	  		return false;
    // } // UNCOMMENT FOR 4G ONLY!

	 	delay(100); // This seems to help the next line run the first time

    // set bearer profile access point name
    if (apn) {
      // Send command AT+SAPBR=3,1,"APN","<apn value>" where <apn value> is the configured APN value.
      if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"APN\","), apn, ok_reply, 10000))
        return false;

      // send AT+CSTT,"apn","user","pass"
      flushInput();

      mySerial->print(F("AT+CSTT=\""));
      mySerial->print(apn);
      if (apnusername) {
		mySerial->print("\",\"");
		mySerial->print(apnusername);
      }
      if (apnpassword) {
		mySerial->print("\",\"");
		mySerial->print(apnpassword);
      }
      mySerial->println("\"");

      DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(F("AT+CSTT=\""));
      DEBUG_PRINT(apn);

      if (apnusername) {
		DEBUG_PRINT("\",\"");
		DEBUG_PRINT(apnusername);
      }
      if (apnpassword) {
		DEBUG_PRINT("\",\"");
		DEBUG_PRINT(apnpassword);
      }
      DEBUG_PRINTLN("\"");

      if (! expectReply(ok_reply)) return false;

      // set username/password
      if (apnusername) {
        // Send command AT+SAPBR=3,1,"USER","<user>" where <user> is the configured APN username.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"USER\","), apnusername, ok_reply, 10000))
          return false;
      }
      if (apnpassword) {
        // Send command AT+SAPBR=3,1,"PWD","<password>" where <password> is the configured APN password.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"PWD\","), apnpassword, ok_reply, 10000))
          return false;
      }
    }

    // open bearer
    if (! sendCheckReply(F("AT+SAPBR=1,1"), ok_reply, 30000))
      return false;

  	// if (_type < FONA_LTE_A) { // UNCOMMENT FOR 4G ONLY!
	    // bring up wireless connection
	    if (! sendCheckReply(F("AT+CIICR"), ok_reply, 10000))
	      return false;
	// } // UNCOMMENT FOR 4G ONLY!

  } else {
    // disconnect all sockets
    if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000))
      return false;

    // close bearer
    if (! sendCheckReply(F("AT+SAPBR=0,1"), ok_reply, 10000))
      return false;

  	// if (_type < FONA_LTE_A) { // UNCOMMENT FOR 4G ONLY!
	    if (! sendCheckReply(F("AT+CGATT=0"), ok_reply, 10000))
	      return false;
	// } // UNCOMMENT FOR 4G ONLY!

  }
  return true;
}

boolean Adafruit_FONA_3G::enableGPRS(boolean onoff) {

  if (onoff) {
    // disconnect all sockets
    //sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000);

    if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
      return false;


    // set bearer profile access point name
    if (apn) {
      // Send command AT+CGSOCKCONT=1,"IP","<apn value>" where <apn value> is the configured APN name.
      if (! sendCheckReplyQuoted(F("AT+CGSOCKCONT=1,\"IP\","), apn, ok_reply, 10000))
        return false;

      // set username/password
      if (apnusername) {
	char authstring[100] = "AT+CGAUTH=1,1,\"";
	char *strp = authstring + strlen(authstring);
	prog_char_strcpy(strp, (prog_char *)apnusername);
	strp+=prog_char_strlen((prog_char *)apnusername);
	strp[0] = '\"';
	strp++;
	strp[0] = 0;

	if (apnpassword) {
	  strp[0] = ','; strp++;
	  strp[0] = '\"'; strp++;
	  prog_char_strcpy(strp, (prog_char *)apnpassword);
	  strp+=prog_char_strlen((prog_char *)apnpassword);
	  strp[0] = '\"';
	  strp++;
	  strp[0] = 0;
	}

	if (! sendCheckReply(authstring, ok_reply, 10000))
	  return false;
      }
    }

    // connect in transparent
    if (! sendCheckReply(F("AT+CIPMODE=1"), ok_reply, 10000))
      return false;
    // open network (?)
    if (! sendCheckReply(F("AT+NETOPEN=,,1"), F("Network opened"), 10000))
      return false;

    readline(); // eat 'OK'
  } else {
    // close GPRS context
    if (! sendCheckReply(F("AT+NETCLOSE"), F("Network closed"), 10000))
      return false;

    readline(); // eat 'OK'
  }

  return true;
}

void Adafruit_FONA::getNetworkInfo(void) {
	getReply(F("AT+CPSI?"));
	getReply(F("AT+COPS?"));
}

uint8_t Adafruit_FONA::GPRSstate(void) {
  uint16_t state;

  if (! sendParseReply(F("AT+CGATT?"), F("+CGATT: "), &state) )
    return -1;

  return state;
}

void Adafruit_FONA::setGPRSNetworkSettings(FONAFlashStringPtr apn,
              FONAFlashStringPtr username, FONAFlashStringPtr password) {
  this->apn = apn;
  this->apnusername = username;
  this->apnpassword = password;
}

boolean Adafruit_FONA::getGSMLoc(uint16_t *errorcode, char *buff, uint16_t maxlen) {

  getReply(F("AT+CIPGSMLOC=1,1"), (uint16_t)10000);

  if (! parseReply(F("+CIPGSMLOC: "), errorcode))
    return false;

  char *p = replybuffer+14;
  uint16_t lentocopy = min(maxlen-1, strlen(p));
  strncpy(buff, p, lentocopy+1);

  readline(); // eat OK

  return true;
}

boolean Adafruit_FONA::getGSMLoc(float *lat, float *lon) {

  uint16_t returncode;
  char gpsbuffer[120];

  // make sure we could get a response
  if (! getGSMLoc(&returncode, gpsbuffer, 120))
    return false;

  // make sure we have a valid return code
  if (returncode != 0)
    return false;

  // +CIPGSMLOC: 0,-74.007729,40.730160,2015/10/15,19:24:55
  // tokenize the gps buffer to locate the lat & long
  char *longp = strtok(gpsbuffer, ",");
  if (! longp) return false;

  char *latp = strtok(NULL, ",");
  if (! latp) return false;

  *lat = atof(latp);
  *lon = atof(longp);

  return true;

}

boolean Adafruit_FONA::postData(const char *request_type, const char *URL, char *body, const char *token) {
  // NOTE: Need to open socket/enable GPRS before using this function

  // Make sure HTTP service is terminated so initialization will run
  sendCheckReply(F("AT+HTTPTERM"), ok_reply, 10000);

  // Initialize HTTP service
  if (! sendCheckReply(F("AT+HTTPINIT"), ok_reply, 10000))
    return false;

  // Set HTTP parameters
  if (! sendCheckReply(F("AT+HTTPPARA=\"CID\",1"), ok_reply, 10000))
    return false;

  // Specify URL
  char auxStr[64];
  sprintf(auxStr, "AT+HTTPPARA=\"URL\",\"%s\"", URL);
  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  // Perform request based on specified request type
  if (request_type == "GET") {
  	if (! sendCheckReply(F("AT+HTTPACTION=0"), ok_reply, 10000))
    	return false;
  }
  else if (request_type == "POST" && strlen(body) > 0) { // POST with content body
  	if (! sendCheckReply(F("AT+HTTPPARA=\"CONTENT\",\"application/json\""), ok_reply, 10000))
    	return false;

    if (strlen(token) > 0) {
	  	sprintf(auxStr, "AT+HTTPPARA=\"USERDATA\",\"Authorization: Bearer %s\"", token);

	  	if (! sendCheckReply(auxStr, ok_reply, 10000))
	  		return false;
	  }

		sprintf(auxStr, "AT+HTTPDATA=%d,10000", strlen(body));
		if (! sendCheckReply(auxStr, "DOWNLOAD", 10000))
	    return false;

		if (! sendCheckReply(body, ok_reply, 10000))
	    return false;

  	if (! sendCheckReply(F("AT+HTTPACTION=1"), ok_reply, 10000))
    	return false;
  }
  else if (request_type == "POST" && strlen(body) == 0) { // POST with query parameters
  	if (! sendCheckReply(F("AT+HTTPACTION=1"), ok_reply, 10000))
    	return false;
  }
  else if (request_type == "HEAD") {
  	if (! sendCheckReply(F("AT+HTTPACTION=2"), ok_reply, 10000))
    	return false;
  }

  // Parse response status and size
  uint16_t status, datalen;
  readline(10000);
  if (! parseReply(F("+HTTPACTION:"), &status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), &datalen, ',', 2))
    return false;

  DEBUG_PRINT("HTTP status: "); DEBUG_PRINTLN(status);
  DEBUG_PRINT("Data length: "); DEBUG_PRINTLN(datalen);

  if (status != 200) return false;

  getReply(F("AT+HTTPREAD"));

  // Terminate HTTP service
  sendCheckReply(F("AT+HTTPTERM"), ok_reply, 10000);

  return true;
}

/********************************* 3G HTTPS FUNCTION *********************************/
boolean Adafruit_FONA_3G::postData3G(const char *server, uint16_t port, const char *connType, char *URL) {
  // NOTE: Need to open socket/enable GPRS before using this function

  // Sample request URL: "GET /dweet/for/{deviceID}?temp={temp}&batt={batt} HTTP/1.1\r\nHost: dweet.io\r\nContent-Length: 0\r\n\r\n"

  // Start HTTPS stack
  if (! sendCheckReply(F("AT+CHTTPSSTART"), ok_reply, 10000))
    return false;

  // Construct the AT command based on function parameters
  char auxStr[64];
  uint8_t connTypeNum = 1;

  if (strcmp(connType, "HTTP") == 0) {
  	connTypeNum = 1;
  }
  if (strcmp(connType, "HTTPS") == 0) {
  	connTypeNum = 2;
  }

  sprintf(auxStr, "AT+CHTTPSOPSE=\"%s\",%d,%d", server, port, connTypeNum);

  // Connect to HTTPS server
  // if (! sendCheckReply(F("AT+CHTTPSOPSE=\"www.dweet.io\",443,2"), ok_reply, 10000)) // Use port 443 and HTTPS
  //   return false;
  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  // Send data to server (takes about 10s to send to dweet.io)
  sprintf(auxStr, "AT+CHTTPSSEND=%i", strlen(URL));

  if (! sendCheckReply(auxStr, ">", 10000))
    return false;

  if (! sendCheckReply(URL, ok_reply, 10000))
    return false;

  // Request URL
  if (! sendCheckReply(F("AT+CHTTPSSEND"), ok_reply, 10000))
    return false;

  // Receive HTTPS response
  if (! sendCheckReply(F("AT+CHTTPSRECV=1"), ok_reply, 10000))
    return false;

  // Close HTTP/HTTPS Session
  if (! sendCheckReply(F("AT+CHTTPSCLSE"), ok_reply, 10000))
    return false;

  // Stop HTTP/HTTPS stack
  if (! sendCheckReply(F("AT+CHTTPSSTOP"), ok_reply, 10000))
    return false;

  return true;
}

/********* MQTT FUNCTIONS  ************************************/

////////////////////////////////////////////////////////////
// MQTT helper functions
void Adafruit_FONA::mqtt_connect_message(char *protocol, byte *mqtt_message, char *clientID, char *username, char *password) {
  uint8_t i = 0;
  byte protocol_length = strlen(protocol);
  byte ID_length = strlen(clientID);
  byte username_length = strlen(username);
  byte password_length = strlen(password);

	mqtt_message[0] = 16;                      // MQTT message type CONNECT

	byte rem_length = 6 + protocol_length;
	// Each parameter will add 2 bytes + parameter length
	if (ID_length > 0) {
		rem_length += 2 + ID_length;
	}
	if (username_length > 0) {
		rem_length += 2 + username_length;
	}
	if (password_length > 0) {
		rem_length += 2 + password_length;
	}

	mqtt_message[1] = rem_length;             // Remaining length of message
	mqtt_message[2] = 0;                       // Protocol name length MSB
	mqtt_message[3] = 6;                       // Protocol name length LSB

	// Use the given protocol name (for example, "MQTT" or "MQIsdp")
	for (int i=0; i<protocol_length; i++) {
		mqtt_message[4 + i] = byte(protocol[i]);
	}

	mqtt_message[4 + protocol_length] = 3;                      // MQTT protocol version

	if (username_length > 0 && password_length > 0) { // has everything
		mqtt_message[5 + protocol_length] = 194;                  // Connection flag with username and password (11000010)
	}
	else if (password_length == 0) { // Only has username
		mqtt_message[5 + protocol_length] = 130;										 // Connection flag with username only (10000010)
	}
	else if (username_length == 0) {	// Only has password
		mqtt_message[5 + protocol_length] = 66;										 // Connection flag with password only (01000010)
	}

	mqtt_message[6 + protocol_length] = 0;                      // Keep-alive time MSB
	mqtt_message[7 + protocol_length] = 15;                     // Keep-alive time LSB
	mqtt_message[8 + protocol_length] = 0;                      // Client ID length MSB
	mqtt_message[9 + protocol_length] = ID_length;       			 // Client ID length LSB

  // Client ID
	for(i = 0; i < ID_length; i++) {
    mqtt_message[10 + protocol_length + i] = clientID[i];
	}

	// Username
	if (username_length > 0) {
		mqtt_message[10 + protocol_length + ID_length] = 0;                     // username length MSB
		mqtt_message[11 + protocol_length + ID_length] = username_length;       // username length LSB

		for(i = 0; i < username_length; i++) {
      mqtt_message[12 + protocol_length + ID_length + i] = username[i];
  	}
	}

	// Password
	if (password_length > 0) {
		mqtt_message[12 + protocol_length + ID_length + username_length] = 0;                     // password length MSB
		mqtt_message[13 + protocol_length + ID_length + username_length] = password_length;       // password length LSB

		for(i = 0; i < password_length; i++) {
      mqtt_message[14 + protocol_length + ID_length + username_length + i] = password[i];
  	}
	}
}

void Adafruit_FONA::mqtt_publish_message(byte *mqtt_message, char *topic, char *message) {
  uint8_t i = 0;
  byte topic_length = strlen(topic);
  byte message_length = strlen(message);

	mqtt_message[0] = 48;                                  // MQTT Message Type PUBLISH
	mqtt_message[1] = 2 + topic_length + message_length;   // Remaining length
	mqtt_message[2] = 0;                                   // Topic length MSB
	mqtt_message[3] = topic_length;                    		 // Topic length LSB

  // Topic
  for(i = 0; i < topic_length; i++) {
    mqtt_message[4 + i] = topic[i];
  }

  // Message
  for(i = 0; i < message_length; i++) {
    mqtt_message[4 + topic_length + i] = message[i];
  }
}

void Adafruit_FONA::mqtt_subscribe_message(byte *mqtt_message, char *topic, byte QoS) {
  uint8_t i = 0;
  byte topic_length = strlen(topic);

	mqtt_message[0] = 130;                // MQTT Message Type SUBSCRIBE
	mqtt_message[1] = 5 + topic_length;   // Remaining length
	mqtt_message[2] = 0;                  // Packet ID MSB
	mqtt_message[3] = 1;                  // Packet ID LSB
	mqtt_message[4] = 0;                  // Topic length MSB
	mqtt_message[5] = topic_length;       // Topic length LSB

  // Topic
  for(i = 0; i < topic_length; i++) {
      mqtt_message[6 + i] = topic[i];
  }

  mqtt_message[6 + topic_length] = QoS;   // QoS byte
}

void Adafruit_FONA::mqtt_disconnect_message(byte *mqtt_message) {
	mqtt_message[0] = 0xE0; // msgtype = connect
	mqtt_message[1] = 0x00; // length of message (?)
}

boolean Adafruit_FONA::mqtt_sendPacket(byte *packet, byte len) {
	// Send packet and get response
	DEBUG_PRINT(F("\t---> "));

	for (int j = 0; j < len; j++) {
		// if (packet[j] == NULL) break; // We've reached the end of the actual content
	  mySerial->write(packet[j]); // Needs to be "write" not "print"
	  DEBUG_PRINT(packet[j]); // Message contents
	  DEBUG_PRINT(" "); // Space out the bytes
  }
  mySerial->write(byte(26)); // End of packet
  DEBUG_PRINT(byte(26));

  readline(3000); // Wait up to 3 seconds to send the data
	DEBUG_PRINTLN("");
  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

	return (strcmp(replybuffer, "SEND OK") == 0);
}

////////////////////////////////////////////////////////////

boolean Adafruit_FONA::MQTTconnect(char *protocol, char *clientID, char *username, char *password) {
	flushInput();
	mySerial->println(F("AT+CIPSEND"));
	readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
	mqtt_connect_message(protocol, mqtt_message, clientID, username, password);


  if (! mqtt_sendPacket(mqtt_message, 20+strlen(clientID)+strlen(username)+strlen(password))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTpublish(char* topic, char* message) {
	flushInput();
	mySerial->println(F("AT+CIPSEND"));
	readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
  mqtt_publish_message(mqtt_message, topic, message);

  if (!mqtt_sendPacket(mqtt_message, 4+strlen(topic)+strlen(message))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTsubscribe(char* topic, byte QoS) {
	flushInput();
	mySerial->println(F("AT+CIPSEND"));
	readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
  mqtt_subscribe_message(mqtt_message, topic, QoS);

  if (!mqtt_sendPacket(mqtt_message, 7+strlen(topic))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTunsubscribe(char* topic) {

}

boolean Adafruit_FONA::MQTTreceive(char* topic, char* buf, int maxlen) {

}

boolean Adafruit_FONA::MQTTdisconnect(void) {

}


/********* TCP FUNCTIONS  ************************************/


boolean Adafruit_FONA::TCPconnect(char *server, uint16_t port) {
  flushInput();

  // close all old connections
  if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000) ) return false;

  // single connection at a time
  if (! sendCheckReply(F("AT+CIPMUX=0"), ok_reply) ) return false;

  // manually read data
  if (! sendCheckReply(F("AT+CIPRXGET=1"), ok_reply) ) return false;


  DEBUG_PRINT(F("AT+CIPSTART=\"TCP\",\""));
  DEBUG_PRINT(server);
  DEBUG_PRINT(F("\",\""));
  DEBUG_PRINT(port);
  DEBUG_PRINTLN(F("\""));


  mySerial->print(F("AT+CIPSTART=\"TCP\",\""));
  mySerial->print(server);
  mySerial->print(F("\",\""));
  mySerial->print(port);
  mySerial->println(F("\""));

  if (! expectReply(ok_reply)) return false;
  if (! expectReply(F("CONNECT OK"))) return false;

  // looks like it was a success (?)
  return true;
}

boolean Adafruit_FONA::TCPclose(void) {
  return sendCheckReply(F("AT+CIPCLOSE"), F("CLOSE OK"));
}

boolean Adafruit_FONA::TCPconnected(void) {
  if (! sendCheckReply(F("AT+CIPSTATUS"), ok_reply, 100) ) return false;
  readline(100);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, "STATE: CONNECT OK") == 0);
}

boolean Adafruit_FONA::TCPsend(char *packet, uint8_t len) {

  DEBUG_PRINT(F("AT+CIPSEND="));
  DEBUG_PRINTLN(len);
#ifdef ADAFRUIT_FONA_DEBUG
  for (uint16_t i=0; i<len; i++) {
  DEBUG_PRINT(F(" 0x"));
  DEBUG_PRINT(packet[i], HEX);
  }
#endif
  DEBUG_PRINTLN();


  mySerial->print(F("AT+CIPSEND="));
  mySerial->println(len);
  readline();

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  if (replybuffer[0] != '>') return false;

  mySerial->write(packet, len);
  readline(3000); // wait up to 3 seconds to send the data

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);


  return (strcmp(replybuffer, "SEND OK") == 0);
}

uint16_t Adafruit_FONA::TCPavailable(void) {
  uint16_t avail;

  if (! sendParseReply(F("AT+CIPRXGET=4"), F("+CIPRXGET: 4,"), &avail, ',', 0) ) return false;


  DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes available"));


  return avail;
}


uint16_t Adafruit_FONA::TCPread(uint8_t *buff, uint8_t len) {
  uint16_t avail;

  mySerial->print(F("AT+CIPRXGET=2,"));
  mySerial->println(len);
  readline();
  if (! parseReply(F("+CIPRXGET: 2,"), &avail, ',', 0)) return false;

  readRaw(avail);

#ifdef ADAFRUIT_FONA_DEBUG
  DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes read"));
  for (uint8_t i=0;i<avail;i++) {
  DEBUG_PRINT(F(" 0x")); DEBUG_PRINT(replybuffer[i], HEX);
  }
  DEBUG_PRINTLN();
#endif

  memcpy(buff, replybuffer, avail);

  return avail;
}



/********* HTTP LOW LEVEL FUNCTIONS  ************************************/

boolean Adafruit_FONA::HTTP_init() {
  return sendCheckReply(F("AT+HTTPINIT"), ok_reply);
}

boolean Adafruit_FONA::HTTP_term() {
  return sendCheckReply(F("AT+HTTPTERM"), ok_reply);
}

void Adafruit_FONA::HTTP_para_start(FONAFlashStringPtr parameter,
                                    boolean quoted) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPPARA=\""));
  DEBUG_PRINT(parameter);
  DEBUG_PRINTLN('"');


  mySerial->print(F("AT+HTTPPARA=\""));
  mySerial->print(parameter);
  if (quoted)
    mySerial->print(F("\",\""));
  else
    mySerial->print(F("\","));
}

boolean Adafruit_FONA::HTTP_para_end(boolean quoted) {
  if (quoted)
    mySerial->println('"');
  else
    mySerial->println();

  return expectReply(ok_reply);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 const char *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 FONAFlashStringPtr value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 int32_t value) {
  HTTP_para_start(parameter, false);
  mySerial->print(value);
  return HTTP_para_end(false);
}

boolean Adafruit_FONA::HTTP_data(uint32_t size, uint32_t maxTime) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPDATA="));
  DEBUG_PRINT(size);
  DEBUG_PRINT(',');
  DEBUG_PRINTLN(maxTime);


  mySerial->print(F("AT+HTTPDATA="));
  mySerial->print(size);
  mySerial->print(",");
  mySerial->println(maxTime);

  return expectReply(F("DOWNLOAD"));
}

boolean Adafruit_FONA::HTTP_action(uint8_t method, uint16_t *status,
                                   uint16_t *datalen, int32_t timeout) {
  // Send request.
  if (! sendCheckReply(F("AT+HTTPACTION="), method, ok_reply))
    return false;

  // Parse response status and size.
  readline(timeout);
  if (! parseReply(F("+HTTPACTION:"), status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), datalen, ',', 2))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_readall(uint16_t *datalen) {
  getReply(F("AT+HTTPREAD"));
  if (! parseReply(F("+HTTPREAD:"), datalen, ',', 0))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_ssl(boolean onoff) {
  return sendCheckReply(F("AT+HTTPSSL="), onoff ? 1 : 0, ok_reply);
}

/********* HTTP HIGH LEVEL FUNCTIONS ***************************/

boolean Adafruit_FONA::HTTP_GET_start(char *url,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen, 30000))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

/*
boolean Adafruit_FONA_3G::HTTP_GET_start(char *ipaddr, char *path, uint16_t port
				      uint16_t *status, uint16_t *datalen){
  char send[100] = "AT+CHTTPACT=\"";
  char *sendp = send + strlen(send);
  memset(sendp, 0, 100 - strlen(send));

  strcpy(sendp, ipaddr);
  sendp+=strlen(ipaddr);
  sendp[0] = '\"';
  sendp++;
  sendp[0] = ',';
  itoa(sendp, port);
  getReply(send, 500);

  return;

  if (! HTTP_setup(url))

    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen))
    return false;

  DEBUG_PRINT("Status: "); DEBUG_PRINTLN(*status);
  DEBUG_PRINT("Len: "); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}
*/

void Adafruit_FONA::HTTP_GET_end(void) {
  HTTP_term();
}

boolean Adafruit_FONA::HTTP_POST_start(char *url,
              FONAFlashStringPtr contenttype,
              const uint8_t *postdata, uint16_t postdatalen,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  if (! HTTP_para(F("CONTENT"), contenttype)) {
    return false;
  }

  // HTTP POST data
  if (! HTTP_data(postdatalen, 10000))
    return false;
  mySerial->write(postdata, postdatalen);
  if (! expectReply(ok_reply))
    return false;

  // HTTP POST
  if (! HTTP_action(FONA_HTTP_POST, status, datalen))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_POST_end(void) {
  HTTP_term();
}

void Adafruit_FONA::setUserAgent(FONAFlashStringPtr useragent) {
  this->useragent = useragent;
}

void Adafruit_FONA::setHTTPSRedirect(boolean onoff) {
  httpsredirect = onoff;
}

/********* HTTP HELPERS ****************************************/

boolean Adafruit_FONA::HTTP_setup(char *url) {
  // Handle any pending
  HTTP_term();

  // Initialize and set parameters
  if (! HTTP_init())
    return false;
  if (! HTTP_para(F("CID"), 1))
    return false;
  if (! HTTP_para(F("UA"), useragent))
    return false;
  if (! HTTP_para(F("URL"), url))
    return false;

  // HTTPS redirect
  if (httpsredirect) {
    if (! HTTP_para(F("REDIR"),1))
      return false;

    if (! HTTP_ssl(true))
      return false;
  }

  return true;
}

/********* HELPERS *********************************************/

boolean Adafruit_FONA::expectReply(FONAFlashStringPtr reply,
                                   uint16_t timeout) {
  readline(timeout);

  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

/********* LOW LEVEL *******************************************/

inline int Adafruit_FONA::available(void) {
  return mySerial->available();
}

inline size_t Adafruit_FONA::write(uint8_t x) {
  return mySerial->write(x);
}

inline int Adafruit_FONA::read(void) {
  return mySerial->read();
}

inline int Adafruit_FONA::peek(void) {
  return mySerial->peek();
}

inline void Adafruit_FONA::flush() {
  mySerial->flush();
}

void Adafruit_FONA::flushInput() {
    // Read all available serial input to flush pending data.
    uint16_t timeoutloop = 0;
    while (timeoutloop++ < 40) {
        while(available()) {
            read();
            timeoutloop = 0;  // If char was received reset the timer
        }
        delay(1);
    }
}

uint16_t Adafruit_FONA::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replybuffer)-1)) {
    if (mySerial->available()) {
      replybuffer[idx] = mySerial->read();
      idx++;
      b--;
    }
  }
  replybuffer[idx] = 0;

  return idx;
}

uint8_t Adafruit_FONA::readline(uint16_t timeout, boolean multiline) {
  uint16_t replyidx = 0;

  while (timeout--) {
    if (replyidx >= 254) {
      //DEBUG_PRINTLN(F("SPACE"));
      break;
    }

    while(mySerial->available()) {
      char c =  mySerial->read();
      if (c == '\r') continue;
      if (c == 0xA) {
        if (replyidx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiline) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
      }
      replybuffer[replyidx] = c;
      //DEBUG_PRINT(c, HEX); DEBUG_PRINT("#"); DEBUG_PRINTLN(c);
      replyidx++;
    }

    if (timeout == 0) {
      //DEBUG_PRINTLN(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  replybuffer[replyidx] = 0;  // null term
  return replyidx;
}

uint8_t Adafruit_FONA::getReply(char *send, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);


  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr send, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);


  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, char *suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix); DEBUG_PRINTLN(suffix);


  mySerial->print(prefix);
  mySerial->println(suffix);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix); DEBUG_PRINTLN(suffix, DEC);


  mySerial->print(prefix);
  mySerial->println(suffix, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, suffix2, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix);
  DEBUG_PRINT(suffix1, DEC); DEBUG_PRINT(','); DEBUG_PRINTLN(suffix2, DEC);


  mySerial->print(prefix);
  mySerial->print(suffix1);
  mySerial->print(',');
  mySerial->println(suffix2, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, ", suffix, ", and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix);
  DEBUG_PRINT('"'); DEBUG_PRINT(suffix); DEBUG_PRINTLN('"');


  mySerial->print(prefix);
  mySerial->print('"');
  mySerial->print(suffix);
  mySerial->println('"');

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

boolean Adafruit_FONA::sendCheckReply(char *send, char *reply, uint16_t timeout) {
  if (! getReply(send, timeout) )
	  return false;
/*
  for (uint8_t i=0; i<strlen(replybuffer); i++) {
  DEBUG_PRINT(replybuffer[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  for (uint8_t i=0; i<strlen(reply); i++) {
    DEBUG_PRINT(reply[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  */
  return (strcmp(replybuffer, reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr send, FONAFlashStringPtr reply, uint16_t timeout) {
	if (! getReply(send, timeout) )
		return false;

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(char* send, FONAFlashStringPtr reply, uint16_t timeout) {
  if (! getReply(send, timeout) )
	  return false;
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, char *suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, suffix2, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix1, suffix2, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, ", suffix, ", and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReplyQuoted(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          uint16_t *v, char divider, uint8_t index) {
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  *v = atoi(p);

  return true;
}

boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          char *v, char divider, uint8_t index) {
  uint8_t i=0;
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  for(i=0; i<strlen(p);i++) {
    if(p[i] == divider)
      break;
    v[i] = p[i];
  }

  v[i] = '\0';

  return true;
}

// Parse a quoted string in the response fields and copy its value (without quotes)
// to the specified character array (v).  Only up to maxlen characters are copied
// into the result buffer, so make sure to pass a large enough buffer to handle the
// response.
boolean Adafruit_FONA::parseReplyQuoted(FONAFlashStringPtr toreply,
          char *v, int maxlen, char divider, uint8_t index) {
  uint8_t i=0, j;
  // Verify response starts with toreply.
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  // Find location of desired response field.
  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  // Copy characters from response field into result string.
  for(i=0, j=0; j<maxlen && i<strlen(p); ++i) {
    // Stop if a divier is found.
    if(p[i] == divider)
      break;
    // Skip any quotation marks.
    else if(p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxlen)
    v[j] = '\0';

  return true;
}

boolean Adafruit_FONA::sendParseReply(FONAFlashStringPtr tosend,
				      FONAFlashStringPtr toreply,
				      uint16_t *v, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, v, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


// needed for CBC and others

boolean Adafruit_FONA_3G::sendParseReply(FONAFlashStringPtr tosend,
				      FONAFlashStringPtr toreply,
				      float *f, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, f, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


boolean Adafruit_FONA_3G::parseReply(FONAFlashStringPtr toreply,
          float *f, char divider, uint8_t index) {
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  *f = atof(p);

  return true;
}
