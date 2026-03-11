#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 5
#define COLS 10
#define TOTAL_SEATS 50
#define MAX_SHOWS 3

// ---------------- STRUCTURES ----------------
typedef struct {
    int bookingID;
    char customerName[50];
    char movie[50];
    int seatCount;
    int seats[10];
    float totalAmount;
} Booking;

// ---------------- GLOBAL DATA ----------------
char movies[MAX_SHOWS][50] = {
    "Avengers",
    "Avatar 2",
    "Inception"
};

int seats[MAX_SHOWS][ROWS][COLS] = {0};
int bookingCounter = 1000;

// ---------------- FUNCTION DECLARATIONS ----------------
void displayShows();
void displaySeats(int show);
void bookTicket();
void saveBooking(Booking b);
void viewBooking();
void occupancyReport();
int seatAvailable(int show, int seatNo);

// ---------------- MAIN ----------------
int main() {

    int choice;

    while (1) {

        printf("\n==============================\n");
        printf(" MOVIE TICKET BOOKING SYSTEM\n");
        printf("==============================\n");

        printf("1. Display Shows\n");
        printf("2. Book Ticket\n");
        printf("3. View Booking\n");
        printf("4. Occupancy Report\n");
        printf("5. Exit\n");

        printf("Enter choice: ");
        scanf("%d",&choice);

        switch(choice)
        {
            case 1:
                displayShows();
                break;

            case 2:
                bookTicket();
                break;

            case 3:
                viewBooking();
                break;

            case 4:
                occupancyReport();
                break;

            case 5:
                printf("Thank you!\n");
                exit(0);

            default:
                printf("Invalid choice!\n");
        }
    }
}

// ---------------- DISPLAY MOVIES ----------------
void displayShows()
{
    printf("\nAvailable Shows\n");

    for(int i=0;i<MAX_SHOWS;i++)
    {
        printf("%d. %s\n",i+1,movies[i]);
    }
}

// ---------------- DISPLAY SEAT LAYOUT ----------------
void displaySeats(int show)
{
    printf("\nSeat Layout (0 = Available, 1 = Booked)\n\n");

    for(int i=0;i<ROWS;i++)
    {
        for(int j=0;j<COLS;j++)
        {
            printf("%d ",seats[show][i][j]);
        }
        printf("\n");
    }

    printf("\nSeat Numbers: 1 - 50\n");
}

// ---------------- CHECK SEAT AVAILABLE ----------------
int seatAvailable(int show, int seatNo)
{
    int r = (seatNo-1)/COLS;
    int c = (seatNo-1)%COLS;

    if(seats[show][r][c] == 0)
        return 1;
    else
        return 0;
}

// ---------------- BOOK TICKET ----------------
void bookTicket()
{
    Booking b;
    int show;
    int price = 150;

    displayShows();

    printf("Select show: ");
    scanf("%d",&show);

    if(show <1 || show>MAX_SHOWS)
    {
        printf("Invalid show!\n");
        return;
    }

    show--;

    displaySeats(show);

    printf("Enter customer name: ");
    scanf("%s",b.customerName);

    printf("Enter number of seats: ");
    scanf("%d",&b.seatCount);

    if(b.seatCount<=0 || b.seatCount>10)
    {
        printf("Invalid seat count!\n");
        return;
    }

    for(int i=0;i<b.seatCount;i++)
    {
        int seatNo;

        printf("Enter seat number: ");
        scanf("%d",&seatNo);

        if(seatNo<1 || seatNo>50)
        {
            printf("Invalid seat number!\n");
            i--;
            continue;
        }

        if(!seatAvailable(show,seatNo))
        {
            printf("Seat already booked!\n");
            i--;
            continue;
        }

        int r = (seatNo-1)/COLS;
        int c = (seatNo-1)%COLS;

        seats[show][r][c] = 1;
        b.seats[i] = seatNo;
    }

    strcpy(b.movie,movies[show]);

    b.bookingID = bookingCounter++;

    b.totalAmount = b.seatCount * price;

    saveBooking(b);

    printf("\n----- BOOKING RECEIPT -----\n");

    printf("Booking ID: %d\n",b.bookingID);
    printf("Customer: %s\n",b.customerName);
    printf("Movie: %s\n",b.movie);

    printf("Seats: ");

    for(int i=0;i<b.seatCount;i++)
        printf("%d ",b.seats[i]);

    printf("\nTotal Amount: %0.2f\n",b.totalAmount);
}

// ---------------- SAVE BOOKING ----------------
void saveBooking(Booking b)
{
    FILE *fp;

    fp = fopen("bookings.dat","ab");

    if(fp==NULL)
    {
        printf("File error!\n");
        return;
    }

    fwrite(&b,sizeof(Booking),1,fp);

    fclose(fp);
}

// ---------------- VIEW BOOKING ----------------
void viewBooking()
{
    FILE *fp;
    Booking b;
    int id,found=0;

    printf("Enter Booking ID: ");
    scanf("%d",&id);

    fp = fopen("bookings.dat","rb");

    if(fp==NULL)
    {
        printf("No bookings found.\n");
        return;
    }

    while(fread(&b,sizeof(Booking),1,fp))
    {
        if(b.bookingID == id)
        {
            printf("\nBooking Found\n");

            printf("Customer: %s\n",b.customerName);
            printf("Movie: %s\n",b.movie);

            printf("Seats: ");

            for(int i=0;i<b.seatCount;i++)
                printf("%d ",b.seats[i]);

            printf("\nTotal: %.2f\n",b.totalAmount);

            found = 1;
            break;
        }
    }

    if(!found)
        printf("Booking not found!\n");

    fclose(fp);
}

// ---------------- OCCUPANCY REPORT ----------------
void occupancyReport()
{
    for(int s=0;s<MAX_SHOWS;s++)
    {
        int booked=0;

        for(int i=0;i<ROWS;i++)
        {
            for(int j=0;j<COLS;j++)
            {
                if(seats[s][i][j]==1)
                    booked++;
            }
        }

        int available = TOTAL_SEATS - booked;

        float percent = ((float)booked/TOTAL_SEATS)*100;

        printf("\nMovie: %s\n",movies[s]);
        printf("Total Seats: %d\n",TOTAL_SEATS);
        printf("Booked Seats: %d\n",booked);
        printf("Available Seats: %d\n",available);
        printf("Occupancy: %.2f%%\n",percent);
    }
}