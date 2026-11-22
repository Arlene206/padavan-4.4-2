#ifndef SCUTCLIENT_CREDENTIALS_H
#define SCUTCLIENT_CREDENTIALS_H

/* Consume a one-shot private credential file; returned strings belong to caller. */
int scutclient_load_credentials_file(const char *path, char **username, char **password);

#endif
