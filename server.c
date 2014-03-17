#include <stdio.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>

long currentTimeMillis();
int main(int argc, char* argv[]) {  //Main
	int udpSocket, clientSocket;
struct sockaddr_in udpServer, udpClient; //Creamos la estructura para el servidor
	int status;
socklen_t addrlen = sizeof(udpClient);
	char buffer[255];
	char requestFileName[255];
	char ip[17];
	u_short clientPort;
	int fd,bytes;
	char filebuffer[10240];
	int contador = 0;
	int file;
	struct stat filestat;

	long start,end;
	int totalSendBytes,readBytes,sendBytes;

//Se crea el socket para UDP, en caso de que no se pueda crear nos imprime un mensaje de error.
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(udpSocket == -1) {
		fprintf(stderr,"Can't create UDP Socket");
	return 1;
	}

   	 udpServer.sin_family = AF_INET;
   	 inet_pton(AF_INET,"0.0.0.0",&udpServer.sin_addr.s_addr);
   	 udpServer.sin_port = htons(5000);

    	status = bind(udpSocket, (struct sockaddr*)&udpServer,sizeof(udpServer));

    	if(status != 0) {
		fprintf(stderr,"Can't bind");
    	}

	bzero(requestFileName,255);

       
	status = recvfrom(udpSocket, requestFileName, 255, 0, (struct sockaddr*)&udpClient, &addrlen );  //El servidor se queda esperando recibir el nombre del archivo que quiere el cliente

	inet_ntop(AF_INET,&(udpClient.sin_addr),ip,INET_ADDRSTRLEN);
	clientPort = ntohs(udpClient.sin_port);
	printf("El cliente queire el archivo: [%s:%i] %s\n",ip,clientPort,requestFileName);

	file = open(requestFileName,O_RDONLY);
	if(file == -1) {
		fprintf(stderr,"Can't open filename %s\n",requestFileName);
	return -1;
	}
  
	fstat(file,&filestat);

	sprintf(buffer,"READY %i\r\n",filestat.st_size);
	sendto(udpSocket,buffer,strlen(buffer),0,(struct sockaddr*)&udpClient,sizeof(udpClient));
	bzero(buffer,255);	
	status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );

	if(strcmp(buffer,"OK")!=0) {
		fprintf(stderr,"El cliente nos mado algo diferente a OK (%s)\n",buffer); //Se envia un OK de que ya se puede empezar a enviar el archivo
	return -1;
	}

    
    while(totalSendBytes < filestat.st_size) {  //Se transfiere el archivo
	readBytes = read(file,filebuffer,10240);
	sendBytes = 0;
	while(sendBytes < readBytes) {
	sendBytes += sendto(udpSocket,filebuffer+sendBytes,readBytes-sendBytes,0,(struct sockaddr*)&udpClient,sizeof(udpClient));
	}
	totalSendBytes += sendBytes;
    }

	sendto(udpSocket, "BYE", strlen("BYE"),0,(struct sockaddr*)&udpClient,sizeof(udpClient));
	close(udpSocket);
	return 0;
} //Fin del main

long currentTimeMillis() { //Se calcula el tiempo de transmision
        long t;
        struct timeval tv;
        gettimeofday(&tv, 0);
        t = (tv.tv_sec*1000)+tv.tv_usec;
        return t;
	}
