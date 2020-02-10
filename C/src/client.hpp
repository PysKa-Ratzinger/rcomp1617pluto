#pragma once

#include <netinet/in.h>

unsigned long start_file_transfer(const struct in_addr bf_addr,
		const unsigned short port,
		const char* file, const char* location);
