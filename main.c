#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
  printf("hello world\n");

  // http is built on top of tcp
  // http/1.0 creates a new tcp connection every request
  
  // for an http server implmentation, we listen for packets in a loop, and group request packets
  // then we respond with a response broken down into separate packets (headers, body, etc)
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  // ?? connect or bind the socket to the address?
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(8080),
    .sin_addr = INADDR_ANY,
  };

  int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  int r = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
  if (r == -1) {
    printf("ERROR: unable to bind to address\n");
    return r;
  }

  // go ahead and listen for incoming connections
  r = listen(fd, 3);
  if (r == -1) {
    printf("ERROR: unable to use socket to listen for incoming connections\n");
    return r;
  }

  printf("Listening on port 8080\n");
  fflush(stdout);

  // we loop and do blocking accept calls for incoming connections
  while (true) {
    // we pass in a sockaddr which is filled with the incoming client's information
    struct sockaddr client_addr;
    int addr_len = sizeof(client_addr);
    int client_fd = accept(fd, &client_addr, (socklen_t *) &addr_len);
    if (client_fd == -1) {
      printf("ERROR: unable to accept incoming client connection\n");
      return -1;
    }

    // new socket is created for the one client connection
    // we can recv from this socket
    size_t size = 1000;
    char *buf = malloc(size);
    // TCP is a byte stream so we need to keep calling recv until an end condition (\r\n). HTTP is the protocol which specifies this, not TCP streams

    ssize_t received_bytes = recv(client_fd, buf, size, 0);
    if (received_bytes > 0) {
      printf("received bytes: %d\n", (int) received_bytes);
      printf("%.*s\n", received_bytes, buf);
    }

    // parse the request
    char current_line[1000];
    int current_line_char = 0;
    for (int i = 0; i < received_bytes; i++) {
      printf("%c", buf[i]);
      current_line[current_line_char] = buf[i];
      current_line_char++;

      if (buf[i] == '\n') {
        printf("line complete: %.*s\n", current_line_char, current_line);
        current_line_char = 0;
      }

      // TODO: find the path from the first header `GET /bootstrap HTTP/1.1`
      // decode the request further
    }
    fflush(stdout);

    free(buf);

    const char *response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 5\r\n"
      "Content-Type: text/plain\r\n"
      "Connection: close\r\n"
      "\r\n"
      "hello";
    r = send(client_fd, response, strlen(response), 0);
    if (r == -1) {
      printf("ERROR: in sending response");
      fflush(stdout);
    }
    printf("sent %zd of %zu bytes\n", r, strlen(response));
    fflush(stdout);

    close(client_fd);
  }
  

  // TODO: scaffolding
  // keep track of multiple requests, examine the request data
  // decode the request data
  // then respond with particular html page

  // close socket
  close(fd);

  // for an http client library, we need to create a tcp connection, send headers, and then body and other parts of the request in a structure that follows the protocol RFC
}

