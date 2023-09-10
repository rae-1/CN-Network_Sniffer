#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<netinet/if_ether.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdbool.h>



void patternSearch(int remaining_data, int payload_size, char pattern[], unsigned char payload_in_ascii[], int len)
{
	bool flag = false;
	
	for(int i = 0; i < payload_size; i++){
		if(payload_in_ascii[i] == pattern[0]){
			int j = i;
			int k = 0;
			while(k < len && j < payload_size && payload_in_ascii[j] == pattern[k]){
				j++;
				k++;
			}
			if(k > len - 1){
				flag = true;
				break;
			}
		}
		if(flag == true){
			break;
		}
	}
	if(flag == true){
		for(int i = 0; i < payload_size; i++){
			printf("%c", payload_in_ascii[i]);
	}
	printf("\n");
	}
}


int main(){
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(raw_socket < 0){
        printf("sudo needed or ");
        printf("Error in opening a raw socket\n");
        return 1;
    }

    unsigned char *buffer = (unsigned char *) malloc(65536);
    memset(buffer, 0, 65536);
    
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);
    

    FILE *payload_dump = fopen("payload_dump.txt", "w");
    if(payload_dump == NULL){
	printf("Error in opening the file");
	return 1;
    }
    

    while(true){
        int buflen = recvfrom(raw_socket,buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);
        if(buflen < 0){
            printf("Error in reading recvfrom function\n");
            return 1;
        }

		struct ethhdr *eth = (struct ethhdr *)(buffer);

		unsigned short iphdrlen;
        struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
        iphdrlen = (ip -> ihl) * 4;
        struct tcphdr *tcp=(struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));

		unsigned char * data = (buffer + iphdrlen + sizeof(struct tcphdr));
		int remaining_data = buflen - (iphdrlen + sizeof(struct tcphdr));
	
    	int payload_size = 0;
    	int packet_count = 0;
		int tcphdrlen = tcp->doff * 4;
		int payload_offset = iphdrlen + tcphdrlen;
        payload_size = buflen - payload_offset;
        
		if (payload_size > 0)
		{
			unsigned char payload[payload_size + 1];
			memcpy(payload, buffer + payload_offset, payload_size);

			unsigned char payload_in_ascii[payload_size + 1];
			for (int i = 0; i < payload_size; i++)
			{
				payload_in_ascii[i] = (char)payload[i];
			}
	    
			//updates:
			if(ntohs(tcp -> th_sum) == 0xf436)
			{
				printf("TCP checksum == 0xf436:\n");
				for(int i = 0; i < payload_size; i++)
				{
					printf("%c", payload_in_ascii[i]);
				}
				printf("\n");
            }
            
            
            
			char pattern1[] = {'F', 'l', 'a', 'g', ':'};	    
			char pattern2[] = {'f','l','a','v','o','r','-'};
			char pattern3[] = {'S', 'e', 'c', 'r', 'e', 't', ':'};
			int len1 = sizeof(pattern1) / sizeof(pattern1[0]);
			int len2 = sizeof(pattern2) / sizeof(pattern2[0]);
			int len3 = sizeof(pattern3) / sizeof(pattern3[0]);
			patternSearch(remaining_data, payload_size, pattern1, payload_in_ascii, len1);
			patternSearch(remaining_data, payload_size, pattern2, payload_in_ascii, len2);
			patternSearch(remaining_data, payload_size, pattern3, payload_in_ascii, len3);
	    
		}
	
		for(int i = 0; i < remaining_data; i++)
		{
			fprintf(payload_dump, "%c", data[i]);
		}
    }

    free(buffer);
    close(raw_socket);
    fclose(payload_dump);
}
