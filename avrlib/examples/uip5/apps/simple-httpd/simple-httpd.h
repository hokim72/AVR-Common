#ifndef __SIMPLE_HTTPD_H__
#define __SIMPLE_HTTPD_H__
#include "uip/psock.h"

struct simple_httpd_state {
	struct psock p;
};


void simple_httpd_appcall(void);
void simple_httpd_init(void);

//#define SIMPLE_HTTPD_APP_CALL_MAP
#define UIP_APPCALL simple_httpd_appcall
typedef struct simple_httpd_state uip_tcp_appstate_t;

#endif /* __SIMPLE_HTTPD_H__ */
