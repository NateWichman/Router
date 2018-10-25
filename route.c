#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <net/if.h>
//#include <linux/ip.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


int main(){

  //Structs citation: www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/
  
  /*Struct for an Ethernet Header, used with <linux/if_ether.h> */
  struct eth_hdr{
	unsigned char dmac[6];//Destination MAK address
	unsigned char smac[6];//Source MAK address
	uint16_t ethertype;//Type or length, if >= 1536, its the type, otherwise length
	unsigned char payload[];//ARP packet, IPv4 Packet, ICMP packet, etc: Packet Contents
  }__attribute__((packed)); //Size of struct is now equal to the sum of its members

  /*Struct for ARP Header */
  struct arp_hdr{
	uint16_t hwtype; //Link layer type: Ethernet = 0x0001
	uint16_t protype; //Protocol type: IPv4 = 0x0800
	unsigned char hwsize; //Size of MAK: 6 bytes
	unsigned char prosize; //Size of IP: 4 bytes
	uint16_t opcode; //Type of ARP: 1 = request, 2 = reply
	//unsigned char data[]; //Payload of IPv4 specific info
	unsigned char smac[6]; //6 byte source MAK address
	uint32_t sip; //Sender's IP address
	unsigned char dmac[6]; //Destinations MAK address
	uint32_t dip; //Destination's IP address
  }__attribute__((packed));

  //File Desciptors for the different ports on the routers
  int eth0_sock;
  int eth1_sock;
  int eth2_sock;
  int eth3_sock;

  //List of sockets 
  fd_set socks;
  FD_ZERO(&socks);

  

  int packet_socket;
  //get list of interface addresses. This is a linked list. Next
  //pointer is in ifa_next, interface name is in ifa_name, address is
  //in ifa_addr. You will have multiple entries in the list with the
  //same name, if the same interface has multiple addresses. This is
  //common since most interfaces will have a MAC, IPv4, and IPv6
  //address. You can use the names to match up which IPv4 address goes
  //with which MAC address.
  struct ifaddrs *ifaddr, *tmp;
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
	//create a packet socket
	//AF_PACKET makes it a packet socket
	//SOCK_RAW makes it so we get the entire packet
	//could also use SOCK_DGRAM to cut off link layer header
	//ETH_P_ALL indicates we want all (upper layer) protocols
	//we could specify just a specific one
	eth1_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth1_sock<0){
	  perror("socket");
	  return 2;
	}
	//Bind the socket to the address, so we only get packets
	//recieved on this specific interface. For packet sockets, the
	//address structure is a struct sockaddr_ll (see the man page
	//for "packet"), but of course bind takes a struct sockaddr.
	//Here, we can use the sockaddr we got from getifaddrs (which
	//we could convert to sockaddr_ll if we needed to)
	if(bind(eth1_sock,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
	  if(!strncmp(&(tmp->ifa_name[3]),"eth0",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
	eth0_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth0_sock<0){
	  perror("socket");
	  return 2;
	}
	if(bind(eth0_sock,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
	  if(!strncmp(&(tmp->ifa_name[3]),"eth2",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
	eth2_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth2_sock<0){
	  perror("socket");
	  return 2;
	}
	if(bind(eth2_sock,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
	  if(!strncmp(&(tmp->ifa_name[3]),"eth3",4)){
	printf("Creating Socket on interface %s\n",tmp->ifa_name);
	eth3_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(eth3_sock<0){
	  perror("socket");
	  return 2;
	}
	if(bind(eth3_sock,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  perror("bind");
	}
      }
    }
  }
  //loop and recieve packets. We are only looking at one interface,
  //for the project you will probably want to look at more (to do so,
  //a good way is to have one socket per interface and use select to
  //see which ones have data)

 //Adding sockets to the list
 FD_SET(eth0_sock, &socks);
 FD_SET(eth1_sock, &socks);
 FD_SET(eth2_sock, &socks);
 FD_SET(eth3_sock, &socks);

  //Searching for MAK address
		struct ifreq ifr;
		struct ifconf ifc;
		char makBuf[1024];
		int success = 0;

		int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		if (sock == -1) {printf("ERROR GETTING MAK ADDRESS\n"); return -1;}

		ifc.ifc_len = sizeof(makBuf);
		ifc.ifc_buf = makBuf;
		if(ioctl(sock, SIOCGIFCONF, &ifc) == -1){printf("ERROR MAK ADDR 2\n"); return -1;}

		struct ifreq* it = ifc.ifc_req;
		const struct ifreq* const end = it +(ifc.ifc_len / sizeof(struct ifreq));

		for(; it != end; ++it){
		  strcpy(ifr.ifr_name, it->ifr_name);
		  if(ioctl(sock, SIOCGIFFLAGS, &ifr) == 0){
		    if(! (ifr.ifr_flags & IFF_LOOPBACK)){
		      if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0){
			success = 1;
			break;
		      }
		    }
		  }
		  else{printf("ERROR MAK ADDR 3\n"); return -1;}
		}

		unsigned char mac_address[6];
		if(success) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);

		printf("\nMAK address found on this system:\n");
		printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
		       mac_address[0],
		       mac_address[1],
		       mac_address[2],
		       mac_address[3],
		       mac_address[4],
		       mac_address[5]
		       );

  printf("Ready to recieve now\n");
  while(1){
	int i;
	int length;
	int sock;
	fd_set tempset = socks;
	select(FD_SETSIZE,&tempset,NULL,NULL,NULL);

    char buf[1500];
    struct sockaddr_ll recvaddr;
    int recvaddrlen=sizeof(struct sockaddr_ll);
    //we can use recv, since the addresses are in the packet, but we
    //use recvfrom because it gives us an easy way to determine if
    //this packet is incoming or outgoing (when using ETH_P_ALL, we
    //see packets in both directions. Only outgoing can be seen when
    //using a packet socket with some specific protocol)

	int n;
    //int n = recvfrom(eth1_sock, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);

	for(i = 0; i<FD_SETSIZE;i++){
		if(FD_ISSET(i,&tempset)){
			length = sizeof(tmp);
			n = recvfrom(i, buf, 1500, 0, (struct sockaddr*)&recvaddr, &recvaddrlen);
		
    //ignore outgoing packets (we can't disable some from being sent
    //by the OS automatically, for example ICMP port unreachable
    //messages, so we will just ignore them here)
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
      continue;
    //start processing all others
    printf("Got a %d byte packet\n", n);
    //what else to do is up to you, you can send packets with send,
    //just like we used for TCP sockets (or you can use sendto, but it
    //is not necessary, since the headers, including all addresses,
    //need to be in the buffer you are sending)

   /*buf[0] = 0; //Setting the type of the icmp packet to 0 for reply
   if( send(packet_socket, buf, sizeof(buf) + 1, 0) == -1){
	printf("There was an error sending");
   }*/

   struct eth_hdr *hdr = (struct eth_hdr *)buf; //Initializing an ether header struct with buf's data

   //Printing Ethernet Header Contents
   printf("Ethernet Header Contents:\n");
   printf("Destination MAK address: %02X:%02X:%02X:%02X:%02X:%02x\n",
		   hdr->dmac[0],
		   hdr->dmac[1],
		   hdr->dmac[2],
		   hdr->dmac[3],
		   hdr->dmac[4],
		   hdr->dmac[5]
	 );
   printf("Source MAK address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		   hdr->smac[0],
		   hdr->smac[1],
		   hdr->smac[2],
		   hdr->smac[3],
		   hdr->smac[4],
		   hdr->smac[5]
	);
   printf("Ether Type: %04X\n",ntohs(hdr->ethertype)); //Hexidecimal Ethernet type
   
   //Should be buf - some length fro eth_hdr?
   struct arp_hdr *ahdr = (struct arp_hdr *)&buf[14]; //Initializing a arp header struct
   printf("\nARP Header Contents:\n");
   printf("Hardware Type: %02d\n",ntohs(ahdr->hwtype));
   printf("Protocol Type: %02d\n",ntohs(ahdr->protype));
   printf("Size of Hardware Type: %02X\n", ahdr->hwsize);
   printf("Size of Prototype: %02X\n", ahdr->prosize);
   printf("Opcode (1: request, 2:reply): %02d\n",ntohs(ahdr->opcode));

 //  struct arp_ipv4 *iphdr = (struct arp_ipv4 *)buf;
   printf("\nARP package contents:\n");
   printf("Source MAK address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		   ahdr->smac[0],
		   ahdr->smac[1],
		   ahdr->smac[2],
		   ahdr->smac[3],
		   ahdr->smac[4],
		   ahdr->smac[5]
	 );
   printf("Source IP address: %02d\n",ntohs(ahdr->sip));
   printf("Destination MAK address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		   ahdr->dmac[0],
		   ahdr->dmac[1],
		   ahdr->dmac[2],
		   ahdr->dmac[3],
		   ahdr->dmac[4],
		   ahdr->dmac[5]
	 );
   printf("Destination IP address: %20d\n",ntohs(ahdr->dip));

   printf("\nChecking if its an ARP package: \n");
   if(ntohs(hdr->ethertype) == 0x0806){
	printf("Its an ARP Package\n");

	printf("Checking to see if it is a ARP request\n");
	if(ntohs(ahdr->opcode) == 1){
		printf("It is an ARP request\n");

		printf("Switching ARP request to ARP reply\n");
		ahdr->opcode = htons(2); //ntohs(2) ?

		printf("Swapping IP header's source and destination address'\n");
   
		//Memcopy
              //  strcpy(ahdr->dmac, mac_address);
		memcpy(ahdr->dmac, mac_address, sizeof(mac_address));

		//Switching the ARP source and Destination Addresses	
	      	char tempBuf[6];
		tempBuf[0] = ahdr->smac[0];
		tempBuf[1] = ahdr->smac[1];
		tempBuf[2] = ahdr->smac[2];
		tempBuf[3] = ahdr->smac[3];
		tempBuf[4] = ahdr->smac[4];
		tempBuf[5] = ahdr->smac[5];

		ahdr->smac[0] = ahdr->dmac[0];
		ahdr->smac[1] = ahdr->dmac[1];
		ahdr->smac[2] = ahdr->dmac[2];
		ahdr->smac[3] = ahdr->dmac[3];
		ahdr->smac[4] = ahdr->dmac[4];
		ahdr->smac[5] = ahdr->dmac[5];

		ahdr->dmac[0] = tempBuf[0];
		ahdr->dmac[1] = tempBuf[1];
		ahdr->dmac[2] = tempBuf[2];
		ahdr->dmac[3] = tempBuf[3];
		ahdr->dmac[4] = tempBuf[4];
		ahdr->dmac[5] = tempBuf[5];

		printf("New ARP Addrs:\n");
		printf("Source Mak: %02X:%02X:%02X:%02X:%02X:%02X\n",
				ahdr->smac[0],
				ahdr->smac[1],
				ahdr->smac[2],
				ahdr->smac[3],
				ahdr->smac[4],
				ahdr->smac[5]
		      );
		printf("Destination Mak: %02X:%02X:%02X:%02X:%02X:%02X\n",
				ahdr->dmac[0],
				ahdr->dmac[1],
				ahdr->dmac[2],
				ahdr->dmac[3],
				ahdr->dmac[4],
				ahdr->dmac[5]
		      );


		//Switching and Updating Ethernet Header
		hdr->dmac[0] = hdr->smac[0];
		hdr->dmac[1] = hdr->smac[1];
		hdr->dmac[2] = hdr->smac[2];
		hdr->dmac[3] = hdr->smac[3];
		hdr->dmac[4] = hdr->smac[4];
		hdr->dmac[5] = hdr->smac[5];

		hdr->smac[0] = mac_address[0];
		hdr->smac[1] = mac_address[1];
		hdr->smac[2] = mac_address[2];
		hdr->smac[3] = mac_address[3];
		hdr->smac[4] = mac_address[4];
		hdr->smac[5] = mac_address[5];  

		printf("Done 1\n");

		uint32_t temp2;
		temp2 = ahdr->sip;
		ahdr->sip = ahdr->dip;
		ahdr->dip = temp2;
		printf("Done 2\n");

		printf("\nReturning ARP reply\n");
		send(i, buf, 42,0);//Always 42 */
		
	}else{
		printf("It is not an ARP request\n");
	}
   } 
   //Checking to see if its an IPv4 package
   else if(ntohs(hdr->ethertype) == 0x800){
	printf("ICMP package received\n");
	//Struct for the IP header
	struct iphdr *ipHeader = (struct iphdr *)&buf[14];
	//Is my memory managment right with the sizeof? Or do i need to dereference?
	struct icmphdr* icmp = (struct icmphdr *)&buf[34];
	
	printf("Ip protocol: %d\n" , ipHeader->protocol);
	if(ipHeader->protocol == 1){
		printf("It is an ICMP packet\n");
		
		 printf("Need to change if 8: %d\n", icmp->type);
		if(icmp->type == 8){
		 	printf("It is an ICMP Request\n");
			
			//Changing The ICMP type to 0 for Echo Reply
			icmp->type = htons(0);

			//Changing source address
			uint32_t tempAddress;
			tempAddress = ipHeader->saddr;
			ipHeader->saddr = ipHeader->daddr;
			ipHeader->daddr = tempAddress;

			//Switching and Updating Ethernet Header
		hdr->dmac[0] = hdr->smac[0];
		hdr->dmac[1] = hdr->smac[1];
		hdr->dmac[2] = hdr->smac[2];
		hdr->dmac[3] = hdr->smac[3];
		hdr->dmac[4] = hdr->smac[4];
		hdr->dmac[5] = hdr->smac[5];

		hdr->smac[0] = mac_address[0];
		hdr->smac[1] = mac_address[1];
		hdr->smac[2] = mac_address[2];
		hdr->smac[3] = mac_address[3];
		hdr->smac[4] = mac_address[4];
		hdr->smac[5] = mac_address[5];  


			send(i, buf, 42,0);
		}
	}
   


   }
   else{
	   printf("It is not an ARP Package\n");
   }
		}
	}


  }


    
    //free the interface list when we don't need it anymore
  freeifaddrs(ifaddr);
  //exit
  return 0;
}



