/*
 * Project  : Railway Reservation System (Project 12)
 * Subject  : C Programming
 * Topics   : Arrays, Structures, File Handling
 * Name     : ____________
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILE_NAME "tickets.dat"
#define TOTAL_TRAINS 5
#define BERTHS 16       /* berths in each class */
#define WL_LIMIT 6      /* max waiting list in each class */
#define MAX_PASS 4      /* max passengers in one ticket */

typedef struct
{
    int trainNo;
    char trainName[30];
    char source[20];
    char destination[20];
    int fare[3];        /* fare[0] = SL, fare[1] = 3A, fare[2] = 2A */
} Train;

typedef struct
{
    char name[30];
    int age;
    char gender;
    int berth;          /* 0 means waiting list */
} Passenger;

typedef struct
{
    int pnr;
    int trainNo;
    int cls;            /* 0 = SL, 1 = 3A, 2 = 2A */
    char date[12];
    int count;
    Passenger p[MAX_PASS];
    int amount;
} Ticket;

Train trains[TOTAL_TRAINS] =
{
    {12904, "Golden Temple Mail", "Amritsar", "Mumbai", {765, 2010, 2925}},
    {12625, "Kerala Express", "Trivandrum", "New Delhi", {905, 2350, 3420}},
    {12839, "Howrah Mail", "Howrah", "Chennai", {720, 1890, 2715}},
    {12417, "Prayagraj Express", "Prayagraj", "New Delhi", {395, 1045, 1480}},
    {12715, "Sachkhand Express", "Nanded", "Amritsar", {840, 2205, 3180}}
};

char className[3][20] = {"Sleeper (SL)", "AC 3 Tier (3A)", "AC 2 Tier (2A)"};
char coachName[3][4] = {"S1", "B1", "A1"};
char berthType[8][3] = {"LB", "MB", "UB", "LB", "MB", "UB", "SL", "SU"};
int cancelCharge[3] = {120, 180, 200};   /* per confirmed passenger */


int find_train(int no)
{
    int i;
    for (i = 0; i < TOTAL_TRAINS; i++)
    {
        if (trains[i].trainNo == no)
            return i;
    }
    return -1;
}

void show_trains()
{
    int i;
    printf("\n%-7s %-20s %-11s %-11s %-6s %-6s %-6s\n", "No.", "Train", "From", "To", "SL", "3A", "2A");
    printf("------------------------------------------------------------------------\n");
    for (i = 0; i < TOTAL_TRAINS; i++)
    {
        printf("%-7d %-20s %-11s %-11s %-6d %-6d %-6d\n", trains[i].trainNo, trains[i].trainName,
               trains[i].source, trains[i].destination, trains[i].fare[0], trains[i].fare[1], trains[i].fare[2]);
    }
}

/* reads date as dd-mm-yyyy and stores it in same format, returns 0 if wrong */
int read_date(char date[])
{
    char temp[20];
    int d, m, y;

    printf("Enter journey date (dd-mm-yyyy): ");
    scanf("%19s", temp);
    if (sscanf(temp, "%d-%d-%d", &d, &m, &y) != 3)
        return 0;
    if (d < 1 || d > 31 || m < 1 || m > 12 || y < 2025 || y > 2030)
        return 0;
    sprintf(date, "%02d-%02d-%04d", d, m, y);
    return 1;
}

/* marks which berths are taken in booked[] and returns waiting list count */
int get_status(int trainNo, int cls, char date[], int booked[])
{
    FILE *fp;
    Ticket t;
    int i, wl = 0;

    for (i = 0; i <= BERTHS; i++)
        booked[i] = 0;

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
        return 0;

    while (fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.trainNo == trainNo && t.cls == cls && strcmp(t.date, date) == 0)
        {
            for (i = 0; i < t.count; i++)
            {
                if (t.p[i].berth == 0)
                    wl++;
                else
                    booked[t.p[i].berth] = 1;
            }
        }
    }
    fclose(fp);
    return wl;
}

int pnr_exists(int pnr)
{
    FILE *fp;
    Ticket t;

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
        return 0;
    while (fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.pnr == pnr)
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* current waiting list number of passenger k in ticket t */
int wl_position(Ticket t, int k)
{
    FILE *fp;
    Ticket r;
    int i, pos = 0;

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
        return 0;

    while (fread(&r, sizeof(Ticket), 1, fp) == 1)
    {
        if (r.trainNo == t.trainNo && r.cls == t.cls && strcmp(r.date, t.date) == 0)
        {
            for (i = 0; i < r.count; i++)
            {
                if (r.p[i].berth == 0)
                {
                    pos++;
                    if (r.pnr == t.pnr && i == k)
                    {
                        fclose(fp);
                        return pos;
                    }
                }
            }
        }
    }
    fclose(fp);
    return 0;
}

void print_ticket(Ticket t)
{
    int i, idx;
    idx = find_train(t.trainNo);

    printf("\n==================================================\n");
    printf(" PNR      : %d\n", t.pnr);
    printf(" Train    : %d %s\n", t.trainNo, trains[idx].trainName);
    printf(" Journey  : %s to %s\n", trains[idx].source, trains[idx].destination);
    printf(" Date     : %s\n", t.date);
    printf(" Class    : %s\n", className[t.cls]);
    printf("--------------------------------------------------\n");
    printf(" %-3s %-20s %-4s %-4s %s\n", "No", "Name", "Age", "Sex", "Status");
    for (i = 0; i < t.count; i++)
    {
        printf(" %-3d %-20s %-4d %-4c ", i + 1, t.p[i].name, t.p[i].age, t.p[i].gender);
        if (t.p[i].berth == 0)
            printf("WL %d\n", wl_position(t, i));
        else
            printf("CNF %s/%d/%s\n", coachName[t.cls], t.p[i].berth, berthType[(t.p[i].berth - 1) % 8]);
    }
    printf("--------------------------------------------------\n");
    printf(" Total Fare : Rs %d\n", t.amount);
    printf("==================================================\n");
}

void check_availability()
{
    int no = 0, idx, c, i, wl, avail;
    int booked[BERTHS + 1];
    char date[12];

    printf("\nEnter train number: ");
    scanf("%d", &no);
    idx = find_train(no);
    if (idx == -1)
    {
        printf("Train not found!\n");
        return;
    }
    if (read_date(date) == 0)
    {
        printf("Invalid date!\n");
        return;
    }

    printf("\n%d %s (%s to %s) on %s\n", no, trains[idx].trainName, trains[idx].source, trains[idx].destination, date);
    printf("----------------------------------------------\n");
    for (c = 0; c < 3; c++)
    {
        wl = get_status(no, c, date, booked);
        avail = 0;
        for (i = 1; i <= BERTHS; i++)
        {
            if (booked[i] == 0)
                avail++;
        }

        if (avail > 0)
            printf("%-16s : AVAILABLE %d\n", className[c], avail);
        else if (wl < WL_LIMIT)
            printf("%-16s : WL %d\n", className[c], wl + 1);
        else
            printf("%-16s : REGRET\n", className[c]);
    }
}

void book_ticket()
{
    Ticket t = {0};
    FILE *fp;
    int idx, i, j, wl, avail, total;
    int booked[BERTHS + 1];
    char ch;

    show_trains();
    printf("\nEnter train number: ");
    scanf("%d", &t.trainNo);
    idx = find_train(t.trainNo);
    if (idx == -1)
    {
        printf("Train not found!\n");
        return;
    }

    if (read_date(t.date) == 0)
    {
        printf("Invalid date!\n");
        return;
    }

    printf("\nSelect class\n");
    for (i = 0; i < 3; i++)
        printf("%d. %s - Rs %d\n", i + 1, className[i], trains[idx].fare[i]);
    printf("Enter choice: ");
    scanf("%d", &t.cls);
    if (t.cls < 1 || t.cls > 3)
    {
        printf("Invalid class!\n");
        return;
    }
    t.cls--;    /* because array index starts from 0 */

    wl = get_status(t.trainNo, t.cls, t.date, booked);
    avail = 0;
    for (i = 1; i <= BERTHS; i++)
    {
        if (booked[i] == 0)
            avail++;
    }
    printf("Berths available: %d | Waiting list: %d\n", avail, wl);

    printf("Number of passengers (1-%d): ", MAX_PASS);
    scanf("%d", &t.count);
    if (t.count < 1 || t.count > MAX_PASS)
    {
        printf("Invalid number of passengers!\n");
        return;
    }
    if (t.count > avail + (WL_LIMIT - wl))
    {
        printf("REGRET! Not enough berths or waiting list left.\n");
        return;
    }

    for (i = 0; i < t.count; i++)
    {
        printf("\n-- Passenger %d --\n", i + 1);
        printf("Name: ");
        scanf(" %29[^\n]", t.p[i].name);
        printf("Age: ");
        scanf("%d", &t.p[i].age);
        if (t.p[i].age < 1 || t.p[i].age > 120)
        {
            printf("Invalid age!\n");
            return;
        }
        printf("Gender (M/F/O): ");
        scanf(" %c", &t.p[i].gender);
        if (t.p[i].gender >= 'a' && t.p[i].gender <= 'z')
            t.p[i].gender = t.p[i].gender - 32;    /* small to capital */
        if (t.p[i].gender != 'M' && t.p[i].gender != 'F' && t.p[i].gender != 'O')
        {
            printf("Invalid gender!\n");
            return;
        }

        /* berth allotment - senior citizens get lower berth if any is free */
        t.p[i].berth = 0;
        if (t.p[i].age >= 60)
        {
            for (j = 1; j <= BERTHS; j++)
            {
                if (booked[j] == 0 && strcmp(berthType[(j - 1) % 8], "LB") == 0)
                {
                    t.p[i].berth = j;
                    break;
                }
            }
        }
        if (t.p[i].berth == 0)
        {
            for (j = 1; j <= BERTHS; j++)
            {
                if (booked[j] == 0)
                {
                    t.p[i].berth = j;
                    break;
                }
            }
        }
        if (t.p[i].berth != 0)
            booked[t.p[i].berth] = 1;     /* if still 0 then passenger is in WL */
    }

    total = trains[idx].fare[t.cls] * t.count;
    if (t.cls > 0)
        total = total + (total * 5) / 100;     /* 5% GST on AC classes */
    t.amount = total;

    printf("\nTotal fare: Rs %d", t.amount);
    if (t.cls > 0)
        printf(" (incl. 5%% GST)");
    printf("\nConfirm booking? (y/n): ");
    scanf(" %c", &ch);
    if (ch != 'y' && ch != 'Y')
    {
        printf("Booking not done.\n");
        return;
    }

    /* random 6 digit pnr, checking that it is not already used */
    do
    {
        t.pnr = 100000 + rand() % 900000;
    } while (pnr_exists(t.pnr));

    fp = fopen(FILE_NAME, "ab");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return;
    }
    fwrite(&t, sizeof(Ticket), 1, fp);
    fclose(fp);

    printf("\nTicket booked!!");
    print_ticket(t);
}

void pnr_status()
{
    FILE *fp;
    Ticket t;
    int pnr = 0, found = 0;

    printf("\nEnter PNR: ");
    scanf("%d", &pnr);

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
    {
        printf("No bookings yet!\n");
        return;
    }
    while (fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.pnr == pnr)
        {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (found == 1)
        print_ticket(t);
    else
        printf("Invalid PNR!\n");
}

/* gives the freed berths to waiting list passengers (first come first serve) */
void promote_waiting(int trainNo, int cls, char date[], int freed[], int nfreed)
{
    FILE *fp;
    Ticket t;
    int i, k = 0, changed;

    fp = fopen(FILE_NAME, "rb+");
    if (fp == NULL)
        return;

    while (k < nfreed && fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.trainNo == trainNo && t.cls == cls && strcmp(t.date, date) == 0)
        {
            changed = 0;
            for (i = 0; i < t.count && k < nfreed; i++)
            {
                if (t.p[i].berth == 0)
                {
                    t.p[i].berth = freed[k];
                    k++;
                    changed = 1;
                    printf("PNR %d - %s is now CONFIRMED (%s/%d)\n", t.pnr, t.p[i].name, coachName[cls], t.p[i].berth);
                }
            }
            if (changed == 1)
            {
                fseek(fp, -(long)sizeof(Ticket), SEEK_CUR);   /* go back to start of this record */
                fwrite(&t, sizeof(Ticket), 1, fp);
                fseek(fp, 0, SEEK_CUR);    /* have to do this before reading again */
            }
        }
    }
    fclose(fp);
}

void cancel_ticket()
{
    FILE *fp, *temp;
    Ticket t, c;
    int pnr = 0, found = 0, i, charge = 0, refund;
    int freed[MAX_PASS], nfreed = 0;
    char ch;

    printf("\nEnter PNR to cancel: ");
    scanf("%d", &pnr);

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
    {
        printf("No bookings yet!\n");
        return;
    }
    while (fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.pnr == pnr)
        {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (found == 0)
    {
        printf("Invalid PNR!\n");
        return;
    }
    c = t;

    print_ticket(c);
    printf("Cancel this ticket? (y/n): ");
    scanf(" %c", &ch);
    if (ch != 'y' && ch != 'Y')
    {
        printf("Ticket not cancelled.\n");
        return;
    }

    for (i = 0; i < c.count; i++)
    {
        if (c.p[i].berth == 0)
        {
            charge = charge + 60;      /* WL ticket - only clerkage charge */
        }
        else
        {
            charge = charge + cancelCharge[c.cls];
            freed[nfreed] = c.p[i].berth;
            nfreed++;
        }
    }
    refund = c.amount - charge;
    if (refund < 0)
        refund = 0;

    /* copy every ticket except this one into temp file */
    fp = fopen(FILE_NAME, "rb");
    temp = fopen("temp.dat", "wb");
    if (fp == NULL || temp == NULL)
    {
        printf("Error opening file!\n");
        return;
    }
    while (fread(&t, sizeof(Ticket), 1, fp) == 1)
    {
        if (t.pnr != pnr)
            fwrite(&t, sizeof(Ticket), 1, temp);
    }
    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);

    printf("\nTicket cancelled.\n");
    printf("Cancellation charges : Rs %d\n", charge);
    printf("Refund amount        : Rs %d\n", refund);

    if (nfreed > 0)
        promote_waiting(c.trainNo, c.cls, c.date, freed, nfreed);
}

void show_chart()
{
    FILE *fp;
    Ticket t;
    char pass[20], date[12];
    int no = 0, idx, c, i, count, total = 0;

    printf("\nEnter admin password: ");
    scanf("%19s", pass);
    if (strcmp(pass, "admin123") != 0)
    {
        printf("Wrong password!\n");
        return;
    }

    printf("Enter train number: ");
    scanf("%d", &no);
    idx = find_train(no);
    if (idx == -1)
    {
        printf("Train not found!\n");
        return;
    }
    if (read_date(date) == 0)
    {
        printf("Invalid date!\n");
        return;
    }

    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL)
    {
        printf("No bookings yet!\n");
        return;
    }

    printf("\n************ RESERVATION CHART ************\n");
    printf("%d %s | %s\n", no, trains[idx].trainName, date);

    for (c = 0; c < 3; c++)
    {
        printf("\n[%s]\n", className[c]);
        printf("%-8s %-20s %-4s %-4s %s\n", "PNR", "Name", "Age", "Sex", "Berth");
        count = 0;
        rewind(fp);     /* start reading file again for every class */
        while (fread(&t, sizeof(Ticket), 1, fp) == 1)
        {
            if (t.trainNo == no && t.cls == c && strcmp(t.date, date) == 0)
            {
                for (i = 0; i < t.count; i++)
                {
                    printf("%-8d %-20s %-4d %-4c ", t.pnr, t.p[i].name, t.p[i].age, t.p[i].gender);
                    if (t.p[i].berth == 0)
                        printf("WL %d\n", wl_position(t, i));
                    else
                        printf("%s/%d/%s\n", coachName[c], t.p[i].berth, berthType[(t.p[i].berth - 1) % 8]);
                    count++;
                }
                total = total + t.amount;
            }
        }
        if (count == 0)
            printf("No passengers\n");
    }
    fclose(fp);
    printf("\nTotal collection: Rs %d\n", total);
}

int main()
{
    char choice = ' ';

    srand(time(NULL));

    do
    {
        printf("\n\n*******************************************\n");
        printf("     TRAIN TICKET RESERVATION SYSTEM\n");
        printf("*******************************************\n");
        printf("1. List of Trains\n");
        printf("2. Check Availability\n");
        printf("3. Book Ticket\n");
        printf("4. PNR Status\n");
        printf("5. Cancel Ticket\n");
        printf("6. Reservation Chart (Admin)\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (scanf(" %c", &choice) != 1)
            break;

        switch (choice)
        {
        case '1':
            show_trains();
            break;
        case '2':
            check_availability();
            break;
        case '3':
            book_ticket();
            break;
        case '4':
            pnr_status();
            break;
        case '5':
            cancel_ticket();
            break;
        case '6':
            show_chart();
            break;
        case '0':
            printf("\nHappy Journey! Bye.\n");
            break;
        default:
            printf("\nWrong choice!\n");
        }
    } while (choice != '0');

    return 0;
}
