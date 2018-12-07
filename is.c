#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

int getSize(const char* s) {
	int count = 0;
	while (*s) {
		count++;
		s++;
	}
	return count;
}

char* appendNull(const char * dir) {
	int size = getSize(dir);
	char* ret = (char*) malloc(1 + sizeof(char) * (size));
	memset(ret, '\0', size + 1);
	strncpy(ret, dir, size);
	return ret;
}

int getInputSize(const char *input) {

	int count = 1;

	while (*input)

	{

		if ((int) ',' == (int) *input)

			count++;

		input++;

	}

	return count;

}


int findInt(char* comp, char* cat[], int size) {

	int counter = 0;

	while (counter < size) {
		if (comp != NULL)
			if (strcmp(comp, cat[counter]) == 0)
				return counter;
		counter++;
	}
	return -1;
}

void getInput(int* input, const char *directory, char *cat[], int size, int i) {

	char * newDir = appendNull(directory);

	char *temp;
	temp = strtok(newDir, ",");


	int counter = 0;

	while (temp != NULL && counter < i) {
		printf("%d,",findInt(temp, cat, size));
		input[counter] = findInt(temp, cat, size);
		temp = strtok(NULL, ",");
		counter++;
	}

}

/*
 Ok so the code doesn't need a main to run it just need what is inside the main to run it returns a comparison array so all you have to do is put this section
 into you main code and it should take argv[2] and sort according to that input.
 You also dont need the while loop
 */
