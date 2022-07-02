#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char *token=NULL;
    char *ck=NULL;
    char **cookies;
    cookies = malloc(sizeof(char*));
    cookies[0]=NULL;
    char *user[1],*book[1],*login[1];
    int sockfd,sockfd_get;
    char buffer[100];
    char username[100],password[100];
    char *check;
    //Initialize one socket for each type of request(GET and POST)
    sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

    int exit=1;
    while(exit){
        //Get the command from STDIN
        fgets(buffer,100,stdin);
        //For 'exit', the client stops
        if(strncmp(buffer,"exit",4)==0){
            exit=0;
            break;
        }
        /*For "register", get the username and password*/
        if(strncmp(buffer,"register",8)==0){
            printf("username=");
            fgets(username,100,stdin);
            //Parse the input
            username[strcspn(username,"\n")]=0;
            username[strlen(username)]='\0';
            printf("password=");
            fgets(password,100,stdin);
            password[strcspn(password,"\n")]=0;
            user[0] = get_user_details(username,password);
            password[strlen(password)]='\0';
            //Build the POST request
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", user, 1,NULL ,NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //Check the server's response
            check = strstr(response,"is taken!");
            if(check!=NULL){
                printf("Username already in use.\n");
                continue;
            }
            else{
                printf("Registration was successful.\n");
            }
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
        }
        /*For "login", get the username and password*/
        if(strncmp(buffer,"login",5)==0){
            if(cookies[0]!=NULL){
                printf("Already logged in.\n");
                continue;
            }
            check=NULL;
            printf("username=");
            fgets(username,100,stdin);
            //Parse the input
            username[strcspn(username,"\n")]=0;
            username[strlen(username)]='\0';
            printf("password=");
            fgets(password,100,stdin);
            password[strcspn(password,"\n")]=0;
            password[strlen(password)]='\0';
            //Build the JSON object with credentials and the POST request
            login[0] = get_user_details(username,password);
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json",login, 1,NULL,NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            //Check the server's response
            check=strstr(response,"Credentials are not good!");
            if(check!=NULL){
                printf("Credentials do not match.\n");
                continue;
            }
            else{
                printf("Login was successful.\n");
            }
            //Get the cookie from the response message
            ck=malloc(102400*sizeof(char));
            get_cookies(ck,response);
            cookies[0] = malloc(102400*sizeof(char));
            strcpy(cookies[0],ck);
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        }
        /*For 'enter_library', check if the client is logged in
        and build the GET request*/
        if(strncmp(buffer,"enter_library",13)==0){
            check=NULL;
            //Check if the client is logged in
            if(cookies[0]!=NULL){
                message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access",NULL ,NULL, cookies,1);
                send_to_server(sockfd_get, message);
                response = receive_from_server(sockfd_get);
                check=strstr(response,"You are not logged in!");
                if(check!=NULL){
                    printf("You are not logged in.\n");
                    continue;
                }
                else{
                    printf("You entered the library.\n");
                }
                //Get the access token from the response message
                char *mes=strstr(response,"token");
                token=get_token(mes);
                sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            }
            else
            {
                printf("You are not logged in.\n");
                continue;
            }
        }
        /*For 'get_books', check if the user is logged in and
        has access to the library*/
        if(strncmp(buffer,"get_books",9)==0){
            check=NULL;
            if(cookies[0]!=NULL || token!=NULL){
                message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books",NULL,token,cookies,1);
                send_to_server(sockfd_get, message);
                response = receive_from_server(sockfd_get);
                check=strstr(response,"Authorization header is missing!");
                if(check!=NULL){
                    printf("You are not authorized.\n");
                    continue;
                }
                //Show all book data
                check=strchr(response,'[');
                printf("%s\n",check);
                sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                continue;
            }
            else{
                printf("You don't have authorization.\n");
                continue;
            }
        }
        /*For 'get_book', check if the user is logged in and
        has access to the library, then get the book id*/
        if(strncmp(buffer,"get_book",8)==0){
            check=NULL;
            if(cookies[0]!=NULL || token!=NULL){
                printf("id=");
                char id[10];
                fgets(id,10,stdin);
                id[strcspn(id,"\n")]=0;
                id[strlen(id)]='\0';
                //Check if the id is a number
                int id_int=atoi(id);
                if(id_int==0){
                    printf("Invalid id.\n");
                    continue;
                }
                char url[100];
                strcpy(url,"/api/v1/tema/library/books/");
                strcat(url,id);
                //Send the request
                message = compute_get_request("34.241.4.235", url,NULL,token,cookies, 1);
                send_to_server(sockfd_get, message);
                response = receive_from_server(sockfd_get);
                if(check!=NULL){
                    printf("You are not authorized.\n");
                    continue;
                }
                //Show the book info
                check=strchr(response,'[');
                printf("%s\n",check);
                sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                continue;
            }else{
                printf("You don't have authorization.\n");
                continue;
            }
        }
        /*For 'add_book', check if the user is logged in and
        has access to the library, then get the book data*/
        if(strncmp(buffer,"add_book",8)==0){
            int check_int;
            if(cookies[0]!=NULL || token!=NULL){
                //Get the book data
                char title[100],author[100],genre[100],publisher[100],page_count[100];
                printf("title=");
                fgets(title,100,stdin);
                title[strcspn(title,"\n")]=0;
                check_int = atoi(title);
                if(check_int!=0){
                    printf("Invalid input.\n");
                    continue;
                }
                printf("author=");
                fgets(author,100,stdin);
                author[strcspn(author,"\n")]=0;
                check_int = atoi(author);
                if(check_int!=0){
                    printf("Invalid input.\n");
                    continue;
                }
                printf("genre=");
                fgets(genre,100,stdin);
                genre[strcspn(genre,"\n")]=0;
                check_int = atoi(genre);
                if(check_int!=0){
                    printf("Invalid input.\n");
                    continue;
                }
                printf("publisher=");
                fgets(publisher,100,stdin);
                publisher[strcspn(publisher,"\n")]=0;
                check_int = atoi(publisher);
                if(check_int!=0){
                    printf("Invalid input.\n");
                    continue;
                }
                printf("page_count=");
                fgets(page_count,100,stdin);
                int count=atoi(page_count);
                if(count==0){
                    printf("Invalid page count.\n");
                    continue;
                }
                //Form the JSON object with book data and build the POST request
                book[0]=get_book_json(title,author,publisher,genre,count);
                message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json",book,1,token,cookies,1);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                check = strstr(response,"Authorization header is missing!");
                if(check!=NULL){
                    printf("You are not authorized.\n");
                    continue;
                }else{
                    printf("Book added.\n");
                    continue;
            }
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            }else{
                printf("You don't have authorization.\n");
                continue;
            }
        }
        /*For 'delete_book', check if the user is logged in and
        has access to the library, then get the book id*/
        if(strncmp(buffer,"delete_book",11)==0){
            if(cookies[0]!=NULL || token!=NULL){
                char id[100];
                printf("id=");
                fgets(id,100,stdin);
                id[strcspn(id,"\n")]=0;
                int id_int=atoi(id);
                //Check if id is valid
                if(id_int==0){
                    printf("Invalid id.\n");
                    continue;
                }
                char dir[100] = "/api/v1/tema/library/books/";
                strcat(dir,id);
                //Send a DELETE request
                message = compute_delete_request("34.241.4.235",dir,NULL,token, cookies,1);
                send_to_server(sockfd_get, message);
                response = receive_from_server(sockfd_get);
                check = strstr(response,"Authorization header is missing!");
                if(check!=NULL){
                    printf("You are not authorized.\n");
                    continue;
                }else{
                    printf("Book deleted.\n");
                    continue;
            }
                sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            }else{
                printf("You don't have authorization.\n");
                continue;
            }
        }
        /*For 'logout', check if client is logged in
        and build the GET request*/
        if(strncmp(buffer,"logout",6)==0){
            if(cookies[0]!=NULL || token!=NULL){
                message = compute_get_request("34.241.4.235","/api/v1/tema/auth/logout",NULL,token,cookies,1);
                send_to_server(sockfd_get, message);
                response = receive_from_server(sockfd_get);
                check=strstr(response,"You are not logged in!");
                if(check!=NULL){
                    printf("You are not logged in.\n");
                    continue;
                }
                else{
                    printf("You logged out.\n");
                }
                //Delete the access token and cookie
                free(cookies[0]);
                cookies[0]=NULL;
                free(token);
                token=NULL;
                sockfd_get = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            }else{
                printf("You are not logged in.\n");
                continue;
            }
        }

    }

    return 0;
}
