
# Client for communication with a web REST API

This project is the 4th homework for the Communication Protocols course at University Polithenica of Bucharest. My implementation is based on the 9th lab.

It creates a small command line interface that connects to a web server (provided by the course team) that stores a list of books for each user. The available commands are:

* register: Registers a new user.
* login: Logs in a user.
* logout: Logs out a user.
* exit: Closes the program.
* enter_library: Must be used before the following commands.
* add_book: adds a book to that users library.
* delete_book: deletes a book from a users library.
* get_books: Returns a list of books (title and id).
* get_book: Returns a book when given its id.


## Directory structure

```bash
├── client.c
├── client.h
├── helpers.c
├── helpers.h
├── Makefile
├── parson.c
├── parson.h
├── README.md
├── requests.c
├── requests.h
└── utils.h
```

The buffer.* helpers.* utils.h files were provided by the courses team in the 9th lab, while the requests.* files were for us to complete during that lab.

The parson.* files contain a json parsing library found online.

The makefile contains compiling instructions.

The readme is this one =).

## Implementation

My implementation resides inside the client.* and requests.* files. It uses the following functions:

1) Helper functions:

    1.1) empty_stdin(): Clear the stdin buffer.

    1.2) strtrim(char *str): Removes the trailling and leading from a string. Used while reading the input of the commands.

    1.3) get_user_json_string(...): Returns a string of the json formatting of the credentials of a user.

    1.4) get_book_json_string(...): Returns a string of the json formatting of a book.

    1.5) receive_*_request(...): Wrappers for the functions inside requests.c

2) Core functions:

    2.1) compute_get_request(...): Returns a string containing the contents of a HTTP GET REQUEST.

    2.2) compute_post_request(...): Returns a string containing the contents of a HTTP POST REQUEST.

    2.3) compute_delete_request(...): Returns a string containing the contents of a HTTP DELETE REQUEST.

3) The main function.

Is basically a huge switch parsing each command. It keeps that of the session cookie returned by the server when logging in and of the JWT Token that grants access to the library of books. 

Errors covered:
    
    - Cannot register a user if the username is taken.

    - Cannot login if the credentials are wrong.

    - Cannot logout if the user is not logged in.

    - Cannot enter the library / add a book / remove a book / display the books if nobody is logged in.

    - Cannot add / remove / display a book if the user has not entered the library prior (no JWT token for accessing the library)

    - Cannot display / remove a book if no book has the specified id.

    
## Compilation
To compile the project run the command:

```bash
make
```

And to run it simply run the command:
```bash
./client
```
## Authors

- [Lazar Ionut-Radu (323CA)](https://github.com/Lazar-Ionut-Radu)
