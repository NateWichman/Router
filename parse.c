#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int main(){
FILE *pFile;
char buf[1000];
char point;
char ipPrefix[100];

char* concat(const char*, const char*);

int ipAddr = 10000;

pFile=fopen("r1-table.txt", "r");
while(fgets(buf,1000,pFile)!=NULL){
	//Getting Ip Prefix
	memset(ipPrefix, 0 , sizeof(ipPrefix));
	printf("%s",buf);
	int i;
	int j = 0;
	for(i = 0; i < sizeof(buf); i++){
		point = buf[i];
		if(point == '/'){
			break;
		}
		else if(point != '.'){
			ipPrefix[j] = point;
			j++;
		}
	}
	
	//Getting Number of bits to evaluate
	printf("\nIpPrefix: %s\n", ipPrefix); 
	char ipBitAmt[2];
	j = 0;
	char point2;
	for(; i < sizeof(buf); i++){
		point2 = buf[i];
		if(point2 == 47){
			continue;
		}
		if(point2 < 48 || point2 > 57){
			break;
		}
		else{
		    printf("point: %c\n",point2);
		    ipBitAmt[j] = point2;
		    j++;
		}
	}
	int bitAmt = ipBitAmt[0] - '0';
	bitAmt = (bitAmt*10) + (ipBitAmt[1] - '0'); 
	printf("bitAmt: %d\n", bitAmt);
	
	//The number of charachters to evaluate
	int numSpots = bitAmt/4;
	char ipAddress[100];
	sprintf(ipAddress, "%i", ipAddr);
	printf("ipAddress: %s\n", ipAddress);

	int flag = 1;
	for(j = 0; j < numSpots; j++){
		printf("ipPrefix: %c\n", ipPrefix[j]);
		printf("ipAddr: %c\n", ipAddress[j]);
		if(ipPrefix[j] != ipAddress[j]){
			printf("DOES NOT EQUAL\n");
			flag = 0;
			break;
		}

	} 
	char eth[3];
	int k = 0;
	if(flag == 1){
		//This IP matches
		for(;i< sizeof(buf); i++){
		//	printf("buf[%d] = %c\n", i, buf[i]);
			/*if(buf[i] == 101 || buf[i] == 116 || buf[i] == 104 || buf[i] == 48 || buf[i] == 49 || buf[i] == 50 || buf[i] == 51){
					eth[k] = buf[i];
					k++;
					if(k == 3){
						break;
					}
			} */
			eth[k] = buf[i];
			k++;
		}
	}
	printf("Eth %s\n", eth);

}
fclose(pFile);
	
return 0;
}

char* concat(const char *s1, const char *s2){
	char *result = (char*)malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcpy(result, s2);
	return result;
}
