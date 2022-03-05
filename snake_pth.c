#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

//#define WORLD_WIDTH 62
//#define WORLD_HEIGHT 22

int WORLD_WIDTH = 62;
int *ptr = &WORLD_WIDTH;

int WORLD_HEIGHT = 22;
int *ptr2 = &WORLD_HEIGHT;

int NB_SERPENTS = 3;
int *ptr3 = &NB_SERPENTS;

#define SNAKEY_LENGTH 8

//functions prototype
void* get_input(void* rank);
void* move_snake(void* rank);
void* verification_fruit(void* rank);
void reset_for_next_round();
void reset_all();
void initialisation();
void move_it(int number);
void check_collision();
void print_winner();
int my_random(int nb);
int nb_intervalle(int min, int max);
void verif_nb_point();
void mur_percuter();


int hasard =0;
int tot_score;

typedef enum direct {up,down,left,right} direction;

typedef struct spart {
    int x;
    int y;
} part;

WINDOW *snakes_world;
WINDOW *score_board;
WINDOW *message_board;
WINDOW *first_board;
WINDOW *final_board;


struct walls {
    part place[100];
};

struct walls wall[13];
size_t size = sizeof( part ) * 100;

struct frogs_mines {
    part place [5];
    bool alive [5];
};

struct fruit {
    part place[1];
    bool alive[1];
    char lettre[2];
};
struct fruit unFruit;

struct frogs_mines frog, mine;

char tabF[5]={'F','C','B','P'};


struct serpent{
    pthread_t sep[4];
    part object [SNAKEY_LENGTH];
    char face;
    int dir;
    int size;
    int score;
    int speed_counter;
    char head;
    bool alive;
};

struct serpent leserp;

struct snakes {
    part object [SNAKEY_LENGTH];
    char face;
    int dir;
    int size;
    int score;
    int speed_counter;
    char head;
    bool alive;
    int cpt_percute;
};

struct snakes snake[3];

int offsetx, offsety;
int ch;
int round_number;
int indicator;
bool restart, quit, restart_2, quit_2;
bool Pause;

int main(int argc, char **argv) {

    int option = 0;
	int Longueur = 62;
	int Largeur = 22;
	int Nb_serpent = 4;

    while ((option = getopt(argc, argv,"L:l:s:")) != -1) {
        switch (option) {
            case 'L' : 
				Longueur = atoi(optarg); 
                if(Longueur<62){
				printf("Longeur minimun 62 cm");
				exit(EXIT_FAILURE);
				return 0;
			}
                *ptr = Longueur;
                
            break;
			case 'l' :
				Largeur = atoi(optarg);
                if(Largeur<22){
				printf("Largeur minimun 22 cm");
				exit(EXIT_FAILURE);
				return 0;
			}
                *ptr2 = Largeur;
			break;
			case 's' :
			Nb_serpent = atoi(optarg);
			if(Nb_serpent>4){
				printf("Maximun 4 serpents");
				exit(EXIT_FAILURE);
				return 0;
			}
            *ptr3 = Nb_serpent;
			break;
			default:
				exit(EXIT_FAILURE);
			//return 0;
        }
    }
    
    initialisation();
    pthread_t thread_handle_1, thread_handle_2, thread_handle_3;
    while(COLS < Longueur || LINES < Largeur){
        clear();
        mvprintw(1, 1, "Agrandissez l'écran...");
        refresh();
        
        usleep(100000);

    }
    
    offsetx = (COLS - *ptr) / 2;
    offsety = (LINES - *ptr2) / 2;
    

    snakes_world = newwin(*ptr2, *ptr, offsety, offsetx);
    score_board = newwin(3, *ptr, offsety + *ptr2, offsetx);
    message_board = newwin(5, *ptr, offsety - 5 , offsetx);
    first_board = newwin(10, *ptr, (LINES - 10) / 2, offsetx);
    final_board = newwin(7, *ptr, (LINES - 7) / 2, offsetx);
    quit = FALSE;
    
    quit_2 = FALSE;
    
    restart = FALSE;
    
    restart_2 = FALSE;
    snake[0].face = '#';
    snake[1].face = '&';
    snake[2].face = 'O';
    while(quit_2 == FALSE){
        clear();
        refresh();
        wclear(first_board);
        mvwprintw(first_board, 8, 1, "PRESS s to START the GAME...");
        mvwprintw(first_board, 1, 1, "Snake_0 is made of:");
        mvwaddch(first_board, 1, 21, snake[0].face | COLOR_PAIR(3));
        mvwprintw(first_board, 2, 1, "Snake_0 moving keys are: W & S & A & D");
        mvwprintw(first_board, 3, 1, "Snake_1 is made of:");
        mvwaddch(first_board, 3, 21, snake[1].face | COLOR_PAIR(5));
        mvwprintw(first_board, 4, 1, "Snake_1 moving keys are: I & J & K & L");
        mvwprintw(first_board, 5, 1, "Snake_2 is made of:");
        mvwaddch(first_board, 5, 21, snake[2].face | COLOR_PAIR(6));
        mvwprintw(first_board, 6, 1, "Snake_2 moving keys are: ARROW Keys");
        box(first_board, 0, 0);
        wrefresh(first_board);
        while(1){
            ch = getch();
            if (ch == 's')
                break;
            if (ch == 'S')
                break;
        }
        clear();
        refresh();
        wclear(message_board);
        reset_all();
        pthread_create(&thread_handle_1, NULL, get_input, (void*)0);
        pthread_create(&thread_handle_2, NULL, move_snake, (void*)1);
        pthread_create(&thread_handle_3, NULL, verification_fruit, (void*)2);
        
        pthread_join(thread_handle_1, NULL);
        pthread_join(thread_handle_2, NULL);
        pthread_join(thread_handle_3, NULL);
        if(round_number == 5){
            clear();
            refresh();
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "PRESS r to RESTART...");
            mvwprintw(message_board, 3, 1, "PRESS q to QUIT...");
            box(message_board, 0, 0);
            wrefresh(message_board);
            print_winner();
        }
        if(restart_2 != TRUE && quit_2 != TRUE){
            while(1){
                ch = getch();
                if (ch == 'r')
                    break;
                if (ch == 'R')
                    break;
                if (ch == 'q'){
                    quit_2 = TRUE;
                    break;
                }
                if (ch == 'Q'){
                    quit_2 = TRUE;
                    break;
                }
            }
        }
    }
    delwin(snakes_world);
    delwin(score_board);
    delwin(message_board);
    delwin(first_board);
    delwin(final_board);
    endwin();

    return 0;

}


void print_winner(){
    wclear(final_board);
    mvwprintw(final_board, 1, 1, "Snake_0 Score: %d", snake[0].score);
    mvwprintw(final_board, 2, 1, "Snake_1 Score: %d", snake[1].score);
    mvwprintw(final_board, 3, 1, "Snake_2 Score: %d", snake[2].score);
    if(snake[0].score > snake[1].score && snake[0].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_0 is Winner");
                  
    else if(snake[1].score > snake[0].score && snake[1].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_1 is Winner");
                  
    else if(snake[2].score > snake[0].score && snake[2].score > snake[1].score)
        mvwprintw(final_board, 5, 1, "Snake_2 is Winner");
    else
        mvwprintw(final_board, 5, 1, "There is No Winner");
    box(final_board, 0, 0);
    wrefresh(final_board);
    
}
void* move_snake(void* rank) {
    
    int i, j, k;
    int nb=4;
    hasard = my_random(nb);
    tot_score=nb_intervalle(50,99);
    while(round_number <= 4 && quit_2 == FALSE  && restart_2 == FALSE){
        wclear(snakes_world);
        wclear(score_board);
        for(j = 0; j < 4; j++){
            if(snake[j].alive)
                move_it(j);
        }

        indicator++;
        if(indicator == 5)
            indicator = 1;
            
    
        /*for(i = 0; i < 5; i++)
        {
            //show all the alive mines with red 'M'
            if(mine.alive[i] == TRUE)
                mvwaddch(snakes_world, (mine.place[i]).y, (mine.place[i]).x, 'M' | COLOR_PAIR(1));
        }*/
        
        
        if(hasard==0){
            mvwaddch(snakes_world, (unFruit.place[0]).y, (unFruit.place[0]).x, tabF[0] | COLOR_PAIR(5));
            unFruit.lettre[0]= tabF[0];
        }
        if(hasard==1){
            mvwaddch(snakes_world, (unFruit.place[0]).y, (unFruit.place[0]).x, tabF[1] | COLOR_PAIR(1));
            unFruit.lettre[0]= tabF[1];
        }
        if(hasard==2){
            mvwaddch(snakes_world, (unFruit.place[0]).y, (unFruit.place[0]).x, tabF[2] | COLOR_PAIR(3));
            unFruit.lettre[0]= tabF[2];
        }
        if(hasard==3){
            mvwaddch(snakes_world, (unFruit.place[0]).y, (unFruit.place[0]).x, tabF[3] | COLOR_PAIR(2));
            unFruit.lettre[0]= tabF[3];
        }
        

       int x, y;
        x = 1; 
        y = 1;
        int x1 = *ptr-2;
        int y1 = *ptr2-2;
        if(round_number==2){
            
            for(i = 0; i < *ptr2; ++i){
                (wall[0].place[i]).x = x;
                (wall[0].place[i]).y = y + i;
                mvwaddch(snakes_world, (wall[0].place[i]).y, (wall[0].place[i]).x, '|' | COLOR_PAIR(4));
                (wall[1].place[i]).x = x1;
                (wall[1].place[i]).y = y + i;
                mvwaddch(snakes_world, (wall[1].place[i]).y, (wall[1].place[i]).x, '|' | COLOR_PAIR(4));
            }
            //lignes horizontales 
            for(j=1;j<*ptr;++j){
                (wall[3].place[j]).x = x + j;
                (wall[3].place[j]).y = y;
                mvwaddch(snakes_world, (wall[3].place[j]).y, (wall[3].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[4].place[j]).x = x + j;
                (wall[4].place[j]).y = *ptr2-2;
                mvwaddch(snakes_world, (wall[4].place[j]).y, (wall[4].place[j]).x, '|' | COLOR_PAIR(4));
            }
        }
        if(round_number==3){
            //murs (obstacles carrés du haut)
            for(int b=0; b< 3; b++){
                (wall[5].place[j]).x = x + 3+b;
                (wall[5].place[j]).y = y+2;
                mvwaddch(snakes_world, (wall[5].place[j]).y, (wall[5].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[6].place[j]).x = x1 - 5 +b;
                (wall[6].place[j]).y = y+2;
                mvwaddch(snakes_world, (wall[6].place[j]).y, (wall[6].place[j]).x, '|' | COLOR_PAIR(4));
            }
            for(int b=0; b< 3; b++){
                (wall[7].place[j]).x = x + 3+b;
                (wall[7].place[j]).y = y+3;
                mvwaddch(snakes_world, (wall[7].place[j]).y, (wall[7].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[8].place[j]).x = x1 - 5 +b;
                (wall[8].place[j]).y = y+3;
                mvwaddch(snakes_world, (wall[8].place[j]).y, (wall[8].place[j]).x, '|' | COLOR_PAIR(4));
            }

            //murs (obstacles carrés du bas) 
            for(int b=0; b< 3; b++){
                (wall[9].place[j]).x = x + 3+b;
                (wall[9].place[j]).y = *ptr2-4;
                mvwaddch(snakes_world, (wall[9].place[j]).y, (wall[9].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[10].place[j]).x = x1 - 5 +b;
                (wall[10].place[j]).y = *ptr2-4;
                mvwaddch(snakes_world, (wall[10].place[j]).y, (wall[10].place[j]).x, '|' | COLOR_PAIR(4));
            }
            for(int b=0; b< 3; b++){
                (wall[11].place[j]).x = x + 3+b;
                (wall[11].place[j]).y = *ptr2-5;
                mvwaddch(snakes_world, (wall[11].place[j]).y, (wall[11].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[12].place[j]).x = x1 - 5 +b;
                (wall[12].place[j]).y = *ptr2-5;
                mvwaddch(snakes_world, (wall[12].place[j]).y, (wall[12].place[j]).x, '|' | COLOR_PAIR(4));
            }
        }
        if(round_number ==4){
            //ligne verticale
            for(i = 0; i < *ptr2; ++i){
                (wall[0].place[i]).x = x;
                (wall[0].place[i]).y = y + i;
                mvwaddch(snakes_world, (wall[0].place[i]).y, (wall[0].place[i]).x, '|' | COLOR_PAIR(4));
                (wall[1].place[i]).x = x1;
                (wall[1].place[i]).y = y + i;
                mvwaddch(snakes_world, (wall[1].place[i]).y, (wall[1].place[i]).x, '|' | COLOR_PAIR(4));
            }
            //lignes horizontales 
            for(j=1;j<*ptr;++j){
                (wall[3].place[j]).x = x + j;
                (wall[3].place[j]).y = y;
                mvwaddch(snakes_world, (wall[3].place[j]).y, (wall[3].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[4].place[j]).x = x + j;
                (wall[4].place[j]).y = *ptr2-2;
                mvwaddch(snakes_world, (wall[4].place[j]).y, (wall[4].place[j]).x, '|' | COLOR_PAIR(4));
            }

            //murs (obstacles carrés du haut)
            for(int b=0; b< 3; b++){
                (wall[5].place[j]).x = x + 3+b;
                (wall[5].place[j]).y = y+2;
                mvwaddch(snakes_world, (wall[5].place[j]).y, (wall[5].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[6].place[j]).x = x1 - 5 +b;
                (wall[6].place[j]).y = y+2;
                mvwaddch(snakes_world, (wall[6].place[j]).y, (wall[6].place[j]).x, '|' | COLOR_PAIR(4));
            }
            for(int b=0; b< 3; b++){
                (wall[7].place[j]).x = x + 3+b;
                (wall[7].place[j]).y = y+3;
                mvwaddch(snakes_world, (wall[7].place[j]).y, (wall[7].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[8].place[j]).x = x1 - 5 +b;
                (wall[8].place[j]).y = y+3;
                mvwaddch(snakes_world, (wall[8].place[j]).y, (wall[8].place[j]).x, '|' | COLOR_PAIR(4));
            }

            //murs (obstacles carrés du bas) 
            for(int b=0; b< 3; b++){
                (wall[9].place[j]).x = x + 3+b;
                (wall[9].place[j]).y = *ptr2-4;
                mvwaddch(snakes_world, (wall[9].place[j]).y, (wall[9].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[10].place[j]).x = x1 - 5 +b;
                (wall[10].place[j]).y = *ptr2-4;
                mvwaddch(snakes_world, (wall[10].place[j]).y, (wall[10].place[j]).x, '|' | COLOR_PAIR(4));
            }
            for(int b=0; b< 3; b++){
                (wall[11].place[j]).x = x + 3+b;
                (wall[11].place[j]).y = *ptr2-5;
                mvwaddch(snakes_world, (wall[11].place[j]).y, (wall[11].place[j]).x, '|' | COLOR_PAIR(4));
                (wall[12].place[j]).x = x1 - 5 +b;
                (wall[12].place[j]).y = *ptr2-5;
                mvwaddch(snakes_world, (wall[12].place[j]).y, (wall[12].place[j]).x, '|' | COLOR_PAIR(4));
            }
            
        }

        check_collision();
        box(snakes_world, 0, 0);
        box(score_board, 0, 0);
        wrefresh(snakes_world);
        wrefresh(score_board);
        if(quit == FALSE && restart == FALSE && Pause == FALSE){
            wclear(message_board);
            
            mvwprintw(message_board, 1, 1, "Appuye sur r pour recommencer...");
            mvwprintw(message_board, 2, 1, "Appue sur q pour quitter...");
            mvwprintw(message_board, 3, 1, "Appuye p mettre sur pause...");
            
            box(message_board, 0, 0);
            
            wrefresh(message_board);
        }
        if(quit == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "Es-tu sûr de vouloir quitter ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            while(quit == TRUE && quit_2 == FALSE);
        }
        if(restart == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "Es-tu sûr de vouloir recommencer ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            while(restart == TRUE && restart_2 == FALSE);
        }
        if(Pause == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "PRESS r to RESUME...");
            box(message_board, 0, 0);
            wrefresh(message_board);
            while(Pause == TRUE);
        }
        if(restart_2 == FALSE && quit_2 == FALSE)
            usleep(500000);
        
    }
}

void* verification_fruit(void* rank){
    int i, j, k, m, x, y, wall_0, wall_1, wall_2, wall_3, wall_4;
    while(round_number <= 4 && quit_2 == FALSE && restart_2 == FALSE){
        
            //if a frog is dead, go in the while and don't come out until the frog becomes alive
            //si une grenouille est morte, allez-y pendant un moment et ne sortez pas jusqu'à ce que la grenouille devienne vivante
            //while(frog.alive[m] == FALSE)
            
            while(unFruit.alive[0] == FALSE){
                x = rand() % 60 + 1;
                y = rand() % 20 + 1;
                for(j = 0; j < 3; ++j){
                    if(snake[j].alive == FALSE)
                        continue;
                    for(i = 0; i < snake[j].size; ++i){
                        if((snake[j].object[i]).x == x && (snake[j].object[i]).y == y)
                            break;
                    }
                    if(i < snake[j].size)
                        break;
                }
                if(j == 3){
                    for(wall_0 = 0; wall_0 < *ptr; ++wall_0)
                    {
                        if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                            break;
                    }
                    for(wall_1 = 0; wall_1 < *ptr; ++wall_1){
                        if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                            break;
                    }
                    for(wall_2 = 0; wall_2 < *ptr; ++wall_2){
                        if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                            break;
                    }
                    for(wall_3 = 0; wall_3< *ptr; ++wall_3){
                        if((wall[3].place[wall_3]).x == x && (wall[3].place[wall_3]).y == y)
                            break;
                    }
                    for(wall_4 = 0; wall_4 < *ptr; ++wall_4){
                        if((wall[4].place[wall_4]).x == x && (wall[4].place[wall_4]).y == y)
                            break;
                    }
                }

                //vérifier si le lieu aléatoire choisi n'est pas sur une grenouille vivante
                /*if(j == 3 && wall_0 == 8 && wall_1 == 5 && wall_2 == 5)
                {
                    for(i = 0; i < 5; ++i)
                    {
                        if((frog.place[i]).x == x && (frog.place[i]).y == y && frog.alive[i] == TRUE)
                            break;
                    }
                }*/
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
                {
                    for(k = 0; k < 5; ++k)
                    {
                        if((mine.place[k]).x == x && (mine.place[k]).y == y && mine.alive[k] == TRUE)
                            break;
                    }
                }
                
                if(j == 3 && wall_0 == *ptr && wall_1 == *ptr && wall_2 == *ptr && wall_3 == *ptr && wall_4 == *ptr){
                (unFruit.place[0]).x = x;
                (unFruit.place[0]).y = y;
                unFruit.alive[0] = TRUE;
                int alea= my_random(4);
                hasard = alea;
                }

                /*if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5 && k == 5)
                {
                    (frog.place[m]).x = x;
                    (frog.place[m]).y = y;
                    (unFruit.place[m]).x = x;
                    (unFruit.place[m]).y = y;
                    frog.alive[m] = TRUE;
                }*/
            }
               
    }
}

int my_random(int nb){
    //srand(time(NULL));
    int alea=(rand()%nb);
    return alea;
}

int nb_intervalle(int min, int max){
    //srand(time(NULL));
    int alea=min+ (rand()%(max + 1 - min));
    return alea;
}

void verif_nb_point(){
    for(int i=0; i<3; i++){
        if(snake[i].score > tot_score + 45){
            snake[i].alive = FALSE;
        }
    }
}

void mur_percuter(){
    for(int i=0; i< 3; i++){
        if(snake[i].cpt_percute >= 3){
            snake[i].alive = FALSE;
        }
    }
}

void check_collision(){
    int i, j, k;
    for(i=0; i<3; i++){
        if((snake[i].object[0]).x == (unFruit.place[0]).x && (snake[i].object[0]).y == (unFruit.place[0]).y && snake[i].alive == TRUE && unFruit.alive[0] == TRUE){
            unFruit.alive[0] = FALSE;
            //mvwaddch(snakes_world, (unFruit.place[0]).y, (unFruit.place[0]).x, '@' | COLOR_PAIR(7));
            if(unFruit.lettre[0]=='F'){
                snake[i].score+=5;
                snake[i].size++;
            }
            if(unFruit.lettre[0]=='C'){
                snake[i].score+=3;
            }
            if(unFruit.lettre[0]=='B'){
                snake[i].score+=2;
                snake[i].size++;
            }
            if(unFruit.lettre[0]=='P'){
                snake[i].score+=1;
                snake[i].size++;
            }
        }
    }
    verif_nb_point();

    //Nombre de murs
    for(i = 0; i < 13; i++){
        //Taille du mur en par rapport au i 
        for(k = 0; k < *ptr; ++k){
            //Nombre du serpent
            for(j = 0; j < 3; ++j){
                //if a snake has hit a wall, it becomes dead
                if((snake[j].object[0]).x == (wall[i].place[k]).x && (snake[j].object[0]).y == (wall[i].place[k]).y && snake[j].alive == TRUE){
                    //snake[j].alive = FALSE;
                    snake[j].score -= 15;
                    snake[j].cpt_percute += 1;
                }
                           
            }
        }
    }

    mur_percuter();
    
    for(j = 0; j < 3; ++j)
    {
        for(i = 1; i < snake[j].size; i++)
        {
            //if a snake has hit itself, it becomes dead
            if((snake[j].object[0]).x == (snake[j].object[i]).x && (snake[j].object[0]).y == (snake[j].object[i]).y)
                snake[j].alive = FALSE;
        }
    }
    //Condition pour passer au round suivant
    //if 2 snakes become dead, the game goes to next round
    for(int i=0; i < 3; i++){
        if(snake[i].score == tot_score){

            round_number++;

            //Nettoyer la map, c'est-à-dire effacer les murs
            for(int h=0; h<5; h++){
            memset(wall[h].place, ' ', size); 
            }

            reset_for_next_round();

        }
    }
    /*if(((snake[0].alive == FALSE) && (snake[1].alive == FALSE))|((snake[0].alive == FALSE) && (snake[2].alive == FALSE))|((snake[1].alive == FALSE) && (snake[2].alive == FALSE)))
    {
        //increase round
        round_number++;
        //reset all the variables for next round


        //Nettoyer la map, c'est-à-dire effacer les murs
        for(int h=0; h<5; h++){
           memset(wall[h].place, ' ', size); 
        }
        
        reset_for_next_round();
    }*/

    //if the round number is less than 5, show the score_board
    if(round_number <= 4)
        mvwprintw(score_board, 1, 1, "|snake_0: %d||snake_1: %d||snake_2: %d||round: %d| tot: %d ", snake[0].score, snake[1].score, snake[2].score, round_number, tot_score);
}


void move_it(int number)
{
    int i, j;
    //move the snake 1 unit
    if((indicator == 1 && snake[number].speed_counter == 0) || (indicator == 3 && snake[number].speed_counter == 0) || (indicator == 1 && snake[number].speed_counter < 0) || snake[number].speed_counter > 0)
    {
        //if snake size is less than 8, increase snake size
        if(snake[number].size < 2)
            (snake[number].size)++;
        //move the snakes part 1 unit forward
        for (i = 7; i > 0; i--)
            (snake[number].object[i]) = (snake[number].object[i-1]);
        //show all the snakes parts but head
        for (i = 1; i < snake[number].size; i++)
            mvwaddch(snakes_world, (snake[number].object[i]).y, (snake[number].object[i]).x, snake[number].face | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                                   (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
        
        int x = (snake[number].object[0]).x;
        int y = (snake[number].object[0]).y;
        //make head according to the user pressed key
        switch (snake[number].dir)
        {
            case up:
               // y - 1 == 0 ? y = WORLD_HEIGHT - 2 : y--;
                y - 1 == 0 ? y = *ptr2 - 2 : y--;
                snake[number].head = '^';
                break;
            case down:
                //y + 1 == WORLD_HEIGHT - 1 ? y = 1 : y++;
                y + 1 == *ptr2 - 1 ? y = 1 : y++;
                snake[number].head = 'v';
                break;
            case right:
               // x + 1 == WORLD_WIDTH - 1 ? x = 1 : x++;
                x + 1 == *ptr - 1 ? x = 1 : x++;
                snake[number].head = '>';
                break;
            case left:
                //x - 1 == 0 ? x = WORLD_WIDTH - 2 : x--;
                x - 1 == 0 ? x = *ptr - 2 : x--;
                snake[number].head = '<';
                break;
            default:
                break;
        }
        
        (snake[number].object[0]).x = x;
        (snake[number].object[0]).y = y;
        mvwaddch(snakes_world, y, x, snake[number].head | ((number == 0) ? COLOR_PAIR(3) :
                                                           (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
    }
    if((indicator == 2 && snake[number].speed_counter == 0) || (indicator == 4 && snake[number].speed_counter == 0) || (indicator != 1 && snake[number].speed_counter < 0))
    {
        for (i = 1; i < snake[number].size; i++)
            mvwaddch(snakes_world, (snake[number].object[i]).y, (snake[number].object[i]).x, snake[number].face | ((number == 0) ? COLOR_PAIR(3) :
                                        
        mvwaddch(snakes_world, (snake[number].object[0]).y, (snake[number].object[0]).x, snake[number].head | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                               (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
    }
    //if the snake[number].speed_counter is more than 0, decrease it.
    /*if(snake[number].speed_counter > 0)
        snake[number].speed_counter--;
    //if the snake[number].speed_counter is less than 0, increase it.
    if(snake[number].speed_counter < 0)
        snake[number].speed_counter++;
    */
    
}


void* get_input (void* rank)
{
    int alea = 0;
    //while the round is less than 5 and the quit & restart variables are FALSE, get input
    while (round_number <= 4 && quit_2 == FALSE && restart_2 == FALSE) {
        alea = rand() % 4;
        ch = getch();
        if(ch != ERR) {
            //switch(alea) {
            switch(ch) {
                case 'w':
                case 'W':
                //case 0:
                    //if(snake[0].head != 'v')
                    if(snake[0].dir != down)
                        snake[0].dir = up;
                    break;
                case 's':
                case 'S':
                //case 1:
                    //if(snake[0].head != '^')
                    if(snake[0].dir != up)
                        snake[0].dir = down;
                    break;
                case 'd':
                case 'D':
                //case 2:
                    //if(snake[0].head != '<')
                    if(snake[0].dir != left)
                        snake[0].dir = right;
                    break;
                case 'a':
                case 'A':
                case 3:
                    //if(snake[0].head != '>')
                    if(snake[0].dir != right)
                        snake[0].dir = left;
                    break;
                case 'i':
                case 'I':
                    if(snake[1].dir != down)
                        snake[1].dir = up;
                    break;
                case 'k':
                case 'K':
                    if(snake[1].dir != up)
                        snake[1].dir = down;
                    break;
                case 'l':
                case 'L':
                    if(snake[1].dir != left)
                        snake[1].dir = right;
                    break;
                case 'j':
                case 'J':
                    if(snake[1].dir != right)
                        snake[1].dir = left;
                    break;
                case KEY_UP:
                    if(snake[2].dir != down)
                        snake[2].dir = up;
                    break;
                case KEY_DOWN:
                    if(snake[2].dir != up)
                        snake[2].dir = down;
                    break;
                case KEY_RIGHT:
                    if(snake[2].dir != left)
                        snake[2].dir = right;
                    break;
                case KEY_LEFT:
                    if(snake[2].dir != right)
                        snake[2].dir = left;
                    break;
                case 'q':
                case 'Q':
                    quit = TRUE;
                    break;
                case 'r':
                case 'R':
                    if(Pause == TRUE)
                    {
                        Pause = FALSE;
                        break;
                    }
                    else
                    {
                        restart = TRUE;
                        break;
                    }
                case 'y':
                case 'Y':if(restart == TRUE)
                    {
                        restart_2 = TRUE;
                        break;
                    }
                    if(quit == TRUE)
                    {
                        quit_2 = TRUE;
                        break;
                    }
                    else
                        break;
                case 'n':
                case 'N':
                    if(restart == TRUE)
                    {
                        restart = FALSE;
                        break;
                    }
                    if(quit == TRUE)
                    {
                        quit = FALSE;
                        break;
                    }
                    else
                        break;
                case 'p':
                case 'P':
                    if(Pause == FALSE)
                    {
                        Pause = TRUE;
                        break;
                    }
                    else
                        break;
                default:
                    break;
            }

        }
    }
}


void reset_all(){
    reset_for_next_round();
    snake[0].score = 0;
    snake[1].score = 0;
    snake[2].score = 0;

    snake[0].cpt_percute = 0;
    snake[1].cpt_percute = 0;
    snake[2].cpt_percute = 0;
    round_number = 2;
    restart = FALSE;
    
    restart_2 = FALSE;
}

void reset_for_next_round(){

    int i, j, k, m, x, y;
    indicator = 1;
    
    for(j = 0; j < 3; ++j){
        snake[j].size = 0;
        snake[j].speed_counter = 1000;
        snake[j].alive = TRUE;
        snake[j].dir = (direction)(rand() % 4);
    }

    (snake[0].object[0]).x = rand() % 10 + 11;
    (snake[0].object[0]).y = rand() % 20 + 1;
    
    (snake[1].object[0]).x = rand() % 10 + 41;
    (snake[1].object[0]).y = rand() % 10 + 1;
    
    (snake[2].object[0]).x = rand() % 10 + 41;
    (snake[2].object[0]).y = rand() % 10 + 11;
    
    for(j = 0; j < 3; ++j){
        for (i = 1; i < SNAKEY_LENGTH; i++){
            if(snake[j].dir == up){
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y + i) < 21 ? (snake[0].object[0]).y + i : (snake[0].object[0]).y + i - 20;
            }
            if(snake[j].dir == down){
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y - i) > 0 ? (snake[0].object[0]).y - i : (snake[0].object[0]).y - i + 20;
            }
            if(snake[j].dir == right){
                (snake[j].object[i]).x = ((snake[0].object[0]).x - i) > 0 ? (snake[0].object[0]).x - i : (snake[0].object[0]).x - i + 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            if(snake[j].dir == left){
                (snake[j].object[i]).x = ((snake[0].object[0]).x + i) < 61 ? (snake[0].object[0]).x + i : (snake[0].object[0]).x + i - 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            
        }
    }
    
    bool again = TRUE;
    
    while(again){
        x = rand() % 10 + 21;
        y = rand() % 20 + 1;
        if((snake[0].object[0]).x == x || (snake[0].object[0]).y == y)
            continue;
        if((snake[1].object[0]).x == x || (snake[1].object[0]).y == y)
            continue;
        if((snake[2].object[0]).x == x || (snake[2].object[0]).y == y)
            continue;

        for(i = 0; i < 5; ++i){
            (wall[2].place[i]).x = x + i;
            (wall[2].place[i]).y = y;
            again = FALSE;
        }
        
    }
    
}

void initialisation()
{
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    srand(time(NULL));
}