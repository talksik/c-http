#include <stdlib.h>
#include <stdio.h>
#include "server.h"
#include <stdint.h>

void handle_bootstrap(request *req, response *resp) {
  printf("received a bootstrap request\n");
  resp->http_code = 200;
  resp->body = "bootstrap";
}

void handle_payment(request *req, response *resp) {
  printf("received a payment request\n");
  resp->http_code = 200;
  resp->body = "payment complete";
}
		      
int main(int argc, char **argv) {
  printf("hello world\n");

  uint16_t port = 8080;
  server* s = create_server("0.0.0.0", port);
  if (!s) {
    printf("ERROR: unable to create server\n");
    return -1;
  }

  register_route(s, "/bootstrap", handle_bootstrap);
  register_route(s, "/payment", handle_payment);

  int r = start_server(s);
  if (r != 0) {
    printf("ERROR: unable to start server\n");
    return -1;
  }
}

