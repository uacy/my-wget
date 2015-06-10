#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>  
#include <unistd.h>     
#include <sys/types.h>  
#include <netdb.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define HTTP_PORT 80
#define MAXLEN 500

/* variabile globale care ajuta la geterminarea argumentelor
si salvarea linkurilor care se downloadeaza recursiv
links este pentru linkuri html
files este pentru fisiere */

char links[1000][100];
char files[1000][100];
int contorlinks = 0,contorfiles = 0;
int flag_e = 0, flag_r = 0, flag_o = 0;
FILE *logfile;
char currentpath[50];

/* mesaj de eroare la citirea argumentelor */

void messageerror(){
		if(flag_o){
			fprintf(logfile,"./myclient [-r] [-e] [-o <fisier_log>] http://<nume_server>/<cale_catre_pagina>\n");
		}else{
			printf("./myclient [-r] [-e] [-o <fisier_log>] http://<nume_server>/<cale_catre_pagina>\n");
  	}
  	exit(-1);
}

/* functie care verifica argumentele si asigneaza optiunile programului
daca o optiune nu este posibila se returneaza un mesaj de eroare */

int checkargv(char *argument){
	if(strcmp(argument,"-r") == 0){ flag_r = 1;}
	else if(strcmp(argument,"-e") == 0){ flag_e = 1;}
	else if(strcmp(argument,"-o") == 0){ 
		flag_o = 1;
		return 2;
	}
//	else if(strstr(argument,".txt") != NULL){ return 2;}
	else if(strstr(argument,"http") != NULL){	return 1;}
	else{
		printf("ajunge la mesajul de eroarea\n");
		messageerror();	
	}
	return 0;
}

/* functie care returneaza direcotrul curent */

char *getcurent (){
  size_t size = 100;

  while (1)
    {
      char *buffer = (char *) malloc (size);
      if (getcwd (buffer, size) == buffer)
        return buffer;
      free (buffer);
      if (errno != ERANGE)
        return 0;
      size *= 2;
    }
}

/**
 * Citeste maxim maxlen octeti din socket-ul sockfd. Intoarce
 * numarul de octeti cititi.
 */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}

/**
 * Citeste maxim maxlen octeti din socket-ul sockfd. Intoarce
 * numarul de octeti cititi.
 */

ssize_t Readwhole(int sockfd, void *vptr, size_t maxlen){
	char *buffer = vptr;
	return read(sockfd,buffer,maxlen);
}


/**
 * Trimite o comanda HTTP si asteapta raspuns de la server.
 * Comanda trebuie sa fie in buffer-ul sendbuf.
 */
int send_command(int sockfd, char sendbuf[]) {
  char recvbuf[MAXLEN];
  int nbytes;
  char CRLF[3];
  
  CRLF[0] = 13; CRLF[1] = 10; CRLF[2] = 0;
  strcat(sendbuf, CRLF);
  if(flag_o){
  	fprintf(logfile,"Trimit: %s",sendbuf);
  }else{
  	printf("Trimit: %s", sendbuf);
  }

  write(sockfd, sendbuf, strlen(sendbuf));

  nbytes = Readline(sockfd, recvbuf, MAXLEN - 1);
 
  recvbuf[nbytes] = 0;
 	
 	if(flag_o){
 		fprintf(logfile,"Am primit: %s",recvbuf);
 	}else{
	  printf("Am primit: %s", recvbuf);
	}

  if(strstr(recvbuf,"40") != NULL || strstr(recvbuf,"50") != NULL) return -1;
  else return 1;
}

/* se sterge numele pagini html si se returneaza calea pana la acea pagina */

char *removehtml(char* cale){
	char *pch,*link,*aux, *buffer;

	buffer = (char *)malloc(strlen(cale) * sizeof(char) + 1);
	strcpy(buffer,cale);
	link = (char *)malloc(MAXLEN * sizeof(char));

	link[0] = '\0';
	pch = strtok(cale,"/");
	while(pch != NULL){
		aux = (char *)malloc(MAXLEN * sizeof(char));
		strcpy(aux,pch);
		if(strstr(aux,"htm") == NULL){
			strcat(link,"/");
			strcat(link,aux);
		}
		pch = strtok(NULL,"/");
	}

	free(buffer);

	return link;
}

/* se obtine calea relativa dintr-un fisier html
se sterge <a href="link" /a> si returneaza link */

char *parser(char *link){
	char *inc, *sf, *linkparsat;

	link = strstr(link,"href");

	inc = strchr(link,'"');
	sf = strchr (inc + 1,'"');

	linkparsat = (char *)malloc(MAXLEN * sizeof(char));

	strncpy(linkparsat,inc + 1, sf - inc);
	linkparsat[sf-inc - 1] = '\0';

	return linkparsat;
}

/* functia adauga un link html care va trebui downloadat mai tarziu
daca fisierul este deja salvat nu se mai adauga */

void addlink(char* link){
	int i = 0;
	for(;i < contorlinks; ++i){
		if(strstr(links[i],link) != NULL) return;
	}

	strcpy(links[contorlinks],link);
	++contorlinks;
}

/* functia adauga un link de tip fisier care va fi downloadat mai tarziu
daca fisierul este deja salvat nu se mai adauga */

void addfile(char *file){
	int i = 0;

	if(strstr(file,"mailto") != NULL) return;
	if(strstr(file,"ftp:") != NULL) return;
	if(strstr(file,"file:") != NULL) return;

	for(;i < contorfiles; ++i){
		if(strstr(files[i],file) != NULL)	return;
	}

	strcpy(files[contorfiles],file);
	++contorfiles;
}

/* funcite care inchide sochetul */

void close_socket(int sockfd){
	close(sockfd);
}

/* functie care verifica daca se afla un link valid pe linia citita
din fisierul html, daca se gasesc conditile <a href="link" /a>
returneaza 1 altfel 0 */

int checklink(char* link){
	if(flag_e){
		if((strstr(link,".") != NULL) && (strstr(link,"href")) != NULL){
			if(!strstr(link,"http") && !strchr(link,'\'') && strchr(link,'#') == NULL){
				return 1;
			}
		}
	}else if((strstr(link,".htm") != NULL) && (strstr(link,"href")) != NULL){
		if(!strstr(link,"http") && !strchr(link,'\'') && strchr(link,'#') == NULL){
			return 1;
		}
	}
	return 0;
}

/* functie care afla adresa ip a unui host
se foloseste pentru a putea crea conexiune la server */

char *get_ip(char *host){
  struct hostent *hent;
  int iplen = 15; 
  char *ip = (char *)malloc(iplen + 1);
  memset(ip, 0, iplen + 1);

  if((hent = gethostbyname(host)) == NULL){
  	if(flag_o){
  		fprintf(logfile, "Can't get IP\n");
  	}else{
   		herror("Can't get IP");
    }
    exit(1);
  }

  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL){
    if(flag_o){
    	fprintf(logfile,"Can't resolve host");
    }else{
    	perror("Can't resolve host");
    }
    exit(1);
  }
  return ip;
}

/* functia care se ocupa sa faca cerere la server
cu pagina sau fisierul dorit */

void getpage(int sockfd, char *pagina){

  char recvbuf[MAXLEN];
  char *comanda = (char*) malloc(MAXLEN*sizeof(char));
  char *pch, numepag[100],*bufferlink;
  int i = 0;
  size_t length;

  sprintf(comanda,"GET %s HTTP/1.0\n\n",pagina);

/* se trimite cererea la server */
  if(send_command(sockfd,comanda) == -1) return;

  free(comanda);

  char* bufferpagina = (char *)malloc(strlen(pagina) * sizeof(char) + 10);
  strcpy(bufferpagina,pagina);

/* se creaza arborele de foldere ca pe server
daca folderul exista deja se schimba directorul curent
daca folderul nu exista, se creaza si se schimba directorul curent */

  pch = strtok(bufferpagina,"/");
  while(pch != NULL){
  	strcpy(numepag,pch);

  	if(strstr(numepag,".") == NULL){
 			DIR *dir = opendir(numepag);
 		
 			if(dir){
 				chdir(numepag);
 				closedir(dir);
 			} else{
 				char *mkdir = (char *)malloc(strlen(numepag) * sizeof(char) + 6);
 				sprintf(mkdir,"mkdir %s",numepag);
 				system(mkdir);
 				chdir(numepag);
 				closedir(dir);
 				free(mkdir);
 			}
  	}

  	pch = strtok(NULL,"/");
  }
  free(bufferpagina);

/* se citeste de la server ce s-a primit
daca este fisier html se scrie text, 
daca este alt tip de fisier se scrie binar */

  if(flag_e == 1){
  	FILE *f = fopen(numepag, "w");

	  while(Readline(sockfd, recvbuf, MAXLEN -1)){
	  	if(strlen(recvbuf) == 2) break;
	  }
	  
		if(strstr(numepag,"htm") != NULL){
			while(Readline(sockfd, recvbuf, MAXLEN -1) > 0){
	  		if(checklink(recvbuf)){
	  			bufferlink = (char *)malloc(MAXLEN *sizeof(char));
	 				bufferpagina = (char *)malloc(strlen(pagina) * sizeof(char) + 1);
	 			
	 				strcpy(bufferpagina,pagina);

	  			sprintf(bufferlink,"%s/%s",removehtml(bufferpagina),parser(recvbuf));
	 			
	  			if(strstr(bufferlink,"htm") != NULL){
				  	addlink(bufferlink);
				  }else{
				  	addfile(bufferlink);
				  }
	  		}
		  		fprintf(f,"%s",recvbuf);
		  	}
		  }else{
		  	while((length = Readwhole(sockfd,recvbuf,MAXLEN - 1)) > 0){
			  	fwrite(recvbuf,1,length,f);
			  }
		 	}
	  	fclose(f);
	  }else{
	  FILE *f = fopen(numepag, "w");

	  while(Readline(sockfd, recvbuf, MAXLEN -1)){
	  	if(strlen(recvbuf) == 2) break;
	  }

	  while(Readline(sockfd, recvbuf, MAXLEN -1) > 0){
	  	if(checklink(recvbuf)){
	  		bufferlink = (char *)malloc(MAXLEN *sizeof(char));
	 			bufferpagina = (char *)malloc(strlen(pagina) * sizeof(char) + 1);
	 			
	 			strcpy(bufferpagina,pagina);

	  		sprintf(bufferlink,"%s/%s",removehtml(bufferpagina),parser(recvbuf));
	 
		  	addlink(bufferlink);
	  	}
	  	fprintf(f,"%s",recvbuf);
	  }
	  fclose(f);
	 }

/* se intoarce la direcotorul initial (/) */

  chdir(currentpath);

  close_socket(sockfd);
}

/* functie care deschide un sochet */

int open_socket(char *hostname){
  	int sockfd;
  	int port = HTTP_PORT;
  	struct sockaddr_in servaddr;
    char server[20];


   /* creaza sochetul */
  	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
  		if(flag_o){
  			fprintf(logfile,"Eroare la creare socket\n");
  		}else{
				printf("Eroare la  creare socket.\n");	
			}
		exit(-1);
  	}  

  /* formarea adresei serverului */
  	memset(&servaddr, 0, sizeof(servaddr));
  	servaddr.sin_family = AF_INET;
  	servaddr.sin_port = htons(port);

  	if (inet_aton(get_ip(hostname), &servaddr.sin_addr) <= 0 ) {
  		if(flag_o){
   	 		fprintf(logfile,"Adresa IP invalida\n");
   	 	}else{
   		 printf("Adresa IP invalida.\n");
			}
   	 exit(-1);
  	}
   
  /*  conectare la server  */
  	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
  		if(flag_o){
  			fprintf(logfile,"Eroare la conectare\n");
  		}else{
   	 		printf("Eroare la conectare\n");
   	 	}
   	 exit(-1);
  	}

  	return sockfd;
}


int main(int argc, char **argv) {
  char *pch, *pagina, *hostname,*pag,server[200],logfilename[50];
  int i,j, recursivitate[5];

/* se verifica argumentele si se aplica */

  if(argc < 2 || argc > 6){
		messageerror();  
  }


  for(i = 1; i < argc; ++i){
  	if(checkargv(argv[i]) == 1){ strcpy(server,argv[i]);}
  	else if(checkargv(argv[i]) == 2){
  		logfile = fopen(argv[i + 1],"w");
  		++i;
  	}
  } 

  if(strstr(server,"http") == NULL){
  	messageerror();
  }

  /* se parseaza serverul si prima pagina primita */

  pch = strtok(server,"/");
  hostname = strtok(NULL,"/");
  pag = strtok(NULL,"");

  pagina = (char *)malloc(strlen(pag) * sizeof(char) + 3);
  sprintf(pagina,"/%s",pag);

 	if(strstr(pagina,"htm") == NULL){
 		messageerror();
 	}

  /* se creaza directorul cu numele serverului */

  char *mkdir = (char *)malloc(strlen(hostname) * sizeof(char) + 6);
  sprintf(mkdir,"mkdir %s",hostname);

  DIR* dir = opendir(hostname);

  if(dir){
  	chdir(hostname);
  	closedir(dir);
  }else{
  	system(mkdir);
  	chdir(hostname);
  	closedir(dir);
  }

  /* se salveaza directorul curent pentru a se intoarce de fiecare
  data dupa ce se duce in ramificatii de foldere */

  strcpy(currentpath,getcurent());
  
   /* se cere pagina de la server */

	getpage(open_socket(hostname),pagina);

	/* aici se aplica recursivitatea daca este ceruta
	pana la al 5-elea nivel */

	if(flag_r){

		recursivitate[0] = 0;

		for(i = 1; i < 5; ++i){
			recursivitate[i] = contorlinks;
			for(j = recursivitate[i-1]; j < recursivitate[i]; ++j){
				printf("NIVEL: %d\n",i+1);
				getpage(open_socket(hostname),links[j]);
			}
		}
	}

	/* aici se aplica argumentul -e (everything) */

	if(flag_e){
		for(i = 0; i < contorfiles; ++i){
			getpage(open_socket(hostname),files[i]);
		}
	}

	if(flag_o){
		fclose(logfile);
	}
	free(mkdir);
	free(pagina);

	return 0;
}