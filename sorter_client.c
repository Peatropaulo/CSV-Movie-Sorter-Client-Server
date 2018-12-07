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
#include "sorter_client.h"
#include "is.c"



void print_usage();
void print_stat(struct file_dir *str);
int check_oDir(char* output); // 0 = Exists 1 = Does not exist 3 = Couldn't create
int check_wDir(char* output); // 0 = Exists 1 = Does not exist
char* get_cwd(char *var);

int main(int argc, char* argv[]) {
//Check Arguments!
	if (argc < 3) {
		print_usage();
		exit(0);
	}
//getopt method to get the input
	int option = 0;
	char * sF = (char *) malloc(sizeof(char) * 25); //sF = sortingField. The arg after "-c"
	char * wDir = (char *) malloc(sizeof(char) * 200); //wDir arg after -d
	char * oDir = (char *) malloc(sizeof(char) * 200); //oDir arg after -o
	char * host = (char *) malloc(sizeof(char) * 200); //oDir arg after -o
	//char* fullPath = (char*) malloc(sizeof(char) * 40);
	char* curr_dir = (char*) malloc(sizeof(char) * 200); //current working directory
	int compare_size = 0;
	int port = 0;

	curr_dir = get_cwd(curr_dir);
	strcpy(wDir, curr_dir);
	strcpy(oDir, curr_dir);

	while ((option = getopt(argc, argv, "-c:-h:-p:-d:-o:")) != -1) {
		switch (option) {
		case 'c':
			strcpy(sF, optarg);
			break;
		case 'd':
			strcpy(wDir, optarg);
			break;
		case 'o':
			strcpy(oDir, optarg);
			break;
		case 'h':
			strcpy(host,optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			print_usage();
			exit(0);
		} //end switch
	} //end while
	  //check if sorting field was provided
	if (sF[0] == '\0') {
		print_usage();
		exit(0);
	} else {
		char*cat[] = { "color", "director_name", "num_critic_for_reviews",
				"duration", "director_facebook_likes", "actor_3_facebook_likes",
				"actor_2_name", "actor_1_facebook_likes"\
				, "gross", "genres",
				"actor_1_name", "movie_title", "num_voted_users",
				"cast_total_facebook_likes", "actor_3_name",
				"facenumber_in_poster", "plot_keywords", "movie_imdb_link"
						"num_user_for_reviews", "language", "country",
				"content_rating", "budget", "title_year",
				"actor_2_facebook_likes", "imdb_score", "aspect_ratio",
				"movie_facebook_likes" };
		compare_size = findInt(sF, cat, 27); // converts the sorting field into int
		//printf("compare_size : %d\n",compare_size);
		if (compare_size<0) {
			print_usage();
			exit(0);
		}
	}
	//check if the Input Directory exists
	if (check_wDir(wDir)) {
		printf("Given input directory doesn't exist. Exiting!\n");
		exit(0);
	}
	//check if the Ouput Directory exists *if it does then continue else create a new one(if couldnt create a new one then exit)
	if ((check_oDir(oDir)) == 3) {
		exit(0);
	}
	if ((wDir[0]) != '/') {	//checks if it is a absolute path; if not then make it
		char * temp = (char *) malloc(sizeof(char) * 200);
		strcpy(temp, curr_dir);
		strcat(temp, "/");
		strcat(temp, wDir);
		strcpy(wDir, temp);
		//printf("%s\n", wDir);
		free(temp);
	}
	if ((oDir[0]) != '/') {	//checks if it is a absolute path; if not then make it
		char * temp = (char *) malloc(sizeof(char) * 200);
		strcpy(temp, curr_dir);
		strcat(temp, "/");
		strcat(temp, oDir);
		strcpy(oDir, temp);
		//printf("%s\n", oDir);
		free(temp);
	}

	// created a struct since when creating pthreads we cannot pass multiple parameters
	struct file_dir dir_struct;
	dir_struct.wDir = wDir;
	dir_struct.oDir = oDir;
	dir_struct.sF = sF;
	dir_struct.compare = compare_size;
	dir_struct.host_name = host;
	dir_struct.port = port;
	print_stat(&dir_struct);

	//ONTO parsing directories

	return 0;
}


void print_stat(struct file_dir *str) {

	printf("_______The sF is %s\n", str->sF);
	printf("The comp col  is %d\n", str->compare);
	printf("The wDir name is %s\n", str->wDir);
	printf("The oDir name is %s\n", str->oDir);


}
void print_usage() {
	fprintf(stderr, "Usage : ./sorter_client -c duration -h grep.cs.rutgers.edu -p 12345\n");
	fprintf(stderr, "Usage : ./sorter_client -c duration -h grep.cs.rutgers.edu -p 12345 -d thisdir/thatdir -o anotherdir \n");
}
int check_wDir(char* input) {
	DIR* dir = opendir(input);
	if (dir) {
		/* Directory exists. */
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		/* Directory does not exist. */
		return 1;
	}
	return 0;
}
int check_oDir(char* output) {
	DIR* dir = opendir(output);
	if (dir) {
		/* Directory exists. */
		closedir(dir);
		return 0;
	} else if (ENOENT == errno) {
		/* Directory does not exist. */
		mkdir(output, 0000700);
		return 1;
	} else {
		/* opendir() failed for some other reason. */
		printf("Couldn't create new Directory at location : %s\n.", output);
		return 3;
	}

}
char* get_cwd(char *var) {
	//get working dir
	char cwd[256];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		perror("getcwd() error");
	else
		//	printf("Current working directory3 is: %s\n", cwd);
		strcpy(var, cwd);
	return var;
}
