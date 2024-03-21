#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>

#define PORT 8037
#define time_since_january 2208988800L

int main() {
    // Create socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // skapar socket för UDP kommunikation
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Sätter till lokal värd

    // Send request to server, tom dataram 
    sendto(client_socket, NULL, 0, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    // Receive time from server
    uint32_t net_time;
    struct sockaddr_in from_addr;
    socklen_t from_addrlen = sizeof(from_addr);
    ssize_t bytes_received = recvfrom(client_socket, (char *)&net_time, sizeof(net_time), 0, (struct sockaddr *)&from_addr, &from_addrlen); // tar emot tiden 
    if (bytes_received < 0) {
        perror("Error receiving data");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Convert time to host byte order
    uint32_t host_time = ntohl(net_time);

    // Convert RFC868 time to time_t
    time_t current_time = (time_t)(host_time - time_since_january); // justerar tiden från servern

    printf("Time from server: %s", ctime(&current_time)); // skriver ut i läsbart format

    // Close socket
    close(client_socket);

    return 0;
}
