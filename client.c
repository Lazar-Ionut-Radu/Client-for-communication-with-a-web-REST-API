#include <stdio.h>      /* printf, sprintf */
#include <ctype.h>      /* isspace */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "client.h"
#include "parson.h"

void empty_stdin (void)
{
    int c = getchar();

    while (c != '\n' && c != EOF)
        c = getchar();
}

void strtrim(char *str)
{
    int i;
    int begin = 0;
    int end = strlen(str) - 1;

    // Find the first index in the array that is not a leading white space.
    while (isspace((unsigned char) str[begin]))
        begin++;

    // Find the last index in the array that is not a trailing white space.
    while ((end >= begin) && isspace((unsigned char) str[end]))
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0'; // Null terminate string.
}

char *get_user_json_string(char *username, char *password) {
    // Create the JSON Object for the authentication information.
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    return json_serialize_to_string(json_value);
}

char *get_book_json_string(char *title, char *author, char *genre, int page_count, char *publisher) {
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "title", title);
    json_object_set_string(json_object, "author", author);
    json_object_set_string(json_object, "genre", genre);
    char page_count_str[MAX_STRING_LEN];
    sprintf(page_count_str, "%d", page_count);
    json_object_set_string(json_object, "page_count", page_count_str);
    json_object_set_string(json_object, "publisher", publisher);

    return json_serialize_to_string(json_value);
}

char *receive_post_request(int sockfd, char host_ip[16], char *url, char *body[1], char *JWTtoken) {
    char *sent_message;
    sent_message = compute_post_request(host_ip, url, "application/json", body, 1, NULL, 0, JWTtoken);

    send_to_server(sockfd, sent_message);

    char *received_message;
    received_message = receive_from_server(sockfd);

    return received_message;
}

char *receive_get_request(int sockfd, char host_ip[16], char *url, char *JWTtoken, char *cookies[1]) {
    char *sent_message;
    sent_message = compute_get_request(host_ip, url, NULL, cookies, 1, JWTtoken);

    send_to_server(sockfd, sent_message);

    char *received_message;
    received_message = receive_from_server(sockfd);

    return received_message;
}

char *receive_delete_request(int sockfd, char host_ip[16], char *url, char *JWTtoken, char *cookies[1]) {

    char *sent_message;
    sent_message = compute_delete_request(host_ip, url, NULL, cookies, 1, JWTtoken);

    send_to_server(sockfd, sent_message);
    
    char *received_message;
    received_message = receive_from_server(sockfd);

    return received_message;
}

int main(int argc, char *argv[])
{
    // Server info.
    char host_ip[16] = "34.254.242.81";
    int port = 8080;

    // Strings params for the following commands.
    char *command = malloc(MAX_STRING_LEN);
    char *username = malloc(MAX_STRING_LEN);
    char *password = malloc(MAX_STRING_LEN); 
    char *book_title = malloc(MAX_STRING_LEN);
    char *book_author = malloc(MAX_STRING_LEN);
    char *book_genre = malloc(MAX_STRING_LEN);
    char *book_publisher = malloc(MAX_STRING_LEN);

    // Non string params.
    int book_id;
    int book_page_count;
    
    // "Boolean" values for managing acces.
    int is_logged_in = 0;
    int has_entered_library = 0;

    // More params.
    char *user_json_string[1];
    char *book_json_string[1];
    char *session_cookie[1];
    char cookie[MAX_STRING_LEN];
    char *JWT_token = malloc(MAX_STRING_LEN);
    
    // Wait for input until prompted to stop the execution.
    while (1) {
        // Read the input command.
        fgets(command, MAX_STRING_LEN, stdin);

        // The exit command.
        if (strcmp(command, "exit\n") == 0) {
            // Break out of the while, return out of main.
            break;
        }

        // The register command.
        if (strcmp(command, "register\n") == 0) {
            // Parameters:
            do {
                printf("username=");
                scanf("%[^\n]s", username);
                strtrim(username);
                empty_stdin();

                // Error msg, wrong format.
                if (!strlen(username))
                    printf("> The username must contain characters other than white space.\n");
            
            } while (!strlen(username));

            do {
                printf("password=");
                scanf("%[^\n]s", password);
                strtrim(password);
                empty_stdin();

                // Error msg, wrong format.
                if (!strlen(password))
                    printf("> The password must contain characters other than white space.\n");
            } while (!strlen(password));
            
            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Get the server response.
            user_json_string[0] = get_user_json_string(username, password);
            char *server_response = receive_post_request(sockfd, host_ip, URL_REGISTER, user_json_string, NULL);

            // Verify if the user is already registered.
            if (strstr(server_response, "is taken"))
                printf("> Username already in use.\n");
            else
                printf("> User registered succesfully.\n");
            
            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The login command.
        if (strcmp(command, "login\n") == 0) {
            // Parameters:
            do {
                printf("username=");
                scanf("%[^\n]s", username);
                strtrim(username);
                empty_stdin();
                // Error msg, wrong format.
                if (!strlen(username))
                    printf("The username must contain characters other than white space.\n");
            } while (!strlen(username));

            do {
                printf("password=");
                scanf("%[^\n]s", password);
                strtrim(password);
                empty_stdin();
                
                // Error msg, wrong format.
                if (!strlen(password))
                    printf("The password must contain characters other than white space.\n");
            } while (!strlen(password));

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Get the server response.
            user_json_string[0] = get_user_json_string(username, password);
            char *server_response = receive_post_request(sockfd, host_ip, URL_LOGIN, user_json_string, NULL);

            // Get the session cookies.
            char *session_cookie_ptr = strstr(server_response, "Set-Cookie:");

            // Check if the login was succesful.
            if (!session_cookie_ptr) {
                printf("> Login failed.\n");
                is_logged_in = 0;
                has_entered_library = 0;

                goto login_out;
            }
            
            strtok(session_cookie_ptr, ";");
            session_cookie_ptr += 12;
            strcpy(cookie, session_cookie_ptr);
            session_cookie[0] = session_cookie_ptr;

            // Print succes message.
            printf("> User logged in succesfully.\n");
            is_logged_in = 1;
            has_entered_library = 0;

login_out:
            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The logout command.
        if (strcmp(command, "logout\n") == 0) {
            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("> Cannot logout if no user is logged in.\n");
                continue;
            }

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Send to the server.
            receive_get_request(sockfd, host_ip, URL_LOGOUT, JWT_token, session_cookie);
            is_logged_in = 0;
            has_entered_library = 0;

            printf("> User logged out.\n");
            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The enter library command.
        if (strcmp(command, "enter_library\n") == 0) {
            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("> Cannot enter the library if no user is logged in.\n");
                continue;
            }
            
            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Send to the server.
            char *received = receive_get_request(sockfd, host_ip, URL_LIBRARY_ACCESS, JWT_token, session_cookie);
            has_entered_library = 1;

            // Error.
            if (!received) {
                printf("> Cannot access the library.\n");
                has_entered_library = 0;
            
                goto enter_library_out;
            }

            // Modify the token.
            memset(JWT_token, 0, MAX_STRING_LEN);
            strcpy(JWT_token, strstr(received, "token"));
            JWT_token += 8;
            JWT_token[strlen(JWT_token) - 2] = '\0';

            printf("> Accessed succesfully.\n");

enter_library_out:
            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The get books command.
        if (strcmp(command, "get_books\n") == 0) {
            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("> Cannot access the books if no user is logged in.\n");
                continue;
            }

            // Error message if the user has not entered the library.
            if (!has_entered_library) {
                printf("> Cannot access the books if the user does not have access to the library.\n");
                continue;
            }

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            char *received = receive_get_request(sockfd, host_ip, URL_BOOK, JWT_token, session_cookie);
            printf("%s\n", strstr(received, "["));

            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The get book command.
        if (strcmp(command, "get_book\n") == 0) {
            // Parameter.
            printf("id=");
            scanf("%d", &book_id);
            empty_stdin();

            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("> Cannot access the book if no user is logged in.\n");
                continue;
            }

            // Error message if the user has not entered the library.
            if (!has_entered_library) {
                printf("> Cannot access the book if the user does not have access to the library.\n");
                continue;
            }

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Send to server.
            char url[MAX_STRING_LEN];
            sprintf(url, "%s/%d", URL_BOOK, book_id);
            char *received = receive_get_request(sockfd, host_ip, url, JWT_token, session_cookie);
            
            // Error
            if (strstr(received, "No book was found"))
                printf("> No book found with that id.\n");
            else // Print result.
                printf("%s\n", strstr(received, "{"));

            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The add book command.
        if (strcmp(command, "add_book\n") == 0) {
            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("Cannot add a book if no user is logged in.\n");
                continue;
            }

            // Error message if the user has not entered the library.
            if (!has_entered_library) {
                printf("Cannot add a book if the user does not have access to the library.\n");
                continue;
            }

            // Parameters.
            do {
                printf("title=");
                scanf("%[^\n]s", book_title);
                strtrim(book_title);
                empty_stdin();

                // Error msg, wrong format.
                if (strlen(book_title) == 0)
                    printf("The title must contain characters other than white space.\n");
            } while(strlen(book_title) == 0);

            do {
                printf("author=");
                scanf("%[^\n]s", book_author);
                strtrim(book_author);
                empty_stdin();
                // Error msg, wrong format.
                if (!strlen(book_author))
                    printf("The author must contain characters other than white space.\n");
            } while(!strlen(book_author));

            do {
                printf("genre=");
                scanf("%[^\n]s", book_genre);
                strtrim(book_genre);
                empty_stdin();

                // Error msg, wrong format.
                if (!strlen(book_genre))
                    printf("The genre must contain characters other than white space.\n");
            } while(!strlen(book_genre));

            do {
                printf("publisher=");
                scanf("%[^\n]s", book_publisher);
                strtrim(book_publisher);
                empty_stdin();

                // Error msg, wrong format.
                if (!strlen(book_publisher))
                    printf("The publisher must contain characters other than white space.\n");
            } while (!strlen(book_publisher));

            printf("page_count=");
            scanf("%d", &book_page_count);
            empty_stdin();

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            book_json_string[0] = get_book_json_string(book_title, book_author, book_genre, book_page_count, book_publisher);
            receive_post_request(sockfd, host_ip, URL_BOOK, book_json_string, JWT_token);

            printf("> Book added successfully.\n");

            // Close the connection.
            close_connection(sockfd);

            continue;
        }

        // The delete book command.
        if (strcmp(command, "delete_book\n") == 0) {
            // Parameter.
            printf("id=");
            scanf("%d", &book_id);
            empty_stdin();

            // Error message if the user is not logged in.
            if (!is_logged_in) {
                printf("> Cannot delete a book if no user is logged in.\n");
                continue;
            }

            // Error message if the user has not entered the library.
            if (!has_entered_library) {
                printf("> Cannot delete a book if the user does not have access to the library.\n");
                continue;
            }

            // Open a connection to the server.
            int sockfd = open_connection(host_ip, port, AF_INET, SOCK_STREAM, 0);

            // Send to server.
            char url[MAX_STRING_LEN];
            sprintf(url, "%s/%d", URL_BOOK, book_id);
            char *received = receive_delete_request(sockfd, host_ip, url, JWT_token, session_cookie);
            
            printf("> Book deleted succesfully.\n");
            empty_stdin();

            // Close the connection.
            close_connection(sockfd);
            
            continue;
        }
    
        // Wrong command error message.
        printf("> Unknown command.\n");
    }

    // Free all the memory used throughout.
    if (command)
        free(command);
    if (username)
        free(username);
    if (password)
        free(password);
    if (book_title)
        free(book_title);
    if (book_author)
        free(book_author);
    if (book_genre)
        free(book_genre);
    if (book_publisher)
        free(book_publisher);

    return 0;
}
