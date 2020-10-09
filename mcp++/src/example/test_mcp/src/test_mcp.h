
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>



class CHttpServerApp : public CacheProc
{
public:
  CHttpServerApp() : arg_vals(NULL) {}
  virtual ~CHttpServerApp()
    {
        if ( arg_vals )
            free(arg_vals);
    }

  virtual void  run(const std::string& conf_file);

  void      dispatch_ccd();
  int       handle_request(char * req_data, unsigned req_data_len);
  int       request_file(int fd, struct stat *st, const char *name);
  void      send_err_msg(int code);

  CFifoSyncMQ     *mq_ccd_2_mcd;
  CFifoSyncMQ     *mq_mcd_2_ccd;
  TCCDHeader      *ccdheader;
  memhead       *m_head;
  unsigned      cur_flow;

  CHttpTemplate   http_template;
  char        inbuf[IN_BUF_SIZE];   // Input buffer.
  char        outbuf[OUT_BUF_SIZE]; // Output buffer.

  char        docroot[PATH_MAX];    // HTTP document root.
  char        defdoc[DEF_NAME_SIZE];  // HTTP default document.
  myalloc_alloc_conf  shmalloc_conf;

  char        **arg_vals;
  int         arg_cnt;
  char        r_clength[CLENGTH_LEN];
};


