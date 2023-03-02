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
// Utils
// -----------------------------------------------------------------------------

// Extract the value from the text
void fauxmoESP::_extractPathFromUrl(const char *url, char *path, size_t path_size) {
    const char *start = strchr(url, '/');
    if (start == nullptr) {
        path[0] = '\0';
        return;
    }
    start = strchr(start + 1, '/');
    if (start == nullptr) {
        path[0] = '\0';
        return;
    }
    const char *end = strchr(start + 1, '/');
    if (end == nullptr) {
        end = url + strlen(url);
    }

    size_t length = min(static_cast<size_t>(end - start - 1), path_size - 1);
    strncpy(path, start + 1, length);
    path[length] = '\0';
}

// Check if text is empty or NULL or too short // TODO Remove too short check
bool fauxmoESP::_isEmpty(char *text) {
    // Check if text is empty
    if (strcmp(text, "") == 0 || text == NULL || strlen(text) <= 5 || text[0] == '\0') {
        return true;
    } else {
        return false;
    }
}

// Check if username is set, if not set a default
bool fauxmoESP::checkUserNameFormat(char *user_name) {
    if ((_isEmpty(user_name) == true) || strlen(user_name) < FAUXMO_USERNAME_MIN_SIZE || strlen(user_name) > FAUXMO_USERNAME_MAX_SIZE) {
        return false;
    } else {
        return true;
    }
} //

// Check if username is set, if not set a default
void fauxmoESP::check_UserName() {
    if (checkUserNameFormat(_user_name) == false) {
        setUserName(FAUXMO_DEFAULT_USERNAME);
    }
}

// Supprime les caractères "\r\n" de la chaîne de caractères donnée
void fauxmoESP::_removeNewlines(char *str) {
    int len = strlen(str); // Longueur de la chaîne de caractères donnée
    int i = 0, j = 0;      // Indices pour parcourir la chaîne de caractères donnée et la chaîne résultante
    while (i < len) {
        // Vérifier si le caractère actuel est "\r" ou "\n"
        if (str[i] != '\r' && str[i] != '\n') {
            // Si le caractère actuel n'est pas "\r" ou "\n", copier dans la chaîne résultante
            str[j++] = str[i];
        }
        i++; // Avancer dans la chaîne de caractères donnée
    }
    // str[j] = '\0'; // Terminer la chaîne résultante ?
}

// Format color Serial output
void fauxmoESP::printInColor(String text, String color) { // TODO: Rewrite for use given Output device

    // Set the appropriate ANSI escape code for the given color
    String escapeCode = "\033[30m"; // TODO remove string
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
    } else {
        // If an invalid color is provided, just reset the color
        escapeCode = "\033[0m";
    }
    Serial.print("\033[32m[FAUXMO]\033[0m ");
    // Print the text in the given color and then reset the color
    Serial.print(escapeCode);
    Serial.print(text);
    Serial.print("\033[0m");
}

// Format color Serial output with newline
void fauxmoESP::printInColor(String text, String color, bool newline) { // TODO use DEBUG FAUXMO

    printInColor(text, color);
    if (newline) {
        Serial.println();
    }
}

// Format color Serial output with newline
void fauxmoESP::printInColorln(String text, String color) {
    printInColor(text, color, true);
}

// Function to extract the value from the URL
unsigned char fauxmoESP::_extractValueFromText(char *url, char *searchString, char lastChar = '\0') { // NOTE: ok
    
    // Find the position of the search string in the URL
    char *searchIndex = strstr(url, searchString);
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

        // Convert the value to an unsigned char and return it
        return (unsigned char)atoi(valueString);
        free(searchIndex);
        free(valueString);
    } else {
        // If the search string was not found in the URL, return 0
        return 0;
    }
}

// Extract the given key from the given JSON string and return the value as an unsigned char
void fauxmoESP::_parseJsonString(String jsonString, bool &state, int &brightness, int &hue, int &saturation, int &kelvin) {

    int i = 0;
    int len = jsonString.length();

#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m _parseJsonString :");
#endif
    unsigned char keysCount = 0;
    while (i < len) {
        // Find the key
        int keyStart = jsonString.indexOf('"', i);
        int keyEnd = jsonString.indexOf('"', keyStart + 1);
        String key = jsonString.substring(keyStart + 1, keyEnd);

        Serial.print("_parseJsonString key: ");

        Serial.println(key);
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

        if (key == "bri") {
            keysCount++;
            brightness = value.toInt();
        }
        if (key == "hue") {
            keysCount++;
            hue = value.toInt();
        }
        if (key == "sat") {
            keysCount++;
            saturation = value.toInt();
        }
        if (key == "ct") {
            keysCount++;
            kelvin = value.toInt();
        }
        if (key == "on") {
            keysCount++;
            state = value == "true" ? true : false;
        }
        // Move the index to the next key-value pair
        i = valueEnd + 1;
    }
    if (keysCount == 1 && state == true) {
        brightness = 254;
    }
}

// Extract the JSON content from the given body
void fauxmoESP::_extract_json_content(char *body, char *content) {
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

// Format the device JSON string for the given device ID and return it in the given buffer
void fauxmoESP::_deviceJson(unsigned char id, char *buffer1, bool all = true) { // NOTE: seams ok

    if (id >= _devices.size())
        snprintf(
            buffer1, strlen(buffer1),
            FAUXMO_DEVICE_JSON_EMPTY);

    fauxmoesp_device_t device = _devices[id];

#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m deviceJson-Sending device info for \"%s\", uniqueID = \"%s\"\n", device.name, device.uniqueid);
#endif

    if (all) {
        snprintf_P(
            buffer1, 512,
            FAUXMO_DEVICE_JSON_TEMPLATE,
            device.name, device.uniqueid,
            device.state ? "true" : "false",
            device.brightness,
            device.hue,
            device.saturation,
            device.kelvin);

    } else {
        snprintf_P(
            buffer1, 512,
            FAUXMO_DEVICE_JSON_TEMPLATE_SHORT,
            device.name, device.uniqueid);
    }

} // _deviceJson

#ifdef USERNAME_PLUS_MODE

// byte to hex string
String fauxmoESP::_byte2hex(uint8_t zahl) {
    String hstring = String(zahl, HEX);
    if (zahl < 16) {
        hstring = "0" + hstring;
    }

    return hstring;
} // _byte2hex

// MD5 hash function for String
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
} // _makeMD5

#endif

// -----------------------------------------------------------------------------
// UDP
// -----------------------------------------------------------------------------

// Send a UDP response to the extracted IP address and port
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

    char response[strlen(FAUXMO_UDP_RESPONSE_TEMPLATE) + 128];
    snprintf_P(
        response, sizeof(response),
        FAUXMO_UDP_RESPONSE_TEMPLATE,
        ip[0], ip[1], ip[2], ip[3],
        _tcp_port,
        mac, mac);

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

// Handle UDP requests
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

// Send a TCP response to the given client
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

// Handle TCP requests for devices descriptions
bool fauxmoESP::_onTCPDescription(AsyncClient *client, char *url, char *body) { // NOTE: seams ok

    (void)url;
    (void)body;

#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPDescription-Handling /description.xml request\n");
#endif

    IPAddress ip = WiFi.localIP();
    // create the uniqueid
    char mac[18];
    snprintf_P(mac, 18, PSTR("%s"), WiFi.macAddress().c_str());

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

    _sendTCPResponse(client, "200 OK", response, "text/xml");
    return true;
}

// Handle TCP requests for Listing all devices
bool fauxmoESP::_onTCPList(AsyncClient *client, char *url, char *body) { // NOTE: seams to be ok

#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m onTCPList-Handling list request\n");
#endif

    char wordsquery[] = "lights";
    // Get all char in the url after the "lights" and finishing at the end of string
    unsigned char id = _extractValueFromText(url, wordsquery);

    //  This will hold the response string
    char response[2048] = {0};

    // Client is requesting all devices
    // one device string is a json string
    if (0 == id) {
        strcat(response, "{");
        for (unsigned char i = 0; i < _devices.size(); i++) {
            if (i > 0) {
                strcat(response, ",");
            }

            char buffer3[strlen_P(FAUXMO_DEVICE_JSON_I) + 512];
            char buffer1[strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE) + 128];
            _deviceJson(i, buffer1, false);
            snprintf_P(
                buffer3, sizeof(buffer3),
                FAUXMO_DEVICE_JSON_I,
                i + 1, buffer1);
            strcat(response, buffer3);
        }
        strcat(response, "}");

    } else { //  Client is requesting a single device

        //  Create a buffer to hold the device json string
        char buffer1[strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE) + 64];
        _deviceJson(id - 1, buffer1);

        // Add buffer to response
        strcat(response, buffer1);
    }
    // Utilisation: appel de la fonction pour la chaîne de caractères "A_str"
    char result[strlen(response)]; // Chaîne résultante avec les caractères "\r\n" retirés
    strcpy(result, response);      // Copier la chaîne de caractères donnée dans la chaîne résultante
    response[0] = 0;
    _removeNewlines(result); // Appel de la fonction pour retirer les caractères "\r\n"
    _sendTCPResponse(client, "200 OK", result, "application/json");
    return true;
}

// Handle TCP requests for device control
bool fauxmoESP::_onTCPControl(AsyncClient *client, char *url, char *body) {
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
        _sendTCPResponse(client, "200 OK", response, "application/json");
        return true;
    }
    // "state" request
    // if the char * body contains the char * "devicetype" and the length of the char * body  is greater than 0
    if ((strstr(url, "state") != NULL) && (body > 0)) {
        char wordsquery[] = "lights";
        // Get all char in the url after the "lights" and finishing at the end of string
        unsigned char id = _extractValueFromText(url, wordsquery, '/');

#ifdef DEBUG_FAUXMO
        DEBUG_MSG_FAUXMO("[FAUXMO] Handling state request\n");
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
            _parseJsonString(strdup(body), state, brightness, hue, saturation, kelvin);

            if (brightness > -1) {
                _devices[id].brightness = brightness;
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

            _devices[id].state = state;
            if ((state == true) && (0 == _devices[id].value))
                _devices[id].value = 254;

            char response[strlen_P(FAUXMO_TCP_STATE_RESPONSE) + 24];
            snprintf_P(
                response, sizeof(response),
                FAUXMO_TCP_STATE_RESPONSE,
                id + 1, _devices[id].state ? "true" : "false", id + 1, _devices[id].brightness
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

// Handle TCP requests
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

    if (strcmp(url, "/description.xml") == 0) {
        return _onTCPDescription(client, url, body);
    }

    // check if the url starts with /api
    if (memcmp(url, "/api", 4) == 0) {
        // Extraire les caractères après le deuxième /
        char path[128];
        _extractPathFromUrl(url, path, sizeof(path));
        Serial.printf("path: %s\n", path);
        Serial.printf("_user_name: %s\n", _user_name);
        if (strcmp(path, _user_name) == 0) {
            if (isGet) {
                return _onTCPList(client, url, body);
            } else {
                return _onTCPControl(client, url, body);
            }
        }
        return false;
    }
    return false;
}

// Handle TCP data
bool fauxmoESP::_onTCPData(AsyncClient *client, void *data, size_t len) {

    if (!_enabled)
        return false;
    // get first line of the request
    char *method13 = strtok((char *)data, " ");
    char *url13 = strtok(NULL, " ");
    url13 = strtok(url13, " ");

    // set the char * p to the data
    char *p = (char *)data;

    while (*p != ' ')
        p++;
    *p = 0;
    p++;

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

    bool isGet = (strncmp(method13, "GET", 3) == 0);

    if (isGet == true && (c == 2)) {
        body = strdup("");
    }

    if (strlen(body) > 0) {
        char new_body[strlen(body)];
        _extract_json_content(body, new_body);
        body = strdup(new_body);
    }
    return _onTCPRequest(client, isGet, url13, body);
}

// Handle TCP clients (new connections)
void fauxmoESP::_onTCPClient(AsyncClient *client) {
    if (_enabled) {
        for (unsigned char i = 0; i < FAUXMO_TCP_MAX_CLIENTS; i++) {
            if (!_tcpClients[i] || !_tcpClients[i]->connected()) {

                _tcpClients[i] = client;

                client->onAck([i](void *s, AsyncClient *c, size_t len, uint32_t time) {}, 0);

                client->onData([this, i](void *s, AsyncClient *c, void *data, size_t len) {
                    _onTCPData(c, data, len);
                },
                               0);
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

                DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Client #%d connected\n", i);
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

// Set the unique id suffix
void fauxmoESP::setUniqueIdSuffix(const char *suffix) {
    _uniqueIdSuffix = suffix;
}

// Get the unique id suffix
void fauxmoESP::getDeviceCount(int *count) { // return _devices.size() ?
    *count = 0;
    for (unsigned char i = 0; i < _devices.size(); i++) {
        (*count)++;
    }
}

// Add a device to the fauxmoESP instance
unsigned char fauxmoESP::addDevice(const char *device_name) {

    fauxmoesp_device_t device;
    // get the device id (index)
    unsigned int device_id = _devices.size();

    // init properties
    device.name = strdup(device_name);
    device.state = false;
    device.value = 0;
    device.brightness = 0;
    device.hue = 0;
    device.saturation = 0;
    device.saturation = 0;
    device.kelvin = 500;

    // create the uniqueid
    char mac[18]; // WiFi.macAddress() mac var
    // <mac address>:<unique id suffix>-<device id> (device id must be a two digit number)
    snprintf_P(mac, 18, PSTR("%s"), WiFi.macAddress().c_str());
    snprintf(device.uniqueid, 27, "%s:%s-%02X", mac, _uniqueIdSuffix, device_id);

    //  Attach
    _devices.push_back(device);
    // devicesNames.push_back(strdup(device_name));

#ifdef DEBUG_FAUXMO
    DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m addDevice-Device '%s' added as #%d\n", device_name, device_id);
#endif

    return device_id;
}

// Return the device by the given id
fauxmoesp_device_t *fauxmoESP::getDevice(unsigned char id) {
    if (id < _devices.size()) {
        return &_devices[id];
    }
    return NULL;
}

// Return the device id by the given name
int fauxmoESP::getDeviceId(const char *device_name) {
    for (unsigned int id = 0; id < _devices.size(); id++) {
        if (strcmp(_devices[id].name, device_name) == 0) {
            return id;
        }
    }
    return -1;
}

// Return true if the device is correctly renamed
bool fauxmoESP::renameDevice(unsigned char id, const char *device_name) {
    if (id < _devices.size()) {
        // Free the old device name
        free(_devices[id].name);
        _devices[id].name = strdup(device_name);

#ifdef DEBUG_FAUXMO
        DEBUG_MSG_FAUXMO("\033[32m[FAUXMO]\033[0m Device #%d renamed to '%s'\n", id, device_name);
#endif

        return true;
    }
    return false;
}

// Return true if the device is correctly renamed
bool fauxmoESP::renameDevice(const char *old_device_name, const char *new_device_name) {
    int id = getDeviceId(old_device_name);
    if (id < 0)
        return false;
    return renameDevice(id, new_device_name);
}

// Return true if the device is correctly removed
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

// Return true if the device is correctly removed
bool fauxmoESP::removeDevice(const char *device_name) {
    int id = getDeviceId(device_name);
    if (id < 0)
        return false;
    return removeDevice(id);
}

// Return the device name by the given id
char *fauxmoESP::getDeviceName(unsigned char id, char *device_name, size_t len) {
    if ((id < _devices.size()) && (device_name != NULL)) {
        strncpy(device_name, _devices[id].name, len);
    }
    return device_name;
}

// Return true if the device id is right and set device state
bool fauxmoESP::setState(unsigned char id, bool state, unsigned char value) {
    if (id < _devices.size()) {
        _devices[id].state = state;
        _devices[id].value = value;
        _devices[id].brightness = value;
        return true;
    }
    return false;
}

// Return true if the device name is right and set device state
bool fauxmoESP::setState(const char *device_name, bool state, unsigned char value) {
    int id = getDeviceId(device_name);
    if (id < 0)
        return false;
    _devices[id].state = state;
    _devices[id].value = value;
    _devices[id].brightness = value;
    return true;
}

// Return true if the device id is right and set device state
bool fauxmoESP::setState(unsigned char id, bool state, unsigned char value, unsigned char brightness, unsigned char hue, unsigned char saturation, short kelvin) {
    if (id < 0)
        return false;
    return setState(_devices[id].name, state, value, brightness, hue, saturation, kelvin);
}

// Return true if the device name is right and set device state
bool fauxmoESP::setState(const char *device_name, bool state, unsigned char value, unsigned char brightness, unsigned char hue, unsigned char saturation, short kelvin) {
    int id = getDeviceId(device_name);
    
    if (id < 0) return false;

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
    _devices[id].brightness = value;

    return true;
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

// Set the user name
void fauxmoESP::setUserName(const char *user_name) {
    if (strlen(user_name) > 0 && strlen(user_name) < sizeof(_user_name)) {
        snprintf(_user_name, strlen(user_name) + 1, "%s", user_name);
    }
}

// Get the user name
void fauxmoESP::getUserName(char *user_name) {
    if (strlen(user_name) > 0 && strlen(user_name) < sizeof(_user_name)) {
        snprintf(user_name, strlen(_user_name), "%s", _user_name);
    }
}

// 
bool fauxmoESP::process(AsyncClient *client, bool isGet, char *url, char *body) {
    // Serial.println("process");
    return _onTCPRequest(client, isGet, url, body);
}

// Handle UDP requests
void fauxmoESP::handle() {
    if (_enabled) _handleUDP();
}

// Enable fauxmoESP. Launch the UDP server and TCP server if internal
void fauxmoESP::enable(bool enable) {
    // Check if user name is valid/empty // TODO check to move after "return"
    check_UserName();

    // Check if we are already in the desired state
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
