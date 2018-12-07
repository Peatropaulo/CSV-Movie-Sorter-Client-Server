#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "sorter_server.h"
#include "sorter.c"

#define NUM 100 // make it big for storage

/*
 * sort when asked to dumb
 */
//Globals
struct mData* records; // struct that hold all the tokens from lines
struct fileNames* fnames;  //save filenames
int ctotal = 0;	// count the total number of entries in the struct
int fcount = 0; // number of files in the data struct

//Functions
int headerDigitCount(int c_s);
void getRecord(char* record, int c_s, int length);
int byteCount(int c_s, int digitCount);

//parsing functions
char* checkIf(char * p);
int isspace(int c);
void Print(struct mData records[], int size);
void print2file(FILE *nf, struct mData records[], int size);
int parse_line(char * line);

//-----------------------------------------------------
int main() {
	int SERVER_PORT = 8876;
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == 0) {
		printf("Error creating socket on server side faild\n");
		exit(EXIT_FAILURE);
	}

	// socket address used for the server

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// Binding
	if (bind(server_socket, (struct sockaddr*) server_address,
			sizeof(server_address)) < 0) {
		printf("Failed to bind socket\n");
		exit(EXIT_FAILURE);
	}

	// Listening waiting time 16
	if (listen(server_socket, 16) < 0) {
		printf("Listening to socket failed\n");
		exit(EXIT_FAILURE);
	}

	int client_socket;
	client_socket = accept(server_socket, NULL, NULL);
	if (client_socket == -1) {
		printf("Problem creating connection to socket %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	char *recieved = "Recieved record.";
	char *ack = "Recording";
	char *sorts = "Sorting";
	char *ret = "Returning";

	//make space for the struct
	records = malloc(NUM * sizeof *records);
	fnames = malloc(NUM *sizeof(*fileNames));

	// record transfer protocol
	int exit = 0;
	char buffer[8];
	while (strcmp(exit == 0)) {
		int readvalue = recv(client_socket, buffer, 7)
		buffer[7] = '/0';
		if (strcmp(buffer, "record") == 0) {
			while (condition == 0) {

				send(client_socket, ack, strlen(ack));
				int headerLength;
				int messageLength;
				char* tempRec;

				headerLength = headerDigitCount(client_socket);
				if (headerLength <= 0) {
					condition = -1;
				} else {

					messageLength = byteCount(client_socket, headerLength);
					tempRec = malloc(sizeof(char) * messageLength);
					getRecord(client_socket, tempRec, messageLength);
					send(client_socket, recieved, strlen(recieved));
					//add parse function here
				if(!parse_line(tempRec) {
							ctotal++;
						}

					}
				}
			}
			if (strcmp(buffer, "sort") == 0) {
				//
				//sort
				//
				//get the sorting field from client ; receive an int if possible
				char *sF_buffer;
				int receive_sF = recv(client_socket, sF_buffer, 4)
				int sF; //sorting field
				if (receive_sF > 0) {
					sF = atoi(sF_buffer);
				} else {
					printf("error getting the sorting field.\n");
				}
				//send message that received sort request
				send(client_socket, sort, strlen(sort));
				//sort now
				quickSort(records, 0, ctotal, sF);

				//print the sorted result to the file
				char filename[100];
				char buffer[4];
				strcpy(filename, "file");
				printf("%s\n", filename);
				sprintf(buffer, "%d", sF);
				strcat(filename, buffer);
				printf("New filename: %s\n", filename);
				//prints and closes the file desc
				FILE nf*;
				nf = fopen(newfile, "w+");
				if(nf != 0){
					printf("Error creating file for storing sorted results!\n");
					exit(EXIT_FAILURE);
				}
				strcpy(fnames[fcount].name,filename);
				fcount++;
				print2file(nf, records, ctotal);

			}
			if (strcmp(buffer, "return") == 0) {
				//
				//return
				//
				exit = 1;
			}

		}

		close( client_socket);
		close( server_socket);
		return 0;

	}			//end main

	//--------------------------------------------------
	void getRecord(int c_s, char* record, int length) {
		int counter = 0;
		int readvalue = 0;
		while (counter < length) {
			readvalue = recv(c_s, record + counter, length - counter);
			counter = readvalue;
		}

		return;
	}

	int byteCount(int c_s, int digitCount) {
		char* buffer = (char*) malloc(sizeof(char) * digitCount);

		int readvalue = 0;
		while (readvalue == 0)
			readvalue = recv(c_s, buffer, 1);
		readvalue = 0;
		readvalue = recv(c_s, buffer, digitCount);
		if (readvalue == digitCount) {
			return atoi(buffer);
		} else {
			printf("%s\n", "Error has occured when reading header");
			exit(EXIT_FAILURE);
			return 0;
		}
	}

	int headerDigitCount(int c_s,) {
		// read header digit count
		char* buffer = (char*) malloc(sizeof(char) * 9);
		int exit_condition = 1;
		int buffer_tracker = 0;
		int readvalue;
		while (exit_condition == 1) {

			readvalue = recv(c_s, buffer + buffer_tracker, 1);
			if (readvalue == 1) {
				buffer_tracker++;
			}
			if (buffer[buffer_tracker] = '@') {
				exit_condition = 0;
				buffer[buffer_tracker] = '\0';
			}
		}
		return atoi(buffer);
	}

	int parse_line(char *line) {

		//char word;	// just a variable
		int count;// this will act as an index counter for each field of the struct
		//char *line ;
		//line = "Color,Gore Verbinski,302,169,563,1000,Orlando Bloom,40000,309404152,Action|Adventure|Fantasy,Johnny Depp,Pirates of the Caribbean: At World's End ,471220,48350,Jack Davenport,0,goddess|marriage ceremony|marriage proposal|pirate|singapore,http://www.imdb.com/title/tt0449088/?ref_=fn_tt_tt_1,1238,English,USA,PG-13,300000000,2007,5000,7.1,2.35,0";

		line = strdup(line); //important to do this or the following will segfault;

		//int sF = 0;	// field being sorted.

		//printf("Retrieved line of length %zu :\n", strlen(line));
		//printf("Given string is %s \n", line);

		char *p; //pointer for each token
		char * temp = NULL; // for special case 11
		char *result = (char *) malloc(8);  //for special case 11
		int stoi;
		float stof;

		count = 0;
		//printf("Line = %s\n",line);
		while ((p = strsep(&line, ",")) != NULL) {
			//printf("Separated data is %s.\n", p);
			//printf("counter is %d \n", count);

			switch (count) {

			case 0:
				strcpy(records[ctotal].color, p);
				//strcpy(rec[ctotal]->color, p);
				//printf("the Color (rec[]) is %s->\n", rec[ctotal]->color);
				//printf("the Color is %s.\n", records[ctotal].color);
				//printf("counter is %d \n", count);
				count++;
				break;
			case 1:
				strcpy(records[ctotal].dName, p);
				//strcpy(rec[ctotal]->dName, p);
				//printf("Directors name (rec[]) is %s->\n", rec[ctotal]->dName);
				//printf("Directors name  is %s.\n", records[ctotal].dName);
				count++;
				break;
			case 2:
				stoi = atoi(p);
				records[ctotal].review = stoi;
				//printf("the #reviewers is %d.\n", records[ctotal].review);
				count++;
				break;
			case 3:
				stoi = atoi(p);
				records[ctotal].duration = stoi;
				//printf("the duration is %d.\n", records[ctotal].duration);
				count++;
				break;
			case 4:
				stoi = atoi(p);
				records[ctotal].dFbLikes = stoi;
				//printf("the #of FB Likes is %d.\n", records[ctotal].dFbLikes);
				count++;
				break;
			case 5:
				stoi = atoi(p);
				records[ctotal].a3FbLikes = stoi;
				//printf("the Actor 3 FB likes is %d.\n",
				//	records[ctotal].a3FbLikes);
				count++;
				break;
			case 6:
				strcpy(records[ctotal].a2Name, p);
				//printf("the Actor 2 Name is %s.\n", records[ctotal].a2Name);
				count++;
				break;
			case 7:
				stoi = atoi(p);
				records[ctotal].a1FbLikes = stoi;
				//printf("the Actor 1 FB Likes is %d.\n",
				//records[ctotal].a1FbLikes);
				count++;
				break;
			case 8:
				stoi = atoi(p);
				records[ctotal].gross = stoi;
				//printf("the gross $ is %d.\n", records[ctotal].gross);
				count++;
				break;
			case 9:
				strcpy(records[ctotal].genres, p);
				//printf("the Genre is %s.\n", records[ctotal].genres);
				count++;
				break;
			case 10:
				strcpy(records[ctotal].a1Name, p);
				//printf("the Actor 1 Name is %s.\n", records[ctotal].a1Name);
				count++;
				break;
			case 11:
				//p = checkIf(p);
				strcpy(records[ctotal].mTitle, p);
				//printf("the Movie title is %s.\n", records[ctotal].mTitle);
				count++;
				break;
			case 12:
				stoi = atoi(p);
				records[ctotal].votes = stoi;
				//printf("the #Votes is %d.\n", records[ctotal].votes);
				count++;
				break;
			case 13:
				stoi = atoi(p);
				records[ctotal].castFbLikes = stoi;
				//printf("the Cast FB Likes is %d.\n",
				//records[ctotal].castFbLikes);
				count++;
				break;
			case 14:
				strcpy(records[ctotal].a3Name, p);
				//printf("the Actor 3 Name is %s.\n", records[ctotal].a3Name);
				count++;
				break;
			case 15:
				stoi = atoi(p);
				records[ctotal].facenum = stoi;
				//printf("the poster Face count is %d.\n",
				//	records[ctotal].facenum);
				count++;
				break;
			case 16:
				strcpy(records[ctotal].plot, p);
				//printf("the Plot is %s.\n", records[ctotal].plot);
				count++;
				break;
			case 17:
				strcpy(records[ctotal].movielink, p);
				//printf("the Movie URL is %s.\n", records[ctotal].movielink);
				count++;
				break;
			case 18:
				stoi = atoi(p);
				records[ctotal].userReview = stoi;
				//printf("the User Review is %d.\n", records[ctotal].userReview);
				count++;
				break;
			case 19:
				strcpy(records[ctotal].language, p);
				//printf("the language is %s.\n", records[ctotal].language);
				count++;
				break;
			case 20:
				strcpy(records[ctotal].country, p);
				//printf("the country is %s.\n", records[ctotal].country);
				count++;
				break;
			case 21:
				strcpy(records[ctotal].cRating, p);
				//printf("the Content Rating is %s.\n", records[ctotal].cRating);
				count++;
				break;
			case 22:
				stoi = atoi(p);
				records[ctotal].budget = stoi;
				//printf("the Budget is %d.\n", records[ctotal].budget);
				count++;
				break;
			case 23:
				stoi = atoi(p);
				records[ctotal].tYear = stoi;
				//printf("the Release Year is %d.\n", records[ctotal].tYear);
				count++;
				break;
			case 24:
				stoi = atoi(p);
				records[ctotal].a2FbLikes = stoi;
				//printf("the Actor 2 FB Likes is %d.\n",
				//	records[ctotal].a2FbLikes);
				count++;
				break;
			case 25:
				stof = atof(p);
				records[ctotal].imdbScore = stof;
				//printf("imdb  is %s\n", p);
				//printf("the IMBD score is %g.\n", records[ctotal].imdbScore);
				count++;
				break;
			case 26:
				stof = atof(p);
				records[ctotal].aRatio = stof;
				//printf("aspect ratio is %s\n", p);
				//printf("the Aspect Ratio is %g.\n", records[ctotal].aRatio);
				count++;
				break;
			case 27:
				stoi = atoi(p);
				records[ctotal].movieFbLikes = stoi;
				//printf("the Facebook Moive Likes data is %d.\n",
				//records[ctotal].movieFbLikes);
				count++;
				break;

			default:
				printf("the added data is NULL.\n");
			}

			//break;

			if (count == 11) {
				char *copy = malloc(strlen(line) + 1);
				char *copy2 = malloc(strlen(line) + 1);
				//printf("count = 12\n");
				temp = line;

				//printf("1 gline string in round 12 is %s.\n", temp);
				if (temp[0] == '"') {
					//	printf("Quotes encountered ******************************\n");
					temp[0] = ' ';
					strcpy(copy, line);
					//	printf("1 copy string in round 12 is %s.\n", copy);
					//	printf("temp string in round 12 is %s.\n", temp);
					result = strtok(line, "\"");
					strcpy(copy2, result);
					result = strtok(NULL, "\"");
					//	printf("2copy string in round 12 is %s.\n", copy);
					///	printf("The result string is %s.\n", result);
					result[0] = ' ';
					line = result;
					//	printf("line string in round 12 is %s.\n", line);
					copy2 = checkIf(copy2);
					strcpy(records[ctotal].mTitle, copy2);
					//	printf("the movie data is %s.\n", records[ctotal].mTitle);
					count++;

				}
			}
		}
		Print(records, ctotal);
		return 0;
	}					//end parse

//function to check space
	char* checkIf(char * p) {
		int var = p[0];
		int it;
		//printf("char in is %s\n",p);
		if (isspace(var)) {
			//printf("var = |%c| is a white-space character\n", var);
			char * new = malloc(strlen(p));
			it = 0;
			if (isspace(p[strlen(p) - 1])) {
				it = 2;
				while (isspace(p[strlen(p) - it])) {
					it++;
				}
				//printf("1 It is %d\n", it);
			}
			//printf("It is %d\n", (strlen(p) - it));
			memcpy(new, &p[1], strlen(p) - it);
			return new;
		} else {
			//printf("var = |%c| is not a white-space character\n", var);
			char * new = malloc(strlen(p));
			it = 0;
			if (isspace(p[strlen(p) - 1])) {
				it = 2;
				while (isspace(p[strlen(p) - it])) {
					it++;
				}
				//printf("char in back end %c\n",(p[strlen(p) - it]));
				//printf("2 It is %d\n", it);
			}
			//printf("It is %d\n", (strlen(p) - it));
			memcpy(new, &p[0], strlen(p) - it);
			return new;
		}
	}
	void Print(struct mData records[], int size) {
		int i;
		for (i = 0; i < size; i++) {
			//printf(" Movie %s. \n",  records[i].mTitle);

			printf(
					"%s,%s,%d,%d,%d,%d,%s,%d,%d,%s,%s,%s,%d,%d,%s,%d,%s,%s,%d,%s,%s,%s,%d,%d,%d,%g,%g,%d\n",
					records[i].color, records[i].dName, records[i].review,
					records[i].duration, records[i].dFbLikes,
					records[i].a3FbLikes, records[i].a2Name,
					records[i].a1FbLikes, records[i].gross, records[i].genres,
					records[i].a1Name, records[i].mTitle, records[i].votes,
					records[i].castFbLikes, records[i].a3Name,
					records[i].facenum, records[i].plot, records[i].movielink,
					records[i].userReview, records[i].language,
					records[i].country, records[i].cRating, records[i].budget,
					records[i].tYear, records[i].a2FbLikes,
					records[i].imdbScore, records[i].aRatio,
					records[i].movieFbLikes);
			//printf("\n");

		}
	}

	void print2file(FILE *nf, struct mData records[], int size) {

		fprintf(nf,
				"color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,"
						"actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,"
						"actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,"
						"title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes\n");
		int i;
		for (i = 0; i < size; i++) {
			//printf(" Movie %s. \n",  records[i].mTitle);

			fprintf(nf,
					"%s,%s,%d,%d,%d,%d,%s,%d,%d,%s,%s,%s,%d,%d,%s,%d,%s,%s,%d,%s,%s,%s,%d,%d,%d,%g,%g,%d\n",
					records[i].color, records[i].dName, records[i].review,
					records[i].duration, records[i].dFbLikes,
					records[i].a3FbLikes, records[i].a2Name,
					records[i].a1FbLikes, records[i].gross, records[i].genres,
					records[i].a1Name, records[i].mTitle, records[i].votes,
					records[i].castFbLikes, records[i].a3Name,
					records[i].facenum, records[i].plot, records[i].movielink,
					records[i].userReview, records[i].language,
					records[i].country, records[i].cRating, records[i].budget,
					records[i].tYear, records[i].a2FbLikes,
					records[i].imdbScore, records[i].aRatio,
					records[i].movieFbLikes);
			//printf("\n");

		}
		fclose(nf);
	}
}

