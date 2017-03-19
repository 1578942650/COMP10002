/*program take a tsv file and some command line argument(positive integer).
integer from command line argument are index of column header, which is also
the sorting key for stage 2 and stage 3
if sorting key is not given, only stage1 will run:
    tells total numbers of row and column as well as what last row looks like.
if given, stage 2 and stage 3 are also executed:
    stage 2: soring the tsv data according to the given key, 
              and print out first, middel and last row to verify.
    stage 3: generate a summary of data based on the given sorting column key*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXCOLUMN 30 /*max number for column is 30*/
#define MAXROW 1000 /*max number for row is 1000*/
#define MAXCHAR 51   /*max number for string is 50 plus a '\0'*/

/*****************************************************************************/

/*functions that doing some basic frequent job in the function*/
int mygetchar();
int abs(int n);
int find_longest_header(int column, char (*text)[MAXCOLUMN][MAXCHAR]);
void row_swap(int row1, int row2, char (*text)[MAXCOLUMN][MAXCHAR]);
int row_comp(int row1, int row2, int* sorting_key, int argc, 
             char (*text)[MAXCOLUMN][MAXCHAR]);
void print_row(int column, int longest_header, int row, 
            char (*text)[MAXCOLUMN][MAXCHAR]);

/*big function that are main component of the program*/
void stage1(int column, int data_row, int longest_header, 
            char (*text)[MAXCOLUMN][MAXCHAR]);
void stage2(int column, int data_row, int longest_header, 
            char (*text)[MAXCOLUMN][MAXCHAR]);
void stage3(int column, int data_row, int longest_header, 
            int* sorting_key, int argc, char (*text)[MAXCOLUMN][MAXCHAR]);
void sorting(int column, int data_row, 
            int* sorting_key, int argc, char (*text)[MAXCOLUMN][MAXCHAR]);

/******************************************************************************/

int main(int argc, char* argv[])
{
    int character;
    int i = 0, j = 0 , k = 0; /*index for string, columnm, row*/
    int data_row = 0, column = 0; /*number of row(header excluded) and column*/
    char text[MAXROW][MAXCOLUMN][MAXCHAR] = {{{0}}};

    /*reading data and classify them into 2d array of string*/
    while ((character = mygetchar())!= EOF)
    {
        if (character != '\t' && character != '\n')
        {
            text[k][j][i] = character;
            i++;
        }
        else if (character == '\t') /*if change to a new column*/
        {
            i = 0; 
            j++;
        }
        else if (character == '\n') /*if change to a new row*/
        {
            column = j+1;
            i = 0;
            j = 0;
            k++;
        }
        data_row = k-1; /* data row does not take header into account*/
    }
    
    int longest_header = find_longest_header(column, text);
    
    /*stage1_output*/
    stage1(column, data_row, longest_header, text);
    
    
    /*if sorting key is given, doing stage 2 and stage 3*/
    if (argc > 1)
    {
        /*extracting sorting key from command line*/
        int sorting_key[MAXCOLUMN] = {0};
        for (i = 0; i < argc-1; i++)
        {
            int number = atoi((argv[i+1]));
            sorting_key[i] = number;
        }
        
        /*sorting the whole data by the given sorting key*/
        sorting(column, data_row, sorting_key, argc, text);
        
        /*stage 2 output*/
        stage2(column, data_row, longest_header, text);
        
        /*stage 3 output*/
        stage3(column, data_row, longest_header, sorting_key, argc, text);
    }
    
    return 0;
    /* Alogrithms are Fun!*/
}

/******************************************************************************/

void stage3(int column, int data_row, int longest_header, 
            int* sorting_key, int argc, char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*Stage3 print a report that summary data based on the sorting key*/
    
    /*finding the longest_entry of last column data to adjust format of output*/
    int k;
    int longest_entry = 0;
    for (k = 0; k < data_row+1; k++)
    {
        if ((int)strlen(text[k][sorting_key[argc-2]-1]) > longest_entry)
        {
            longest_entry = (int)(strlen(text[k][sorting_key[argc-2]-1]));
        }
    }
    
    /*finding the number of "-"that need to be print out*/
    int line_long = 4*(argc - 1 - 1) + longest_entry + 1 + 5;
    
    /*stage 3 output*/
    printf("\n");
    printf("Stage 3 Output\n");
    
    /*print("-")*/
    int i;
    for (i = 0; i < line_long; i++) printf("-");
    printf("\n");
    
    /*print column header, eg: country, gender, age, year.*/
    int j; /*index for sorting key in key array
           sorting_key[j]-1 is the coorespond column header given j*/
    for (j = 0; j < argc - 1 - 1; j++)
    {
        /*print space*/
        for ( i = 0; i < j*4; i++) printf(" ");
        /*print column header*/
        printf("%s\n", text[0][sorting_key[j]-1]);
    }
    
        /*printing the last line of header ie: header  Count.*/
    int last_column_length = (int)(strlen(text[0][sorting_key[j]-1]));
    
            /*printing the last column header*/
    for ( i = 0; i < j*4; i++) printf(" ");    
    printf("%s", text[0][sorting_key[j]-1]); 
    
            /*print string "Count"*/
    for ( i = 0; i < abs(longest_entry - last_column_length) + 1; i++) 
        printf(" ");
    printf("Count\n");
    
    
    /*print("-");*/
    for (i = 0; i < line_long; i++) printf("-");
    printf("\n");
    
    /*print out stage 3 summary report except last row*/
    int row;
    int count = 1;
    for (row = 1; row <=data_row ; row++)
    {
        int same;
        if (row == data_row)
        {
            same = 1; /*special case for last line of data row*/
        }
        else
            same = row_comp(row, row+1, sorting_key, argc, text);
        
        if (same == 0) /*if they are the same row*/
        {
            count += 1; /*if are the same rwo, we should count it*/
        }
        else 
        {
            /**if not identical, print out last row's data**/
            
            /*print column data except last column*/
            int num_key = (argc -1) - 1;
            int comparing = 1;/*indicate whether we need to compare column*/
            for (j = 0; j < num_key ; j++)
            {
                /*print column header:     
                    if difference occur at a column, 
                    the following columnshould print with out comparing,
                    otherwise we omit the same column*/
                    
                /*current row's column data*/
                char *text1 = text[row][sorting_key[j] - 1];
                /*last row's column data*/
                char *text2 = text[row - count][sorting_key[j] - 1];
                
                if (strcmp(text1, text2) == 0 && comparing)
                {
                    /*if the same column data as last row 
                    and no difference so far, we do nothing.*/
                    continue;
                }
                else
                {
                    comparing = 0; /*differences column has made*/
                    /*print space*/
                    for ( i = 0; i < j*4; i++) printf(" ");
                    printf("%s\n", text[row][sorting_key[j]-1]);
                }
            }
            
            /*print the last column data*/
            for ( i = 0; i < j*4; i++) printf(" ");
            printf("%s", text[row][sorting_key[j]-1]);
            
            /*print the last line and count:
              argc - 2 means the last key given from command_line
              sorting_key[argc-2]-1 is column index correspond to this key*/
            char *last_column_data = text[row][sorting_key[argc-2]-1];
            int second_last_length = (int)strlen(last_column_data);
            int width = abs(longest_entry - second_last_length) + 1;
            for ( i = 0; i < width; i++) printf(" ");
            printf("%5d\n", count);
            count = 1; /*As couting for this type of row is over*/
        }
    }


    
    /*print the final line("-"), end of report.*/
    for (i = 0; i < line_long; i++) printf("-");
    printf("\n");
}

/******************************************************************************/

void stage2(int column, int data_row, int longest_header, 
            char (*text)[MAXCOLUMN][MAXCHAR])
{
    printf("\n");
    printf("Stage 2 Output\n");
    
    /*first row*/
    printf("row 1 is:\n");
    print_row(column, longest_header, 1, text);
    
    /*middle row*/
    printf("row %d is:\n", (data_row / 2 + data_row % 2));
    print_row(column, longest_header, (data_row / 2 + data_row % 2), text);
    
    /*last row*/
    printf("row %d is:\n", data_row);
    print_row(column, longest_header, data_row, text);
}

/******************************************************************************/

void stage1(int column, int data_row, int longest_header, 
            char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*stage 1 output*/
    printf("Stage 1 Output\n");
    printf("input tsv data has %d rows and %d columns\n", data_row, column);
    printf("row %d is: \n", data_row);
    print_row(column, longest_header, data_row, text);
}

void sorting(int column, int data_row, 
             int* sorting_key, int argc, char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*doing sorting given the sorting key, is the basis of stage2 and stage 3*/
    int current_row, row2;
    for (current_row = 1; current_row < data_row+1; current_row++)
    {
        int smallest_row = current_row;
        for (row2 = current_row + 1; row2 < data_row+1; row2++)
        {
            int comparing_result = row_comp(row2, smallest_row, 
                                            sorting_key, argc, text);
            if (comparing_result == 1) /*if row2 is smaller*/
            {
                smallest_row = row2;
            }
        }

        /*put smallest row to current position and move rest down by 1 row*/
        int row_index;
        for (row_index = smallest_row; row_index > current_row; row_index--)
            row_swap(row_index-1, row_index, text); 
    }
}

/******************************************************************************/

int row_comp(int row1, int row2, 
             int* sorting_key, int argc, char (*text)[MAXCOLUMN][MAXCHAR])
{
   /*comparing the raw by given sorting key,
     1 means row1 < row2, 
     -1 means row 1 > rows2, 
     0 means identical*/
   int i = 0;
   int last_level_same = 1; /*indicate there last sorting key are the same*/
   while (i < (argc - 1)) /*going through all sorting key*/
   {
       if (last_level_same == 1)
       {
           char *text1 = text[row1][sorting_key[i]-1];
           char *text2 = text[row2][sorting_key[i]-1];
           int strcmp_result = strcmp(text1, text2);
           if (strcmp_result < 0)
           {
                return 1; /*row 1 should place before row2*/
           }
           else if (strcmp_result == 0)
           {
                last_level_same = 1; /*identical row*/
           }
           else
           {
                return -1;  /*row 1 should place after row2*/
           }
       }
       i++;
   }
   return 0;  /*they are identical row*/
}

/******************************************************************************/

void row_swap(int row1, int row2, char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*used in sorting functiom, swap two different row*/
    char temp[MAXCOLUMN][MAXCHAR]= {{0}}; /*temporary memmory to store row1*/
    memcpy(temp, text[row1], sizeof(text[row1]));
    memcpy(text[row1], text[row2], sizeof(text[row1]));
    memcpy(text[row2], temp, sizeof(text[row1]));
}

/******************************************************************************/

void print_row(int column, int longest_header, int row, 
               char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*widely used in stage 1 and stage 2 to print out the target row*/
    int j;
    for (j = 0; j < column; j++)
    {
        printf("%4d: ", j+1);  /*print index for column*/
        printf("%-*s", longest_header + 4, text[0][j]);  /*print column header*/
        printf("%s\n", text[row][j]);  /*print correspond data*/
    }   
}

/******************************************************************************/

int find_longest_header(int column, char (*text)[MAXCOLUMN][MAXCHAR])
{
    /*this function find the longest header*/
    int max_column_length = 0;
    int j; /*index for column*/
    for (j = 0; j < column; j++)
    {
        if (strlen(text[0][j]) > max_column_length)
            max_column_length = strlen(text[0][j]);
    }
    return max_column_length;   
}

/******************************************************************************/

int mygetchar() 
{
	int c;
	while ((c=getchar())=='\r') {
	}
	return c;
}

/******************************************************************************/

int abs(int n)
{
    if (n < 0)
        return -n;
    return n;
}

/******************************************************************************/
/* Alogrithms are Fun!*/