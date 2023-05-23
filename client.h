#ifndef CLIENT_H
#define CLIENT_H

#define MAX_STRING_LEN 1024

#define URL_REGISTER       "/api/v1/tema/auth/register"
#define URL_LOGIN          "/api/v1/tema/auth/login"
#define URL_LOGOUT         "/api/v1/tema/auth/logout"
#define URL_BOOK           "/api/v1/tema/library/books"
#define URL_LIBRARY_ACCESS "/api/v1/tema/library/access"

// Simple helper-function to empty stdin (for reading inside loops).
void empty_stdin (void);

// Removes leading & trailing white spaces from a string.
void strtrim(char *str);

// Returns a json string for a user's credentials.
char *get_user_json_string(char *username, char *password);

// Returns a json string of a book.
char *get_book_json_string(char *title, char *author, char *genre, int page_count, char *publisher);

// Wrapper functions for communication with the server. Returns the message from the server.
char *receive_post_request(int sockfd, char host_ip[16], char *url, char *body[1], char *JWTtoken);
char *receive_get_request(int sockfd, char host_ip[16], char *url, char *JWTtoken, char *cookies[1]);
char *receive_delete_request(int sockfd, char host_ip[16], char *url, char *JWTtoken, char *cookies[1]);

#endif // CLIENT_H
