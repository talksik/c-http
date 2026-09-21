#include "server.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
  char *path;
  route_handler handler;
} handler_metadata;

struct server_s {
  uint16_t port;
  char *addr;
  int server_fd;

  handler_metadata **handlers;
  size_t handlers_len;
};

server *create_server(const char *addr, uint16_t port_number) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    printf("ERROR: unable to create socket\n");
    return NULL;
  }
  
  server *s = malloc(sizeof(server));
  s->server_fd = fd;
  s->port = port_number;
  s->addr = strdup(addr);
  s->handlers = NULL;
  s->handlers_len = 0;

  return s;
}

void destroy_server(server *s) {
  // TODO: go through all current and pending connections and close them
  if (s->server_fd != -1) {
    close(s->server_fd);
  }

  free(s->addr);

  for (int i = 0; i < s->handlers_len; i++) {
    free(s->handlers[i]);
  }
  free(s->handlers);
  
  free(s);
}

int start_server(server *s) {
  struct in_addr sin_addr;
  int r = inet_aton(s->addr, &sin_addr);
  if (r == 0) {
    printf("ERROR: unable to get address from provided input\n");
    return -1;
  }
  
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(s->port),
    .sin_addr = sin_addr,
  };

  int yes = 1;
  setsockopt(s->server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  r = bind(s->server_fd, (struct sockaddr *) &addr, sizeof(addr));
  if (r == -1) {
    printf("ERROR: unable to bind to address\n");
    return -1;
  }

  r = listen(s->server_fd, SOMAXCONN);
  if (r == -1) {
    printf("ERROR: unable to use socket to listen for incoming connections\n");
    return -1;
  }

  while (true) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_storage);
    int client_fd = accept(s->server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1) {
      printf("WARNING: unable to accept incoming client communication\n");
      continue;
    }
    
    if (client_addr.ss_family == AF_INET) {
      struct sockaddr_in *v4_addr = (struct sockaddr_in *)&client_addr;
      // use v4_addr->sin_port, etc.
    } 
    else if (client_addr.ss_family == AF_INET6) {
      struct sockaddr_in6 *v6_addr = (struct sockaddr_in6 *)&client_addr;
      // use v6_addr->sin6_port, etc.
    }

    // TODO: call recv until we have the end of the headers and request
    // append to a buffer, resizing along the way?

    // TODO: helper method which takes the client_fd and creates a request
    // we can call recv until we receive \r\n\r\n
    bool processing = true;
    char data[4096] = {0};
    size_t data_size = sizeof(data);
    size_t total_bytes = 0;
    
    while (processing) {
      // TODO: if data buffer full, realloc
      
      ssize_t received_bytes = recv(client_fd, data + total_bytes, data_size - total_bytes - 1, 0);

      if (received_bytes > 0) {
	total_bytes += received_bytes;
      }

      // check if the last of the data is \r\n\r\n
      if (total_bytes >= 4 && data[total_bytes - 1] == '\n' && data[total_bytes - 2] == '\r'
	  && data[total_bytes - 3] == '\n' && data[total_bytes - 4] == '\r') {
	printf("Data: %s\n", data);
	printf("Found end of the request\n");
	processing = false;
      }
    }

    request *req = malloc(sizeof(request));
    req->contents = data;
    response *resp = malloc(sizeof(response));
    resp->http_code = 404; // default
    resp->body = "default";

    // find the relevant route handler, call it
    for (int i = 0; i < s->handlers_len; i++) {
      printf("checking if route handler matches. path: %s\n", s->handlers[i]->path);
      if (strstr(data, s->handlers[i]->path) != NULL) {
	s->handlers[i]->handler(req, resp);

	printf("response: %d\n", resp->http_code);
	char response_content[1000];
	snprintf(response_content, sizeof(response_content),
	  "HTTP/1.1 %d OK\r\n"
	  "Content-Length: %d\r\n"
	  "Content-Type: text/plain\r\n"
	  "Connection: close\r\n"
	  "\r\n"
          "%s",
	  resp->http_code, strlen(resp->body), resp->body);
        int r = send(client_fd, response_content, strlen(response_content), 0);
	if (r == -1) {
	  printf("ERROR: in sending response\n");
	}

        break;
      }
    }
    free(req);
    free(resp);
	  
    close(client_fd);
  }

  return 0;
}

int register_route(server *s, const char *route, route_handler handler) {
  handler_metadata *new_handler = malloc(sizeof(handler_metadata));
  // TODO: handle error from malloc
  
  new_handler->path = strdup(route);
  new_handler->handler = handler;

  // relloc to make room for one more pointer
  size_t new_len = s->handlers_len + 1;
  handler_metadata **new_array = realloc(s->handlers, new_len * sizeof(handler_metadata *));
  // TODO: handle error of realloc
  
  s->handlers = new_array;
  s->handlers[s->handlers_len] = new_handler;
  s->handlers_len = new_len;

  printf("registered route: %s\n", route);
  printf("total handlers registered: %d\n", s->handlers_len);
}
