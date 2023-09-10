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
#include<time.h>

struct port_to_pid{
	int port_number;
	int pid;
};

int getID(int client_port_number)
{
    char command[256];
    snprintf(command, sizeof(command), "fuser -n tcp %d", client_port_number);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), fp);
    int process_id;
    
    if (sscanf(line, "%d", &process_id) != 1) {
     	process_id = -1;
    }

    pclose(fp);
    return process_id;
}



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
    
    
    struct port_to_pid mapping[100000];
    for(int i = 0; i < 100000; i++){
    	mapping[i].port_number = -1;
    	mapping[i].pid = -1;
    }
    
    time_t start_time = time(NULL);
    int duration = 10;
    int count = 0;
    while(time(NULL) - start_time <= duration)
    {
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

        iphdrlen = (ip -> ihl) * 4;
        struct tcphdr *tcp=(struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
        
        int port = htons(tcp -> source);
        int pid  = getID(port);
        printf("Source Port : %d, and PID : %d\n" , port, pid);
	
	mapping[count].port_number = port;
	mapping[count].pid = pid;
	count++;

    }


	while(true){
		int port;
		printf("Enter a port number: ");
		scanf("%d", &port);
		
		bool found = false;
		for(int i = 0; i < 100000; i++){
			if(mapping[i].port_number == port && mapping[i].port_number != -1){
				printf("Match found, the corresponding PID is %d\n", mapping[i].pid);
				found = true;
				break;
			}
		}
		
		if(found == false){
			printf("No match found");
		}
	}


    free(buffer);
    close(raw_socket);
}
