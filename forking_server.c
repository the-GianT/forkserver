#include "pipe_networking.h"
#include <signal.h>

void process(char *s);
void subserver(int from_client);

static void sighandler(int signo) {
  if (signo == SIGINT) {
    remove("luigi");
    exit(0);
  }
}

int main() {
  signal(SIGINT, sighandler);
  int from_client;
  int to_client;
  while (1) {
    from_client = server_setup();
    int f = fork();
    if (f) {
      remove("luigi");
      printf("[server] handshake: removed wkp\n");
      close(from_client);
    }
    else {
      subserver(from_client);
    }
  }
}

void subserver(int from_client) {
  int to_client = server_connect(from_client);
  char buf[BUFFER_SIZE];
  while (read(from_client, buf, sizeof(buf))) {
    printf("[subserver] received from client: %s\n", buf);
    process(buf);
    write(to_client, buf, sizeof(buf));
  }
}

void process(char * s) {
  int i = 0; 
  while (s[i]) {
    s[i] = toupper(s[i]);
    i++;
  }
}
