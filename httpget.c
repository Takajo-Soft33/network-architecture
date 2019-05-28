#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#define ZEROFILL(x) memset(&(x), 0, sizeof(x))

typedef struct {
  char *scheme;
  char *hostname;
  int port;
  char *path;
} URL;

char *url_parse(URL *, char *);
void url_free(URL *);

void strq_push(char *, int, char);

int main(int argc, char **argv) {
  if(argc < 4) {
    fprintf(stderr,
      "Usage: %s URL BODY_FILENAME HEADER_FILENAME\n"
      "    URL = http://<HOST><PATH>\n",
      argv[0]);
    return 1;
  }
  
  char *url_str = argv[1];
  char *body_filename = argv[2];
  char *header_filename = argv[3];
  
  URL url;
  char *url_error = url_parse(&url, url_str);
  if(NULL != url_error) {
    fprintf(stderr, "URL error: %s\n", url_error);
    return 1;
  }
  
  // resolv
  struct hostent *hostentry = gethostbyname(url->host);
  
  // addr
  struct sockaddr_in server_address;
  ZEROFILL(server_address);
  server_address.sin_family = AF_INET;
  memcpy(server_address.sin_addr,
    hostentry->h_addr_list[0],
    hostentry->h_length);
  server_address.sin_port = htons((u_short) url->port);
  
  url_free(&url);
  
  // socket
  int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(0 > connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) {
    perror("connect");
    exit(errno);
  }
  
  FILE *server_fp = fdopen(sock, "w+");
  if(NULL == server_fp) {
    fprintf(stderr, "バークレイソケットのfdopen()が失敗しました\n");
    return 1;
  }
  
  // http
  fprintf(server_fp, "GET %s HTTP/1.0\n\n", url->path);
  fflush(server_fp);
  
  // header
  FILE *header_fp = fopen(header_filename, "w");
  char *back = "\0\0\0\0\0";
  int i;
  while(EOF != (i = fgetc(server_fp))) {
    fputc(i, header_fp);
    strq_push(back, 4, (char) i);
    if(0 == strncmp(back, "\r\n\r\n", 4))
      break;
  }
  fclose(header_fp);
  
  // body
  FILE *body_fp = fopen(body_filename, "w");
  while(EOF != (i = fgetc(server_fp))) {
    fputc(i, body_fp);
  }
  fclose(body_fp);
  
  fclose(server_fp);
  close(sock);
  
  return 0;
}

#define SCHEME "http://"
#define SCHEME_LEN (sizeof(SCHEME) - 1)
const char *url_default_path = "/";

int url_parse(URL *url, char *str) {
  int len = strlen(str);
  
  // scheme
  if(0 != strncmp(str, SCHEME, SCHEME_LEN)
    return "URLがhttp://で始まっていません";
  url->scheme = "http";
  
  char *str_hostname = str + SCHEME_LEN;
  char *str_path = strchr(str_hostname, '/');
  
  // hostname
  int hostname_len = (NULL != str_path) ?
    str_path - str_hostname :
    len - SCHEME_LEN;
  url->hostname = (char *) malloc(hostname_len + 1);
  strncpy(url->hostname, str_hostname, hostname_len);
  
  // port
  url->port = 80; //default
  
  // path (path [?query] [#fragment])
  int path_len = len - SCHEME_LEN - hostname_len;
  if(NULL != str_path) {
    url->path = (char *) malloc(path_len);
    strncpy(url->path, str_path, path_len);
  } else
    url->path = url_default_path;
  
  return NULL;
}

void url_free(URL *url) {
  free(url->hostname);
  if(url_default_path != url->path) free(url->path);
}

void strq_push(char *q, int capacity, char c) {
  int len = strlen(q);
  if(capacity > len) q[len] = c;
  else {
    for(int i = 1; i < capacity; i++) q[i-1] = q[i];
    q[capacity - 1] = c;
  }
}


"ab\0\0\0", 4
  i=0; q[i+1]='b'; q="bb\0\0\0"
  i=1; q[i+1]='\0'

  q=1




