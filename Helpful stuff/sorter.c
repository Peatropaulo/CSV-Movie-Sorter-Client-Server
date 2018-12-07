#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "sorter.h"

void get_str(char**, char**);

char* key = NULL;
char* headers_m = NULL;
pthread_mutex_t mut;
pthread_mutex_t header_mut;
record** records_m = NULL;
size_t size_m = 0;

pthread_mutex_t tpool_mut;
pthread_t tpool[TPOOL_SIZE];
size_t tpool_i = 0;

uint32_t port = 0;
struct sockaddr_in server_addr;
struct hostent *server = NULL;

char* columns[] = {"color",
                   "director_name",
                   "num_critic_for_reviews",
                   "duration",
                   "director_facebook_likes",
                   "actor_3_facebook_likes",
                   "actor_2_name",
                   "actor_1_facebook_likes",
                   "gross",
                   "genres",
                   "actor_1_name",
                   "movie_title",
                   "num_voted_users",
                   "cast_total_facebook_likes",
                   "actor_3_name",
                   "facenumber_in_poster",
                   "plot_keywords",
                   "movie_imdb_link",
                   "num_user_for_reviews",
                   "language",
                   "country",
                   "content_rating",
                   "budget",
                   "title_year",
                   "actor_2_facebook_likes",
                   "imdb_score",
                   "aspect_ratio",
                   "movie_facebook_likes"
                  };

// print a message if it's used wrong
void print_usage() {
	puts("Usage: ./sorter -c COL -h HOSTNAME -p PORT [-d ROOT] [-o OUT] where COL is one of: \n");
	for (int i=0; i<28; ++i) {
		puts(columns[i]);
	}
	puts("\nwhere the server is running on HOSTNAME:PORT, and ROOT is the optional base directory to search in, and OUT is the optional output directory.");
}

// check if a directory exists
void check_dir(char* path) {
	DIR* d = opendir(path);
	if (d) {
		closedir(d);
		return;
	} else {
		printf("Directory %s doesn't exist", path);
		exit(-1);
	}
}

int read_sock(int sockfd, char* buf, int len, int flags, fd_set *socks) {
	select(sockfd+1, socks, NULL, NULL, NULL);
	return recv(sockfd, buf, len, flags);
}

void* newfile(void* pathin) {
	char* path = (char*)pathin;
	record** records;
	int size = 0;
	int current = 0;

	// start processing a file
	// check file extensions
	if (strcmp(".csv", &path[strlen(path)-4])) return 0;
	if (strstr(path, "-sorted-")) return 0;
	FILE* csv_in = fopen(path, "r");
	free(pathin);
	fseek(csv_in, 0L, SEEK_END);
	size_t filesize = ftell(csv_in);
	rewind(csv_in);
	// get column names
	char* headers = NULL;
	size_t n = 0;
	getline(&headers, &n, csv_in);
	headers = trim(headers);

	// check number of columns
	size_t commas = 0;
	for (size_t i = 0; i < strlen(headers); ++i) {
		commas += headers[i] == ',';
	}
	if (commas != 27) return 0;

	//store the header if not already stored
	pthread_mutex_lock(&header_mut);
	if (headers_m == NULL) {
		headers_m = alloc(strlen(headers)+1);
		strcpy(headers_m, headers);
	}
	pthread_mutex_unlock(&header_mut);
	free(headers);

	// the file is valid, continue with processing
	// allocate space for some records to start
	size = filesize/300;
	records = alloc(size*sizeof(record*));

	// open a socket and connect
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		puts("Couldn't open a socket");
		exit(-1);
	}
	if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0){
    	puts("Couldn't connect to server");
    	exit(-1);
	}
	char buf[20];
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(sockfd,&socks);
	write(sockfd, "record", strlen("record"));
	buf[read_sock(sockfd, buf, 20, 0, &socks)] = 0;
	printf("buf '%s'\n", buf);

	char* raw = NULL;
	// get the first record
	n = 0;
	getline(&raw, &n, csv_in);
	// loop through every record and store it in memory
	while (raw && raw[0]) {
		// trim whitespace
		raw = trim(raw);
		size_t rawlen = strlen(raw);
		char len_str[10];
		sprintf(len_str, "%lu", rawlen);
		char len_len_str[10];
		sprintf(len_len_str, "%lu", strlen(len_str));

		char* sendbuf = malloc(strlen(raw)+strlen(len_str)+3);
		sprintf(sendbuf, "%lu@%s%s",strlen(len_str), len_str, raw);
		//puts(sendbuf);

		write(sockfd, sendbuf, strlen(sendbuf));
		read_sock(sockfd, buf, 20, 0, &socks);
		//puts(buf);
	
		// do some housekeeping to prepare for the next line
		if (current >= size) {
			size += 100;
			records = realloc(records, size*sizeof(record*));
			if (!records) {
				puts("Couldn't allocate memory.");
				exit(-1);
			}
		}
		//get next line
		raw = NULL;
		n = 0;
		if (getline(&raw, &n, csv_in) < 0) {
			free(raw);
			break;
		}
	}
	puts("closing socket");
	write(sockfd, "0@", strlen("0@"));
	read_sock(sockfd, buf, 20, 0, &socks);
	//close(sockfd);

	pthread_mutex_lock(&mut);
	size_t old_size = size_m;
	size_m += current;
	records_m = realloc(records_m, size_m*sizeof(record*));
	if (!records_m) {
		puts("Couldn't allocate memory.");
		exit(-1);
	}
	memcpy(&records_m[old_size], records, current*sizeof(record*));
	pthread_mutex_unlock(&mut);
	free(records);
	return 0;
}

void* newdir(void* pathin) {
	// initialize some flags and variables
	char* cur_path = (char*)pathin;
	DIR* cur_dir = opendir(cur_path);
	struct dirent* sub;
	while ((sub = readdir(cur_dir))) {
		// skip current directory and parent directory or bad things happen
		if (!strcmp(sub->d_name, ".") || !strcmp(sub->d_name, "..")) continue;
		char* newpath = alloc(PATH_MAX);
		strcpy(newpath, cur_path);
		strcat(newpath, "/");
		strcat(newpath, sub->d_name);

		pthread_mutex_lock(&tpool_mut);
		if (tpool[tpool_i]) {
			pthread_join(tpool[tpool_i], NULL);
		}
		if (sub->d_type == DT_REG) {
			// if it's a file, create a thread
			if (pthread_create(&tpool[tpool_i], NULL, newfile, newpath)) {
				perror("pthread_create");
				puts("Thread error");
				exit(-1);
			}
		} else if (sub->d_type == DT_DIR) {
			// if it's a directory, create a thread
			if (pthread_create(&tpool[tpool_i], NULL, newdir, newpath)) {
				perror("pthread_create");
				puts("Thread error");
				exit(-1);
			}
		} else {
			// if it's not a file or directory, skip
			tpool_i -= 1;
			continue;
		}
		tpool_i = (tpool_i+1)%TPOOL_SIZE;
		pthread_mutex_unlock(&tpool_mut);
	}

	closedir(cur_dir);
	free(cur_path);
	return 0;
}

int main(int argc, char** argv) {
	// check if there's enough args
	if (argc < 7) {
		print_usage();
		return -1;
	}
	// assign default values
	char* pwd_path = alloc(PATH_MAX);
	getcwd(pwd_path, PATH_MAX);
	char* cur_path = alloc(PATH_MAX);
	getcwd(cur_path, PATH_MAX);
	char* out_path = alloc(PATH_MAX);
	getcwd(out_path, PATH_MAX);

	for (int i = 0; i < argc-1; ++i) {
		// skip if not a switch
		if (argv[i][0] != '-') continue;
		// exit if a switch has no value
		if (i+1 == argc) {print_usage(); return -1;}
		switch (argv[i++][1]) {
		case 'h':
			server = gethostbyname(argv[i]);
			break;
		case 'p':
			port = atoi(argv[i]);
			break;
		case 'c':
			// set a key
			key = argv[i];
			break;
		case 'd':
			// set a base dir
			if (argv[i][0] != '/') {
				// relative path
				strcpy(cur_path, pwd_path);
				strcat(cur_path, "/");
				strcat(cur_path, argv[i]);
			} else {
				// absolute path
				strcpy(cur_path, argv[i]);
			}
			check_dir(cur_path);
			break;
		case 'o':
			if (argv[i][0] != '/') {
				// relative path
				strcpy(out_path, pwd_path);
				strcat(out_path, "/");
				strcat(out_path, argv[i]);
			} else {
				// absolute path
				strcpy(out_path, argv[i]);
			}
			check_dir(out_path);
			break;
		default:
			// unknown switch
			print_usage();
			return -1;
		}
	}

	// ensure a key was provided
	if (key == NULL) {
		puts("Please enter a column name to sort on with the -c flag");
		return -1;
	}
	// check if key is valid
	bool flag = false;
	for (int i=0; i<28; ++i) {
		if (!strcmp(columns[i],key)) {
			flag = true;
			break;
		}
	}
	if (!flag) {
		printf("%s is not a valid column name\n", key);
		return -1;
	}
	if(port == 0) {
		print_usage();
		return -1;
	}
	if(server == NULL) {
		print_usage();
		return -1;
	}
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
	server_addr.sin_port = htons(port);

	pthread_mutex_init(&mut, NULL);
	pthread_mutex_init(&header_mut, NULL);
	pthread_mutex_init(&tpool_mut, NULL);
	newdir(cur_path);

	bool done = false;
	while (!done) {
		done = true;
		for (size_t j = 0; j < TPOOL_SIZE; ++j) {
			pthread_mutex_lock(&tpool_mut);
			if (tpool[j]) {
				done = false;
				pthread_join(tpool[j], NULL);
				tpool[j] = 0;
			}
			pthread_mutex_unlock(&tpool_mut);
		}
	}

	//retrieve sorted files
	// open a socket and connect
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		puts("Couldn't open a socket");
		exit(-1);
	}
	if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0){
    	puts("Couldn't connect to server");
    	exit(-1);
	}
	char buf[20];
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(sockfd,&socks);
	write(sockfd, "sorter", strlen("sorter"));
	read_sock(sockfd, buf, 20, 0, &socks);
	for(int i = 0; i < 28; ++i){
		if(!strcmp(key, columns[i])) {
			sprintf(buf,"%d",i);
			write(sockfd, buf, strlen(buf));
			read_sock(sockfd, buf, 20, 0, &socks);
		}
	}
	send(sockfd, "return", strlen("return"),0);
	
	// save the sorted csv
	char* csv_out_path = alloc(PATH_MAX);
	sprintf(csv_out_path, "%s/AllFiles-sorted-%s.csv", out_path, key);
	FILE* csv_out = fopen(csv_out_path, "w");
	fprintf(csv_out, "%s\n", headers_m);
	for (size_t i = 0; i<size_m; ++i) {
		print_record(csv_out, records_m[i]);
	}
	fclose(csv_out);
	return 0;
}

// get the next comma delimited string in the line
char* empty = "";
void get_str(char** raw, char** str) {
	char *temp = NULL;
	temp = strsep(raw, ",");
	temp = trim(temp);
	sscanf(temp, "%m[^,]", str);
	if (!*str) {
		*str = empty;
	}
}

// protected allocation
void* alloc(size_t size) {
	void* temp = malloc(size);
	if (!temp) {
		printf("Couldn't allocate %lu bytes.\n", size);
		fflush(stdout);
		exit(-1);
	} else {
		return temp;
	}
}

//adapted from stackoverflow
char* trim(char* str) {
	size_t len = 0;
	char *frontp = str;
	char *endp = NULL;

	//some error checking
	if (str == NULL) {return NULL;}
	if (str[0] == '\0') {return str;}

	len = strlen(str);
	endp = str+len;

	// trim the front
	while (isspace((unsigned char) *frontp)) {++frontp;}
	// trim the end
	if (endp != frontp) {
		while (isspace((unsigned char) *(--endp)) && endp != frontp) {}
	}

	// error checking
	if (str+len-1 != endp)
		*(endp+1) = '\0';
	else if (frontp != str && endp == frontp)
		*str = '\0';

	endp = str;
	if (frontp != str) {
		while (*frontp) { *endp++ = *frontp++; }
		*endp = '\0';
	}
	return str;
}

// print a record in csv format
void print_record(FILE* f, record *r) {
	fprintf(f, "%s,%s,%hu,%hu,%hu,%hu,%s,%u,%u,%s,%s,",
	        r->color,
	        r->director_name,
	        r->num_critic_for_reviews,
	        r->duration,
	        r->director_facebook_likes,
	        r->actor_3_facebook_likes,
	        r->actor_2_name,
	        r->actor_1_facebook_likes,
	        r->gross,
	        r->genres,
	        r->actor_1_name);
	if (strchr(r->movie_title, ',')) {
		fprintf(f, "\"%s\"", r->movie_title);
	} else {
		fprintf(f, r->movie_title);
	}
	fprintf(f, ",%u,%u,%s,%hhu,%s,%s,%hu,%s,%s,%s,%lu,%hu,%u,%f,%f,%u\n",
	        r->num_voted_users,
	        r->cast_total_facebook_likes,
	        r->actor_3_name,
	        r->facenumber_in_poster,
	        r->plot_keywords,
	        r->movie_imdb_link,
	        r->num_user_for_reviews,
	        r->language,
	        r->country,
	        r->content_rating,
	        r->budget,
	        r->title_year,
	        r->actor_2_facebook_likes,
	        r->imdb_score,
	        r->aspect_ratio,
	        r->movie_facebook_likes);
}

// allow comparisons on an arbitrary field
float cmp(record* r1, record* r2, char* key) {
	if (strcmp(key, "color") == 0) {
		return strcmp(r1->color, r2->color);
	} else if (strcmp(key, "director_name") == 0) {
		return strcmp(r1->director_name, r2->director_name);
	} else if (strcmp(key, "num_critic_for_reviews") == 0) {
		return r1->num_critic_for_reviews - r2->num_critic_for_reviews;
	} else if (strcmp(key, "duration") == 0) {
		return r1->duration - r2->duration;
	} else if (strcmp(key, "director_facebook_likes") == 0) {
		return r1->director_facebook_likes - r2->director_facebook_likes;
	} else if (strcmp(key, "actor_3_facebook_likes") == 0) {
		return r1->actor_3_facebook_likes - r2->actor_3_facebook_likes;
	} else if (strcmp(key, "actor_2_name") == 0) {
		return strcmp(r1->actor_2_name, r2->actor_2_name);
	} else if (strcmp(key, "actor_1_facebook_likes") == 0) {
		return r1->actor_1_facebook_likes - r2->actor_1_facebook_likes;
	} else if (strcmp(key, "gross") == 0) {
		return r1->gross - r2->gross;
	} else if (strcmp(key, "genres") == 0) {
		return strcmp(r1->genres, r2->genres);
	} else if (strcmp(key, "actor_1_name") == 0) {
		return strcmp(r1->actor_1_name, r2->actor_1_name);
	} else if (strcmp(key, "movie_title") == 0) {
		return strcmp(r1->movie_title, r2->movie_title);
	} else if (strcmp(key, "num_voted_users") == 0) {
		return r1->num_voted_users - r2->num_voted_users;
	} else if (strcmp(key, "cast_total_facebook_likes") == 0) {
		return r1->cast_total_facebook_likes - r2->cast_total_facebook_likes;
	} else if (strcmp(key, "actor_3_name") == 0) {
		return strcmp(r1->actor_3_name, r2->actor_3_name);
	} else if (strcmp(key, "facenumber_in_poster") == 0) {
		return r1->facenumber_in_poster - r2->facenumber_in_poster;
	} else if (strcmp(key, "plot_keywords") == 0) {
		return strcmp(r1->plot_keywords, r2->plot_keywords);
	} else if (strcmp(key, "movie_imdb_link") == 0) {
		return strcmp(r1->movie_imdb_link, r2->movie_imdb_link);
	} else if (strcmp(key, "num_user_for_reviews") == 0) {
		return r1->num_user_for_reviews - r2->num_user_for_reviews;
	} else if (strcmp(key, "language") == 0) {
		return strcmp(r1->language, r2->language);
	} else if (strcmp(key, "country") == 0) {
		return strcmp(r1->country, r2->country);
	} else if (strcmp(key, "content_rating") == 0) {
		return strcmp(r1->content_rating, r2->content_rating);
	} else if (strcmp(key, "budget") == 0) {
		return r1->budget - r2->budget;
	} else if (strcmp(key, "title_year") == 0) {
		return r1->title_year - r2->title_year;
	} else if (strcmp(key, "actor_2_facebook_likes") == 0) {
		return r1->actor_2_facebook_likes - r2->actor_2_facebook_likes;
	} else if (strcmp(key, "imdb_score") == 0) {
		return r1->imdb_score - r2->imdb_score;
	} else if (strcmp(key, "aspect_ratio") == 0) {
		return r1->aspect_ratio - r2->aspect_ratio;
	} else if (strcmp(key, "movie_facebook_likes") == 0) {
		return r1->movie_facebook_likes - r2->movie_facebook_likes;
	} else {
		puts("This should never happen");
		exit(-1);
	}
}
