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

// TODO: handlers need a way to respond to requests, maybe return a response with http code and body response
// we will provide them an already-allocated resp struct which they may fill
typedef void (*route_handler)(request *req, response *resp);

// addr should be in dot-separated notation
// returns NULL if error during creation
server *create_server(const char *addr, uint16_t port_number);

void destroy_server(server *s);

// blocking call which handles inbound connections and requests
// returns -1 on internal failure 
int start_server(server *s);
int register_route(server *s, const char *route, route_handler handler);

// TODO: implementation wise, we need the server to handle incoming connections, read the incoming bytes of the connection in another thread, and then route handlers from different threads...will this lead to various problems?
  /* - what if a route handler uses some shared resource (e.g. service handler) to fulfill the request? and different calls of the handler from different threads calls that service handler? */
  /* - keep it single-threaded for now. process one request at a time */

#endif // SERVER_H
