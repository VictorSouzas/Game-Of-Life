#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
// #define DEBUG
/*
 * RA: 20866585	Victor de Mattos Souza
 * RA: 20751756 Matheus Felipe Fischer
 */

struct args{
	int i;
	int j;
	int size;
	char* buffer;
	char* nextGenBuffer;
};

int countNeighbors(char* buffer, int i, int j, int size);
void *aliveOrDeath(void *arg_p);
void nextGen(char* buffer, char* nextGenBuffer, int* generation, int size);
void draw(FILE* output, char* buffer, int size);
void update(char* buffer, char* nextGenBuffer, int size);
char get(char* buffer, int size, int i, int j);
void set(char* buffer, int size, int i, int j, char value);
int checksum(char* buffer, int size);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {

	char* buffer;
	char* nextGenBuffer;
	int generation = 0, i, j, maxgen, flag;
	char inp;
	int size = 6;
	flag = 1;
	maxgen = atoi(argv[2]);
	FILE * input = fopen(argv[1], "r");

	FILE * output = fopen("saida.txt", "w");
	if(input == 0 || output == 0){
		fprintf(stderr, "erro");
	}
	buffer =(char*) malloc((size*size) * sizeof(char));
	nextGenBuffer =(char*) malloc((size*size) * sizeof(char));
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			fscanf(input, "%c", &inp);
			set(buffer, size, i, j, inp);
			printf("%c", get(buffer, size, i, j)  );
		}
		printf("\n");
		fscanf(input, "\n");
	}
	
	while(flag && (generation < maxgen)) {
		update(buffer, nextGenBuffer, size);
		nextGen(buffer, nextGenBuffer, &generation, size);

		draw(output, buffer, size);
		flag = (checksum(buffer, size) == checksum(nextGenBuffer, size));
	}


	fclose(input);
	fclose(output);
	free(buffer);
	free(nextGenBuffer);
}
void nextGen(char* buffer, char* nextGenBuffer, int* generation, int size){
	for (int i = 0; i < size; ++i){
		for (int j = 0; j < size; ++j){
			set(buffer, size, i, j, get(nextGenBuffer, size, i, j));
		}
	}
	*generation = *generation + 1;
}

void update(char* buffer, char* nextGenBuffer, int size) {
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j = j + 3) {
			pthread_t thread_1, thread_2, thread_3;
			struct args args_1 = {i, j, size, buffer, nextGenBuffer};
			struct args args_2 = {i, j + 1, size, buffer, nextGenBuffer};
			struct args args_3 = {i, j + 2, size, buffer, nextGenBuffer};

			struct args *p_args_1 = &args_1;
			struct args *p_args_2 = &args_2;
			struct args *p_args_3 = &args_3;

			int one = pthread_create(&thread_1, NULL, aliveOrDeath, (void*) p_args_1);
			int two = pthread_create(&thread_2, NULL, aliveOrDeath, (void*) p_args_2);
			int tree = pthread_create(&thread_3, NULL, aliveOrDeath, (void*) p_args_3);

			pthread_join(one, NULL);
			pthread_join(two, NULL);
			pthread_join(tree, NULL); 
			#ifdef DEBUG
				draw(stdout, buffer, size);
			#endif
		}
	}
}
int countNeighbors(char* buffer, int i, int j, int size){
	int North = ((i == 0) ? 0 :  (get(buffer, size, i-1, j) == '1' ? 1 : 0));
    int NorthWest = ((i == 0 || j == 0) ? 0 : (get(buffer, size, i-1, j-1) == '1' ? 1 : 0));
    int NorthEast = ((i == 0 || j == size-1) ? 0 : (get(buffer, size, i-1, j+1) == '1' ? 1 : 0));
    int West = ((j == 0)? 0: (get(buffer, size, i, j-1) == '1' ? 1 : 0));
    int East = ((j == size-1)? 0 : (get(buffer, size, i, j+1) == '1' ? 1 : 0));
    int South = ((i == size-1)? 0 : (get(buffer, size, i+1, j) == '1' ? 1 : 0));
    int SouthWest = ((i == size-1 || j == 0)? 0 : (get(buffer, size, i+1, j-1) == '1' ? 1 : 0));
    int SouthEast = ((i == size-1 || j == size-1)? 0 : (get(buffer, size, i+1, j+1) == '1' ? 1 : 0));
    return North + South + East + West + NorthEast + NorthWest + SouthEast + SouthWest;
}
void *aliveOrDeath(void* arg_p){
	pthread_mutex_lock(&mutex);
	struct args *arg = arg_p;
	int count = countNeighbors(arg->buffer, arg->i, arg->j, arg->size);
	if((get(arg->buffer, arg->size, arg->i, arg->j) == '1') && (count > 3 || count < 2)){
		set(arg->nextGenBuffer, arg->size, arg->i, arg->j, '0');
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	if (get(arg->buffer, arg->size, arg->i, arg->j) == '0' && count == 3){
		set(arg->nextGenBuffer, arg->size, arg->i, arg->j, '1');
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	if(get(arg->buffer, arg->size, arg->i, arg->j) == '1' && (count <= 3 || count >= 2)){
		set(arg->nextGenBuffer, arg->size, arg->i, arg->j, '1');
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	set(arg->nextGenBuffer, arg->size, arg->i, arg->j, get(arg->buffer, arg->size, arg->i, arg->j));
	pthread_mutex_unlock(&mutex);
	return NULL;
}
void draw(FILE* output, char * buffer, int size) {
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			printf("%c", get(buffer, size, i, j));
			// fprintf(output,"%c", get(buffer, size, i, j));
		}
		printf("\n");
		// fprintf(output,"\n");
	}
}

char get(char* buffer, int size, int i, int j) {
	return *(buffer + ((i * size) + j));
}

void set(char* buffer, int size, int i, int j, char value) {
	*(buffer + ((i * size) + j)) = value;
}

int checksum(char* buffer, int size) {
	int i, j, sum = 0;
	for(i = 0; i < size; i++) {
		if (*(buffer + i) == '1')
			sum += i;
	}

	return sum;
}

