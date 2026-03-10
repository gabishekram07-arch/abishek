/*
 * ============================================================
 *   MOVIE TICKET BOOKING SYSTEM
 *   Single Theatre | 5 Rows x 10 Seats = 50 Seats per Show
 *   Compile: emcc main.c -o index.html
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Constants ─────────────────────────────────────────── */
#define MAX_SHOWS      5
#define ROWS           5
#define COLS           10
#define TOTAL_SEATS    (ROWS * COLS)
#define MAX_BOOKINGS   200
#define MAX_NAME       64
#define BOOKING_FILE   "bookings.dat"

/* ─── Structs ────────────────────────────────────────────── */

typedef struct {
    int  id;
    char title[64];
    char time[16];
    float price;
    int  seats[ROWS][COLS];   /* 0 = available, 1 = booked */
} Show;

typedef struct {
    char bookingID[16];
    char customerName[MAX_NAME];
    int  showID;
    int  seatCount;
    int  seatRows[TOTAL_SEATS];
    int  seatCols[TOTAL_SEATS];
    float totalAmount;
} Booking;

/* ─── Globals ────────────────────────────────────────────── */
Show     shows[MAX_SHOWS];
Booking  bookings[MAX_BOOKINGS];
int      bookingCount = 0;
int      showCount    = 0;

/* ─── Forward Declarations ───────────────────────────────── */
void  initShows(void);
void  displayShows(void);
void  bookTickets(void);
void  viewBooking(void);
void  occupancyReport(void);
void  saveBookings(void);
void  loadBookings(void);
void  printReceipt(int idx);
void  printSeatMap(int showIdx);
int   findShowIndex(int id);
int   findBookingIndex(const char *bid);
void  generateBookingID(char *buf);
int   validateSeat(int showIdx, int row, int col);
void  clearInputBuffer(void);

/* ═══════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════ */
int main(void) {
    initShows();
    loadBookings();

    int choice;
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║   WELCOME TO CINEPLEX BOOKING SYSTEM ║\n");
    printf("╚══════════════════════════════════════╝\n");

    do {
        printf("\n┌─────────────── MAIN MENU ───────────────┐\n");
        printf("│  1. View Available Shows                │\n");
        printf("│  2. Book Tickets                        │\n");
        printf("│  3. View Booking by ID                  │\n");
        printf("│  4. Occupancy Report                    │\n");
        printf("│  0. Exit                                │\n");
        printf("└─────────────────────────────────────────┘\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("[!] Invalid input. Please enter a number.\n");
            choice = -1;
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: displayShows();    break;
            case 2: bookTickets();     break;
            case 3: viewBooking();     break;
            case 4: occupancyReport(); break;
            case 0: printf("\nGoodbye! Enjoy your movie!\n\n"); break;
            default:
                printf("[!] Invalid choice. Enter 0–4.\n");
        }
    } while (choice != 0);

    return 0;
}

/* ═══════════════════════════════════════════════════════════
   1. INIT SHOWS  (hardcoded)
   ═══════════════════════════════════════════════════════════ */
void initShows(void) {
    showCount = 5;

    shows[0] = (Show){1, "Interstellar",         "10:00 AM", 150.0f};
    shows[1] = (Show){2, "The Dark Knight",       "01:00 PM", 180.0f};
    shows[2] = (Show){3, "Inception",             "04:00 PM", 180.0f};
    shows[3] = (Show){4, "Avengers: Endgame",     "07:00 PM", 200.0f};
    shows[4] = (Show){5, "Oppenheimer",           "10:00 PM", 220.0f};

    for (int s = 0; s < showCount; s++)
        memset(shows[s].seats, 0, sizeof(shows[s].seats));
}

/* ═══════════════════════════════════════════════════════════
   2. DISPLAY SHOWS
   ═══════════════════════════════════════════════════════════ */
void displayShows(void) {
    printf("\n┌────┬────────────────────────────┬──────────┬───────────┬────────────┐\n");
    printf("│ ID │ Movie                      │ Time     │ Price(Rs) │  Available │\n");
    printf("├────┼────────────────────────────┼──────────┼───────────┼────────────┤\n");
    for (int i = 0; i < showCount; i++) {
        int booked = 0;
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                if (shows[i].seats[r][c]) booked++;
        printf("│ %-2d │ %-26s │ %-8s │ %-9.0f │ %2d / %-5d │\n",
               shows[i].id, shows[i].title, shows[i].time,
               shows[i].price, TOTAL_SEATS - booked, TOTAL_SEATS);
    }
    printf("└────┴────────────────────────────┴──────────┴───────────┴────────────┘\n");
}

/* ═══════════════════════════════════════════════════════════
   3. BOOK TICKETS
   ═══════════════════════════════════════════════════════════ */
void bookTickets(void) {
    displayShows();

    /* Choose show */
    int showID;
    printf("\nEnter Show ID to book: ");
    if (scanf("%d", &showID) != 1) { clearInputBuffer(); printf("[!] Invalid input.\n"); return; }
    clearInputBuffer();

    int si = findShowIndex(showID);
    if (si < 0) { printf("[!] Show ID %d not found.\n", showID); return; }

    /* Show seat map */
    printSeatMap(si);

    /* Number of tickets */
    int n;
    printf("How many seats to book (1–%d)? ", TOTAL_SEATS);
    if (scanf("%d", &n) != 1 || n < 1 || n > TOTAL_SEATS) {
        clearInputBuffer();
        printf("[!] Invalid number of seats.\n");
        return;
    }
    clearInputBuffer();

    /* Check enough seats available */
    int available = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (!shows[si].seats[r][c]) available++;
    if (n > available) {
        printf("[!] Only %d seats available.\n", available);
        return;
    }

    /* Customer name */
    char name[MAX_NAME];
    printf("Enter customer name: ");
    fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) { printf("[!] Name cannot be empty.\n"); return; }

    /* Select each seat */
    Booking bk;
    memset(&bk, 0, sizeof(bk));
    bk.seatCount = n;
    bk.showID    = showID;
    strncpy(bk.customerName, name, MAX_NAME - 1);

    for (int k = 0; k < n; k++) {
        int row, col;
        while (1) {
            printf("  Seat %d/%d – Enter row (1-%d) and column (1-%d): ",
                   k+1, n, ROWS, COLS);
            if (scanf("%d %d", &row, &col) != 2) {
                clearInputBuffer();
                printf("  [!] Enter two numbers.\n");
                continue;
            }
            clearInputBuffer();
            if (!validateSeat(si, row - 1, col - 1)) {
                printf("  [!] Seat R%dC%d is invalid or already booked. Try again.\n", row, col);
            } else {
                bk.seatRows[k] = row - 1;
                bk.seatCols[k] = col - 1;
                shows[si].seats[row-1][col-1] = 1;   /* mark booked */
                break;
            }
        }
    }

    /* Finalise booking */
    bk.totalAmount = n * shows[si].price;
    generateBookingID(bk.bookingID);

    if (bookingCount >= MAX_BOOKINGS) {
        printf("[!] Booking limit reached.\n");
        /* undo seat marks */
        for (int k = 0; k < n; k++)
            shows[si].seats[bk.seatRows[k]][bk.seatCols[k]] = 0;
        return;
    }

    bookings[bookingCount++] = bk;
    saveBookings();
    printReceipt(bookingCount - 1);
}

/* ═══════════════════════════════════════════════════════════
   4. VIEW BOOKING BY ID
   ═══════════════════════════════════════════════════════════ */
void viewBooking(void) {
    char bid[16];
    printf("\nEnter Booking ID: ");
    fgets(bid, sizeof(bid), stdin);
    bid[strcspn(bid, "\n")] = '\0';

    int idx = findBookingIndex(bid);
    if (idx < 0) {
        printf("[!] Booking '%s' not found.\n", bid);
        return;
    }
    printReceipt(idx);
}

/* ═══════════════════════════════════════════════════════════
   5. OCCUPANCY REPORT
   ═══════════════════════════════════════════════════════════ */
void occupancyReport(void) {
    printf("\n════════════ OCCUPANCY REPORT ════════════\n");
    printf("%-28s %5s %6s %6s %8s\n",
           "Show", "Total", "Booked", "Avail", "Occ%");
    printf("──────────────────────────────────────────\n");
    for (int i = 0; i < showCount; i++) {
        int booked = 0;
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                if (shows[i].seats[r][c]) booked++;
        int avail = TOTAL_SEATS - booked;
        float pct = (booked * 100.0f) / TOTAL_SEATS;
        printf("%-28s %5d %6d %6d %7.1f%%\n",
               shows[i].title, TOTAL_SEATS, booked, avail, pct);
    }
    printf("══════════════════════════════════════════\n");
}

/* ═══════════════════════════════════════════════════════════
   FILE HANDLING – SAVE
   ═══════════════════════════════════════════════════════════ */
void saveBookings(void) {
    FILE *fp = fopen(BOOKING_FILE, "wb");
    if (!fp) { printf("[!] Could not save bookings.\n"); return; }

    /* Save show seat states */
    for (int i = 0; i < showCount; i++)
        fwrite(shows[i].seats, sizeof(shows[i].seats), 1, fp);

    /* Save booking records */
    fwrite(&bookingCount, sizeof(int), 1, fp);
    fwrite(bookings, sizeof(Booking), bookingCount, fp);

    fclose(fp);
}

/* ═══════════════════════════════════════════════════════════
   FILE HANDLING – LOAD
   ═══════════════════════════════════════════════════════════ */
void loadBookings(void) {
    FILE *fp = fopen(BOOKING_FILE, "rb");
    if (!fp) return;   /* first run – file doesn't exist yet */

    for (int i = 0; i < showCount; i++)
        fread(shows[i].seats, sizeof(shows[i].seats), 1, fp);

    fread(&bookingCount, sizeof(int), 1, fp);
    if (bookingCount > MAX_BOOKINGS) bookingCount = 0;
    fread(bookings, sizeof(Booking), bookingCount, fp);

    fclose(fp);
    printf("[✓] Loaded %d existing booking(s).\n", bookingCount);
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Print Receipt
   ═══════════════════════════════════════════════════════════ */
void printReceipt(int idx) {
    Booking *b = &bookings[idx];
    int si = findShowIndex(b->showID);
    printf("\n╔══════════════ BOOKING RECEIPT ═══════════════╗\n");
    printf("║  Booking ID  : %-30s ║\n", b->bookingID);
    printf("║  Customer    : %-30s ║\n", b->customerName);
    if (si >= 0) {
        printf("║  Show        : %-30s ║\n", shows[si].title);
        printf("║  Time        : %-30s ║\n", shows[si].time);
    }
    printf("║  Seats       : ");
    for (int k = 0; k < b->seatCount; k++) {
        printf("R%dC%d", b->seatRows[k]+1, b->seatCols[k]+1);
        if (k < b->seatCount - 1) printf(", ");
    }
    printf("\n");
    printf("║  Total Paid  : Rs %-27.2f ║\n", b->totalAmount);
    printf("╚═══════════════════════════════════════════════╝\n");
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Print Seat Map
   ═══════════════════════════════════════════════════════════ */
void printSeatMap(int si) {
    printf("\n  Seat Map for \"%s\" [O=Available, X=Booked]\n\n", shows[si].title);
    printf("       ");
    for (int c = 0; c < COLS; c++) printf("C%-2d", c+1);
    printf("\n");
    for (int r = 0; r < ROWS; r++) {
        printf("  R%d  ", r+1);
        for (int c = 0; c < COLS; c++)
            printf(" %-2s", shows[si].seats[r][c] ? "X" : "O");
        printf("\n");
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Find show index by ID
   ═══════════════════════════════════════════════════════════ */
int findShowIndex(int id) {
    for (int i = 0; i < showCount; i++)
        if (shows[i].id == id) return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Find booking index by BookingID string
   ═══════════════════════════════════════════════════════════ */
int findBookingIndex(const char *bid) {
    for (int i = 0; i < bookingCount; i++)
        if (strcmp(bookings[i].bookingID, bid) == 0) return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Generate unique BookingID  (BK + timestamp + counter)
   ═══════════════════════════════════════════════════════════ */
void generateBookingID(char *buf) {
    static int counter = 0;
    snprintf(buf, 16, "BK%05ld%02d", (long)(time(NULL) % 100000), ++counter % 100);
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Validate seat (bounds + not already booked)
   ═══════════════════════════════════════════════════════════ */
int validateSeat(int showIdx, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return 0;
    if (shows[showIdx].seats[row][col]) return 0;
    return 1;
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Clear leftover characters from stdin
   ═══════════════════════════════════════════════════════════ */
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}