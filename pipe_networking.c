#include "pipe_networking.h"

/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  int pd; // pipe descriptor
  
  if (mkfifo("WKP", 0644) == -1) { // Well Known Pipe
    printf("Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  printf("Waiting for client...\n");
  pd = open("WKP", O_RDONLY);
  if (pd == -1) {
    printf("Error: %s\n", strerror(errno));
    remove("WKP");
    exit(EXIT_FAILURE);
  }
  
  remove("WKP");
  return pd;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  char buf[HANDSHAKE_BUFFER_SIZE];

  if (read(from_client, buf, sizeof(buf)) == -1) {
    printf("Error: %s\n", strerror(errno));
    // remove("WKP");
    close(from_client);
    exit(EXIT_FAILURE);
  }
  printf("pipe name received: %s\n", buf);
  
  // Open the pipe whose name the server received from the client:
  *to_client = open(buf, O_WRONLY);
  if (*to_client == -1) {
    printf("Error: %s\n", strerror(errno));
    // remove("WKP");
    close(from_client);
    exit(EXIT_FAILURE);
  }
  
  if (write(*to_client, ACK, sizeof(ACK)) == -1) {
    printf("Error: %s\n", strerror(errno));
    close(pd);
    close(*to_client);
    exit(EXIT_FAILURE);
  }
  
  if (read(pd, buf, sizeof(buf)) == -1) {
    printf("Error: %s\n", strerror(errno));
    close(pd);
    close(*to_client);
    exit(EXIT_FAILURE);
  }
  
  if (strncmp(buf, ACK, sizeof(buf)) == 0) {
    printf("Confirmation message received: \"%s\"\n", buf);
  } else {
    printf("Error: received message \"%s\" instead of confirmation message \"%s\".\n", buf, ACK);
    close(pd);
    close(*to_client);
    exit(EXIT_FAILURE);
  }
  
  return pd;
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {

  int from_client;

  char buffer[HANDSHAKE_BUFFER_SIZE];

  mkfifo("luigi", 0600);

  //block on open, recieve mesage
  printf("[server] handshake: making wkp\n");
  from_client = open( "luigi", O_RDONLY, 0);
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake: received [%s]\n", buffer);

  remove("luigi");
  printf("[server] handshake: removed wkp\n");

  //connect to client, send message
  *to_client = open(buffer, O_WRONLY, 0);
  write(*to_client, buffer, sizeof(buffer));

  //read for client
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake received: %s\n", buffer);

  return from_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[HANDSHAKE_BUFFER_SIZE];

  //send pp name to server
  printf("[client] handshake: connecting to wkp\n");
  *to_server = open( "luigi", O_WRONLY, 0);
  if ( *to_server == -1 )
    exit(1);

  //make private pipe
  sprintf(buffer, "%d", getpid() );
  mkfifo(buffer, 0600);

  write(*to_server, buffer, sizeof(buffer));

  //open and wait for connection
  from_server = open(buffer, O_RDONLY, 0);
  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  printf("[client] handshake: received [%s]\n", buffer);

  //remove pp
  remove(buffer);
  printf("[client] handshake: removed pp\n");

  //send ACK to server
  write(*to_server, ACK, sizeof(buffer));

  return from_server;
}
