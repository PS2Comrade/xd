#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Process a single client connection fd until it disconnects */
void protocol_handle_client(int fd);

#endif /* PROTOCOL_H */
