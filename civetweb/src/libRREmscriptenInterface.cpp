#include "../../cdl/CDL.hpp"
#include <queue>
#include "../../include/libRR.h"
#include <unistd.h>

void setup_web_server() {
	printf("Setting up web server not required with Emscripten \n");
}

void stop_web_server() {
	// Not required with Emscripten
}