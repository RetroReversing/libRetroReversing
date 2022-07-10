#include "../../cdl/CDL.hpp"
#include <queue>
#include "../include/civetweb.h"
#include "../../include/libRR.h"
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif
#include <unistd.h>

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

#define BUF_SIZE 1024*1024*4
// char buf[1024*1024*4]; // Needs to hold large JSON
int
PostResponser(struct mg_connection *conn, void *cbdata)
{
	json message_json;
	long long r_total = 0;
	int r, s;

	char* buf = new char[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if (0 != strcmp(ri->request_method, "POST")) {
		/* Not a POST request */
		int ret = mg_get_request_link(conn, buf, BUF_SIZE);

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

	r = mg_read(conn, buf, BUF_SIZE);
	while (r > 0) {
		r_total += r;
		// s = mg_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = mg_read(conn, buf, BUF_SIZE);
	}
	
  // cout << "\n\n" << buf << "\n\n\n";
	printf("About to start Parsing message_json\n");
	message_json = json::parse(buffer_to_string(buf));
	printf("Finished Parsing message_json\n");
	string result = libRR_parse_message_from_web(message_json);
	SendJSON(conn, result.c_str());
	delete[] buf;

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


void print_cwd() {
	char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("INFO: Current working dir: %s\n\n", cwd);
   } else {
       perror("getcwd() error");
   }
}

extern char retro_base_directory[4096];

void setup_web_server() {
	printf("Setting up web server on Port 1234 \n");
	print_cwd();
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = log_message;
	string url_rewrite = "/game/="+libRR_project_directory+"";
	printf("\n\n\nINFO: Setting up project directory rewrite: %s\n", url_rewrite.c_str());
	const char* frontend_folder = "./libRetroReversing/websrc/dist";

	if( access( "./libRetroReversing/websrc/dist/index.html", F_OK ) == 0 ) {
    // file exists
		printf("INFO: Found libRetroReversing frontend folder\n");
	} else {
			// file doesn't exist
			printf("ERROR Can't find libRetroReversing frontend folder\n");
			printf("Will try to use retro arch /system folder \n");
			frontend_folder = ((string)retro_base_directory + "/libRetroReversing/websrc/dist/").c_str();
	}

	/* Initialize the library */
	mg_init_library(0);
	// const char* options[] = new const char*[];
	char *options[] = {
		"document_root",
	(char*) frontend_folder,
	"listening_ports",
	"127.0.0.1:1234", //"1234",
	"request_timeout_ms",
	"10000",
	"url_rewrite_patterns",
	(char*) url_rewrite.c_str(),
	"error_log_file",
	"error.log",
	"enable_directory_listing",
	"yes",
	// "allow_sendfile_call",
	// "no",
	"access_control_allow_methods",
	"", //"POST,GET,OPTIONS", // Make sure to disable CONNECT http method
	"access_control_allow_headers",
	"",
	"access_control_allow_origin",
	"localhost",
	0
};
printf("server options: %s\n\n", options[17]);

    /* Start the server */
    ctx = mg_start(&callbacks, 0, (const char**) options);

    mg_set_request_handler(ctx, "/postresponse", PostResponser, 0);

	// TODO: get both console name and game name here
	string url_to_open = "http://localhost:1234/#/" + string(libRR_console)+"/"+ string(libRR_game_name);

    /* Add some handler */
    // mg_set_request_handler(ctx, "/test", FileHandler, 0);
		#ifdef _WIN32
		ShellExecute(0, 0, url_to_open.c_str(), 0, 0 , SW_SHOW );
		#else
		string command = "open " + url_to_open;
		system(command.c_str());
		#endif
}

void stop_web_server() {
  mg_stop(ctx);
  mg_exit_library();
}