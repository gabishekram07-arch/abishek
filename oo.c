#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <emscripten/emscripten.h>

#define ROWS 5
#define COLS 10
#define MAX_SHOWS 3
#define DATA_FILE "theater_data.bin"
#define BOOKINGS_FILE "bookings.bin"

typedef struct{
    char title[50];
    int seats[ROWS][COLS];
    float price;
}Show;

typedef struct{
    int bookingID;
    char customerName[50];
    int showIndex;
    int numSeats;
    char seatPositions[200];
    float totalAmount;
}Booking;

Show shows[MAX_SHOWS];

static int sub_state = 0;
static int selected_show = -1;
static int seats_to_book = 0;
static int seats_entered = 0;

char current_name[50];
char current_seats_str[200];

/* -------- INPUT HANDLING -------- */

char input_buffer[100];
int input_ready = 0;

EMSCRIPTEN_KEEPALIVE
void processInput(char *input){
    strcpy(input_buffer, input);
    input_ready = 1;
}

/* -------- FUNCTIONS -------- */

void displaySeats(int showIdx);
void showOccupancyReport();
void saveToFile();
void loadFromFile();
int parseSeat(char *input,int *r,int *c);
void finalizeBooking();
void searchBookingByID(int id);

int parseSeat(char *input,int *r,int *c){
    if(strlen(input) < 2) return 0;

    char row = input[0];

    if(row >= 'A' && row <= 'Z')
        *r = row - 'A';
    else
        return 0;

    *c = atoi(&input[1]) - 1;

    if(*r >=0 && *r < ROWS && *c >=0 && *c < COLS)
        return 1;

    return 0;
}

/* -------- MAIN LOOP -------- */

void main_loop(){

    char input[100];

    if(!input_ready)
        return;

    strcpy(input, input_buffer);
    input_ready = 0;

    if(sub_state == 0){
        int num = atoi(input);

        if(num == 1){
            printf("\nEnter Show Number (1-3): ");
            fflush(stdout);
            sub_state = 1;
            return;
        }

        else if(num == 2){
            printf("\nSelect Show (1-3): ");
            fflush(stdout);
            sub_state = 2;
            return;
        }

        else if(num == 3){
            printf("\nEnter Booking ID: ");
            fflush(stdout);
            sub_state = 6;
            return;
        }

        else if(num == 4){
            showOccupancyReport();
            return;
        }
    }

    else if(sub_state == 1){
        int s = atoi(input) - 1;

        if(s>=0 && s<MAX_SHOWS)
            displaySeats(s);
        else
            printf("\nInvalid show number\n");

        sub_state = 0;
        return;
    }

    else if(sub_state == 2){
        int s = atoi(input) - 1;

        if(s>=0 && s<MAX_SHOWS){
            selected_show = s;

            printf("\n=========== SCREEN ===========\n");
            displaySeats(s);

            printf("\nSelected Movie: %s\n",shows[s].title);
            printf("\nHow many seats: ");
            fflush(stdout);

            sub_state = 3;
        }
        else{
            printf("\nInvalid show\n");
        }
        return;
    }

    else if(sub_state == 3){
        seats_to_book = atoi(input);

        if(seats_to_book <=0){
            printf("\nInvalid number\n");
            return;
        }

        printf("\nEnter customer name: ");
        fflush(stdout);
        sub_state = 4;
        return;
    }

    else if(sub_state == 4){
        strcpy(current_name,input);

        seats_entered = 0;
        current_seats_str[0] = '\0';

        printf("\nSelect %d seats\n",seats_to_book);
        fflush(stdout);

        sub_state = 5;
        return;
    }

    else if(sub_state == 5){
        int r,c;

        if(parseSeat(input,&r,&c)){
            if(shows[selected_show].seats[r][c] == 0){

                shows[selected_show].seats[r][c] = 1;

                strcat(current_seats_str,input);
                strcat(current_seats_str," ");

                seats_entered++;

                if(seats_entered < seats_to_book){
                    printf("\nSelect next seat (%d/%d)\n",
                    seats_entered+1,seats_to_book);
                    fflush(stdout);
                }
                else{
                    finalizeBooking();
                    sub_state = 0;
                }
            }
            else{
                printf("\nSeat already booked\n");
                fflush(stdout);
            }
        }
        else{
            printf("\nInvalid seat\n");
            fflush(stdout);
        }
        return;
    }

    else if(sub_state == 6){
        int id = atoi(input);
        searchBookingByID(id);

        sub_state = 0;
        return;
    }
}

/* -------- OTHER FUNCTIONS -------- */

void searchBookingByID(int id){

    FILE *fp = fopen(BOOKINGS_FILE,"rb");

    if(!fp){
        printf("\nNo bookings found\n");
        return;
    }

    Booking b;
    int found = 0;

    while(fread(&b,sizeof(Booking),1,fp)){
        if(b.bookingID == id){

            printf("\nBooking ID: %d\n",b.bookingID);
            printf("Customer: %s\n",b.customerName);
            printf("Movie: %s\n",shows[b.showIndex].title);
            printf("Seats: %s\n",b.seatPositions);
            printf("Total: $%.2f\n",b.totalAmount);

            found = 1;
            break;
        }
    }

    fclose(fp);

    if(!found)
        printf("\nBooking not found\n");
}

void finalizeBooking(){

    Booking b;

    b.bookingID = rand()%9000 + 1000;

    strcpy(b.customerName,current_name);
    b.showIndex = selected_show;
    b.numSeats = seats_to_book;
    strcpy(b.seatPositions,current_seats_str);

    b.totalAmount = seats_to_book * shows[selected_show].price;

    FILE *fp = fopen(BOOKINGS_FILE,"ab");

    if(fp){
        fwrite(&b,sizeof(Booking),1,fp);
        fclose(fp);
    }

    saveToFile();

    printf("\nBooking Successful!\n");
    printf("Booking ID: %d\n",b.bookingID);
    printf("Total Amount: $%.2f\n",b.totalAmount);
}

void displaySeats(int showIdx){

    printf("\n--- %s ---\n",shows[showIdx].title);

    for(int r=0;r<ROWS;r++){
        for(int c=0;c<COLS;c++){
            if(shows[showIdx].seats[r][c])
                printf("X ");
            else
                printf(". ");
        }
        printf("\n");
    }
}

void showOccupancyReport(){

    printf("\n=== OCCUPANCY REPORT ===\n");

    for(int i=0;i<MAX_SHOWS;i++){

        int booked = 0;

        for(int r=0;r<ROWS;r++)
            for(int c=0;c<COLS;c++)
                if(shows[i].seats[r][c])
                    booked++;

        printf("%s : %d/%d seats booked\n",
        shows[i].title,booked,ROWS*COLS);
    }
}

void saveToFile(){

    FILE *fp = fopen(DATA_FILE,"wb");

    if(fp){
        fwrite(shows,sizeof(Show),MAX_SHOWS,fp);
        fclose(fp);
    }
}

void loadFromFile(){

    FILE *fp = fopen(DATA_FILE,"rb");

    if(fp){
        fread(shows,sizeof(Show),MAX_SHOWS,fp);
        fclose(fp);
    }
    else{
        strcpy(shows[0].title,"Dune Part Two");
        shows[0].price = 12;

        strcpy(shows[1].title,"Oppenheimer");
        shows[1].price = 10;

        strcpy(shows[2].title,"The Batman");
        shows[2].price = 11;

        for(int i=0;i<MAX_SHOWS;i++)
            for(int r=0;r<ROWS;r++)
                for(int c=0;c<COLS;c++)
                    shows[i].seats[r][c] = 0;
    }
}

int main(){

    setvbuf(stdout, NULL, _IONBF, 0);

    srand(time(NULL));
    loadFromFile();

    printf("System Ready\n");

    emscripten_set_main_loop(main_loop,0,1);
    return 0;
}