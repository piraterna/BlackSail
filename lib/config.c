#include <blacksail/blacksail.h>
#include <blacksail/config.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct blacksail_config cfg = {
	.client_id = "BlackSailClient12345",
	.port = 9831
};

bool blacksail_update_config(struct blacksail_config *new_cfg)
{
	if (new_cfg == NULL) {
		return false;
	}

	if (strlen(new_cfg->client_id) < 20) {
		return false;
	}

	strncpy(cfg.client_id, new_cfg->client_id, 20);
	cfg.client_id[20] = '\0';
	cfg.port = new_cfg->port;

	return true;
}
