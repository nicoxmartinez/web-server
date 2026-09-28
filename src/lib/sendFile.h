#ifndef SEND_FILE_H
#define SEND_FILE_H

void send_file(int client_socket, const char *file_path, const char *http_status);

#endif