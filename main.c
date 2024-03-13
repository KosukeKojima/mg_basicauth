// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, , see https://mongoose.ws/tutorials/tls/#how-to-build

#include "mongoose.h"

// The very first web page in history. You can replace it from command line
static const char *s_url = "http://ipv4.mydns.jp/login.html";
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds
static const char *id = "ID";
static const char *pwd = "PASSWORD";

bool done = false;              // Event handler flips it to true

// Print HTTP response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->data
    *(uint64_t *) c->data = mg_millis() + s_timeout_ms;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *) c->data &&
        (c->is_connecting || c->is_resolving)) {
      mg_error(c, "Connect timeout");
    }

  } else if (ev == MG_EV_CONNECT) {
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url);

    if (mg_url_is_ssl(s_url)) {
      struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
                                 .name = mg_url_host(s_url)};
      mg_tls_init(c, &opts);
    }

    // Send request
    int content_length = s_post_data ? strlen(s_post_data) : 0;
    mg_printf(c,
              "%s %s HTTP/1.1\r\n"
              "Host: %.*s\r\n",
              s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int) host.len,
              host.ptr, content_length);
    mg_http_bauth(c, id, pwd) ;
    mg_printf(c, "%s", "\r\n");
    mg_send(c, s_post_data, content_length);


    //bXlkbnM1NDg5NjE6a1M1NnBnZXpNSjI=

    }
    
    else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("%.*s", (int) hm->message.len, hm->message.ptr);
    c->is_draining = 1;        // Tell mongoose to close this connection
    *(bool *) c->fn_data = true;  // Tell event loop to stop

      if( mg_http_status(hm) == 200 ){

        printf("\nBasic Auth OK\n");
        
          done = true;
        
        }
      
        *(bool *) c->fn_data = true;  // Error, tell event loop to stop

    }else if (ev == MG_EV_ERROR) {
    
    }
}

int main(int argc, char *argv[]) {

    const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log level
    if (log_level == NULL) log_level = "4";       // Default is verbose
    struct mg_mgr mgr;              // Event manager
    
    if (argc > 1) s_url = argv[1];  // Use URL provided in the command line
    mg_log_set(atoi(log_level));    // Set to 0 to disable debug
    mg_mgr_init(&mgr);              // Initialise event manager
    mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
    while (!done) mg_mgr_poll(&mgr, 50);      // Event manager loops until 'done'
    mg_mgr_free(&mgr);                        // Free resources

  return 0;

}