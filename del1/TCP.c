#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

void error(int client_sd) {
    const char *response_404 = "HTTP/1.1 404 Not Found\r\nServer: Demo Web Server\r\n\r\nFile not found";
    write(client_sd, response_404, strlen(response_404));
}

void SendContentType(const char *type, int socket) {
    char response[BUFFER_SIZE];
    if (strcmp(type, "jpg") == 0) {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nServer: Demo Web Server\r\nContent-Type: image/jpeg\r\n\r\n");
    } else if (strcmp(type, "html") == 0) {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nServer: Demo Web Server\r\nContent-Type: text/html\r\n\r\n");
    } else {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nServer: Demo Web Server\r\n\r\n");
    }

    write(socket, response, strlen(response));
}

void process_http_request(int client_sd, const char *filename, const char *content_type) { // hanterar http begäran ifrån klient
    FILE *file = fopen(filename, "rb"); // öppnar den begärda filen som tagit emot av klienten. Öppnas i läsläge

    if (file != NULL) {
        SendContentType(content_type, client_sd); // Anger rätt innehållstyper

        char buffer[BUFFER_SIZE];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) { 
            write(client_sd, buffer, bytes_read); // hela filen skickas
        }

        fclose(file);
    } else {
        error(client_sd);
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;// håller koll på klientens och serverns adresser
    int server_socket, client_socket; 
    socklen_t client_addrlen = sizeof(client_addr); // storleken på klientens adress

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // skapar socket, med IPV4, TCP och IP nivå
    if (server_socket < 0) { // kontroll om socket skapats
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in)); //säkerställer så att alla fält i strukturen är korrekt inställda innan de används
    server_addr.sin_family = AF_INET; //IPV4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT); //konvertera till network byte order

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) { // binder servers socket till specifika adressen och porten
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, SOMAXCONN) < 0) { // lyssnar på serverns socket för inkommande anslutningar från klienter
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    while (1) { // inkommande anslutningar
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addrlen);// accepterar inkommande anslutning och skapar specifik anslutnings socket
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        char buffer[BUFFER_SIZE]; // vi lagrar HTTP begäran
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE); // läser av HTTP-begäaran från klienten till bufferten
        if (bytes_read < 0) { // kontroll om läsningen lyckades
            perror("Fel vid läsning från socket");
            close(client_socket);
            continue;
        } else if (bytes_read == 0) {
            close(client_socket);
            continue;
        }

        buffer[bytes_read] = '\0';

        char *token = strtok(buffer, " "); // filnamnet, extraherar filnamn med filtyp
        token = strtok(NULL, " ");
        if (token) { //kolalr om filnamnet och filtyp har extraherats på begäran
            memmove(token, token + 1, strlen(token)); // tar bort / från filnamnet

            char temp[BUFFER_SIZE]; 
            strcpy(temp, token); // kopierar filnamnet för att undvika att token ändras

            char *content_type = "text/html";
            char *ContentToken = strtok(temp, ".");
            ContentToken = strtok(NULL, " "); // Delar upp filnamnet för att extrahera filtypen
            if (ContentToken) {
                process_http_request(client_socket, token, ContentToken); // om filtypen har extraherats så skickas http_request annars error
            } else {
                error(client_socket);
            }
        }

        close(client_socket);
    }
    close(server_socket);

    return 0;
}
