#include <stdio.h>
#include <malloc.h> 
#include <pthread.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <math.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

Display *dspl; 
int screen;
Window hwnd; 
XEvent event; 
GC gc; 

typedef struct {
	int id; 
	float x, y; 
	float oldX, oldY; 
	int idGirl; 
	int idPlace; 
	int dancing; 
	int reverence; 
	pthread_mutex_t dance; 
	pthread_mutex_t wait; 
} Boy;

typedef struct {
	int id; 
	float x, y; 
	float oldX, oldY; 
	int idBoy; 
	pthread_mutex_t lock; 
	pthread_mutex_t wait; 
} Girl;

typedef struct {
	int id; 
	float x, y; 
	int busy;
} Place;

static int stop; 

static Place* places; 
static Boy* boys; 
static Girl* girls;

static int countBoys; 
static int countGirls; 
static int countDancing; 

void* BoyActions (void* thread_data){

	Boy *boy = (Boy*) thread_data; 

	int idGirl; 
	idGirl = rand()%countGirls; 
	float timeW1 = 1; 
	float timeW2 = 180; 

	while(1){

		while(girls[idGirl].idBoy != boy -> id){    
			pthread_mutex_lock(&girls[idGirl].lock); 
			girls[idGirl].idBoy = boy -> id; 
			pthread_mutex_unlock(&girls[idGirl].wait); 
			pthread_mutex_lock(&boy -> wait); 
			if(girls[idGirl].idBoy != boy -> id) pthread_mutex_unlock(&girls[idGirl].lock);
		}

		boy -> dancing = 1; 
		
		for(int i=0; i<12; i++){
			
			if(places[i].busy==0){
				boy->idPlace = places[i].id;
				boy->x = places[i].x;
				boy->y = places[i].y;
				places[i].busy = 1;
				break; 
			}
		}
		
		girls[idGirl].x = boy->x+20;
		girls[idGirl].y = boy->y;
		pthread_mutex_lock(&boy->dance); 

		while(stop==0){
		
			boy -> x = places[boy -> idPlace].x + 10 * cos (timeW1);
			boy -> y = places[boy -> idPlace].y + 10 * sin (timeW1);

			girls[idGirl].x = places[boy -> idPlace].x + 10 * cos (timeW2);
			girls[idGirl].y = places[boy -> idPlace].y + 10 * sin (timeW2);

			if(timeW1 == 360) timeW1 = 1;
			if(timeW2 == 360) timeW2 = 1;
			
			timeW1 += 0.01;
			timeW2 += 0.01;
			usleep(5000);
		}

		boy -> reverence = 1; 
		sleep(4); 
		boy -> reverence = 0; 

		for(int i=0; i<12; i++)
			if(boy->idPlace == places[i].id){
				places[i].busy = 0;
				break;
			}
			
		pthread_mutex_unlock(&girls[idGirl].wait); 
		pthread_mutex_unlock(&girls[idGirl].lock); 
		
		boy -> dancing = 0;
		boy -> x = boy -> oldX;
		boy -> y = boy -> oldY;
		sleep(1);

	}
}

void* GirlActions (void* thread_data){

	Girl *girl = (Girl*) thread_data; 

	int idBoy; 

	while(1){
		pthread_mutex_lock(&girl->wait); 
		idBoy = girl->idBoy - 1; 
			if (1 == rand()%8) girl->idBoy = -1; 
		pthread_mutex_unlock(&boys[idBoy].wait); 
		pthread_mutex_lock(&girl->wait);
		
		girl->x = girl->oldX;
		girl->y = girl->oldY;
		sleep(1);
	}
}

void *LogicActions (void *unused){

	while(countDancing>1){

		sleep(8); 
		stop = 0; 
		
		for(int i = 0; i < countBoys; i++)
		
			if(boys[i].dancing == 1)	pthread_mutex_unlock(&boys[i].dance);
		sleep(20);
		stop=1; 
		countDancing--; 
	}
}

void InitBoys(){

	boys = (Boy*) malloc(countBoys * sizeof(Boy)); 
	for(int i = 0; i < countBoys; i++){
		boys[i].id = i+1;
		boys[i].oldX = 150;
		boys[i].oldY = 100 + (i*20);
		boys[i].x = boys[i].oldX;
		boys[i].y = boys[i].oldY;
		boys[i].idGirl = -1;
		boys[i].idPlace = -1;
		boys[i].dancing = 0;
		pthread_mutex_lock(&boys[i].dance);
		pthread_mutex_lock(&boys[i].wait);
	}
}

void InitGirls(){

	girls = (Girl*) malloc(countGirls * sizeof(Girl));
	
	for(int i=0; i<countGirls; i++){
		girls[i].id = i+1;
		girls[i].oldX = 200 + (i*20)	;
		girls[i].oldY = 50;
		girls[i].x = girls[i].oldX;
		girls[i].y = girls[i].oldY;
		girls[i].idBoy = -1;
		pthread_mutex_lock(&girls[i].wait);
	}
}

void InitPlaces(){

	places = (Place*) malloc(12 * sizeof(Place));

	for(int i=0; i<12; i++){
		places[i].id=i;
		places[i].busy=0;
	}

	places[0].x = 250; places[0].y = 150;
	places[1].x = 350; places[1].y = 150;
	places[2].x = 450; places[2].y = 150;
	places[3].x = 250; places[3].y = 200;
	places[4].x = 350; places[4].y = 200;
	places[5].x = 450; places[5].y = 200;
	places[6].x = 250; places[6].y = 250;
	places[7].x = 350; places[7].y = 250;
	places[8].x = 450; places[8].y = 250;
	places[9].x = 250; places[9].y = 300;
	places[10].x = 350; places[10].y = 300;
	places[11].x = 450; places[11].y = 300;
}

void InitStreams(){

	pthread_t* threadsBoys = (pthread_t*) malloc(countBoys * sizeof(pthread_t)); 
	pthread_t* threadsGirls = (pthread_t*) malloc(countGirls * sizeof(pthread_t));
	pthread_t threadsLogic;

	for(int i=0; i<countBoys; i++)	pthread_create(&(threadsBoys[i]), NULL, BoyActions, &boys[i]);  
	for(int i=0; i<countGirls; i++)	pthread_create(&(threadsGirls[i]), NULL, GirlActions, &girls[i]); 
								
	pthread_create(&threadsLogic, NULL, LogicActions, NULL); 
}

void CreateWindow(){

	dspl = XOpenDisplay(NULL); 
  if (dspl == NULL) {
  printf("Error XOpenDisplay\n");
  exit(1);
  }
  screen = XDefaultScreen(dspl); 

  hwnd = XCreateSimpleWindow(dspl, RootWindow(dspl, screen), 100, 50,
  800, 600, 3, BlackPixel(dspl, screen),
  WhitePixel(dspl, screen)); 

  if (hwnd == 0) {
  printf("Error XCreateSimpleWindow\n");
  exit(1);
  }

  XSelectInput(dspl, hwnd, ExposureMask | StructureNotifyMask); 
  XMapWindow(dspl, hwnd); 
  gc = XDefaultGC(dspl, screen); 

  while(1) {
  XEvent event; 
  XNextEvent(dspl, &event); 
  if (MapNotify == event.type) break; 
  }
}

void Draw(){

	char bufStr[3];

	while (1){

		XDrawRectangle(dspl, hwnd, gc, 200, 100, 400, 400); 
		XDrawString(dspl, hwnd, gc, 350, 80,  "Count dance:", 12);

		sprintf(bufStr, "%d", countDancing); 

			if(countDancing < 10) XDrawString(dspl, hwnd, gc, 430, 80,  bufStr, 1);
			else if(countDancing > 9) XDrawString(dspl, hwnd, gc, 430, 80,  bufStr, 2);


		for(int i=0; i<countBoys; i++){
			XDrawString(dspl, hwnd, gc, boys[i].x, boys[i].y,  "B", 1);
			if(boys[i].reverence == 1)	XDrawString(dspl, hwnd, gc, boys[i].x, boys[i].y-10,  "Reverence..", 11);
		}
		
		
		for(int i=0; i<countGirls; i++)
			XDrawString(dspl, hwnd, gc, girls[i].x, girls[i].y,  "g", 1);
				
		XFlush(dspl); 
		XClearWindow(dspl, hwnd); 
		usleep(50000); 
	}
}

int main() {

	countBoys = 12;
	countGirls = 15;
	countDancing = 5;
	
	InitPlaces(); 
	InitBoys(); 
	InitGirls(); 
	CreateWindow(); 
	InitStreams(); 
	Draw();
	return 0;
	
}
