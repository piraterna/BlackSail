#ifndef _BLACKSAIL_CONFIG_H
#define _BLACKSAIL_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

struct blacksail_config {
	// client identification string
	char client_id[21];

	// the port we will listen on
	uint16_t port;
};

bool blacksail_update_config(struct blacksail_config *new_cfg);

#endif /* _BLACKSAIL_CONFIG_H */
