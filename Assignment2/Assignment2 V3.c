/*Authorship:
  Name: Siyi Guo
  Student number: 737008
  Login name: siyig1;
  created: 16/10/2016;
  modified: 17/10/2016;
  This program takes an file and bunch of command line argument as input.
  it has 3 stages:
  stage1: 
    open the file and store data into a customice dynamic data structure.
  stage2:
    read command line argument from user as keyword, 
    and find item using binary search
  stage3:
    search for the keyword in all documentation in the given file,
    and return the top3 document that has strongest match.
    where the match is detemined by the score that is given by a formula*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXCHAR 999 /*for all stages*/
#define MAXQUER 20 /*for command line argument from stage2 and stage 3*/
#define SPACE 32 /*utf code for space*/
#define K 1.2
#define B 0.75
/*****************************************************************************/
/*structure for data that scanned in stage2 and used din stage 3*/
typedef struct {
    int index;
    double score;
}doc_pair;

/*structure for <d, fdt> pair for each word*/
typedef struct {
    int d;
    int fdt;
}pair;

/*structure for each word*/
typedef struct {
    int length;
    char *word;
    int ft;
    pair *d_fdt;
}Dictionary;

/*structture to store data in stage 1*/
typedef struct {
    /*thi sis to create dynamic array*/
    Dictionary *word_d;
    size_t size;

    /*number of documents in collections*/
    int N; 

    /*dynamic array for each document length*/
    /*document n'd length is sotred in index n*/
    /*useful in stage2 and stage 3*/
    int *doc_len; 
    size_t size_doc;

    double avg_doc_len;
}Array;
/*****************************************************************************/
/*basic small tool*/
void display_pair(pair *head_p, int total_pair);
int mygetchar();

/*function that doing most jobs*/
Array* scan_data(char *file_name, int *total_pair_p, int *total_word_p, 
                 Array *text);

void stage1(Array *text, int total_pair, int total_word);

void stage2and3(Array *text, int *total_term);

void stage2(Array *text, char (*query)[MAXCHAR + 1], 
            int num_query, int *total_term, int (*key_doc_ind));
int binary_search(Array *text, char *word, int total_term);

void stage3(Array *text, int num_query, int (*key_doc_ind));
void best3(double (*doc_score), doc_pair (*top3), double N);
/*****************************************************************************/
int main(int argc, char* argv[])
{
    int total_pair = 0;
    int total_word = 0;
    char* file_name = argv[1];

    /*scan data with dynamic arrays*/
    Array text;
    scan_data(file_name, &total_pair, &total_word, &text);

    /*stage1*/
    stage1(&text, total_pair, total_word);

    /*stage2and3*/
    stage2and3(&text, &total_word);

    /*this is the end, with the most magic sentence in this semester*/
    printf("Ta da-da-daaah...\n");

    return 0;
}
/*****************************************************************************/
Array* scan_data(char *file_name, int *total_pair_p, int *total_word_p, 
                 Array *text)
{
    /*this part opens a file and scan the data
      scanning store the basic information for each word read.
      but also count:
      1. number of document.
      2. each document length.
      3. average document length.
      4. total word scanned.
      5. total <d, fdt> pair scanned*/

    FILE *collection = fopen(file_name, "r");

    /*if not open successfule*/
    if (collection == NULL)
    {
        printf("No such file exist\n");

        /*if there is no such file, exit*/
        exit(EXIT_FAILURE);
    }

    /*if open successfully*/
    char string[MAXCHAR+1];
    int next_is_ft = 0;
    int num_terms = 0; /*count total <d,fdt> pair for current word*/
    int total_word = 0; /*total word in the file*/
    int total_pair = 0; /*total pairs in the file*/
    int index_for_pair = 0; /*using for store data for <d, fdt> pair*/
    double total_document_length = 0;

    /*initialize dynamic array for word*/
    text->word_d = malloc(2 * sizeof(Dictionary));

    if (text->word_d == NULL) /*check memory*/
    {
        printf("Out of Memory!\n");
        exit(EXIT_FAILURE);
    }

    text->size = 2;
    text->N = 0;

    /*initialize dynamic array for <d, fdt>*/
    text->doc_len = calloc(2, 2 * sizeof(int));
    text->size_doc = 2;


    /*scanning data */
    while ((fscanf(collection, "%s", string))!= EOF)
    {
        /*alloc exact memory for string that have read*/
        int length = strlen(string);
        char *str = malloc(length + 1);

        
        if (str != NULL) /*check memory*/
        {
            strcpy(str, string);
        }

        /*if this is a word*/
        if (isalpha(str[0]))
        {
            /*count for number of items*/
            total_word++;

            /*expand space for word*/
            if (total_word > text->size)
            {
                text->size *= 2;
                text->word_d = realloc(text->word_d, 
                                       text->size * sizeof(Dictionary));
            }
                
            /*store word and word length into dynamic array*/
            text->word_d[total_word-1].word = malloc(length+1);

            if (text->word_d[total_word-1].word == NULL) /*check memory*/
            {
                printf("Out of Memory!\n");
                exit(EXIT_FAILURE);
            }

            memcpy(text->word_d[total_word-1].word, str, length+1);
            text->word_d[total_word-1].length = length;

            /* flag next number is ft*/
            next_is_ft = 1;

            /*count for <d, fdt> pair in current word*/
            num_terms = 1;
            index_for_pair = 0;
        }   

        /*if it is a number*/
        else if (isdigit(str[0]))
        {
            /*if this number is ft number*/
            if (next_is_ft) 
            {
                int ft = atoi(str);
                /*flag next_number is not ft*/
                next_is_ft = 0;

                /*strore ft*/
                text->word_d[total_word-1].ft = ft;

                /*count total number of pairs*/
                total_pair += ft;

                /*allocate correct length for <d, fdt> pair*/
                text->word_d[total_word-1].d_fdt 
                        = malloc(ft * sizeof(pair));

                if (text->word_d[total_word-1].d_fdt == NULL) /*check memory*/
                {
                    printf("Out of Memory!\n");
                    exit(EXIT_FAILURE);
                }

            }

            /*store each <d, fdt> pari*/
            else 
            {
                if (num_terms %2 != 0) /*if this is d*/
                {
                    int d = atoi(str);
                    /*when num_pair is odd number again, store this new pair*/  
                    text->word_d[total_word-1].d_fdt[index_for_pair].d = d;

                    /*create dynamic array to store length of each document*/
                    if (d > text->size_doc)
                    {
                        /*expand extra memory for new document*/
                        text->doc_len = realloc(text->doc_len, 
                                                (d + 1) * sizeof(int));

                        /*initial the extra memory*/
                        int i;
                        for( i = text->size_doc + 1; i < d + 1; i++)
                        {
                            text->doc_len[i] = 0;
                        }

                        /*update the size of this new array*/
                        text->size_doc = d;
                    }
                    
                    /*count number of documents in this collections*/
                    if (d > text->N)
                    {
                        text->N = d;
                    }
                }
                else /*if this is fdt*/
                {
                    int fdt = atoi(str);

                    /*store fdt*/
                    text->word_d[total_word-1].d_fdt[index_for_pair].fdt = fdt;

                    /*update document d's length*/
                    /*note document N has doc_number = N*/
                    /*it wast 1 sapce of int for understandiability*/
                    int doc_number = 
                        text->word_d[total_word-1].d_fdt[index_for_pair].d;
                    text->doc_len[doc_number] += fdt;
                    total_document_length += fdt;

                    /*index for pair has increased*/
                    index_for_pair++;
                }
            } 
        }   
        free(str);
        num_terms++;
    }

    /*close the file after scanning data*/
    fclose(collection);

    /*calculate average document length*/
    text->avg_doc_len = total_document_length / text->N;

    /*returning total pair and word that has scanned*/
    *total_word_p = total_word;
    *total_pair_p = total_pair;
    return text;
}

void stage1(Array *text, int total_pair, int total_word)
{
    /*stage 1 output total word and <d, fdt> pair that has been scanned
      and output the first and last two term with their <d, fdt> pair
      data.
      Note, <d, fdt> pair only show up to 10 pairs*/
    printf("Stage 1 Output\n");
    printf("index has %d terms and %d (d,fdt) pairs\n", total_word, 
            total_pair);

    printf("term 1 is \"%s\":\n", text->word_d[0].word);
    display_pair(text->word_d[0].d_fdt, text->word_d[0].ft);

    printf("term 2 is \"%s\":\n", text->word_d[1].word);
    display_pair(text->word_d[1].d_fdt, text->word_d[1].ft);

    printf("term %d is \"%s\":\n", total_word - 1, 
            text->word_d[total_word - 2].word);
    display_pair(text->word_d[total_word - 2].d_fdt, 
                 text->word_d[total_word - 2].ft);

    printf("term %d is \"%s\":\n", total_word, 
            text->word_d[total_word - 1].word);
    display_pair(text->word_d[total_word - 1].d_fdt, 
                 text->word_d[total_word - 1].ft);

    printf("\n");
}

void stage2and3(Array *text, int *total_term)
{
    /*stage 2 and stage 3 are performed repeatlly 
      every time user input keyword, so it best to combine them together*/
    int character;
    char query[MAXQUER][MAXCHAR + 1] = {{0}};
    int i = 0, j = 0; /*index for iterm and character*/

    while ((character = mygetchar())!= EOF)
    {
        if (character == '\n') 
        {
            /*add \0 to last string*/
            query[i][j] = '\0'; 
            
            /*if change to user input "Enter", perform stage2 and stage3*/
            int key_doc_ind[MAXQUER]={0}; /*this will be change in stage2*/
            stage2(text, query, i, total_term, key_doc_ind);
            stage3(text, i, key_doc_ind);

            /*reinitialize thing back to normal*/
            i = 0;
            j = 0;

        }
        else if (character == SPACE) 
        {
            /*if it is a space, store previous character as a string*/
            query[i][j] = '\0';
            j = 0;
            i++;
        }
        else 
        {
            /*if this is a character, let it be*/
            query[i][j] = character;
            j++;
        }
    }
}

void stage2(Array *text, char (*query)[MAXCHAR + 1], 
            int num_query, int *total_term, int (*key_doc_ind))
{
    /*this stage simply search query word in the input file
      and return the position of them*/
    printf("Stage 2 Output\n");
    int i; /*index for query that readed*/
    for (i = 0; i <= num_query; i++)
    {
        /*perform stage 2 action*/
        char *word = query[i];
        int term_index = binary_search(text, word, *total_term);
        if (term_index == 0)
        {
            printf("    \"%s\" is not indexed", word);
        }
        else
        {
            printf("    \"%s\" is term %d", word, term_index);
        }
        printf("\n");

        /*store data for stage3*/
        key_doc_ind[i] = term_index;
    }
    printf("\n");
}

int binary_search(Array *text, char *target, int total_term)
{
    /*use binary search in stage 2 to searach query item*/
    int first = 0;
    int last = total_term - 1;
    int middle = (first + last)/2;
    while (first <= last)
    {
        char *word = text->word_d[middle].word;
        if (strcmp(word, target) == 0) /*if same string*/
            return middle + 1;
        else if (strcmp(word, target) < 0) /*if word before target*/
            first = middle + 1;
        else
            last = middle - 1;

        middle = (first + last)/2;
    }
    return 0;
}

void stage3(Array *text, int num_query, int (*key_doc_ind))
{
    /*at this stage find the strong math guveb query word*
      the strongest match is determined by the score from formula,
      where formula is a simplifed version of a seach alogrithm*/

    printf("Stage 3 Output\n");

    int que_ind = 0;

    /*create array to sotre all document space*/
    double *doc_score = calloc((text->N + 1), (text->N + 1) * sizeof(double));

    /*document index and store for strongest 3 document*/
    doc_pair *top3 = calloc(3, 3 * sizeof(doc_pair));

    /*calculate score for each document given query*/
    for (que_ind = 0; que_ind <= num_query; que_ind++)
    {
        /*target word at which position in input file*/
        /*note this one stars from 1*/
        /*so in array index sould be -1*/
        int key_ind = key_doc_ind[que_ind]; 

        if (key_ind != 0) /*if target word is indexed*/
        {
            key_ind -= 1;

            /*get current word's <d, fdt> data*/
            pair *d_fdt = text->word_d[key_ind].d_fdt; 

            /*number of total pair that word has*/
            double ft = text->word_d[key_ind].ft; 

            /*calculate every pair*/
            int pair_ind = 0;

            for (pair_ind = 0; pair_ind < ft; pair_ind++)
            {
                /*calculate Ld*/
                int doc_ind = d_fdt[pair_ind].d; /*index for document*/

                double d = text->doc_len[doc_ind]; /*document d's length*/
                double avgd = text->avg_doc_len;
                double Ld = K*((1-B) + B*d/avgd);

                /*calculate score in document d for current word*/
                double fdt = d_fdt[pair_ind].fdt;
                double N = text->N;
                double score_q = ( ((1+K)*fdt/(Ld + fdt)) * 
                                  log((N + 0.5) / ft) / log(2.0) );

                /*store score into the score array*/
                doc_score[doc_ind] += score_q;
            }
        }   
    }

    /*find the strongest 3 the best match*/
    best3(doc_score, top3, text->N);

    /*this is stage 3 output*/
    int topN = 0;
    for (topN = 0; topN < 3; topN++)
    {
        if (top3[topN].score != 0)
        {
            printf("    document %3d: score %6.3f\n", top3[topN].index, 
                                                      top3[topN].score);
        }
    }

    /*end of this stage*/
    free(doc_score);
    free(top3);

    printf("\n");
}

void best3(double (*doc_score), doc_pair (*top3), double N)
{
    int doc_ind;
    for (doc_ind = 1; doc_ind <= N; doc_ind ++)
    {
        double score = doc_score[doc_ind];

        /*if biggest*/
        if ( (score >= top3[0].score)
            && (score > top3[1].score) 
            && (score > top3[2].score))
        {
            /*if they have the same score, ranked by document num*/
            if (score == top3[0].score) 
            {
                /*move second doc to last doc, and delete last doc*/
                top3[2].index = top3[1].index;
                top3[2].score = top3[1].score;

                /*ranked by index, since later document has higher index*/
                /*so keep original unchanged*/
                top3[1].index = doc_ind;
                top3[1].score = score;
            }

            else
            {
                top3[2].index = top3[1].index;
                top3[2].score = top3[1].score;
                top3[1].index = top3[0].index;
                top3[1].score = top3[0].score;
                top3[0].index = doc_ind;
                top3[0].score = score;
            }
        }
        /*if second biggest*/
        else if ((score >= top3[1].score) && (score > top3[2].score))
        {
            /*if they have the same score, ranked by document num*/
            if (score == top3[1].score)
            {
                /*replace last doc score with this score*/
                /*ranked by index, since later document has higher index*/
                /*so keep original unchanged*/
                top3[2].index = doc_ind;
                top3[2].score = score;
            }
            else
            {
                top3[2].index = top3[1].index;
                top3[2].score = top3[1].score;
                top3[1].index = doc_ind;
                top3[1].score = score;  
            }
            
        }
        /*if third biggest*/
        else if (score >= top3[2].score) 
        {
            /*if the same score, no change as we ranked by index*/
            if (score != top3[2].score)
            {
                top3[2].index = doc_ind;
                top3[2].score = score;
            }
        }
    }
}
/*****************************************************************************/
void display_pair(pair *d_fdt, int total_pair)
{
    /*small tool that used to show <d, fdt> pair in stage 1*/
    int i;
    printf("    ");
    for (i = 0;;i++)
    {
        /*print out up to 9 pair here as pair 10 is a special condition*/
        if (i < total_pair - 1 && i < 9)
            printf("%d,%d; ", d_fdt[i].d, d_fdt[i].fdt);
        else
            break;
    }

    /*up to 10 pair*/
    if (i == 9)
        printf("%d,%d; ...\n", d_fdt[i].d, d_fdt[i].fdt);
    else
        printf("%d,%d\n", d_fdt[i].d, d_fdt[i].fdt);
}

int mygetchar() 
{
    /*small tool to get char*/
    int c;
    while ((c=getchar())=='\r') {
    }
    return c;
}
/*****************************************************************************/
/*Alogrithms are fun!*/
/*and it is more than fun.*/
