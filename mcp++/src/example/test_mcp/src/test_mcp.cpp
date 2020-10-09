

#include "tfc_cache_proc.h"
#include "tfc_net_ccd_define.h"
#include "tfc_base_config_file.h"
#include "tfc_base_str.h"
#include "tfc_base_http.h"
#include "myalloc.h"
#include "tfc_net_open_shmalloc.h"


// #define __DEBUG_SIM      // Debug switch.
// #define __CONN_CLOSE   // Connection Close.

#define OUT_BLOCK_SIZE    (8192)                  // Output block size.
#define OUT_BUF_SIZE    (CCD_HEADER_LEN + sizeof(memhead))    // Output buffer size.
#ifdef HTTP_LIMIT_LENGTH
#define IN_BUF_SIZE     (HTTP_MAX_LENGTH + sizeof(TCCDHeader))  // Input buffer size.
#else
#define IN_BUF_SIZE     ((1<<20) + sizeof(TCCDHeader))      // Input buffer size.
#endif

#define ARG_CNT_MAX     32
#define KEY_STR_SIZE    16
#define DEF_NAME_SIZE   128
#define SEM_NAME_SIZE   DEF_NAME_SIZE

#define CLENGTH_LEN     32

#ifndef CONVERT_URI_HEAD_LEN
#define CONVERT_URI_HEAD_LEN  7
#endif

using namespace std;
using namespace tfc::base;
using namespace tfc::cache;
using namespace tfc::net;
using namespace tfc::http;

static char       uri_buf[PATH_MAX];
static char       path_buf[PATH_MAX];

static char   r_code_200[] = "200";
static char   r_code_400[] = "400";
static char   r_code_403[] = "403";
static char   r_code_404[] = "404";
static char   r_code_500[] = "500";

static char   r_reason_200[] = "OK";
static char   r_reason_400[] = "Bad Request";
static char   r_reason_403[] = "Forbidden";
static char   r_reason_404[] = "Not Found";
static char   r_reason_500[] = "Internal Server Error";

void
http_server_disp_ccd(void *priv);

static int
server_build_template(CHttpTemplate *p_template)
{
  char          http_head[HTTP_HEAD_MAX];
  char          *data = http_head;
  char          *head = NULL;
  http_template_arg_t   args[ARG_CNT_MAX];
  int           head_len;

  data += sprintf(data, "HTTP/1.1 ");
  args[0].offset = data - http_head;
  head = data;
  data += sprintf(data, "    ");
  args[0].max_length = data - head - 1;

  args[1].offset = data - http_head;
  head = data;
  data += sprintf(data, "                                                                \r\n");
  args[1].max_length = data - head - 2;

  data += sprintf(data, "Server: MCP-Simple-HTTP\r\n");
  data += sprintf(data, "Content-Length: ");
  args[2].offset = data - http_head;
  head = data;
  data += sprintf(data, "                                \r\n");
  args[2].max_length = data - head - 2;

  data += sprintf(data, "Cache-Control: no-cache\r\n");
#ifdef __CONN_CLOSE
  data += sprintf(data, "Connection: Close\r\n");
#else
  data += sprintf(data, "Connection: Keep-Alive\r\n");
#endif

  data += sprintf(data, "\r\n");

  head_len = data - http_head;

  return p_template->Init(http_head, head_len, args, 3);
}

int check_path( char *path )
{
  char        *str = path;
  unsigned short    dirs = 0, udirs = 0;
  char        *pos = NULL;
  int         len;

  if ( *path != '/' ) {
    if ( strlen(path) > CONVERT_URI_HEAD_LEN && strncmp(path, "http://", CONVERT_URI_HEAD_LEN) == 0 ) {
      pos = strchr(path + CONVERT_URI_HEAD_LEN, '/');
      if ( pos ) {
        len = strlen(pos);
        memmove(path, pos, len);
        path[len] = 0;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  while ( *str != 0x0 && ( udirs == 0 || udirs < dirs ) ) {
    if ( *str == '/' ) {
      while ( *str == '/' ) {
        str++;
      }
      if( *str == '.' && *(str + 1) == '.' && *(str + 2) == '/' ) {
        udirs++;
        str += 3;
        continue;
      }
      else if( *str == '.' && *(str + 1) == '/' ) {
        str += 2;
        continue;
      }
      dirs++;
    }
    str++;
  }

  return (udirs < dirs ? 0 : -1);
}

void CHttpServerApp::run(const  string &conf_file)
{
  CFileConfig& page = * new CFileConfig();
  page.Init(conf_file);

  // HTTP reponse template.
  arg_cnt = 3;
  arg_vals = (char **)calloc(arg_cnt, sizeof(char *));
  if ( !arg_vals )
  {
    cout<<"Alloc memory for HTTP template arg values fail!"<<endl;
    err_exit();
  }
  arg_vals[2] = r_clength;  // arg_vals[0] point to one of r_code_XXX, arg_vals[1] point to one of r_reason_XXX.
  if ( server_build_template(&http_template) )
  {
    cout<<"Build http response template fail!"<<endl;
    err_exit();
  }

  // Default document.
  try
  {
    string str_defdoc = page["root\\http\\default_doc"];
    if ( str_defdoc.length() > DEF_NAME_SIZE - 1 ) {
      cout<<"Too long \"default_doc\" value \""<<str_defdoc.c_str()<<"\"!"<<endl;
      err_exit();
    }
    strcpy(defdoc, str_defdoc.c_str());
  } catch (...)
  {
    cout<<"Get default document fail!"<<endl;
    err_exit();
  }

  // Document root.
  try
  {
    string str_docroot = page["root\\http\\docroot"];
    char t_docroot[PATH_MAX];
    t_docroot[PATH_MAX - 1] = 0;
    strncpy(t_docroot, str_docroot.c_str(), PATH_MAX - 1);
    if ( !realpath(t_docroot, docroot) ) {
      cout<<"Get real path of \""<<t_docroot<<"\" fail! "<<strerror(errno)<<endl;
      err_exit();
    }
    // Docroot must be dir.
    struct stat st;
    if ( !stat(docroot, &st) ) {
      if( !S_ISDIR(st.st_mode) ) {
        cout<<"Docroot must be directory!"<<endl;
        err_exit();
      }
    } else {
      cout<<"Get docroot stat fail! "<<strerror(errno)<<endl;
      err_exit();
    }
    // Erase last '/' although root is '/'. Because first byte of uri must be '/'.
    int tmplen = strlen(docroot);
    if ( docroot[tmplen - 1] == '/' ) {
      docroot[tmplen - 1] = 0;
    }
  } catch (...) {
    cout<<"Get docroot fail!"<<endl;
    err_exit();
  }

  // MCP mqs.
  mq_ccd_2_mcd = _mqs["mq_ccd_2_mcd"];
  mq_mcd_2_ccd = _mqs["mq_mcd_2_ccd"];
  if ( add_mq_2_epoll(mq_ccd_2_mcd, http_server_disp_ccd, this) ) {
    cout<<"Add input mq to EPOLL fail!"<<endl;
    err_exit();
  }

  string alloc_conf;
  try {
    alloc_conf = page["root\\shmalloc\\shmalloc_conf_file"];
  } catch(...) {
    err_exit();
  }

  // Shm alloc initialize.
  // if ( tfc::net::OpenShmAlloc(alloc_conf) ) {
  //  cout<<"Open shmalloc fail!"<<endl;
  //  return;
  // }

  CFileConfig& allocpage = * new CFileConfig();
  allocpage.Init(alloc_conf);
  try {
    shmalloc_conf.shmkey = from_str<unsigned>(allocpage["root\\shmkey"]);
    shmalloc_conf.shmsize = from_str<unsigned long>(allocpage["root\\shmsize"]);
    strcpy(shmalloc_conf.semname, allocpage["root\\semname"].c_str());
    shmalloc_conf.minchunksize = from_str<unsigned>(allocpage["root\\minchunksize"]);
    shmalloc_conf.maxchunksize = from_str<unsigned>(allocpage["root\\maxchunksize"]);
    shmalloc_conf.maxchunknum = from_str<unsigned>(allocpage["root\\maxchunknum"]);
  } catch(...) {
    cout<<"Get shmalloc configration fail!"<<endl;
    err_exit();
  }

  // Buffer format.
  ccdheader = (TCCDHeader *)outbuf;
  m_head = (memhead *)(outbuf + CCD_HEADER_LEN);

  // Main loop.
  cout<<"Simple HTTP Server has been launched!"<<endl;

  while ( !stop ) {
    run_epoll_4_mq();
  }

  // Server exit normally.
  cout<<"Simple HTTP Server has been shutdown!"<<endl;
}

void
CHttpServerApp::send_err_msg(int code)
{
  int     msg_len = 0, ret_len = 0;
  char    *msg = NULL;
  char    *shm_buf = NULL;

  m_head->mem = NULL_HANDLE;

  strcpy(r_clength, "0");
  if ( code == 200 ) {
    // Should not here, If run here set content-Length to 0.
    arg_vals[0] = r_code_200;
    arg_vals[1] = r_reason_200;
  } else if ( code == 400 ) {
    arg_vals[0] = r_code_400;
    arg_vals[1] = r_reason_400;
  } else if ( code == 403 ) {
    arg_vals[0] = r_code_403;
    arg_vals[1] = r_reason_403;
  } else if ( code == 404 ) {
    arg_vals[0] = r_code_404;
    arg_vals[1] = r_reason_404;
  } else {
    // Else return "500 Internal Server Error".
    arg_vals[0] = r_code_500;
    arg_vals[1] = r_reason_500;
  }

  msg_len = http_template.ProduceRef(&msg, &ret_len, arg_vals, arg_cnt);
  if ( msg_len <= 0 ) {
    cout<<"Produce response message fail!"<<endl;
    goto close_conn;
  }

  m_head->len = (unsigned)msg_len;
  ccdheader->_type = ccd_req_data_shm;
  m_head->mem = myalloc_alloc((unsigned)((unsigned)msg_len < shmalloc_conf.minchunksize ? shmalloc_conf.minchunksize : msg_len));
  if ( m_head->mem == NULL_HANDLE ) {
    cout<<"Shm alloc fail!"<<endl;
    goto close_conn;
  }
  shm_buf = (char *)myalloc_addr(m_head->mem);
  if ( !shm_buf ) {
    cout<<"Get shm address fail!"<<endl;
    goto err_out;
  }
  memcpy(shm_buf, msg, msg_len);
  if ( mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN + sizeof(memhead), cur_flow) < 0 ) {
    cout<<"Enqueue fail!"<<endl;
    goto err_out;
  }

  return;

err_out:
  if ( m_head->mem != NULL_HANDLE ) {
    myalloc_free(m_head->mem);
    m_head->mem = NULL_HANDLE;
  }

close_conn:
  ccdheader->_type = ccd_req_disconnect;
  mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN, cur_flow);

  return;
}

int
CHttpServerApp::request_file(int fd, struct stat *st, const char *name)
{
  long long   f_size = st->st_size;
  long long   left = st->st_size;
  int       msg_len = 0, ret_len = 0;
  char      *head_msg = NULL;
  char      *shm_buf = NULL;
  int       body_send_len = 0;

  // Send head first.
  m_head->mem = NULL_HANDLE;

  arg_vals[0] = r_code_200;
  arg_vals[1] = r_reason_200;
  sprintf(r_clength, "%lld", f_size);

  msg_len = http_template.ProduceRef(&head_msg, &ret_len, arg_vals, arg_cnt);
  if ( msg_len <= 0 ) {
    cout<<"200 head produce response 200 head message fail!"<<endl;
    goto head_close_conn;
  }

  m_head->len = (unsigned)msg_len;
  ccdheader->_type = ccd_req_data_shm;
  m_head->mem = myalloc_alloc((unsigned)((unsigned)msg_len < shmalloc_conf.minchunksize ? shmalloc_conf.minchunksize : msg_len));
  if ( m_head->mem == NULL_HANDLE ) {
    cout<<"200 head shm alloc fail!"<<endl;
    goto head_close_conn;
  }
  shm_buf = (char *)myalloc_addr(m_head->mem);
  if ( !shm_buf ) {
    cout<<"200 head get shm address fail!"<<endl;
    goto head_err_out;
  }
  memcpy(shm_buf, head_msg, msg_len);
  if ( mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN + sizeof(memhead), cur_flow) < 0 ) {
    cout<<"200 head enqueue fail!"<<endl;
    goto head_err_out;
  }

  goto head_ok;

head_err_out:
  if ( m_head->mem != NULL_HANDLE ) {
    myalloc_free(m_head->mem);
    m_head->mem = NULL_HANDLE;
  }

head_close_conn:
  ccdheader->_type = ccd_req_disconnect;
  mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN, cur_flow);

  return -1;

head_ok:
  while ( left > 0 ) {
    m_head->mem = NULL_HANDLE;
    m_head->len = 0;
    body_send_len = 0;
    if ( left > OUT_BLOCK_SIZE ) {
      body_send_len = OUT_BLOCK_SIZE;
      left -= body_send_len;
    } else {
      body_send_len = left;
      left -= body_send_len;
    }
    ccdheader->_type = ccd_req_data_shm;
    m_head->len = body_send_len;
    m_head->mem = myalloc_alloc((unsigned)((unsigned)body_send_len < shmalloc_conf.minchunksize ? shmalloc_conf.minchunksize : body_send_len));
    if ( m_head->mem == NULL_HANDLE ) {
      cout<<"Body shm alloc fail!"<<endl;
      goto body_close_conn;
    }
    shm_buf = (char *)myalloc_addr(m_head->mem);
    if ( !shm_buf ) {
      cout<<"Body get shm address fail!"<<endl;
      goto body_err_out;
    }
    if ( read(fd, shm_buf, body_send_len) != body_send_len ) {
      cout<<"Read file fail!"<<endl;
      goto body_err_out;
    }
    if ( mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN + sizeof(memhead), cur_flow) < 0 ) {
      cout<<"Body enqueue fail!"<<endl;
      goto body_err_out;
    }
  }

#ifdef __DEBUG_SIM
  if ( left != 0 ) {
    cout<<"ERROR left is "<<left<<endl;
  }
  ccdheader->_type = ccd_req_disconnect;
  mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN, cur_flow);
#endif

  return 0;

body_err_out:
  if ( m_head->mem != NULL_HANDLE ) {
    myalloc_free(m_head->mem);
    m_head->mem = NULL_HANDLE;
  }
body_close_conn:
  ccdheader->_type = ccd_req_disconnect;
  mq_mcd_2_ccd->enqueue(outbuf, CCD_HEADER_LEN, cur_flow);

  return -1;
}

int
CHttpServerApp::handle_request(char * req_data, unsigned req_data_len)
{
  CHttpParse    http_parse;
  char      *param_pos = NULL;
  struct stat   st;
  int       fd;
  int       ret = 0;
  int       code = 0;

  if ( http_parse.Init(req_data, req_data_len) ) {
    cout<<"Initialize HTTP parse fail!"<<endl;
    code = 400;
    goto err_code;
  }

  uri_buf[PATH_MAX - 1] = 0;
  strncpy(uri_buf, http_parse.HttpUri(), PATH_MAX - 2);

  if ( check_path(uri_buf) ) {
    cout<<"Invalid HTTP request URI \""<<uri_buf<<"\"!"<<endl;
    code = 403;
    goto err_code;
  }

  param_pos = strchr(uri_buf, '?');
  if ( param_pos ) {
    *param_pos = 0;
    param_pos++;
  }

  path_buf[PATH_MAX - 1] = 0;
  snprintf(path_buf, PATH_MAX - 2, "%s%s", docroot, uri_buf);

  if ( stat(path_buf, &st) ) {
    cout<<"Get request file \""<<path_buf<<"\" status fail! "<<strerror(errno)<<endl;
    code = 404;
    goto err_code;
  }

  if ( S_ISDIR(st.st_mode) ) {
    strncat(path_buf, defdoc, PATH_MAX - 2);
    if ( stat(path_buf, &st) ) {
      cout<<"Get request file \""<<path_buf<<"\" status fail! "<<strerror(errno)<<endl;
      code = 404;
      goto err_code;
    }
  }

  fd = open(path_buf, O_RDONLY);
  if ( fd == -1 ) {
    cout<<"Open request file \""<<path_buf<<"\" fail! "<<strerror(errno)<<endl;
    // File existed because stat call success, but could not open. So return "403 Forbidden".
    code = 403;
    goto err_code;
  }

  if ( request_file(fd, &st, path_buf) ) {
    cout<<"Request file \""<<path_buf<<"\" fail!"<<endl;
    // Already return message in request file.
    ret = -1;
  }

  close(fd);

  return ret;

err_code:
  send_err_msg(code);

  return -1;
}

void CHttpServerApp::dispatch_ccd()
{
  unsigned    data_len;
  int       ret;
  char      *http_data = NULL;

  do {
    data_len = 0;
    ret = mq_ccd_2_mcd->try_dequeue(inbuf, IN_BUF_SIZE, data_len, cur_flow);
    if( !ret && data_len > 0 ) {
      TCCDHeader *ccd_header = (TCCDHeader *)inbuf;
      if ( ccd_header->_type == ccd_rsp_data_shm ) {
        memhead *tmp_memhead = (memhead *)(inbuf + sizeof(TCCDHeader));
        http_data = (char*)myalloc_addr(tmp_memhead->mem);
        if ( http_data ) {
          if ( handle_request(http_data, tmp_memhead->len) ) {
            cout<<"Handle HTTP request fail!"<<endl;
          }
        } else {
          cout<<"Get shm address fail!"<<endl;
        }
        myalloc_free(tmp_memhead->mem);
      } else if ( ccd_header->_type == ccd_rsp_data ) {
        http_data = (char *)(inbuf + sizeof(TCCDHeader));
        if ( handle_request(http_data, data_len - sizeof(TCCDHeader)) ) {
          cout<<"Handle HTTP request fail!"<<endl;
        }
      } else if ( ccd_header->_type == ccd_rsp_connect ) {
        // cout<<"CCD request data type ccd_rsp_connect!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect ) {
        cout<<"CCD request data type ccd_rsp_disconnect!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_overload ) {
        cout<<"CCD request data type ccd_rsp_overload!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_overload_conn ) {
        cout<<"CCD request data type ccd_rsp_overload_conn!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_overload_mem ) {
        cout<<"CCD request data type ccd_rsp_overload_mem!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect_timeout ) {
        cout<<"CCD request data type ccd_rsp_disconnect_timeout!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect_local ) {
        cout<<"CCD request data type ccd_rsp_disconnect_local!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect_peer_or_error ) {
        cout<<"CCD request data type ccd_rsp_disconnect_peer_or_error!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect_overload ) {
        cout<<"CCD request data type ccd_rsp_disconnect_overload!"<<endl;
        continue;
      } else if ( ccd_header->_type == ccd_rsp_disconnect_error ) {
        cout<<"CCD request data type ccd_rsp_disconnect_error!"<<endl;
        continue;
      } else {
        // Never occur.
        cout<<"Invalid CCD request data type!"<<endl;
        continue;
      }
    }
  }while( !stop && !ret && data_len > 0 );
}

void http_server_disp_ccd(void *priv)
{
  CHttpServerApp *http_server = (CHttpServerApp *)priv;
  http_server->dispatch_ccd();
}

extern "C"
{
  CacheProc* create_app()
  {
    return new CHttpServerApp();
  }

  const char *get_plugin_version_func()
  {
    static const char *version = "simple_http: 1.0.0";
    return version;
  }

  const char *get_addinfo_0_func()
  {
    static const char *add0 = "simple_http-add0";
    return add0;
  }
}


