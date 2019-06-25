#include "httpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 8888

int main(void)
{
    int s;
    http_request request;         /*クライアントの情報を構造体にまとめている*/

    s=server_init(PORT);          /*サーバとしての初期化を行う*/
    
    printf("Listening iebsd__.ie.tokuyama.ac.jp:%d\n", PORT);

    for(;;){
        accept_client(s,&request);  /*accept()とfdopenを行う*/

        printf("Accept\n");

        read_request(&request);     /*相手からのリクエストを得る*/
        proc_request(&request);     /*リクエストを処理する*/
        send_response(&request);    /*相手へレスポンスを送出*/

        release_client(&request);   /*ファイルポインタ類を閉じる*/
    }
    
    return 0;                     /*呼び出されないがmain()の返り値を記述*/
}



/******************************************************************
以下、このファイルのコメントの指示に従って,プログラムを書き上げて
下さい。その関数の前後の「#if 0」と「#endif」を取り除くと、
あなたが記述した関数が有効になります。
#if 0 〜 #endif までは、コメントアウトされている状態です
#if 0 の「0」を「1」に書き換えると、その関数部分のコメントアウトが
外れて、コンパイルされるようになります。

トップダウンスタイル(main関数側)から実装するもよし、
ボトムアップスタイル(main関数から遠い側)から実装するもよし。
お好きな方からどうぞ。
(このあたりは、ソフト工学で習っていると思うけど・・・・)
******************************************************************/




/***********************************************************
main()関数から呼び出される関数
サーバとしての初期化を行う

    socket()で、ソケットの作成を行いなさい。

    受信するアドレスやポートをserver_addressにセットしなさい。
    なお、この関数が呼び出される際の引数のポート(port)を受信
    ポートとして設定しなさい。

    bind()で、でソケットに上記のserver_addressの値を割り当てなさい。

    listen()で、ソケットを待ち受け状態にしなさい。

    返り値は、ソケットのファイルディスクリプタの値とする。
***********************************************************/
int server_init(int port)
{
  int s;
  static struct sockaddr_in server_address;
  // ソケットを作成
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }
  // bindするアドレスを設定
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = INADDR_ANY;
  // bind
  if(bind(s, &server_address, sizeof(server_address)) < 0) {
    perror("bind");
    exit(1);
  }
  // listen
  listen(s, 5);
  
  return s;
}


/***********************************************************
main()関数から呼び出される関数
acceptを行い、ファイルディスクリプタからファイルポインタを得る

    http_request構造体のreqをクリアしなさい。

    req->client_addressをゼロクリアしなさい。

    accept()で接続要求を受け付けなさい。

    受け付けたファイルディスクリプタをreq->fdに格納しなさい。

    受け付けたファイルディスクリプタをファイルポインタと
    結び付けなさい。その結び付けたファイルポインタを
    req->fpに格納しなさい。
***********************************************************/
void accept_client(int s,http_request *req)
{
  socklen_t c_len;
  clear_request(req);
  // 待ち受け
  req->fd = accept(s, (struct sockaddr *) &(req->client_address), &c_len);
  req->fp = fdopen(req->fd, "r+");
}


/***********************************************************
main()関数から呼び出される関数
相手からのリクエストを調べる

    メソッドタイプを判別する関数 read_method()を引数reqで呼び出しなさい。
    パス部を取得する関数 read_path()を引数reqで呼び出しなさい。
***********************************************************/
void read_request(http_request *req)
{
  read_method(req);
  read_path(req);
}


/***********************************************************
main()関数から呼び出される関数
リクエストを処理する関数

    req->method が ERROR ならば、req->http_response にBAD_REQを格納して
    この関数を終了しなさい。

    ファイルのステータスをセットする関数 make_file_status()を
    引数reqで呼び出しなさい。

    req->http_response が OK ならば、拡張子でcontent-typeを決定する
    set_file_ext()を引数reqで呼び出しなさい。
***********************************************************/
void proc_request(http_request *req)
{
  if(ERROR == req->method) {
    req->http_response = BAD_REQ;
    return;
  }
  
  make_file_status(req);
  
  if(OK == req->http_response)
    set_file_ext(req);
}


/***********************************************************
main()関数から呼び出される関数
レスポンスを返す関数

    ヘッダを送出する関数 send_header()を引数reqで呼び出しなさい。
    オブジェクトボディを送出する関数 send_body()を引数reqで呼び出しなさい。
***********************************************************/
void send_response(http_request *req)
{
  send_header(req);
  send_body(req);
}


/***********************************************************
main()関数から呼び出される関数
クライアントとの接続をクローズする関数

    ファイルポインタ req->fp を閉じなさい
    ファイルディスクリプタ req->fd を閉じなさい
    リクエスト構造体を初期化しなさい
***********************************************************/
void release_client(http_request *req)
{
  fclose(req->fp); //内部で close(req->fd) する
  clear_request(req);
}



/**********************************************************
server_init()から呼び出される関数。
http_request構造体のクリアを行う。

  http_request構造体を次のように初期化する。
              fd:クライアントとのソケット              未使用時は、-1
              fp:クライアントとのファイルポインタ      未使用時は、NULL
          method:クライアントからのリクエストメソッド  未使用時は、ERROR
  client_address:クライアントのアドレス                未使用時は、ゼロクリア
        pathname:クライアントからのパス名              未使用時は、先頭に'\0'
   http_response:クライアントへのステータスコード      未使用時は、INT_SRV_E
         content:パス名のコンテントタイプ              未使用時は、NULL

**********************************************************/
void clear_request(http_request *req)
{
  /*ここは、おまけで解答そのもの。コメントアウトを外せば、動作します。*/
  req->fd=-1;
  req->fp=NULL;
  req->method=ERROR;
  memset(&req->client_address,0,sizeof(req->client_address));
  req->pathname[0]='\0';
  req->http_response=INT_SRV_E;
  req->content=NULL;
}




/***********************************************************
read_request()から呼び出される関数。
クライアントから送られてきたメソッドを解析する。

    クライアントからHTTPリクエストがソケット(ファイルポインタ)に届きます。
    それを読み出して、GETメソッドであれば、req->methodにGETを
    HEADメソッドであれば、req->methodにHEADを格納しなさい。

    なお、HTTPリクエストのリクエストラインは、
    GET /index.html HTTP/1.0\r\n
    のように、
    ○先頭にメソッドの種類{"GET"や"HEAD"や"PUT"など}が届く
    ○空白が届く
    ○パス名が届く
    ○空白が届く
    ○HTTPのバージョンが届く
    ○\r\nが届く
    のようになっている。

    理解できないメソッド名には、req->methodにERRORを格納しなさい。
***********************************************************/
void read_method(http_request *req)
{
  typedef struct {
    char          *name;
    method_number  number;
  }method_type;

  char buf[100];
  char *p;
  method_type *mp;
  int flag=-1;
  static method_type method[]={
    {"GET" ,GET},
    {"HEAD",HEAD},
    {NULL  ,ERROR}
  };
  static int n_methods = sizeof(method) / sizeof(method_type);

  p = buf;
  while(' ' != (*p++ = fgetc(req->fp)));
  *p = '\0';
  
  for(int i = 0; i < n_methods; i++) {
    if(strcmp(buf, method[i].name) == 0) {
      req->method = method[i].number;
      return;
    }
  }
  
  req->method = ERROR;
}




/***********************************************************
read_request()から呼び出される関数。
クライアントから送られてきたパス名をreq->pathnameに格納する

    クライアントから送られてきたパス名をreq->pathnameに格納しなさい。

    ただし,パス名に%が送られてきたら、次の2桁の16進数値を
    そのまま格納しなさい。
    例えば、%7eが送られてきたら、'~'になるように変換する。

    なお、pathnameを'\0'で終端させることを忘れないように。

***********************************************************/
void read_path(http_request *req)
{
  static const char* hex="00112233445566778899AaBbCcDdEeFf";
  char *p=req->pathname;

  char c;
  while(' ' != (c = fgetc(req->fp))) {
    if(c == '%') {
      int h = (strchr(hex, fgetc(req->fp)) - hex) >> 1;
      int l = (strchr(hex, fgetc(req->fp)) - hex) >> 1;
      c = (char) (h << 4 | l);
    }
    
    *p++ = c;
  }
  
  *p = '\0';
}




/***********************************************************
proc_request()から呼び出される関数。
ファイルのステータスをチェックして、ステータスコードを決定する関数

    req->pathname のファイルのステータスをstat()を用いて
    取得しなさい。(取得した値は、req->file_statusに格納する)

    ファイルの状態によって、ステータスコードを決定し、
    以下の値をreq->http_responseに格納しなさい。
    ○ファイルがない場合は、NOT_FOUNDを格納する。
    ○パスの中に検索を許可されていないディレクトリがある場合は、
      FORBIDDENを格納する。
    ○stat()で読み出せない場合は、INT_SRV_Eを格納する。
    ○通常ファイルで,他人も読める場合は、OKを格納する。
    ○他人が読めない場合は、FORBIDDENを格納する。
    ○ディレクトリへのアクセスの場合は、FORBIDDENを格納する。
    ○通常ファイル以外(デバイスファイルとか)であれば、
      BAD_REQを格納する。

      なお、NOT_FOUNDやFORBIDDENなどの値は,httpd.hで定義されて
      いる定数である。
      また、これ以外の実装でもよい。(例えば、ディレクトリへの
      アクセスは、そのディレクトリのファイル一覧を提示するなど
      の追加実装を行ってもよい)

      注意事項：ここでファイルのアクセス制御を間違えると
                あなたが作成したhttpデーモン経由で、
		他人に読みだし禁止のファイルでも、他人が
		読み出すことができますので、要注意。

		なお、正解と称しているライブラリも
		一部、仕様のミスがあります。
***********************************************************/
void make_file_status(http_request *req)
{
  char *dn = req->pathname;
  while(0 != strcmp(".", dn)) {
    struct stat st;
    if(0 != stat(dn, &st)) {
      req->http_response = NOT_FOUND;
      return;
    }
    
    if(dn == req->pathname) {
      if(S_IFDIR & st.st_mode || !(S_IROTH & st.st_mode)) {
        req->http_response = FORBIDDEN;
        return;
      }
      
      if(!(S_IFREG & st.st_mode)) {
        req->http_response = BAD_REQ;
        return;
      }
    }
    
    if(!(S_IXOTH & st.st_mode)) {
      req->http_response = FORBIDDEN;
      return;
    }
    
    dn = dirname(dn);
  }
  
  req->http_response = OK;
}


/***********************************************************
send_response()から呼び出される関数。
レスポンスのヘッダを送出する関数

    HTTPのレスポンス形式である
       HTTP/1.0 200 OK\r\n
    のような文字列(ステータスライン)を送出する。

    ○最初の HTTP/1.0 は固定とする。
    ○次の200は、ステータスコード(req->http_response)とする。
    ○最後のOKは、reason_phrase()関数で検索した
      リーズンフレーズとする。


    可能ならば時間のヘッダをつける
    Date: Wed, 11 Feb 2004 21:46:33 GMT\r\n
    のような形式を送出する。

    サーバに関するヘッダ
    Server: XXXXXXXXXXXXXXXXXXXX\r\n
    は、かっこよい名前をつけて、送出しよう。

    すぐに回線を切断するので、
    Connection: close\r\n
    をヘッダとして送出しよう。

    【重要】コンテントタイプのヘッダ
    Content-Type: text/html \r\n
    は、正しく送出しないと、ブラウザが正常動作しません。
    正しくreqをセットしているならば、req->contentに"text/html"などの
    文字列のポインタが入っています。
    req->http_response が OK の場合は、"Content-Type: "に続いて、
    req->contentの文字列を送信する。
    そうでない場合(OKでないとき)は、"Content-Type: "に続いて、
    "text/html"を送信する。


    最後にヘッダとオブジェクトボディを分離する
    \r\n
    を送信する。
***********************************************************/
void send_header(http_request *req)
{
  time_t now;
  char time_data[100];
  
  now = time(NULL);

  fprintf(req->fp, "HTTP/1.0 %d %s\r\n", req->http_response, reason_phrase(req->http_response));
  
  if(0 != strftime(time_data, sizeof(time_data), "%a, %d %b %Y %T %Z", localtime(now))) {
    printf("Date: %s\r\n", time_data);
  }
  
  fprintf(req->fp,
	  "Server: New International Technology's TokuyamA\r\n"
	  "Content-Type: %s\r\n"
	  "Connection: close\r\n"
	  "\r\n",
	 req->content);
}




/***********************************************************
send_response()から呼び出される関数。
オブジェクトボディを送出する

    req->http_response が OK でない場合は、次のことを実行しなさい。
           p=reason_phrase(req->http_response);
           fprintf(req->fp,
	           "<HTML>\r\n"
	           "<HEAD><TITLE>%3d %s</TITLE></HEAD>\r\n"
	           "<BODY><H1>%3d %s</H1></BODY>\r\n"
	           "</HTML>\r\n",
	           req->http_response,p,
	           req->http_response,p);

   req->methodがHEADの場合は、この関数を終了しなさい。

   req->pathname のファイルをRead Onlyでfopen()する。
   上記のファイルを1文字ずつ読み込んでは、
   req->fpに送出する(EOFがくるまで繰り返す)。
   上記でfopen()したファイルポインタを閉じる。
***********************************************************/
void send_body(http_request *req)
{
  char *p;
  FILE *file_fp;
  int c;
  
  if(OK != req->http_response) {
    p = reason_phrase(req->http_response);
    fprintf(req->fp,
            "<HTML>\r\n"
            "<HEAD><TITLE>%3d %s</TITLE></HEAD>\r\n"
            "</HTML>\r\n",
            req->http_response, p,
            req->http_response, p);
    
    return;
  }
  
  if(HEAD == req->method) return;
  
  if(NULL == (file_fp = fopen(req->pathname, "rb"))) {
    perror("fopen");
    return;
  }
  
  while(0 <= (c = fgetc(file_fp))) fputc(c, req->fp);
  
  fclose(file_fp);
}


/***********************************************************
send_header()から呼び出される関数
ステータスコードによって、返すリーズンフレーズを決める

    i には、{OK,BAD_REQ,FORBIDDEN,NOT_FOUND}などのステータスコードが
    入っている。以下のコードの時,次の文字列を返すこと。

    コード       返す文字列
    OK           "OK"
    BAD_REQ      "Bad Request"
    FORBIDDEN    "Forbidden"
    NOT_FOUND    "Not Found"
    METHOD_NA    "Method Not Allow"
    INT_SRV_E    "Internal Server Error"

    ヒント:既に書いてある構造体を検索すればよい。
**********************************************************/
char* reason_phrase(response_number i)
{
  typedef struct{
    response_number num;
    char *phrase;
  }response_type;
  static response_type response[]={
    {OK        ,"OK"},
    {BAD_REQ   ,"Bad Request"},
    {FORBIDDEN ,"Forbidden"},
    {NOT_FOUND ,"Not Found"},
    {METHOD_NA ,"Method Not Allow"},
    {INT_SRV_E ,"Internal Server Error"},
    {0,NULL}
  };
  response_type *p;

  for(p = response; p->phrase != NULL; p++)
    if(i == p->num) return p->phrase;
  
  return NULL;
}





/***********************************************************
proc_request()から呼び出される関数
ファイルの拡張子を調べ、content-typeを決定する

    pathnameに格納されている文字列で、以下の条件の位置に
    p2というポインタを設定せよ。

    pathnameに格納されている文字列の一番後ろの '.'の次の
    文字へのポインタ

    但し、'.' より後ろに '/' があった場合は、p2にNULLを代入
    また、'.' がない場合もp2にNULLを代入。

         例：/home/i0/nitta/public_html/index.html であれば
                                              ^ここへのポインタをp2に格納
             /test.test/index であれば NULLポインタをp2に格納
	     /testtest/index  であれば NULLポインタをp2に格納

    p2を引数として、search_content_type()を呼び出す。
    その返り値をreq->contentに格納せよ。
**********************************************************/
void set_file_ext(http_request *req)
{
  char *p2;
  p2 = strrchr(req->pathname, '.');
  req->content = search_content_type(p2);
}





/*********************************************************
set_file_ext()から呼び出される関数
拡張子からcontent-typeを決定する

    この関数を呼び出されたときの引数が"html"という文字列であれば、
    "text/html"という文字列へのポインタを返しなさい。

    同様に、"gif"であれば、"image/gif"を返しなさい。

    その他は、mime type を調べて,実装が可能ならば、実装しなさい。

    判らない場合は、"application/octet-stream"とする。
*********************************************************/
char* search_content_type(char *p2)
{
  typedef struct{
    char *ext;
    char *type;
  }c_type;
  c_type *cp;
  static c_type content_type[]={
  {"html","text/html"},
  {"htm","text/html"},
  {"txt","text/plain"},
  {"gif","image/gif"},
  {"png","image/png"},
  {NULL,"application/octet-stream"}
  };
  for(cp = content_type; cp->ext; cp++)
    if(0 == strcmp(cp->ext, p2)) return cp->type;
  return "application/octet-stream";
}


