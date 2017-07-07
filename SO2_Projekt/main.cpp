#include <iostream>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define TimeToEnd 1000000

using namespace std;

pthread_mutex_t drawMutex = PTHREAD_MUTEX_INITIALIZER;	 //
pthread_mutex_t diggerMutex = PTHREAD_MUTEX_INITIALIZER; //
pthread_mutex_t digger2Mutex = PTHREAD_MUTEX_INITIALIZER; //
pthread_mutex_t mineMutex = PTHREAD_MUTEX_INITIALIZER; //
pthread_mutex_t eleMutex = PTHREAD_MUTEX_INITIALIZER; //



pthread_cond_t mine1Cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t mine2Cond = PTHREAD_COND_INITIALIZER;

struct digger {
	int size = 0;
};

struct train {
	int capacity = 0;
	int flag = 1;
};
struct power {
	int coal = 0;
};
struct mine {
	int coal = 300;
};
power ele;
digger digger1, digger2;
mine miner;
train train1, train2;

int neverEnd = 0;

//Watek koparki, która pobiera wegiel z kopalni nastepnie
//jesli pociag jest i nie jest pelny laduje na niego wegiel

void *digger1Thread(void *arg){
    do{
    	if(miner.coal >= 10)
    	{
    		if(digger1.size < 10)
    		{
    			 pthread_mutex_lock(&mineMutex);
    			 miner.coal--;
    			 pthread_mutex_unlock(&mineMutex);
    			 pthread_mutex_lock(&diggerMutex);
    			 digger1.size++;
    			 usleep(80000);
    			 pthread_mutex_unlock(&diggerMutex);

    			 pthread_mutex_lock(&drawMutex);
    			 mvprintw(4, 0, "Digger: ");
    			 mvprintw(4, 10, "%d ", digger1.size);
    			 pthread_mutex_unlock(&drawMutex);
    		}

    		if(digger1.size == 10)
    		{
    	    	pthread_mutex_lock(&diggerMutex);
    			while(train1.flag == 2){
    			 //kiedy pociag w kopalni
    			  pthread_cond_wait(&mine1Cond, &diggerMutex);
    			 }
    	    	pthread_mutex_unlock(&diggerMutex);

    			if(train1.capacity <100 && train1.flag == 1)
    			{
					pthread_mutex_lock(&diggerMutex);
					train1.capacity+=digger1.size;
					digger1.size = 0;
					pthread_mutex_unlock(&diggerMutex);
					usleep(80000);

					pthread_mutex_lock(&drawMutex);
					mvprintw(4, 25, "Train1: ");
					mvprintw(4, 35, "%d ", train1.capacity);
					pthread_mutex_unlock(&drawMutex);

    			}

    		}
    	}

    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

//Watek koparki, która pobiera wegiel z kopalni nastepnie
//jesli pociag jest i nie jest pelny laduje na niego wegiel

void *digger2Thread(void *arg){
    do{
    	if(miner.coal >= 10)
    	{
    		if(digger2.size < 10)
    		{
    			 pthread_mutex_lock(&mineMutex);
    			 miner.coal--;
    			 pthread_mutex_unlock(&mineMutex);
    			 pthread_mutex_lock(&digger2Mutex);
    			 digger2.size++;
    			 usleep(80000);
    			 pthread_mutex_unlock(&digger2Mutex);

    			 pthread_mutex_lock(&drawMutex);
    			 mvprintw(5, 0, "Digger2: ");
    			 mvprintw(5, 10, "%d ", digger2.size);
    			 pthread_mutex_unlock(&drawMutex);
    		}

    		if(digger2.size == 10)
    		{
    	    	pthread_mutex_unlock(&digger2Mutex);
    			while(train2.flag == 2){
    			 //kiedy pociag w kopalni
    			  pthread_cond_wait(&mine2Cond, &digger2Mutex);
    			 }
    	    	pthread_mutex_unlock(&digger2Mutex);

    			if(train2.capacity <100 && train2.flag == 1)
    			{
					pthread_mutex_lock(&digger2Mutex);
					train2.capacity+=digger2.size;
					digger2.size = 0;
					usleep(80000);
					pthread_mutex_unlock(&digger2Mutex);

					pthread_mutex_lock(&drawMutex);
					mvprintw(5, 25, "Train2: ");
					mvprintw(5, 35, "%d ", train2.capacity);
					pthread_mutex_unlock(&drawMutex);
    			}

    		}
    	}

    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

//Watek pociagu, odpowiada za ustalenie gdzie dany pociag się znajduje

void *trainThread(void *arg){
    do{

    	if(train1.capacity == 0)
    	{
    		pthread_mutex_lock(&diggerMutex);
    		train1.flag = 1;
    		pthread_cond_signal(&mine1Cond);
    		pthread_mutex_unlock(&diggerMutex);
    		usleep(7000);
    	}


    	if(train1.capacity == 100)
    	{
    		pthread_mutex_lock(&diggerMutex);
    		train1.flag = 2;
    		pthread_mutex_unlock(&diggerMutex);

    		usleep(7000);
    	}

    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

//Watek pociagu, odpowiada za ustalenie gdzie dany pociag się znajduje

void *train2Thread(void *arg){
	do{
			if(train2.capacity == 0)
			{

				pthread_mutex_lock(&digger2Mutex);
				train2.flag = 1;
				pthread_cond_signal(&mine2Cond);
				pthread_mutex_unlock(&digger2Mutex);

				usleep(7000);
			}


			if(train2.capacity == 100)
			{
				pthread_mutex_lock(&digger2Mutex);
				train2.flag = 2;
				pthread_mutex_unlock(&digger2Mutex);

				usleep(7000);
			}
	    }while( neverEnd != TimeToEnd);
	    pthread_exit(NULL);
}

//Watek elektrowni jesli pociag znajduje sie w elektrowni rozladowuje pociag. Tylko jeden na raz.

void *eleThread(void *arg){
    do{

    	if(train1.capacity > 0 && train1.flag==2)
		{
    		pthread_mutex_lock(&diggerMutex);

			train1.capacity--;
			pthread_mutex_unlock(&diggerMutex);
			pthread_mutex_lock(&eleMutex);
			ele.coal++;
			pthread_mutex_unlock(&eleMutex);
			usleep(16000);

			pthread_mutex_lock(&drawMutex);
			mvprintw(4, 25, "Train1: ");
			mvprintw(4, 35, "%d ", train1.capacity);
			pthread_mutex_unlock(&drawMutex);

			pthread_mutex_lock(&drawMutex);
			mvprintw(4, 45, "Ele: ");
			mvprintw(4, 50, "%d  ", ele.coal);
			pthread_mutex_unlock(&drawMutex);
		}
    	else if(train2.capacity > 0 && train2.flag==2)
    	{
    		pthread_mutex_lock(&digger2Mutex);

    		train2.capacity--;
    		pthread_mutex_unlock(&digger2Mutex);
    		pthread_mutex_lock(&eleMutex);
			ele.coal++;

			pthread_mutex_unlock(&eleMutex);
			usleep(16000);

			pthread_mutex_lock(&drawMutex);
			mvprintw(5, 25, "Train2: ");
			mvprintw(5	, 35, "%d ", train2.capacity);
			pthread_mutex_unlock(&drawMutex);

			pthread_mutex_lock(&drawMutex);
			mvprintw(4, 45, "Ele: ");
			mvprintw(4, 50, "%d  ", ele.coal);
			pthread_mutex_unlock(&drawMutex);
    	}

    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

//Watek nautry, ktory zapobiega zaglodzeniu pozostalych watkow

void *natureThread(void *arg){
    do{
    	if(miner.coal < 100)
    	{
    		miner.coal += 1000;
    	}
    	usleep(5000);
    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

//Watek czasy, ktory zapobiega przedwczesnemu zakonczeniu programu

void *timeThread(void *arg){
    do{
        neverEnd++;
        usleep(10000);
    }while( neverEnd != TimeToEnd);
    pthread_exit(NULL);
}

int main() {

	pthread_t tra1, tra2, digg1, digg2, time, elec, nature;

	//Inicjowanie ncurses
	initscr();
	curs_set(FALSE);
	//Odpalanie watkow
	if(pthread_create(&time, NULL, timeThread, NULL)){
		printf("Error, watek timeThread");
		abort();
	}
	if(pthread_create(&digg1, NULL, digger1Thread, NULL)){
		printf("Error, watek digger1Thread");
		abort();
	}
	if(pthread_create(&digg2, NULL, digger2Thread, NULL)){
		printf("Error, watek digger2Thread");
		abort();
	}
	if(pthread_create(&tra1, NULL, trainThread, NULL)){
		printf("Error, watek trainThread");
		abort();
	}
	if(pthread_create(&tra2, NULL, train2Thread, NULL)){
		printf("Error, watek train2Thread");
		abort();
	}
	if(pthread_create(&elec, NULL, eleThread, NULL)){
		printf("Error, watek eleThread");
		abort();
	}
	if(pthread_create(&nature, NULL, natureThread, NULL)){
		printf("Error, watek natureThread");
		abort();
	}
	//Rysowanie przy pomocy ncurses
	while(neverEnd != TimeToEnd){

		pthread_mutex_lock(&drawMutex);

		mvprintw(0, 0, "Czas: ");
		mvprintw(0, 25, "%d 10^-2s", neverEnd);

		mvprintw(3, 0, "Wegiel w kopalni: ");
		mvprintw(3, 25, "%d ", miner.coal);


		mvprintw(7, 0, "MINE");
		mvprintw(7, 50, "ELECTRO");
		mvprintw(8, 21,  "====================================");
		mvprintw(11, 21, "====================================");
		if(train1.flag == 1)
		{
			mvprintw(8, 22, "<1===%d% >",train1.capacity);
			mvprintw(8, 50, "          ");
		}
		if(train1.flag == 2)
		{
			mvprintw(8, 22, "          ");
			mvprintw(8, 50, "<1===%d% >",train1.capacity);
		}
		if(train2.flag == 1)
		{
			mvprintw(11, 22, "<2===%d% >",train2.capacity);
			mvprintw(11, 50, "          ");
		}
		if(train2.flag == 2)
		{
			mvprintw(11, 50, "<2===%d% >",train2.capacity);
			mvprintw(11, 22, "          ");
		}
		if(digger1.size < 10)
		{
			mvprintw(8, 0, ">|%d0% |",digger1.size);
			mvprintw(8, 13,"         ");
		}
		else
		{
			mvprintw(8, 13, "|%d0% |<",digger1.size);
			mvprintw(8, 0,"         ");
		}
		if(digger2.size < 10)
		{
			mvprintw(11, 0, ">|%d0% |",digger2.size);
			mvprintw(11, 13,"         ");
		}
		else
		{
			mvprintw(11, 13, "|%d0% |<",digger2.size);
			mvprintw(11, 0,"         ");
		}

		refresh();
		pthread_mutex_unlock(&drawMutex);
		}
	//Konczenie watkow
	 if ( pthread_join (time , NULL ) ) {
		printf("Error, watek time");
		abort();
	 }
	 if ( pthread_join (tra1 , NULL ) ) {
		printf("Error, watek tra1");
		abort();
	 }
	 if ( pthread_join (tra2 , NULL ) ) {
		printf("Error, watek tra2");
		abort();
	 }
	 if ( pthread_join (digg1 , NULL ) ) {
		printf("Error, watek digger1");
		abort();
	 }
	 if ( pthread_join (digg2 , NULL ) ) {
		printf("Error, watek digger2");
		abort();
	 }
	 if ( pthread_join (elec , NULL ) ) {
		printf("Error, watek elec");
		abort();
	}
	 if ( pthread_join (nature , NULL ) ) {
		printf("Error, watek nature");
		abort();
	 }
	 //Destrukcja mutexow
	if (pthread_mutex_destroy(&drawMutex)) {
		cout << "blad przy destrukcji drawMutex.";
		abort();
	}

	if (pthread_mutex_destroy(&mineMutex)) {
		cout << "blad przy destrukcji mineMutex.";
		abort();
	}
	if (pthread_mutex_destroy(&diggerMutex)) {
		cout << "blad przy destrukcji diggerMutex.";
		abort();
	}
	if (pthread_mutex_destroy(&eleMutex)) {
		cout << "blad przy destrukcji eleMutex.";
		abort();
	}

	 //Destrukcja zmiennych warunkowej

	if (pthread_cond_destroy(&mine1Cond)) {
		cout<<"blad przy destrukcji mine1Cond.";
		abort();
	}
	if (pthread_cond_destroy(&mine2Cond)) {
		cout<<"blad przy destrukcji mine2Cond.";
		abort();
	}

	endwin();
	return 0;
}
