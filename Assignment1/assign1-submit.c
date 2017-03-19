/*
    This source code is created and compiled for 2016 Semester 2 COMP10002
    Assignment 1. This is the submission version for the whole project. It can
    read in and parse the tsv file.
    Author:         Tingsheng Lai
    Student Number: 781319
    Login Name:     tingshengl
    Date:           September, 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* macro implementation of max function */
#define max(a, b) ((a) > (b) ? (a) : (b))

/* Specified in the question */
#define MAXCOL 30           /* Maximum columns */
#define MAXLINE 1000        /* Maximum lines excluding heading */
#define MAXCHAR 50          /* Maximum characters in on cell */

/* Optimised cut-off value for combo sorting */
#define CUTOFF 9

/* Custom types for convenience */
typedef void * pointer;     /* Generic pointer type */
typedef char * string;      /* Represents each strings */
typedef string * line_t;    /* Represents a whole line */
typedef line_t * data_t;    /* Represents the whole table */

/* Data structure represents the whole tsv file */
typedef struct {
    line_t heading;             /* Record headings */
    data_t lines;               /* Storage of data */
    int colcount, rowcount;     /* Counting numbers of row and column */
    int *colmax;                /* Maximum length in one column */
} __tsv;


/************************** Function Prototypes *******************************/
void allocate(int, int, ...);
void reset(int, ...);
void clean(__tsv*, int **, int *);
int process_line(int, __tsv *, int);
int getch();
void get_tsv_column(__tsv *);
void parse_tsv_lines(__tsv *);
void print_formatted_line(int, const __tsv*, ...);
void tprint(int k, const char *format, ...);
void print_category(const __tsv*, const int[], int);
int compare(const line_t, const line_t, const int[], int);
void insert_sort(__tsv *, int, int, const int[], int);
void merge(__tsv *, int, int, int, const int[], int);
void merge_sort(__tsv *, int, int, const int[], int);
void sort(__tsv*, const int[], int);
int ceiling(int, int);
/******************************************************************************/

/* Main Function */
int main(int argc, string argv[]) {
    /* Initialisation */
    int i = 0;
    __tsv tsv;
    int *spec = NULL, specn = 0;
    specn = argc - 1;
    allocate(2, sizeof(int), &spec, specn, &tsv.colmax, MAXCOL);
    /*
        Avoid potential overflowing in parse_tsv_lines,
        so allocate MAXLINE + 1 for the data
    */
    allocate(2, sizeof(pointer), &tsv.heading, MAXCOL, &tsv.lines, MAXLINE + 1);
    get_tsv_column(&tsv);
    parse_tsv_lines(&tsv);

    /* Stage 1 */
    printf("Stage 1 Output\n");
    printf("input tsv data has %d rows and %d columns\n", tsv.rowcount,
           tsv.colcount);
    print_formatted_line(1, &tsv, tsv.rowcount - 1);
    printf("\n");

    /* Ignore stage 2 and 3 outputs if no parameters specified */
    if(argc == 1) {
        clean(&tsv, &spec, &specn);
        return 0;
    }

    /* Stage 2 */
    for(i = 0; i < specn; i++)
        spec[i] = atoi(argv[i + 1]);
    sort(&tsv, spec, specn);
    printf("Stage 2 Output\n");
    print_formatted_line(3, &tsv, 0, ceiling(tsv.rowcount, 2) - 1,
                         tsv.rowcount - 1);
    printf("\n");

    /* Stage 3 */
    printf("Stage 3 Output\n");
    print_category(&tsv, spec, specn);

    /* Clean and avoid memory leaking */
    clean(&tsv, &spec, &specn);
    return 0;
}


/*
    Function to allocate memory for same type of pointers.
    Parameters:
        n:      number of pointers
        size:   size of the type of the pointers
        (...):  variable length of parameters, it should be in pairs of
                (pointer *, size of the pointer) and the number of pairs should
                equal to n
    No return value
    Note:
        The maximum memory to be allocated will be approx. 3.2MB which is not
        very big, so ignored the if condition to enhance runtime efficiency.
        The total memory allowance on the testing server is 16GB where the
        usual free memory is about 6GB
*/
void allocate(int n, int size, ...) {
    int i = 0, num = 0;
    va_list vl;
    pointer *ptr = NULL;
    va_start(vl, size);
    for(; i < 2 * n; i += 2) {
        ptr = va_arg(vl, pointer *);
        num = va_arg(vl, int);
        *ptr = calloc(num, size);
    }
    va_end(vl);
}

/*
    Variable length, reset several pointers from parameters
    Parameters:
        n:      number of pointers
        (...):  variable length of parameters, all in type of pointer *, the
                number of pointers should conform to n
    No return value
*/
void reset(int n, ...) {
    int i = 0;
    va_list vl;
    pointer *ptr = NULL;
    va_start(vl, n);
    for(; i < n; i++) {
        ptr = va_arg(vl, pointer *);
        free(*ptr);
        *ptr = NULL;
    }
    va_end(vl);
}

/*
    Cleaning process, free all memory allocated
    Parameters:
        tsv:    pointer to a __tsv instance
        spec:   pointer to an int array which stores the specification
        specn:  pointer to an int which records the number of specification
    No return value
*/
void clean(__tsv *tsv, int **spec, int *specn) {
    int i = 0, j = 0;
    for(; i < tsv->colcount; i++) {
        reset(1, &tsv->heading[i]);
        for(j = 0; j < tsv->rowcount; j++)
            reset(1, &tsv->lines[j][i]);
    }
    for(i = 0; i < tsv->rowcount; i++)
        reset(1, &tsv->lines[i]);
    reset(4, &tsv->heading, &tsv->lines, &tsv->colmax, spec);
    tsv->colcount = tsv->rowcount = *specn = 0;
}

/*
    Custom getchar function, bypass \r
    No parameters needed
    (Referred to Professor Alistair Moffat on
     http://people.eng.unimelb.edu.au/ammoffat/teaching/10002/ass1/)
    Same return value as normal getchar()
*/
int getch() {
    int c = 0;
    while((c = getchar()) == '\r');
    return c;
}

/*
    Process every line consecutively
    Parameters:
        head:   a boolean type, 1 means the current line is heading, and
                0 means it is processing the data. If it is 1 then pos will
                be ignored
        pos:    indicates the current position in the file
    Return:
        The latest state returned by scanf
*/
int process_line(int head, __tsv *tsv, int pos) {
    char a[MAXCHAR + 1] = {0};

    /* Determine what is currently processing */
    line_t ptr = head ? tsv->heading : tsv->lines[pos];
    int count = 0, size = 0, status = 0;
    while((status = scanf("%[^\t\n\r]s", a)) != EOF) {
        /* Determine the maximum length in current column */
        size = strlen(a);
        tsv->colmax[count] = max(size, tsv->colmax[count]);

        /* Allocate space and store the value */
        allocate(1, sizeof(char), &ptr[count], size + 1);
        strncpy(ptr[count], a, size);

        /* Clear the content in a, avoid circumstance like \t\t */
        memset(a, 0, MAXCHAR + 1);
        count++;
        if(getch() == '\n')
            break;
    }

    /* If this is the heading line, alter the column counter */
    if(head)
        tsv->colcount = count;
    return status;
}

/*
    Get the column heading, and also count the number of column
    Parameters:
        tsv: the pointer to a __tsv instance
    No return value
*/
void get_tsv_column(__tsv *tsv) {
    /* Assign headings to the data storage */
    process_line(1, tsv, 0);

    /* Truncate the unused part for memory reuse */
    tsv->heading = realloc(tsv->heading, sizeof(pointer) * tsv->colcount);
    tsv->colmax = realloc(tsv->colmax, sizeof(int) * tsv->colcount);
}

/*
    Get all remaining data other than the header
    Parameters:
        same as above
    No return value
*/
void parse_tsv_lines(__tsv *tsv) {
    int count = 0;

    /* Assign all data to the storage */
    do {
        allocate(1, sizeof(pointer), &tsv->lines[count], tsv->colcount);
    } while(process_line(0, tsv, count++) != EOF);

    /*
        As in initial allocation in main function, it allocate one more line
        to avoid potential overflow, so it needs to be truncated first
    */
    reset(1, &tsv->lines[count]);
    count--;

    /* Truncate the unused part for memory reuse */
    tsv->lines = realloc(tsv->lines, sizeof(pointer) * count);

    /* Store the row counter */
    tsv->rowcount = count;
}

/*
    Print the lines selected in the format specified
    Parameters:
        n:      number of lines to be printed
        tsv:    pointer to a const __tsv instance
        (...):  the indices of the lines stored in tsv
    No return value
*/
void print_formatted_line(int n, const __tsv *tsv, ...) {
    int i = 0, j = 0, size = 0, row = 0;
    va_list vl;

    /* Determine the width which printf should use */
    for(; i < tsv->colcount; i++)
        size = max(size, strlen(tsv->heading[i]));

    /* Output the specified lines */
    va_start(vl, tsv);
    size = ceiling(size, 2) * 2 + 2;
    for(i = 0; i < n; i++) {
        row = va_arg(vl, int);

        /* Automatic formatting */
        printf("row %d is:\n", row + 1);
        for(j = 0; j < tsv->colcount; j++)
            printf("%4d: %-*s %s\n", j + 1, size, tsv->heading[j],
                   tsv->lines[row][j]);
    }
    va_end(vl);
}

/*
    Custom print function, wrapper of vprintf
    Parameters:
        k:      number of 4-spaces printed before hand
        format: conform to vprintf
        (...):  conform to vprintf
    No return value
*/
void tprint(int k, const char *format, ...) {
    va_list args;
    int i = 0;

    /* Print the leading spaces */
    for(; i < k; i++)
        printf("    ");

    /* Pass the remaining arguments to vprintf */
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/*
    Stage 3 printing function, categorise the data as well
    Parameters:
        tsv:    pointer to a const __tsv instance
        spec:   specification passed from the main function
        specn:  indicates the number of elements in spec
    No return value
*/
void print_category(const __tsv *tsv, const int spec[], int specn) {
    int width = tsv->colmax[spec[specn - 1] - 1],
        totwidth = width + 2 + 4 * specn, i = 0, j = 0, count = 0;

    /* Initialise the separate line */
    string sl = NULL;
    allocate(1, sizeof(char), &sl, totwidth + 1);
    memset(sl, '-', totwidth);

    /* Print the heading and separate line */
    printf("%s\n", sl);
    for(; i < specn - 1; i++)
        tprint(i, "%s\n", tsv->heading[spec[i] - 1]);
    tprint(i, "%-*s Count\n", width, tsv->heading[spec[i] - 1]);
    printf("%s\n", sl);

    /* Print the main categorisation */
    for(i = 0; i < specn - 1; i++)
        tprint(i, "%s\n", tsv->lines[0][spec[i] - 1]);

    /* Start comparing each line */
    for(count = 1, i = 1; i < tsv->rowcount; i++, count++)
        for(j = 0; j < specn; j++)
            if(strcmp(tsv->lines[i - 1][spec[j] - 1],
                      tsv->lines[i][spec[j] - 1])) {
                tprint(specn - 1, "%-*s %5d\n", width,
                       tsv->lines[i - 1][spec[specn - 1] - 1], count);
                count = 0;
                for(; j < specn - 1; j++)
                    tprint(j, "%s\n", tsv->lines[i][spec[j] - 1]);
            }
    /* Print the last line */
    tprint(specn - 1, "%-*s %5d\n", width,
           tsv->lines[i - 1][spec[specn - 1] - 1], count);

    /* Finalise the printing */
    printf("%s\n", sl);
    reset(1, &sl);
}

/*
    Comparison function for the merge sort, specialisation
    Parameters:
        lhs:    left hand side of the comparison
        rhs:    right hand side of the comparison
        spec:   as above
        specn:  as above
    Return:
        The last result returned by strcmp where the first difference
        encountered
*/
int compare(const line_t lhs, const line_t rhs, const int spec[], int specn) {
    int i = 0, a = 0, result = 0;

    /* Loop over all specified columns */
    while(!result && i < specn) {
        a = spec[i] - 1;
        result = strcmp(lhs[a], rhs[a]);
        i++;
    }
    return result;
}

/*
    For combo, define the insertion sort
    Parameters:
        tsv:    pointer to a __tsv instance
        p:      start of the index
        r:      end of the index
        spec:   as above
        specn:  as above
    No return value
*/
void insert_sort(__tsv *tsv, int p, int r, const int spec[], int specn) {
    int i = p + 1, j = i;
    for(; i < r; i++) {
        line_t key = tsv->lines[i];
        for(j = i - 1; j >= p && compare(key, tsv->lines[j], spec, specn) < 0;
            j--)
            tsv->lines[j + 1] = tsv->lines[j];
        tsv->lines[j + 1] = key;
    }
}

/*
    Merging process for merge sort
    Parameters:
        tsv:    as above
        p:      as above
        q:      the middle of the index
        r:      as above
        spec:   as above
        specn:  as above
    No return value
*/
void merge(__tsv *tsv, int p, int q, int r, const int spec[], int specn) {
    int i = p, j = q, k = 0, lsize = q - p, rsize = r - q;

    /* Pre-initialisation */
    data_t left = NULL, right = NULL;
    allocate(2, sizeof(pointer), &left, lsize, &right, rsize);
    for(i = p; i < q; i++)
        left[k++] = tsv->lines[i];
    for(k = 0, j = q; j < r; j++)
        right[k++] = tsv->lines[j];

    /* Main process of merging */
    i = j = 0;
    for(k = p; k < r; k++) {
        if(j == rsize || (i != lsize &&
                          compare(left[i], right[j], spec, specn) <= 0))
            tsv->lines[k] = left[i++];
        else
            tsv->lines[k] = right[j++];
    }

    /* Free memory */
    reset(2, &left, &right);
}

/*
    Actual implementation of merge sort
    Parameters:
        (All same as insert_sort)
    No return value
*/
void merge_sort(__tsv *tsv, int p, int r, const int spec[], int specn) {
    /* Avoid integer overflowing */
    int q = p + (r - p) / 2;

    /* Optimised cut-off value */
    if(r - p < CUTOFF) {
        insert_sort(tsv, p, r, spec, specn);
        return;
    }

    merge_sort(tsv, p, q, spec, specn);
    merge_sort(tsv, q, r, spec, specn);
    merge(tsv, p, q, r, spec, specn);
}

/*
    Wrapper of merge sort combine with insertion sort, all interval is [0, n)
    Parameters:
        tsv:    as above
        spec:   as above
        specn:  as above
    No return value
*/
void sort(__tsv *tsv, const int spec[], int specn) {
    merge_sort(tsv, 0, tsv->rowcount, spec, specn);
}


/*
    Ceiling function with custom implementation
    Parameters:
        n:  numerator
        d:  denominator
    Return:
        Same as ceil(n/d) which is defined in stdlib.h but it is a int type
*/
int ceiling(int n, int d) {
    if(n % d)
        n += d;
    return n / d;
}

/*
    Algorithms Are DEFINITELY Fun!
    COMP10002 is the best subject EVER!
    Hope you win the lottery next time!
        to Professor Alistair Moffat
*/
