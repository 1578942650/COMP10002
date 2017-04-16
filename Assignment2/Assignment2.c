/*Authorship:
  Name: Siyi Guo
  Student number: 737008
  Login name: siyig1*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define MAXCHAR 999 /*for all stages*/
#define MAXPQUER 20 /*for command line argument from stage2 and stage 3*/
/*****************************************************************************/
typedef struct pair pair;
typedef struct Dictionary Dictionary;
/*****************************************************************************/
typedef struct pair
{
	int d;
	int fdt;
	pair *next;
}pair;

typedef struct Dictionary
{
	int length;
	char *word;
	int ft;
	pair *d_fdt;
	Dictionary *next; 
}Dictionary;
/*****************************************************************************/
int get_string_size(char *string);
void display(Dictionary *head_w);
void display_pair(pair *head_p, int total_pair);
Dictionary* scan_data(char *file_name, int *total_pair, int *total_word);
void create_index(int total_word, Dictionary *current_w, Dictionary *(*word_dd));
void stage1(Dictionary *head_w, int *total_pair, int *total_word);
/*****************************************************************************/
int main(int argc, char* argv[])
{
	int total_pair = 0;
	int total_word = 0;
	char* file_name = argv[1];
	/*scan data with dynamic arrays*/
	Dictionary *head_w= scan_data(file_name, &total_pair, &total_word);

	/*create index for scanned data*/
	Dictionary *word_dd[total_word];
	create_index(total_word, head_w, word_dd);
	
	/*display index*/
	Dictionary *current_w = head_w;
	int i;
	for (i = 0; i < total_word-1; i++)
	{
		word_dd[i] = current_w->next;
		printf("%s\n", word_dd[i]->word);
		current_w = current_w->next;
	}

	/*stage1*/
	stage1(head_w, &total_pair, &total_word);

	/*display the data*/
	//display(head_w);
}

/*****************************************************************************/
void create_index(int total_word, Dictionary *current_w, Dictionary *(*word_dd))
{
	int i;
	for (i = 0; i < total_word-1; i++)
	{
		word_dd[i] = current_w->next;
		printf("%s\n", word_dd[i]->word);
		current_w = current_w->next;
	}
}

void stage1(Dictionary *head_w, int *total_pair_p, int *total_word_p)
{
	printf("Stage 1 Output\n");
	printf("inde has %d terms and %d (d,fdt) pairs\n", *total_word_p, *total_pair_p);
	printf("term 1 is \"%s\":\n", head_w->word);
	display_pair(head_w->d_fdt, (head_w->ft));
	printf("term 2 is \"%s\":\n", head_w->next->word);
	display_pair(head_w->next->d_fdt, (head_w->next->ft));
}

void display_pair(pair *head_p, int total_pair)
{
	pair *current_p = head_p;
	int i;
	printf("    ");
	for (i = 0; i < total_pair - 1;i++)
	{
		printf("%d,%d; ", current_p->d, current_p->fdt);
		/*go to next pair*/
		current_p = current_p->next;
	}
	printf("%d,%d\n", current_p->d, current_p->fdt);
}

Dictionary* scan_data(char *file_name, int *total_pair_p, int *total_word_p)
{
	FILE *text = fopen(file_name, "r");

	/*if open successfully*/
	char string[999];
	int next_is_ft = 0;
	int num_pairs = 0; /*count <d,fdt> pair*/
	int total_word = 0;
	int total_pair = 0;

	/*create linked structure*/
		/*structure for word*/
	Dictionary *prev_w = NULL;
	Dictionary *head_w = NULL;
	Dictionary *current_w = NULL;
	
	/*structure for <d, fdt>*/
	pair *prev_p = NULL;
	pair *head_p = NULL;
	pair *current_p = NULL;

	/*scanning data */
	while ((fscanf(text, "%s", string))!= EOF)
	{
		/*alloc exact memory for data that have read*/
		int length = get_string_size(string);
		char* str = (char*)malloc(length + 1);
		str = string;

		/*if this is a word*/
		if (isalpha(str[0]))
		{
			/*count for number of items*/
			total_word++;
			
			/*create space for this word*/
			current_w = (Dictionary *)malloc(sizeof(Dictionary));

			/*store data*/
			current_w->length = length;
			current_w->word = (char*)malloc(length + 1);
			memcpy(current_w->word, str, length+1); 

			/*create next data structure*/
			if (head_w == NULL)
				head_w = current_w;
			else
			{
				/*store previous word <d, fdt> data into previous word*/
				prev_w->d_fdt = head_p;

		/*should I free all previous memory??*/
				head_p = NULL; /*initial head for nex <d, fdt> pair*/
				prev_p->next = NULL; /*close previous word's <d,fdt> pari*/

				/*creat link between current word and previous word*/
				prev_w->next = current_w;
			}

			/* flag next number is ft*/
			next_is_ft = 1;

			/*count for <d, fdt> pair*/
			num_pairs = 1;

			/*let current word become the "previous_word: of next word*/
			prev_w = current_w;
		}   

		/*if it is a number*/
		else if (isdigit(str[0]))
		{
			if (next_is_ft)
			{
				/*flag next_number is not ft*/
				next_is_ft = 0;

				/*strore ft*/
				current_w->ft = atoi(str);

				/*count total number of pairs*/
				total_pair += atoi(str);
			}
			else
			{
				/*storing d, fdt data*/
				if (num_pairs % 2 != 0)
				{
					/*current_p run once*/
					/*when num_pair is odd number again, this is a new pair*/
					current_p = (pair*)malloc(sizeof(pair));
					current_p->d = atoi(str); 
				}
				else
				{
					/*current_p run last*/
					current_p->fdt = atoi(str);

					if (head_p == NULL)
					{
						head_p = current_p;
					}
					else
					{
						/*create paird to next pair*/
						prev_p->next = current_p;
					}

					/*store this pair as "previous pair" for next pair*/
					prev_p = current_p;
				}
			}
		}
		free(str);
		num_pairs++;
	}

	/*store last words's <d, fdt> data, as we always sotre n-1 word data at word n*/
	prev_w->d_fdt = head_p;
	free(current_p);
	free(current_w);

	/*returning data*/
	*total_word_p = total_word;
	*total_pair_p = total_pair;
	return head_w;
}

int get_string_size(char *string)
{
	int i = 0;
	while ( string[i] != '\0')
	{
		i++;
	}
	return i;
}

void display(Dictionary *head_w)
{
	printf("\n the following is displaying data to check error\n");
	Dictionary *current_w = head_w;
	while(current_w != NULL)
	{
		/*show word and ft*/
		printf("length is %d, %s %d", current_w->length, current_w->word , current_w->ft);
		
		/*show <d, fdt>*/
		pair *head_p = current_w->d_fdt;
		pair *current_p = head_p;
		while (current_p != NULL)
		{
			printf(" %d %d", current_p->d, current_p->fdt);
			current_p = current_p->next;
		}

		printf("\n");

		/*iterate to next word*/
		current_w = current_w->next;
	}
}
/*****************************************************************************/
