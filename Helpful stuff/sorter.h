#ifndef _SORTER_H
#define _SORTER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// column names
extern char* columns[];
// struct to store a record
typedef struct record {
	char * color;
	char* director_name;
	uint16_t num_critic_for_reviews;
	uint16_t duration;
	uint16_t director_facebook_likes;
	uint16_t actor_3_facebook_likes;
	char* actor_2_name;
	uint32_t actor_1_facebook_likes;
	uint32_t gross;
	char* genres;
	char* actor_1_name;
	char* movie_title;
	uint32_t num_voted_users;
	uint32_t cast_total_facebook_likes;
	char* actor_3_name;
	uint8_t facenumber_in_poster;
	char* plot_keywords;
	char* movie_imdb_link;
	uint16_t num_user_for_reviews;
	char* language;
	char* country;
	char* content_rating;
	uint64_t budget;
	uint16_t title_year;
	uint32_t actor_2_facebook_likes;
	float imdb_score;
	float aspect_ratio;
	uint32_t movie_facebook_likes;
} record;

// protected allocation
void* alloc(size_t);
// trim whitespace from a string
char* trim(char*);
// print a record in csv format
void print_record(FILE*, record*);
// allow comparisons on an arbitrary field
float cmp(record*, record*, char*);

#endif
