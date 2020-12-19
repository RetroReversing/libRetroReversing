#include "../../cdl/CDL.hpp"
#include <queue>
#include "../include/civetweb.h"
#include "../../include/libRR.h"

/* Server context handle */
  struct mg_context *ctx;

	static int
SendJSON(struct mg_connection *conn, const char* json_str)
{
	size_t json_str_len = strlen(json_str);

	/* Send HTTP message header */
	mg_send_http_ok(conn, "application/json; charset=utf-8", json_str_len);

	/* Send HTTP message content */
	mg_write(conn, json_str, json_str_len);

	// mg_printf(conn, "\r\n");

	return (int)json_str_len;
}

  
char buf[1024*1024*4]; // Needs to hold large JSON
int
PostResponser(struct mg_connection *conn, void *cbdata)
{
	long long r_total = 0;
	int r, s;

	memset(buf, 0, sizeof(buf));

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if (0 != strcmp(ri->request_method, "POST")) {
		/* Not a POST request */
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

	// mg_printf(conn,
	//           "HTTP/1.1 200 OK\r\nConnection: "
	//           "close\r\nTransfer-Encoding: chunked\r\n");
	// mg_printf(conn, "Content-Type: application/json\r\n\r\n");

	r = mg_read(conn, buf, sizeof(buf));
	while (r > 0) {
		r_total += r;
		// s = mg_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = mg_read(conn, buf, sizeof(buf));
	}
	
  // cout << "\n\n" << buf << "\n\n\n";
	string result = libRR_parse_message_from_web(buffer_to_string(buf));
	SendJSON(conn, result.c_str());

	return 1;
}



int
log_message(const struct mg_connection *conn, const char *message)
{
	puts("\nLog message:");
	puts(message);
	return 1;
}

extern string libRR_project_directory;
extern string libRR_export_directory;

struct mg_callbacks callbacks;
void setup_web_server() {
  printf("Setting up web server on Port 1234 \n");
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.log_message = log_message;
	string url_rewrite = "/game/="+libRR_project_directory;
	/* Initialize the library */
	mg_init_library(0);
	const char *options[] = {
		"document_root",
	"./libRetroReversing/websrc/dist",
	"listening_ports",
	"127.0.0.1:1234", //"1234",
	"request_timeout_ms",
	"10000",
	"error_log_file",
	"error.log",
	"enable_directory_listing",
	"no",
	// "allow_sendfile_call",
	// "no",
	"access_control_allow_methods",
	"", //"POST,GET,OPTIONS", // Make sure to disable CONNECT http method
	"access_control_allow_headers",
	"",
	"access_control_allow_origin",
	"localhost",
	"url_rewrite_patterns",
	url_rewrite.c_str(),
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