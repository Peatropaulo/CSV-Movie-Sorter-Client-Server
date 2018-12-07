struct mData{
	char color[20];
	char dName[30];
	int review;
	int duration;
	int dFbLikes; // director facebook likes
	int a3FbLikes; // actor 3 facebook likes
	char a2Name[30];
	int a1FbLikes; // actor 1 facebook likes
	int gross;
	char genres[50];
	char a1Name[30]; // actor 1 name
	char mTitle[200]; // movie title
	int votes;
	int castFbLikes;
	char a3Name[30];
	int facenum; // facenumber in poster
	char plot[80]; //plot key words
	char movielink[300];
	int userReview; //num of user reviews
	char language[15];
	char country[20];
	char cRating[8]; // content rating
	int budget;
	int tYear;
	int a2FbLikes; // actor 2 facebook likes
	float imdbScore;
	float aRatio; // aspect ratio
	int movieFbLikes; // movie facebook likes
	/*color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,
	actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,
	facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,
	country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes
	*/
};
struct fileNames{
	char name[100];
};


