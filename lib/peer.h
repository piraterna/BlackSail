#ifndef _PEER_H
#define _PEER_H

char *build_handshake(struct torrent *t, char *peer_id);
bool send_handshake(struct peer *p, const char *handshake);

#endif /* _PEER_H */
