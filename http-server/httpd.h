#ifndef _HTTPD_H_
#define _HTTPD_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>			
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>     
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

typedef enum{ERROR=0,GET,HEAD} method_number;

typedef enum{
OK       =200,
BAD_REQ  =400,
FORBIDDEN=403,
NOT_FOUND=404,
METHOD_NA=405,
INT_SRV_E=500} response_number;

typedef struct{
  int fd;
  FILE* fp;
  method_number method;
  struct sockaddr_in client_address;
  char pathname[FILENAME_MAX];
  struct stat file_status;
  response_number http_response;
  char *content;
}http_request;

extern char* reason_phrase(response_number i);
extern char* search_content_type(char *p2);

extern int   server_init(int port);
extern void  accept_client(int s,http_request *req);

extern void  clear_request(http_request *req);
extern void  make_file_status(http_request *req);
extern void  read_method(http_request *req);
extern void  read_path(http_request *req);
extern void  set_file_ext(http_request *req);
extern void  send_header(http_request *req);
extern void  read_request(http_request *req);
extern void  proc_request(http_request *req);
extern void  send_body(http_request *req);
extern void  send_response(http_request *req);
extern void  release_client(http_request *req);

#endif
