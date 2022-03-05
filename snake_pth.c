#include <ncurses.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

//snake_board dimensions. they are 2 units longer because of the boarders.
//#define WORLD_WIDTH 62
//#define WORLD_HEIGHT 22

int WORLD_WIDTH = 62;
int *ptr = &WORLD_WIDTH;

int WORLD_HEIGHT = 22;
int *ptr2 = &WORLD_HEIGHT;

int NB_SERPENTS = 3;
int *ptr3 = &NB_SERPENTS;

//maximum of the snake's length
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

//defining a new typr for direction of the snake
typedef enum direct {up,down,left,right} direction;

//defining a struct for elements place (x,y) on the board
//définir une structure pour les éléments place (x,y) sur le tableau
typedef struct spart {
    int x;
    int y;
} part;

//define windows
WINDOW *snakes_world;
WINDOW *score_board;
WINDOW *message_board;
WINDOW *first_board;
WINDOW *final_board;


//defining a struct for walls places made of a 5 elements array. each element is type of part (x,y).
//définissant une struct pour les emplacements des murs constitués d'un tableau de 5 éléments. 
//chaque élément est un type de partie (x,y).
struct walls {
    part place[100];
};

//define 3 walls with type walls
//définir 3 murs avec des murs de type
struct walls wall[13];
size_t size = sizeof( part ) * 100;

//define a struct for frogs/mines made of a 5 elements array for the places of each frog/mine and a boolean variable for detecting wether the frog/mine is hit or not
//définir une structure pour les grenouilles/mines composée d'un tableau de 5 éléments pour les emplacements de chaque grenouille/mine et d'une variable booléenne 
//pour détecter si la grenouille/mine est touchée ou non
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

//define a frog and a mine of type frogs_mies
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

//define a struct for snakes. each variable is explained inside.
struct snakes {
    //an array for the place of each part of the snake on the snake_board
    part object [SNAKEY_LENGTH];
    //a character for the place of the snake
    char face;
    //a variable for the moving direction of the snake
    //une variable pour la direction de déplacement du serpent
    //UTILE
    int dir;
    //a variable for the size of the snake between 1 & 8
    //PAS UTILE
    int size;
    //a variable for the score of the snake
    //une variable pour le score du serpent
    //UTILE
    int score;
    //a variable for the speed of snake
    //PAS UTILE
    int speed_counter;
    //a character for the head of the snake depend on the direction it moves
    //PAS UTILE
    char head;
    //A VOIR 
    //a varible for detecting if the snake is alive or dead
    bool alive;

    int cpt_percute;
};

//define a 3 elements array of type snakes for 3 snake
//définir un tableau de 3 éléments de type serpents pour 3 serpents
//Peut être utiliser des pointeurs
struct snakes snake[3];// = {snake[0], snake[1], snake[2]};

//variable for x & y of the starting point of the snake_board according to the screen size & board size. the snake_board is in the center of the screen.
int offsetx, offsety;
//input character from the keyboard
int ch;
//round number between 1 & 4
int round_number;
//a variable to show wether the snake may move in this 0.5ms or it may stay.
int indicator;
//variables for restarting, quiting & pausing the game.
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
    
    //initialize every thing for the screen.
    initialisation();
    //Définissez 3 poignées de fil pour obtenir la clé, déplacer les serpents et faire des grenouilles/mines.
    pthread_t thread_handle_1, thread_handle_2, thread_handle_3;
    //if the scren size is smaller than required, stay in the while
    while(COLS < Longueur || LINES < Largeur){
        //clear the screen.
        clear();
        //show a message to expand the screen
        mvprintw(1, 1, "Agrandissez l'écran...");
        //refresh the screen.
        refresh();
        
        usleep(100000);

    }
    
    offsetx = (COLS - *ptr) / 2;
    offsety = (LINES - *ptr2) / 2;
    
    //make the game board
    snakes_world = newwin(*ptr2, *ptr, offsety, offsetx);
    //make score board to show scores during the game.
    score_board = newwin(3, *ptr, offsety + *ptr2, offsetx);
    //make message board to show restart, quit & pause messages
    message_board = newwin(5, *ptr, offsety - 5 , offsetx);
    //make first_board to show the facts about the game in the start
    first_board = newwin(10, *ptr, (LINES - 10) / 2, offsetx);
    //make final_board to show scores & the winner at the end of the game
    final_board = newwin(7, *ptr, (LINES - 7) / 2, offsetx);
    //make all the quit and restart variables FALSE in the start
    quit = FALSE;
    
    quit_2 = FALSE;
    
    restart = FALSE;
    
    restart_2 = FALSE;
    //define snakes faces
    snake[0].face = '#';
    snake[1].face = '&';
    snake[2].face = 'O';
    //while quit_2 variable is not TRUE, run the game
    while(quit_2 == FALSE){
        //clear the whole screen
        clear();
        //refresh the whole screen
        refresh();
        //clear the first_board
        wclear(first_board);
        //show the message at the end of the first board.
        mvwprintw(first_board, 8, 1, "PRESS s to START the GAME...");
        //show the character that snake_0 is made of
        mvwprintw(first_board, 1, 1, "Snake_0 is made of:");
        mvwaddch(first_board, 1, 21, snake[0].face | COLOR_PAIR(3));
        //show moving keys for snake_0
        mvwprintw(first_board, 2, 1, "Snake_0 moving keys are: W & S & A & D");
        //show the character that snake_1 is made of
        mvwprintw(first_board, 3, 1, "Snake_1 is made of:");
        mvwaddch(first_board, 3, 21, snake[1].face | COLOR_PAIR(5));
        //show moving keys for snake_1
        mvwprintw(first_board, 4, 1, "Snake_1 moving keys are: I & J & K & L");
        //show the character that snake_2 is made of
        mvwprintw(first_board, 5, 1, "Snake_2 is made of:");
        mvwaddch(first_board, 5, 21, snake[2].face | COLOR_PAIR(6));
        //show moving keys for snake_2
        mvwprintw(first_board, 6, 1, "Snake_2 moving keys are: ARROW Keys");
        //make border for first_board
        box(first_board, 0, 0);
        //refresh the first board
        wrefresh(first_board);
        //while the user has not pressed 's' key, stay in the while and do not start the game.
        while(1){
            ch = getch();
            if (ch == 's')
                break;
            if (ch == 'S')
                break;
        }
        //clear the whole screen
        clear();
        //refresh the whole screen
        refresh();
        //clear the message_board
        wclear(message_board);
        //reset all the variables for a new game
        reset_all();
        //create threads
        //thread to get key from the keyboard
        //Déplacer les serpents
        pthread_create(&thread_handle_1, NULL, get_input, (void*)0);
        //thread to move snake and & referee
        pthread_create(&thread_handle_2, NULL, move_snake, (void*)1);
        //thread to make mine or frog if a mine or frog hit.
        pthread_create(&thread_handle_3, NULL, verification_fruit, (void*)2);
        
        //join the threads after finishing
        pthread_join(thread_handle_1, NULL);
        pthread_join(thread_handle_2, NULL);
        pthread_join(thread_handle_3, NULL);
        //if the 4 rounds of the game is finished and the game is not restarted or quited, show the winner
        if(round_number == 5){
            //clear the whole screen
            clear();
            //refresh the whole screen
            refresh();
            //clear the message_board
            wclear(message_board);
            //print restart & quit message in the message board.
            mvwprintw(message_board, 1, 1, "PRESS r to RESTART...");
            mvwprintw(message_board, 3, 1, "PRESS q to QUIT...");
            //make border for the message_board
            box(message_board, 0, 0);
            //refresh the message_board
            wrefresh(message_board);
            //print the winner
            print_winner();
        }
        //if 'r' or 'q' has not been pressed for restart or quit, wait for them to be pressed.
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
    //delete all the boards
    delwin(snakes_world);
    
    delwin(score_board);
    
    delwin(message_board);
    
    delwin(first_board);
    
    delwin(final_board);
    //end the windows
    endwin();

    return 0;

}


void print_winner(){
    //clear the final_board
    wclear(final_board);
    //show all the snakes scores
    mvwprintw(final_board, 1, 1, "Snake_0 Score: %d", snake[0].score);
    mvwprintw(final_board, 2, 1, "Snake_1 Score: %d", snake[1].score);
    mvwprintw(final_board, 3, 1, "Snake_2 Score: %d", snake[2].score);
    //show the winner
    if(snake[0].score > snake[1].score && snake[0].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_0 is Winner");
                  
    else if(snake[1].score > snake[0].score && snake[1].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_1 is Winner");
                  
    else if(snake[2].score > snake[0].score && snake[2].score > snake[1].score)
        mvwprintw(final_board, 5, 1, "Snake_2 is Winner");
    //show a message if there is no winner
    else
        mvwprintw(final_board, 5, 1, "There is No Winner");
    //make border for final_board
    box(final_board, 0, 0);
    //refresh final_board
    wrefresh(final_board);
    
}
//thread to move snake & referee
void* move_snake(void* rank) {
    
    int i, j, k;
    //while the round is less than 5 and the quit & restart variables are FALSE, run the game
    int nb=4;
    hasard = my_random(nb);
    tot_score=nb_intervalle(50,99);
    while(round_number <= 4 && quit_2 == FALSE  && restart_2 == FALSE){
        //hasard;
        //clear the snakes_world
        wclear(snakes_world);
        //clear the score_board
        wclear(score_board);
        //move all the snakes
        for(j = 0; j < 4; j++){
            //if a snake is alive, move it
            //Fonction pour bouger le serpent
            if(snake[j].alive)
                move_it(j);
        }

        //increase indicator
        indicator++;
        //keep indicator between 1 & 4
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


        //check for all the collisions
        check_collision();
        //make border for snakes_world
        box(snakes_world, 0, 0);
        //make border for score_board
        box(score_board, 0, 0);
        //refresh snakes_world
        wrefresh(snakes_world);
        //refresh score_board
        wrefresh(score_board);
        //if quit & restart & pause variable is FALSE, show their messages
        if(quit == FALSE && restart == FALSE && Pause == FALSE){
            wclear(message_board);
            
            mvwprintw(message_board, 1, 1, "Appuye sur r pour recommencer...");
            mvwprintw(message_board, 2, 1, "Appue sur q pour quitter...");
            mvwprintw(message_board, 3, 1, "Appuye p mettre sur pause...");
            
            box(message_board, 0, 0);
            
            wrefresh(message_board);
        }
        //if quit is TRUE, show the message if the user is sure to quit
        if(quit == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "Es-tu sûr de vouloir quitter ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user says wether is sure to quit or not
            while(quit == TRUE && quit_2 == FALSE);
        }
        //if restart is TRUE, show the message if the user is sure to restart
        if(restart == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "Es-tu sûr de vouloir recommencer ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user says wether is sure to restart or not
            while(restart == TRUE && restart_2 == FALSE);
        }
        //if the pause is TRUE, show the message that press 'r' to resume
        if(Pause == TRUE){
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "PRESS r to RESUME...");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user press 'r'
            while(Pause == TRUE);
        }
        //if restart & quit variables are FALSE, delay 0.5ms
        if(restart_2 == FALSE && quit_2 == FALSE)
            usleep(500000);
        
    }
}
//thread to make frog/mine if a frog/mine hit
//fil pour faire grenouille/mine si une grenouille/mine frappe
void* verification_fruit(void* rank){
    int i, j, k, m, x, y, wall_0, wall_1, wall_2, wall_3, wall_4;
    //while the round is less than 5 and the quit & restart variables are FALSE, run the game
    while(round_number <= 4 && quit_2 == FALSE && restart_2 == FALSE){
        
            //if a frog is dead, go in the while and don't come out until the frog becomes alive
            //si une grenouille est morte, allez-y pendant un moment et ne sortez pas jusqu'à ce que la grenouille devienne vivante
            //while(frog.alive[m] == FALSE)
            
            while(unFruit.alive[0] == FALSE){
                x = rand() % 60 + 1;
                y = rand() % 20 + 1;
                //check wether the chosen random place is not on an alive snake
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
                //check wether the chosen random place is not on a wall
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

                //check wether the chosen random place is not on an alive frog
                //vérifier si le lieu aléatoire choisi n'est pas sur une grenouille vivante
                /*if(j == 3 && wall_0 == 8 && wall_1 == 5 && wall_2 == 5)
                {
                    for(i = 0; i < 5; ++i)
                    {
                        if((frog.place[i]).x == x && (frog.place[i]).y == y && frog.alive[i] == TRUE)
                            break;
                    }
                }*/
                //check wether the chosen random place is not on an alive mine
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
                {
                    for(k = 0; k < 5; ++k)
                    {
                        if((mine.place[k]).x == x && (mine.place[k]).y == y && mine.alive[k] == TRUE)
                            break;
                    }
                }
                //if all the above criteria met, chose the chosen random place for the frog & make it alive

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


//move for a snake
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
        //show the head of the snake
        mvwaddch(snakes_world, y, x, snake[number].head | ((number == 0) ? COLOR_PAIR(3) :
                                                           (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
    }
    //show the snake in its last position
    if((indicator == 2 && snake[number].speed_counter == 0) || (indicator == 4 && snake[number].speed_counter == 0) || (indicator != 1 && snake[number].speed_counter < 0))
    {
        //show the snake in its last place
        for (i = 1; i < snake[number].size; i++)
            mvwaddch(snakes_world, (snake[number].object[i]).y, (snake[number].object[i]).x, snake[number].face | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                                   (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
        //show the last head of the snake
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



//thread to get character from keyboard
//Permet de gérer le déplacement de serpents
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
                //if user pressed 'w' and the snake_0 did not go down, it goes up
                case 'w':
                case 'W':
                //case 0:
                    //if(snake[0].head != 'v')
                    if(snake[0].dir != down)
                        snake[0].dir = up;
                    break;
                //if user pressed 's' and the snake_0 did not go up, it goes down
                case 's':
                case 'S':
                //case 1:
                    //if(snake[0].head != '^')
                    if(snake[0].dir != up)
                        snake[0].dir = down;
                    break;
                //if user pressed 'd' and the snake_0 did not go left, it goes right
                case 'd':
                case 'D':
                //case 2:
                    //if(snake[0].head != '<')
                    if(snake[0].dir != left)
                        snake[0].dir = right;
                    break;
                //if user pressed 'a' and the snake_0 did not go right, it goes left
                case 'a':
                case 'A':
                case 3:
                    //if(snake[0].head != '>')
                    if(snake[0].dir != right)
                        snake[0].dir = left;
                    break;
                //if user pressed 'i' and the snake_1 did not go down, it goes up
                case 'i':
                case 'I':
                    if(snake[1].dir != down)
                        snake[1].dir = up;
                    break;
                //if user pressed 'k' and the snake_1 did not go up, it goes down
                case 'k':
                case 'K':
                    if(snake[1].dir != up)
                        snake[1].dir = down;
                    break;
                //if user pressed 'l' and the snake_1 did not go left, it goes right
                case 'l':
                case 'L':
                    if(snake[1].dir != left)
                        snake[1].dir = right;
                    break;
                //if user pressed 'j' and the snake_1 did not go right, it goes left
                case 'j':
                case 'J':
                    if(snake[1].dir != right)
                        snake[1].dir = left;
                    break;
                //if user pressed KEY_UP and the snake_2 did not go down, it goes up
                case KEY_UP:
                    if(snake[2].dir != down)
                        snake[2].dir = up;
                    break;
                //if user pressed KEY_DOWN and the snake_2 did not go up, it goes down
                case KEY_DOWN:
                    if(snake[2].dir != up)
                        snake[2].dir = down;
                    break;
                //if user pressed KEY_RIGHT and the snake_2 did not go left, it goes right
                case KEY_RIGHT:
                    if(snake[2].dir != left)
                        snake[2].dir = right;
                    break;
                //if user pressed KEY_LEFT and the snake_2 did not go right, it goes left
                case KEY_LEFT:
                    if(snake[2].dir != right)
                        snake[2].dir = left;
                    break;
                //if the user pressed 'q', quit variable becomes TRUE
                case 'q':
                case 'Q':
                    quit = TRUE;
                    break;
                case 'r':
                case 'R':
                    //if the user press 'r' and is in the pause, pause variable becomes FALSE
                    if(Pause == TRUE)
                    {
                        Pause = FALSE;
                        break;
                    }
                    //else, restart variable becomes TRUE
                    else
                    {
                        restart = TRUE;
                        break;
                    }
                case 'y':
                case 'Y':
                    //if the user press 'y' and restart variable is TRUE, restart 2 variable becomes TRUE
                    if(restart == TRUE)
                    {
                        restart_2 = TRUE;
                        break;
                    }
                    //if the user press 'y' and quit variable is TRUE, quit 2 variable becomes TRUE
                    if(quit == TRUE)
                    {
                        quit_2 = TRUE;
                        break;
                    }
                    else
                        break;
                case 'n':
                case 'N':
                    //if the user press 'n' and restart variable is TRUE, restart variable becomes FALSE
                    if(restart == TRUE)
                    {
                        restart = FALSE;
                        break;
                    }
                    //if the user press 'n' and quit variable is TRUE, quit variable becomes FALSE
                    if(quit == TRUE)
                    {
                        quit = FALSE;
                        break;
                    }
                    else
                        break;
                case 'p':
                case 'P':
                    //if the user press 'p' and pause variable is TRUE, pause variable becomes FALSE
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

void leswalls(int round_nb){
    int i,j,x,y;
    switch(round_nb){
        case 1:
       /* for(i=0; i<*ptr2;i++){
           mvwaddch(snakes_world, i, *ptr2-(*ptr2-1), '|' | COLOR_PAIR(4));
           mvwaddch(snakes_world, i, *ptr2+((*ptr-*ptr2)-2), '|' | COLOR_PAIR(4));
       }
       for(i=0; i<*ptr;i++){
           mvwaddch(snakes_world, *ptr-(*ptr-1), i, '-' | COLOR_PAIR(4));
           mvwaddch(snakes_world, *ptr-((*ptr-*ptr2)+2), i, '-' | COLOR_PAIR(4));
       }*/

        break;
        case 2: 
        //lignes verticales
            x = 1; 
            y = 1;
            int x1 = 60;
           
            for(i = 0; i < 21; ++i){
                (wall[0].place[i]).x = x;
                (wall[0].place[i]).y = y + i;
                (wall[1].place[i]).x = x1;
                (wall[1].place[i]).y = y + i;
            }
            //lignes horizontales 
            for(j=1;j<59;++j){
                //(wall[1].place[j]).x = x+1;
                //(wall[1].place[j]).y = y;
                (wall[3].place[j]).x = x + j;
                (wall[3].place[j]).y = y;
                (wall[4].place[j]).x = x + j;
                (wall[4].place[j]).y = 20;

            }
            break;
        case 3:
            break;
        case 4:
//lignes verticales
            x = 1; 
            y = 1;
            x1 = 60;
        //lignes horizontales    
            for(i = 0; i < 21; ++i){
                (wall[0].place[i]).x = x;
                (wall[0].place[i]).y = y + i;
                (wall[1].place[i]).x = x1;
                (wall[1].place[i]).y = y + i;
            }
            for(j=1;j<59;++j){
                //(wall[1].place[j]).x = x+1;
                //(wall[1].place[j]).y = y;
                (wall[3].place[j]).x = x + j;
                (wall[3].place[j]).y = y;
                (wall[4].place[j]).x = x + j;
                (wall[4].place[j]).y = 20;

            }
            break;
        default:
            break;
    }
}
//reset every thing for starting the game
void reset_all()
{
    //reset every thing for the next round
    reset_for_next_round();
    //reset snakes scores
    snake[0].score = 0;
    
    snake[1].score = 0;
    
    snake[2].score = 0;

    snake[0].cpt_percute = 0;
    snake[1].cpt_percute = 0;
    snake[2].cpt_percute = 0;
    //reset round_number
    round_number = 2;
    //reset restart variables
    restart = FALSE;
    
    restart_2 = FALSE;
}
//reset every thing for the next round
void reset_for_next_round()
{

    int i, j, k, m, x, y;
    //reset move indicator
    indicator = 1;
    
    for(j = 0; j < 3; ++j)
    {
        snake[j].size = 0;
        snake[j].speed_counter = 1000;
        snake[j].alive = TRUE;
        snake[j].dir = (direction)(rand() % 4);
    }
    //give a rondom starting point to snake 0
    (snake[0].object[0]).x = rand() % 10 + 11;
    (snake[0].object[0]).y = rand() % 20 + 1;
    //give a random starting point to snake 1
    (snake[1].object[0]).x = rand() % 10 + 41;
    (snake[1].object[0]).y = rand() % 10 + 1;
    //give a random starting point to snake 2
    (snake[2].object[0]).x = rand() % 10 + 41;
    (snake[2].object[0]).y = rand() % 10 + 11;
    //give the places for all other 7 parts of the snake depend on its random starting direction
    for(j = 0; j < 3; ++j)
    {
        for (i = 1; i < SNAKEY_LENGTH; i++)
        {
            if(snake[j].dir == up)
            {
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y + i) < 21 ? (snake[0].object[0]).y + i : (snake[0].object[0]).y + i - 20;
            }
            if(snake[j].dir == down)
            {
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y - i) > 0 ? (snake[0].object[0]).y - i : (snake[0].object[0]).y - i + 20;
            }
            if(snake[j].dir == right)
            {
                (snake[j].object[i]).x = ((snake[0].object[0]).x - i) > 0 ? (snake[0].object[0]).x - i : (snake[0].object[0]).x - i + 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            if(snake[j].dir == left)
            {
                (snake[j].object[i]).x = ((snake[0].object[0]).x + i) < 61 ? (snake[0].object[0]).x + i : (snake[0].object[0]).x + i - 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            
        }
    }
    
    bool again = TRUE;
    //find a random place for the first vertical wall with x between 1 & 10
    
    

    again = TRUE;
    //find a random place for the horizantal wall with starting x between 21 & 30 and y between 1 & 20
    
    while(again)
    {
        x = rand() % 10 + 21;
        y = rand() % 20 + 1;
        //if the x or y is equal to a x or y of the head of a snake, repeat
        if((snake[0].object[0]).x == x || (snake[0].object[0]).y == y)
            continue;
        if((snake[1].object[0]).x == x || (snake[1].object[0]).y == y)
            continue;
        if((snake[2].object[0]).x == x || (snake[2].object[0]).y == y)
            continue;
        //else, the x & y are good and fill the wall.place array with appropriate amount
        for(i = 0; i < 5; ++i)
        {
            (wall[2].place[i]).x = x + i;
            (wall[2].place[i]).y = y;
            //make again FALSE so it comes out of the while loop
            again = FALSE;
        }
        
    }
    
    
    m = 0;
    
    int wall_0, wall_1, wall_2;
    //find random place for frogs
 
    m = 0;
    //find random place for mines
    while(m < 5)
    {
        x = rand() % 60 + 1;
        y = rand() % 20 + 1;
        //if x & y is equal to x & y of a snakes head, repeat
        for(j = 0; j < 3; ++j)
        {
            if((snake[j].object[0]).x == x && (snake[j].object[0]).y == y)
                break;
        }
        //if x & y is equal to x & y of a walls place, repeat
        if(j == 3)
        {
            for(wall_0 = 0; wall_0 < 5; ++wall_0)
            {
                if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                    break;
            }
            for(wall_1 = 0; wall_1 < 5; ++wall_1)
            {
                if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                    break;
            }
            for(wall_2 = 0; wall_2 < 5; ++wall_2)
            {
                if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                    break;
            }
        }
        //if x & y is equal to x & y of a frog, repeat
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5)
        {
            for(i = 0; i < 5; ++i)
            {
                if((frog.place[i]).x == x && (frog.place[i]).y == y)
                    break;
            }
        }
        //if x & y is equal to x & y of a previous mine, repeat
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
        {
            for(k = 0; k < m; ++k)
            {
                if((mine.place[k]).x == x && (mine.place[k]).y == y)
                    break;
            }
        }
        //if all the above criteria met, fill the mine.place with above x & y
        //si tous les critères ci-dessus sont remplis, remplissez le mine.place avec ci-dessus x & y
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5 && k == m)
        {
            (mine.place[m]).x = x;
            (mine.place[m]).y = y;
            //increase m
            m++;
        }
    }
    //make all the frogs and mines alive
    for(i = 0; i < 5; ++i)
    {
        frog.alive[i] = TRUE;
        mine.alive[i] = TRUE;
    }
    
}
//initialize screen
void initialisation()
{
    initscr();
    //no echo for input
    noecho();
    cbreak();
    //no delay for input
    nodelay(stdscr, TRUE);
    //use this to use keyboard arrow keys
    keypad(stdscr, TRUE);
    //make curser disapper
    curs_set(FALSE);
    start_color();
    //color pairs for making colorful objects
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    //for making random number
    srand(time(NULL));
}