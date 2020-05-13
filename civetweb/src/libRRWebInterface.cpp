#include "../../cdl/CDL.hpp"
#include <queue>
#include "../include/civetweb.h"

/* Server context handle */
  struct mg_context *ctx;

  
int
PostResponser(struct mg_connection *conn, void *cbdata)
{
	long long r_total = 0;
	int r, s;

	char buf[2048];

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if (0 != strcmp(ri->request_method, "POST")) {
		/* Not a POST request */
		char buf[1024];
		int ret = mg_get_request_link(conn, buf, sizeof(buf));

		mg_printf(conn,
		          "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n");
		mg_printf(conn, "Content-Type: text/plain\r\n\r\n");
		mg_printf(conn,
		          "%s method not allowed in the POST handler\n",
		          ri->request_method);
		if (ret >= 0) {
			mg_printf(conn,
			          "use a web tool to send a POST request to %s\n",
			          buf);
		}
		return 1;
	}

	if (ri->content_length >= 0) {
		/* We know the content length in advance */
	} else {
		/* We must read until we find the end (chunked encoding
		 * or connection close), indicated my mg_read returning 0 */
	}

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nConnection: "
	          "close\r\nTransfer-Encoding: chunked\r\n");
	mg_printf(conn, "Content-Type: text/plain\r\n\r\n");

	r = mg_read(conn, buf, sizeof(buf));
	while (r > 0) {
		r_total += r;
		s = mg_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = mg_read(conn, buf, sizeof(buf));
	}
	mg_printf(conn, "0\r\n");
  cout << buf << "\n";

	return 1;
}



int
log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}

struct mg_callbacks callbacks;
void setup_web_server() {
  printf("Setting up web server on Port 1234 \n");
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.log_message = log_message;
   /* Initialize the library */
    mg_init_library(0);
    const char *options[] = {
      "document_root",
		"./libRetroReversing/websrc/",
		"listening_ports",
		"1234",
		"request_timeout_ms",
		"10000",
		"error_log_file",
		"error.log",
    "enable_directory_listing",
    "yes",
    0
	};

    /* Start the server */
    ctx = mg_start(&callbacks, 0, options);

    mg_set_request_handler(ctx, "/postresponse", PostResponser, 0);

    /* Add some handler */
    // mg_set_request_handler(ctx, "/test", FileHandler, 0);
		system("open http://localhost:1234");
}

void stop_web_server() {
  mg_stop(ctx);
  mg_exit_library();
}