#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <stddef.h>

typedef struct server_s server;

typedef struct {
  char *key;
  char *value;
} header;

typedef struct {
  /* header *headers; */
  /* size_t headers_len; */
  /* char *path; */
  /* char *method; */
  /* char *body; */
  char *contents;
} request;

typedef struct {
  uint16_t http_code;
  char *body;
} response;

typedef void (*route_handler)(request *req, response *resp);

// addr should be in dot-separated notation (e.g. "0.0.0.0")
// returns NULL if error during creation
server *create_server(const char *addr, uint16_t port_number);

void destroy_server(server *s);

// blocking call which handles inbound connections and requests
// returns -1 on internal failure 
int start_server(server *s);
int register_route(server *s, const char *route, route_handler handler);

#endif // SERVER_H
