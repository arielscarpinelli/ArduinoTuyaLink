#include <WiFiClientSecure.h>

#ifdef ESP8266
    #include <ESP8266HTTPClient.h>
#else
    #include <HTTPClient.h>
#endif

#include "TuyaDebug.h"

extern "C" {

#include "http_client_interface.h"

}


http_client_status_t http_client_request(const http_client_request_t* request, http_client_response_t* response) {
    HTTPClient http;
    WiFiClientSecure client;
    X509List cert(request->cacert, request->cacert_len);

    client.setTrustAnchors(&cert);

    http.begin(client, request->host, request->port, request->path, true);

    for (int h = 0; h < request->headers_count; h++) {
        http.addHeader(request->headers[h].key, request->headers[h].value);
    }
    http.setTimeout(request->timeout_ms);
    int status = http.sendRequest(request->method, request->body, request->body_length);

    if (status < 0) {
        return HTTP_CLIENT_SEND_FAULT;
    }

    response->status_code = status;

    int read = client.read(response->buffer, response->buffer_length);
    response->body = response->buffer;
    response->body_length = read;

    return HTTP_CLIENT_SUCCESS;
  
}

