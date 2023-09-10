#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<netinet/if_ether.h>
#include<arpa/inet.h>
#include<unistd.h>

int main(){
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(raw_socket < 0){
        printf("sudo needed!!\n");
        printf("Error in opening a raw socket\n");
        return 1;
    }

    unsigned char *buffer = (unsigned char *) malloc(65536); //to receive data
    memset(buffer, 0, 65536);
    
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);
    
    int packet_count = 0;
    while(true){
        //Receive a network packet and copy in to buffer
	
        int buflen = recvfrom(raw_socket,buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);
        if(buflen < 0){
            printf("Error in reading recvfrom function\n");
            return 1;
        }
        
        struct ethhdr *eth = (struct ethhdr *)(buffer);

        unsigned short iphdrlen;
        struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));

        struct sockaddr_in source, dest;
        memset(&source  , 0, sizeof(source));
        memset(&dest    , 0, sizeof(dest));
        source.sin_addr.s_addr  = ip -> saddr;
        dest.sin_addr.s_addr    = ip -> daddr;

        printf("IP info:\n");
        printf("\t|-Source IP : %s\n     ", inet_ntoa(source.sin_addr));
        printf("\t|-Destination IP : %s\n", inet_ntoa(dest.sin_addr));

        //Extracting the Port info:
        iphdrlen = (ip -> ihl) * 4;
        struct tcphdr *tcp=(struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
	
	printf("Extracting TCP Port info:\n");
        printf("\t|-Source Port : %d\n     " , ntohs(tcp -> source));
        printf("\t|-Destination Port : %d\n" , ntohs(tcp -> dest));
	printf("\t|-Number of packets: %d\n", packet_count);
	
	packet_count++;
    }

    free(buffer);
    close(raw_socket);
}
