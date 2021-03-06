/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0

Esempio preso da https://os.mbed.com/docs/mbed-os/v6.15/apis/wi-fi.html

Inizialmente verifica se la scheda target è di tipo ODIN_W2 , che supporta 
direttamente lo stack Ethernet, alrimenti, come fallback, utilizza la ESP8266 che viene
gestita mediante comandi AT  . 
Le costanti MBED_CONF_ESP8266_TX(MBED_CONF_ESP8266_ vengono impostate automaticamente sulla base della scheda selezionata 
Connettere ai relativi piedini i segnali Tx e Rx della ESP8266 ( UART1) 
Per identificare i piedini della UART 1 fare riferimento 
alle piedinature fornite da mbed, vedi ad esempio https://os.mbed.com/platforms/ST-Nucleo-F401RE/ . 

 */
#include "mbed.h"
#include "TCPSocket.h"

#define TARGET_FF_ARDUINO 1


#if TARGET_UBLOX_EVK_ODIN_W2
#include "OdinWiFiInterface.h"
    OdinWiFiInterface wifi;
#else
#if !TARGET_FF_ARDUINO
   #error [NOT_SUPPORTED] Only Arduino form factor devices are supported at this time
#endif
#include "ESP8266Interface.h"

ESP8266Interface wifi(MBED_CONF_ESP8266_TX, MBED_CONF_ESP8266_RX);

#endif



const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

void scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("Scan:\r\n");

    int count = wifi->scan(NULL, 0); 

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;

    ap = new WiFiAccessPoint[count];
    //ap = new WiFiAccessPoint[count];
    
    count = wifi->scan(ap, count);
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\r\n", count);

    delete[] ap;
}

void http_demo(NetworkInterface *net)
{
    // Open a socket on the network interface, and create a TCP connection to mbed.org
    TCPSocket socket;
    socket.open(net);

    SocketAddress a;
    net->gethostbyname("ifconfig.io", &a);
    a.set_port(80);
    socket.connect(a);
    // Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: ifconfig.io\r\n\r\n";
    int scount = socket.send(sbuffer, sizeof sbuffer);
    printf("sent %d [%.*s]\n", scount, strstr(sbuffer, "\r\n") - sbuffer, sbuffer);

    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    int rcount = socket.recv(rbuffer, sizeof rbuffer);
    printf("recv %d [%.*s]\n", rcount, strstr(rbuffer, "\r\n") - rbuffer, rbuffer);

    // Close the socket to return its memory and bring down the network interface
    socket.close();
}

int main()
{
    SocketAddress a;

    printf("WiFi example\r\n\r\n");

    scan_demo(&wifi);

    printf("\r\nConnecting...\r\n");
    int ret = wifi.connect( MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2 );   
    /* npn funziona il passaggio parametri con MBED_CONF_NSAPI_DEFAULT_WIFI_SECURITY  da mbed_app.json, non aggiunge automaticamente NSAPI_SECURITY_ 
      e quinsi si deve fornire direttamente la costante NSAPI_SECURITY_WPA_WPA2 */

    if (ret != 0) {
        printf("\r\nConnection error\r\n");
        return -1;
    }

    printf("Success\r\n\r\n");
    printf("MAC: %s\r\n", wifi.get_mac_address());
    wifi.get_ip_address(&a);
    printf("IP: %s\r\n", a.get_ip_address());
    wifi.get_netmask(&a);
    printf("Netmask: %s\r\n", a.get_ip_address());
    wifi.get_gateway(&a);
    printf("Gateway: %s\r\n", a.get_ip_address());
    printf("RSSI: %d\r\n\r\n", wifi.get_rssi());

    http_demo(&wifi);

    wifi.disconnect();

    printf("\r\nDone\r\n");
}