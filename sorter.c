#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
struct arraySort{
	struct mData* array;
	int start;
	int end;
	int comp_ptr;
};

int compare(struct mData leftArr, struct mData rightArr, int comp_ptr) {
	switch (comp_ptr) {
	case 0:
		return strcmp(leftArr.color, rightArr.color);
		break;
	case 1:
		return strcmp(leftArr.dName, rightArr.dName);
		break;
	case 2:
		return (leftArr.review) - (rightArr.review);
		break;
	case 3:
		return (leftArr.duration) - (rightArr.duration);
		break;
	case 4:
		return (leftArr.dFbLikes) - (rightArr.dFbLikes);
		break;
	case 5:
		return (leftArr.a3FbLikes) - (rightArr.a3FbLikes);
		break;
	case 6:
		return strcmp(leftArr.a2Name, rightArr.a2Name);
		break;
	case 7:
		return (leftArr.a1FbLikes) - (rightArr.a1FbLikes);
		break;
	case 8:
		return (leftArr.gross) - (rightArr.gross);
		break;
	case 9:
		return strcmp(leftArr.genres, rightArr.genres);
		break;
	case 10:
		return strcmp(leftArr.a1Name, rightArr.a1Name);
		break;
	case 11:
		return strcmp(leftArr.mTitle, rightArr.mTitle);
		break;
	case 12:
		return (leftArr.votes) - (rightArr.votes);
		break;
	case 13:
		return (leftArr.castFbLikes) - (rightArr.castFbLikes);
		break;
	case 14:
		return strcmp(leftArr.a3Name, rightArr.a3Name);
		break;
	case 15:
		return (leftArr.facenum) - (rightArr.facenum);
		break;
	case 16:
		return strcmp(leftArr.plot, rightArr.plot);
		break;
	case 17:
		return strcmp(leftArr.movielink, rightArr.movielink);
		break;
	case 18:
		return (leftArr.userReview) - (rightArr.userReview);
		break;
	case 19:
		return strcmp(leftArr.language, rightArr.language);
		break;
	case 20:
		return strcmp(leftArr.country, rightArr.country);
		break;
	case 21:
		return strcmp(leftArr.cRating, rightArr.cRating);
		break;
	case 22:
		return (leftArr.budget) - (rightArr.budget);
		break;
	case 23:
		return (leftArr.tYear) - (rightArr.tYear);
		break;
	case 24:
		return (leftArr.a2FbLikes) - (rightArr.a2FbLikes);
		break;
	case 25:
		return (int) (leftArr.imdbScore - rightArr.imdbScore);
		break;
	case 26:
		return (int) (leftArr.aRatio - rightArr.aRatio);
		break;
	case 27:
		return leftArr.movieFbLikes - rightArr.movieFbLikes;
		break;
	default:
		return (leftArr.review) - (rightArr.review);
		break;

	}
}
void swap(struct mData* a, struct mData* b)
{
	struct mData temp = *a;
	*a = *b;
	*b = temp;
}

int randPartition(struct mData* array,int left, int right,int comp_ptr)
{

	int pivotPoint = ( rand() % (right-left+1)) + left;
	swap(&array[right],&array[pivotPoint]);

	int index = left;
	int i = left;
	for(;i<right;i++)
	{
		if(compare(array[i],array[right],comp_ptr) < 0)//array[i] < array[right])
		{
			swap(&array[i],&array[index]);
			index++;
		}
	}

		swap(&array[index],&array[right]);

		return index;
}

void quickSort(struct mData *array,int start, int end,int comp_ptr)
{
	if(start < end)
	{
		int mid = randPartition(array,start,end,comp_ptr);
		quickSort(array,start,mid-1,comp_ptr);
		quickSort(array,mid+1,end,comp_ptr);
	}
}
