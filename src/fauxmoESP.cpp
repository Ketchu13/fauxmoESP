/*

FAUXMO ESP

Copyright (C) 2016-2020 by Xose Pérez <xose dot perez at gmail dot com>

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "fauxmoESP.h"
#include <Arduino.h>
#include <vector>

// -----------------------------------------------------------------------------
// UDP
// -----------------------------------------------------------------------------
void fauxmoESP::printInColor(String text, String color) {
  // Set the appropriate ANSI escape code for the given color
  String escapeCode = "\033[30m";
  if (color == "black") {
    escapeCode = "\033[30m";
  } else if (color == "red") {
    escapeCode = "\033[31m";
  } else if (color == "green") {
    escapeCode = "\033[32m";
  } else if (color == "yellow") {
    escapeCode = "\033[33m";
  } else if (color == "blue") {
    escapeCode = "\033[34m";
  } else if (color == "magenta") {
    escapeCode = "\033[35m";
  } else if (color == "cyan") {
    escapeCode = "\033[36m";
  } else if (color == "lightgray") {
    escapeCode = "\033[37m";
  } else if (color == "darkgray") {
    escapeCode = "\033[90m";
  } else if (color == "lightred") {
    escapeCode = "\033[91m";
  } else if (color == "lightgreen") {
    escapeCode = "\033[92m";
  } else if (color == "lightyellow") {
    escapeCode = "\033[93m";
  } else if (color == "lightblue") {
    escapeCode = "\033[94m";
  } else if (color == "pink") {
    escapeCode = "\033[95m";
  } else if (color == "lightcyan") {
    escapeCode = "\033[96m";
  } else if (color == "white") {
    escapeCode = "\033[97m";
  }

  else {
    // If an invalid color is provided, just reset the color
    escapeCode = "\033[0m";
  }
  Serial.print("\033[32m[FAUXMO]\033[0m ");
  // Print the text in the given color and then reset the color
  Serial.print(escapeCode);
  Serial.print(text);
  Serial.print("\033[0m");
}
void fauxmoESP::printInColor(String text, String color, bool newline) {
  printInColor(text, color);
  if (newline) {
    Serial.println();
  }
}
void fauxmoESP::printInColorln(String text, String color) {
  printInColor(text, color, true);
}
// Function to extract the value from the URL
unsigned char extractValueFromText(char *url, char *searchString, char lastChar = '\0') { // NOTE: ok
  // Find the position of the search string in the URL
  char *searchIndex = strstr(url, searchString);
  // printInColor("url: ", "lightblue");
  // Serial.println(url);
  if (searchIndex != NULL) {
    // Get the index of the start of the substring we want to extract
    int startIndex = searchIndex - url + strlen(searchString) + 1;
    // findt the index position of lastChar character in url after the start index
    int endIndex = startIndex;
    while (url[endIndex] != lastChar && url[endIndex] != '\0') {
      endIndex++;
    }
    // Extract the substring from the URL
    char valueString[20];
    snprintf(valueString, sizeof(valueString), "%.*s", endIndex - startIndex, url + startIndex);
    // printInColor("valueString: ", "lightblue");
    // Serial.println(valueString);

    // Convert the value to an unsigned char and return it
    return (unsigned char)atoi(valueString);
    free (searchIndex);
    free (valueString);
  } else {
    // If the search string was not found in the URL, return 0
    return 0;
  }
}

void parseJsonString(
    String jsonString,
    bool &state,
    int &brightness,
    int &hue,
    int &saturation,
    int &kelvin) {
  int i = 0;
  int len = jsonString.length();
  Serial.print("jsonString: ");
  Serial.println(jsonString);  
  while (i < len) {
    // Find the key
    int keyStart = jsonString.indexOf('"', i);
    int keyEnd = jsonString.indexOf('"', keyStart + 1);
    String key = jsonString.substring(keyStart + 1, keyEnd);

    // Find the value
    int valueStart = jsonString.indexOf(':', keyEnd + 1);
    int valueEnd = jsonString.indexOf(',', valueStart + 1);
    if (valueEnd == -1) {
      valueEnd = jsonString.indexOf('}', valueStart + 1);
    }
    String value = jsonString.substring(valueStart + 1, valueEnd);
    Serial.print("key: ");
    Serial.println(key);
    Serial.print("value: ");
    Serial.println(value);
    
    if (key == "on") {
      state = value == "true";
    }
    if (key == "bri") {
      brightness = value.toInt();
    }
    if (key == "hue") {
      hue = value.toInt();
    }
    if (key == "sat") {
      saturation = value.toInt();
    }
    if (key == "ct") {
      kelvin = value.toInt();
    }
    // Move the index to the next key-value pair
    i = valueEnd + 1;
  }
}

int indexOfStringInCharArray(char *haystack, char *needle) {
  int pos = -1;
  int len = strlen(needle);
  // needle size can vary, so we need to check all the possible positions
  for (int i = 0; i < (int)strlen(haystack); i++) {
    if (haystack[i] == needle[0]) {
      // check if the rest of the needle matches
      bool match = true;
      for (int j = 1; j < len; j++) {
        if (haystack[i + j] != needle[j]) {
          match = false;
          break;
        }
      }
      if (match) {
        pos = i;
        break;
      }
    }
  }
  return pos;
}

void extract_json_content(char *body, char *content) {
  int start_index = -1;
  int end_index = -1;
  int depth = 0;
  int i = 0;

  // Find the starting and ending indexes of the JSON content
  while (body[i] != '\0') {
    if (body[i] == '{') {
      if (start_index == -1) {
        start_index = i;
      }
      depth++;
    } else if (body[i] == '}') {
      depth--;
      if (depth == 0) {
        end_index = i;
        break;
      }
    }
    i++;
  }

  // Copy the content to the output buffer
  if (start_index != -1 && end_index != -1) {
    int len = end_index - start_index + 1;
    strncpy(content, &body[start_index], len);
    content[len] = '\0';
  } else {
    // No JSON content found
    content[0] = '\0';
  }
}

void fauxmoESP::_sendUDPResponse() {

#ifdef DEBUG_FAUXMO
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m sendUDPResponse-Responding to M-SEARCH request\n");
#endif
  IPAddress ip = WiFi.localIP();
  char mac[18];
  snprintf_P(mac, 18, PSTR("%s"), WiFi.macAddress().c_str());
  // replace ':' with ''  and convert to lower case
  for (int i = 0; i < 17; i++) {
    if (mac[i] == ':') {
      for (int j = i; j < 17; j++) {
        mac[j] = mac[j + 1];
      }
    }
    mac[i] = tolower(mac[i]);
  }

  
  printInColor("mac: ", "lightblue");
  Serial.println(mac);
  Serial.print(".");
  printInColor("_tcp_port: ", "lightblue");
  Serial.println(_tcp_port);
  Serial.print(".");
  
  char response[strlen(FAUXMO_UDP_RESPONSE_TEMPLATE) + 128];
  snprintf_P(
      response, sizeof(response),
      FAUXMO_UDP_RESPONSE_TEMPLATE,
      ip[0], ip[1], ip[2], ip[3],
      _tcp_port,
      mac, mac);
  printInColor("response: ", "lightblue");
  Serial.println(response);
#if DEBUG_FAUXMO_VERBOSE_UDP
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m UDP response sent to %s:%d\n%s", _udp.remoteIP().toString().c_str(), _udp.remotePort(), response);
#endif

  _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
#if defined(ESP32)
  _udp.printf(response);
#else
  _udp.write(response);
#endif
  _udp.endPacket();
}

void fauxmoESP::_handleUDP() {

  int len = _udp.parsePacket();
  if (len > 0) {
    unsigned char data[len + 1];
    _udp.read(data, len);
    data[len] = 0;

#if DEBUG_FAUXMO_VERBOSE_UDP
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m UDP packet received\n%s", (const char *)data);
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m UDP request %s\n", data);
#endif

    // check if "M-SEARCH" is in the unsigned char array without converting it to a string
    if (strstr((const char *)data, "M-SEARCH") != NULL) {
      // check if "ssdp:discover" or "upnp:rootdevice" or "device:basic:1" is in the request
      if ((strstr((const char *)data, "ssdp:discover") != NULL) || (strstr((const char *)data, "upnp:rootdevice") != NULL) || (strstr((const char *)data, "device:basic:1") != NULL)) {
        _sendUDPResponse();
      }
    }
  }
}

// -----------------------------------------------------------------------------
// TCP
// -----------------------------------------------------------------------------

void fauxmoESP::_sendTCPResponse(AsyncClient *client, const char *code, char *body, const char *mime) {

  char headers[strlen_P(FAUXMO_TCP_HEADERS) + 32];
  snprintf_P(
      headers, sizeof(headers),
      FAUXMO_TCP_HEADERS,
      code, mime, strlen(body));

#if DEBUG_FAUXMO_VERBOSE_TCP
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m sendTCPResponse-Response:\n%s%s\n", headers, body);
#endif

  client->write(headers);
  client->write(body);
}

char *fauxmoESP::_deviceJson(unsigned char id, bool all = true) { // NOTE: seams ok
  // Serial.print("_devices.size() = ");
  // Serial.println(_devices.size());
  // Serial.print("id = ");
  // Serial.println(id);
  if (id >= _devices.size())
    // const char * to char * conversion
    return (char *)FAUXMO_DEVICE_JSON_EMPTY;

  fauxmoesp_device_t device = _devices[id];

#ifdef DEBUG_FAUXMO
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m deviceJson-Sending device info for \"%s\", uniqueID = \"%s\"\n", device.name, device.uniqueid);
#endif

  if (all) {
    // allocate memory for the buffer
    char buffer1[strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE) + 64];
    // Serial.println(strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE));
    // Serial.println(strlen_P(device.name));
    // Serial.println(strlen_P(device.uniqueid));
    // Serial.println(strlen_P(device.state ? "true" : "false"));
    // check if device parameters are set
    // check length of each parameter
    
    snprintf_P(
        buffer1, sizeof(buffer1),
        FAUXMO_DEVICE_JSON_TEMPLATE,
        device.name, device.uniqueid,
        device.state ? "true" : "false",
        device.brightness,
        device.hue,
        device.saturation,
        device.kelvin);
    // Serial.println("buffer1");
    // Serial.println(buffer1);
    return strdup(buffer1);
    
  } else {
    char buffer2[strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE_SHORT) + 64];
    // Serial.println(strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE_SHORT));
    // Serial.println(strlen_P(device.name));
    // Serial.println(strlen_P(device.uniqueid));
    snprintf_P(
        buffer2, sizeof(buffer2),
        FAUXMO_DEVICE_JSON_TEMPLATE_SHORT,
        device.name, device.uniqueid);
    // Serial.println("buffer2");
    // Serial.println(buffer2);
    return strdup(buffer2);
  }

} // _deviceJson

String fauxmoESP::_byte2hex(uint8_t zahl) {
  String hstring = String(zahl, HEX);
  if (zahl < 16) {
    hstring = "0" + hstring;
  }

  return hstring;
}

String fauxmoESP::_makeMD5(String text) {
  unsigned char bbuf[16];
  String hash = "";
  MD5Builder md5;
  md5.begin();
  md5.add(text);
  md5.calculate();

  md5.getBytes(bbuf);
  for (uint8_t i = 0; i < 16; i++) {
    hash += _byte2hex(bbuf[i]);
  }
  return hash;
}

bool fauxmoESP::_onTCPDescription(AsyncClient *client, char *url, char *body) { // NOTE: seams ok

  (void)url;
  (void)body;

#ifdef DEBUG_FAUXMO
  //DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPDescription-Handling /description.xml request\n");
#endif

  IPAddress ip = WiFi.localIP();
  // create the uniqueid
  char mac[18];
  // WiFi.macAddress() mac var
  // add to ns the <mac address>:<unique id suffix>-<device id> (device id must be a two digit number)
  snprintf_P(mac, 18, PSTR("%s"), WiFi.macAddress().c_str());
  // Serial.print("347-uniqueid: ");
  // Serial.println(mac);
  //  replace ':' with ''  and convert to lower case
  for (int i = 0; i < (int)strlen(mac); i++) {
    if (mac[i] == ':') {
      for (int j = i; j < (int)strlen(mac); j++) {
        mac[j] = mac[j + 1];
      }
    }
    mac[i] = tolower(mac[i]);
  }

#ifdef DEBUG_FAUXMO
  /*printInColor("ip:", "yellow");
  Serial.println(ip);
  printInColor("MAC:", "yellow");
  Serial.println(mac);*/
  /*printInColor("onTCPDescription-url:", "yellow");
  Serial.println(url);
  printInColor("onTCPDescription-body: ", "yellow");
  if (body != NULL && strlen(body) > 0) {
    Serial.println(body);
  } else {
    Serial.println("NULL");
  }*/

  /*//  get remote client ip address
  u_int32_t ipaddr = client->getRemoteAddress();
  //  convert to string
  String remoteAdrr = String(ipaddr & 0xFF) + "." + String((ipaddr >> 8) & 0xFF) + "." + String((ipaddr >> 16) & 0xFF) + "." + String((ipaddr >> 24) & 0xFF);
  Serial.println("remoteAdrr:");
  Serial.println(remoteAdrr);*/
#endif

  char response[strlen_P(FAUXMO_DESCRIPTION_TEMPLATE) + 256];
  snprintf_P(
      response, sizeof(response),
      FAUXMO_DESCRIPTION_TEMPLATE,
      ip[0], ip[1], ip[2], ip[3], _tcp_port,
      ip[0], ip[1], ip[2], ip[3], _tcp_port,
      mac, mac);
  //printInColor("onTCPDescription-384-response:", "red");
  //Serial.println(response);
  _sendTCPResponse(client, "200 OK", response, "text/xml");
  return true;
}
char *removeCrLf(const char *A_str) {
  // Find the length of the input string
  size_t inputLen = strlen(A_str);

  // Allocate memory for the output string
  char *outputStr = new char[inputLen + 1];

  // Initialize the output string index and the "found" flag
  size_t outputIndex = 0;
  bool foundCrLf = false;

  // Loop through the input string
  for (size_t i = 0; i < inputLen; i++) {
    // Check if the current character is a carriage return
    if (A_str[i] == '\r') {
      // Check if the next character is a newline
      if (i < inputLen - 1 && A_str[i + 1] == '\n') {
        // Set the "found" flag to true
        foundCrLf = true;
        // Skip the newline character
        i++;
        // Continue to the next iteration of the loop
        continue;
      }
    }

    // Copy the current character to the output string
    outputStr[outputIndex++] = A_str[i];
  }

  // Null-terminate the output string
  outputStr[outputIndex] = '\0';

  // Check if the "found" flag is set
  if (foundCrLf) {
    // Free the memory allocated for the original string
    delete[] A_str;

    // Return the new string
    return outputStr;
  } else {
    // The original string did not contain "\r\n", so just return it
    return const_cast<char *>(A_str);
  }
  delete[] outputStr;
}
bool fauxmoESP::_onTCPList(AsyncClient *client, char *url, char *body) { // NOTE: seams to be ok

#ifdef DEBUG_FAUXMO
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPList-Handling list request\n");
#endif
  // printInColor("_onTCPList url:", "yellow");
  // Serial.println(url);
  //  Serial.println(body);
  char wordsquery[] = "lights";
  // Get all char in the url after the "lights" and finishing at the end of string
  unsigned char id = extractValueFromText(url, wordsquery);
  // printInColor("onTCPList-id:", "yellow");
  // Serial.println(id);
  //  This will hold the response string
  char response[2048];
  printInColor("onTCPList-409-response empty:", "blue");
  Serial.println(strlen(response) > 0 ? "NO" : "YES");
  
  // Client is requesting all devices
  // one device string is a json string
  if (0 == id) {
    strcat(response, "{");
    for (unsigned char i = 0; i < _devices.size(); i++) {
      if (i > 0) {
        strcat(response, ",");
      }

      char buffer3[strlen_P(FAUXMO_DEVICE_JSON_I) + 256];
      char *jdevice = _deviceJson(i, false);
      printInColor("onTCPList-430-jdevice:", "magenta");
      Serial.println(jdevice);

      // print jdevice length
      printInColor("onTCPList-jdevice length: ", "magenta");
      Serial.println(strlen(jdevice));

      snprintf_P(
          buffer3, sizeof(buffer3),
          FAUXMO_DEVICE_JSON_I,
          i + 1, jdevice);
      strcat(response, buffer3);
    }
    strcat(response, "}");
  } else { //  Client is requesting a single device
    char buffer4[strlen_P(FAUXMO_DEVICE_JSON_J) + 512];
    char *jdevice = _deviceJson(id - 1);
    printInColor("onTCPList-jdevice:", "magenta");
    Serial.println(jdevice);
    snprintf_P(
        buffer4, sizeof(buffer4),
        FAUXMO_DEVICE_JSON_J,
        jdevice);
    // Add buffer to response
    strcat(response, buffer4);
  }
   // close the json string

  printInColor("OnTCPList-response: ", "blue");
  char *modifiedStr = removeCrLf(response);
  Serial.println(modifiedStr);
  _sendTCPResponse(client, "200 OK", (char *)modifiedStr, "application/json");
  // erase response
  

  return true;
}

bool fauxmoESP::_onTCPControl(AsyncClient *client, char *url, char *body) {
  printInColor("onTCPControl-url:", "red");
  Serial.println(url);
  printInColor("onTCPControl-body:", "red");
  Serial.println(body);
  printInColor("onTCPControl-username:", "red");
  Serial.println(_user_name);
  // "devicetype" request
  // check if the char * body contains the char * "devicetype"
  if (strstr(body, "devicetype") != NULL) {
#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPControl-Handling devicetype request\n");
#endif
    char response[strlen_P(FAUXMO_DEVICETYPE_RESPONSE) + 256];
    snprintf_P(
        response, sizeof(response),
        FAUXMO_DEVICETYPE_RESPONSE,
        _user_name);
    printInColor("onTCPControl-response:", "red");
    Serial.println(response);
    _sendTCPResponse(client, "200 OK", response, "application/json");
    return true;
  }

  // "state" request
  // "state" request
  // if the char * body contains the char * "devicetype" and the length of the char * body  is greater than 0
  if ((strstr(url, "state") != NULL) && (body > 0)) {
    char wordsquery[] = "lights";
    // Get all char in the url after the "lights" and finishing at the end of string
    unsigned char id = extractValueFromText(url, wordsquery, '/');
    printInColor("517-id:", "red");
    Serial.println(id);
#ifdef DEBUG_FAUXMO
    // DEBUG_MSG_FAUXMO("[FAUXMO] Handling state request\n");
#endif

    if (id > 0) {
      --id;

      int brightness = -1;
      int hue = -1;
      int saturation = -1;
      int kelvin = -1;
      bool state = false;
      // body is a JSON string
      // extract parameters from JSON string
      Serial.println("parsing json");
      parseJsonString(strdup(body), state, brightness, hue, saturation, kelvin);

      if (brightness > -1) {
        _devices[id].value = brightness;
        _devices[id].state = (brightness > 0);
      }

      if (hue > -1) {
        // int to unsigned char
        _devices[id].hue = hue;
      }

      if (saturation > -1) {
        _devices[id].saturation = saturation;
      }

      if (kelvin > -1) {
        _devices[id].kelvin = kelvin;
      }

      if (state == false) {
        _devices[id].state = false;
      } else {
        _devices[id].state = true;
        if (_devices[id].value <= 0) {
          _devices[id].value = 255;
          _devices[id].brightness =_devices[id].value;
        }
      }

      char response[strlen_P(FAUXMO_TCP_STATE_RESPONSE) + 24];
      snprintf_P(
          response, sizeof(response),
          FAUXMO_TCP_STATE_RESPONSE,
          id + 1,
          _devices[id].state ? "true" : "false",
          id + 1,
          _devices[id].brightness
#ifdef ALEXA_EXTENDED
          ,
          id + 1,
          _devices[id].hue,
          id + 1,
          _devices[id].saturation,
          id + 1,
          _devices[id].kelvin
#endif
      );
      printInColor("onTCPControl-sending response: ", "lightgreen");
      Serial.println(response);
      _sendTCPResponse(client, "200 OK", response, "text/xml");
  
      if (_setCallback) {
        //_setCallback(id, _devices[id].name, _devices[id].state, _devices[id].value);
        // update the state of the device
        _setCallback(
            id,                      // device id
            _devices[id].name,       // device name
            _devices[id].state,      // device state
            _devices[id].value,      // device brightness
            _devices[id].brightness, // device brightness
            _devices[id].hue,        // device hue
            _devices[id].saturation, // device saturation
            _devices[id].kelvin      // device kelvin
        );
      }
      return true;
    }
  }
  return false;

}

bool fauxmoESP::_onTCPRequest(AsyncClient *client, bool isGet, char *url, char *body) {
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPRequest enable: %s\n", _enabled ? "true" : "false");
  if (!_enabled) {

    return false;
  }

#if DEBUG_FAUXMO_VERBOSE_TCP
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPRequest isGet: %s\n", isGet ? "true" : "false");
  DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPRequest URL: %s\n", url);
  if (!isGet)
    DEBUG_MSG_FAUXMO("\033[FAUXMO]\033[0m onTCPRequest Body:\n%s\n", body);
#endif
  if (strlen(body) > 0) {
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMOlo]\033[0m onTCPRequest Body:\n%s\n", body);
  }
  // DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Body:\n%s\n", body);
  if (strcmp_P(url, PSTR("/description.xml")) == 0) {
    return _onTCPDescription(client, url, body);
  }
  // check if the url starts with /api
  if (strncmp_P(url, PSTR("/api"), 4) == 0) {
    if (isGet) {
      return _onTCPList(client, url, body);
    } else {
      return _onTCPControl(client, url, body);
    }
  }
  return false;
}

bool fauxmoESP::_onTCPData(AsyncClient *client, void *data, size_t len) {

  if (!_enabled)
    return false;
  printInColor("onTCPData-data: ", "red");
  Serial.print((char *)data);
  Serial.println();
  // get first line of the request
  char *line = strtok((char *)data, "\r\n");

  //printInColor("onTCPData-line: ", "red");
  //Serial.print(line);
  //Serial.println(".");
  /*char* method13 = strtok((char *)data, " ");
  char* url12 = strtok(NULL, " ");
  char* url13 = strtok(url12, " ");

  printInColor("onTCPData-Method: ", "red");
  Serial.println(method13);
  Serial.println(".");

  printInColor("onTCPData-URL: ", "red");
  Serial.println(url13);
  Serial.println(".");
  char* headersEnd = strstr((char *)data, "\r\n\r\n");
  char *body;
  char* headersEnd2 = strstr((char *)data, "\n\n");
  if (headersEnd != NULL) {
    char *data_ = headersEnd + 4; // On ajoute 4 pour sauter les "\r\n\r\n"
    Serial.print("onTCPData-696-Data: ");
    Serial.println(data_);

    // alloc var body to the real size of the body content and prevent overflow of the buffer and preserve the data
    body = (char *)malloc(strlen(data_) + 1);
    strcpy(body, data_);
    Serial.print("onTCPData-702-Body: ");
    Serial.println(body);

  } else if (headersEnd2 != NULL) {

    char *data_ = headersEnd + 4; // On ajoute 4 pour sauter les "\r\n\r\n"
    Serial.print("onTCPData-708-Data: ");
    Serial.println(data_);

    // alloc var body to the real size of the body content and prevent overflow of the buffer and preserve the data
    body = (char *)malloc(strlen(data_) + 1);
    strcpy(body, data_);
    Serial.print("onTCPData-714-Body: ");
    Serial.println(body);

  } else {

    Serial.println("onTCPData-719-No body");
    // set body to empty string
    body = (char *)malloc(1);
    strcpy(body, "");
  }

  Serial.println("onTCPData-725-tcpdata-Body:");
  Serial.println(body);

  bool isGet = (strncmp(method13, "GET", 3) == 0);

  if (strlen(body) > 0) {
    // cut body after the last "}"
    char new_body[strlen(body)];
    extract_json_content(body, new_body);
    Serial.print("\033[32m[FAUXMO]\033[0m onTCPData body: ");
    Serial.println(new_body);
    body = strdup(new_body);
  } else {
    Serial.println("\033[32m[FAUXMO]\033[0m onTCPData body is empty.");
  }
  Serial.print("\033[32m[FAUXMO]\033[0m onTCPData-742 url: ");
  Serial.println(url13);
  return _onTCPRequest(client, isGet, url13, body);
}*/

  char *method13 = strtok((char *)data, " ");
  char *url13 = strtok(NULL, " ");
  url13 = strtok(url13, " ");

  //Serial.print("tcpdata-Method: ");
  //Serial.println(method13);

  //Serial.print("tcpdata-URL: ");
  //Serial.println(url13);

  // set the char * p to the data
  char *p = (char *)data;

#if DEBUG_FAUXMO_VERBOSE_TCP
  ;
#endif
  /*Serial.print("\033[32m[FAUXMO]\033[0m TCP request data\n");
  Serial.println((char *)data);
  Serial.print("\033[32m[FAUXMO]\033[0m TCP request p\n");
  Serial.println(String(p));*/

  // Method is the first word of the request
  char *method = p;

  while (*p != ' ')
    p++;
  *p = 0;
  p++;

  // Split word and flag start of url
  char *url = p;

  // Find next space
  while (*p != ' ')
    p++;
  *p = 0;
  p++;

  // Find double line feed
  unsigned char c = 0;
  while ((*p != 0) && (c < 2)) {
    if (*p != '\r') {
      c = (*p == '\n') ? c + 1 : 0;
    }
    p++;
  }
  char *body = p;

  // Serial.print("\033[32m[FAUXMO]\033[0m method: ");
  // Serial.println(method);
  bool isGet = (strncmp(method13, "GET", 3) == 0);
  // if "\r\n"*2 or "\n"*2 is found in p, then the body starts after that and ends after the next "\r\n" or "\n"
  // if not found, then the body is empty
  if (isGet == true && (c == 2)) {
    body = strdup("");
  }
  if (strlen(body) > 0) {
    // cut body after the last "}"
    char new_body[strlen(body)];
    extract_json_content(body, new_body);
    Serial.print("\033[32m[FAUXMO]\033[0m onTCPData body: ");
    Serial.println(new_body);
    body = strdup(new_body);
  } else {
    Serial.println("\033[32m[FAUXMO]\033[0m onTCPData body is empty.");
  }
  Serial.print("\033[32m[FAUXMO]\033[0m onTCPData742 url: ");
  Serial.println(url13);
  return _onTCPRequest(client, isGet, url13, body);
}
void fauxmoESP::_onTCPClient(AsyncClient *client) {
  if (_enabled) {
    for (unsigned char i = 0; i < FAUXMO_TCP_MAX_CLIENTS; i++) {
      if (!_tcpClients[i] || !_tcpClients[i]->connected()) {

        _tcpClients[i] = client;

        client->onAck([i](void *s, AsyncClient *c, size_t len, uint32_t time) {}, 0);

        client->onData([this, i](void *s, AsyncClient *c, void *data, size_t len) { _onTCPData(c, data, len); }, 0);
        client->onDisconnect([this, i](void *s, AsyncClient *c) {
          if (_tcpClients[i] != NULL) {
            _tcpClients[i]->free();
            _tcpClients[i] = NULL;
          } else {
            DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Client %d already disconnected\n", i);
          }
          delete c;
          DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Client #%d disconnected\n", i);
        },
                             0);

        client->onError([i](void *s, AsyncClient *c, int8_t error) {
          DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Error %s (%d) on client #%d\n", c->errorToString(error), error, i);
        },
                        0);

        client->onTimeout([i](void *s, AsyncClient *c, uint32_t time) {
          DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Timeout on client #%d at %i\n", i, time);
          c->close();
        },
                          0);

        client->setRxTimeout(FAUXMO_RX_TIMEOUT);

        // DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Client #%d connected\n", i);
        return;
      }
    }
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Rejecting - Too many connections\n");
  } else {
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Rejecting - Disabled\n");
  }

  client->onDisconnect([](void *s, AsyncClient *c) {
    c->free();
    delete c;
  });
  client->close(true);
}

// -----------------------------------------------------------------------------
// Devices
// -----------------------------------------------------------------------------

fauxmoESP::~fauxmoESP() {

  // Free the name for each device
  for (auto &device : _devices) {
    free(device.name);
  }

  // Delete devices
  _devices.clear();
}

void fauxmoESP::setUniqueIdSuffix(const char *suffix) {
  _uniqueIdSuffix = suffix;
}

void fauxmoESP::getDeviceCount(int *count) { // return _devices.size() ?
  *count = 0;
  for (unsigned char i = 0; i < _devices.size(); i++) {
    (*count)++;
  }
}

unsigned char fauxmoESP::addDevice(const char *device_name) {

  fauxmoesp_device_t device;
  unsigned int device_id = _devices.size();

  // init properties
  device.name = strdup(device_name);
  device.state = false;
  device.value = 0;
  device.brightness= device.state? 255 : 0;
  device.hue=0;
  device.saturation= 0;
  device.saturation=0;
  device.kelvin= 0;

  // create the uniqueid
  char mac[18];

  // WiFi.macAddress() mac var
  // <mac address>:<unique id suffix>-<device id> (device id must be a two digit number)
  snprintf_P(mac, 18, PSTR("%s"), WiFi.macAddress().c_str());

  snprintf(device.uniqueid, 27, "%s:%s-%02X", mac, _uniqueIdSuffix, device_id);

  //  Attach
  _devices.push_back(device);
  // devicesNames.push_back(strdup(device_name));

  // DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m addDevice-Device '%s' added as #%d\n", device_name, device_id);

  return device_id;
}

// return the device by the given id
fauxmoesp_device_t *fauxmoESP::getDevice(unsigned char id) {
  if (id < _devices.size()) {
    return &_devices[id];
  }
  return NULL;
}

int fauxmoESP::getDeviceId(const char *device_name) {
  for (unsigned int id = 0; id < _devices.size(); id++) {
    if (strcmp(_devices[id].name, device_name) == 0) {
      return id;
    }
  }
  return -1;
}

bool fauxmoESP::renameDevice(unsigned char id, const char *device_name) {
  if (id < _devices.size()) {
    free(_devices[id].name);
    _devices[id].name = strdup(device_name);
#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Device #%d renamed to '%s'\n", id, device_name);
#endif
    return true;
  }
  return false;
}

bool fauxmoESP::renameDevice(const char *old_device_name, const char *new_device_name) {
  int id = getDeviceId(old_device_name);
  if (id < 0)
    return false;
  return renameDevice(id, new_device_name);
}

bool fauxmoESP::removeDevice(unsigned char id) {
  if (id < _devices.size()) {
    free(_devices[id].name);
    _devices.erase(_devices.begin() + id);
#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Device #%d removed\n", id);
#endif
    return true;
  }
  return false;
}

bool fauxmoESP::removeDevice(const char *device_name) {
  int id = getDeviceId(device_name);
  if (id < 0)
    return false;
  return removeDevice(id);
}

char *fauxmoESP::getDeviceName(unsigned char id, char *device_name, size_t len) {
  if ((id < _devices.size()) && (device_name != NULL)) {
    strncpy(device_name, _devices[id].name, len);
  }
  return device_name;
}

bool fauxmoESP::setState(unsigned char id, bool state, unsigned char value) {
  if (id < _devices.size()) {
    _devices[id].state = state;
    _devices[id].value = value;
    return true;
  }
  return false;
}

bool fauxmoESP::setState(const char *device_name, bool state, unsigned char value) {
  int id = getDeviceId(device_name);
  if (id < 0)
    return false;
  _devices[id].state = state;
  _devices[id].value = value;
  return true;
}

bool fauxmoESP::setState(unsigned char id, bool state, unsigned char value, unsigned char brightness, unsigned char hue, unsigned char saturation, unsigned char kelvin) {
  if (id < 0)
    return false;
  return setState(_devices[id].name, state, value, brightness, hue, saturation, kelvin);
}

bool fauxmoESP::setState(const char *device_name, bool state, unsigned char value, unsigned char brightness, unsigned char hue, unsigned char saturation, unsigned char kelvin) {
  int id = getDeviceId(device_name);
  if (id < 0)
    return false;

  if (brightness > -1)
    _devices[id].brightness = brightness;
  if (hue > -1)
    _devices[id].hue = hue;
  if (saturation > -1)
    _devices[id].saturation = saturation;
  if (kelvin > -1)
    _devices[id].kelvin = kelvin;

  _devices[id].state = state;
  _devices[id].value = value;

  return true;
}
// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
void fauxmoESP::setUserName(const char *user_name) {
  /*printInColor("setUserName: ", "green");
  Serial.println(user_name);
  printInColor("len user_name: ", "green");
  Serial.println(strlen(user_name));
  printInColor("sizeof _user_name: ", "green");
  Serial.println(sizeof(_user_name));*/

  if (strlen(user_name) > 0 && strlen(user_name) < sizeof(_user_name)) {
    snprintf(_user_name, strlen(user_name), "%s", user_name);
    printInColor("snprintf username rslt: ", "green");
    Serial.println(_user_name);
  }
  // snprintf(_user_name, strlen(user_name), "%s", user_name);
}
void fauxmoESP::getUserName(char *user_name) {
  /*printInColor("getUserName: ", "green");
  Serial.println(user_name);
  printInColor("len user_name: ", "green");
  Serial.println(strlen(user_name));
  printInColor("sizeof _user_name: ", "green");
  Serial.println(sizeof(_user_name));*/

  if (strlen(user_name) > 0 && strlen(user_name) < sizeof(_user_name)) {
    snprintf(user_name, strlen(_user_name), "%s", _user_name);
    printInColor("snprintf username rslt: ", "green");
    Serial.println(user_name);
  }
  // snprintf(_user_name, strlen(user_name), "%s", user_name);
}

bool fauxmoESP::process(AsyncClient *client, bool isGet, char *url, char *body) {
  Serial.println("process");
  return _onTCPRequest(client, isGet, url, body);
}

void fauxmoESP::handle() {
  if (_enabled)
    _handleUDP();
}

void fauxmoESP::enable(bool enable) {
  if (_user_name == NULL || strlen(_user_name) <= 5) {
    // set a default username
    setUserName("DFpBIHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr");
  }

  if (enable == _enabled)
    return;
  _enabled = enable;

#ifdef DEBUG_FAUXMO
  if (_enabled) {
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Enabled\n");
  } else {
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Disabled\n");
  }
#endif

  if (_enabled) {
    // Start TCP server if internal
    if (_internal) {
      if (NULL == _server) {
        _server = new AsyncServer(_tcp_port);
        _server->onClient(
            [this](void *s, AsyncClient *c) { _onTCPClient(c); },
            0);
      }
      _server->begin();
    }

// UDP setup
#ifdef ESP32
    _udp.beginMulticast(FAUXMO_UDP_MULTICAST_IP, FAUXMO_UDP_MULTICAST_PORT);
#else
    _udp.beginMulticast(WiFi.localIP(), FAUXMO_UDP_MULTICAST_IP, FAUXMO_UDP_MULTICAST_PORT);
#endif
#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m UDP server started\n");
#endif
  }
}
