/*Authorship:
  Name: Siyi Guo
  Student number: 737008
  Login name: siyig1*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXCHAR 999 /*for all stages*/
#define MAXPQUER 20 /*for command line argument from stage2 and stage 3*/
#define K 1.2
#define B 0.75
/*****************************************************************************/
typedef struct pair pair;
typedef struct Dictionary Dictionary;
typedef struct Array Array;
typedef struct doc_pair doc_pair;
/*****************************************************************************/
typedef struct doc_pair
{
	int index;
	float score;
}doc_pair;

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

typedef struct Array
{
	Dictionary *word_d;
	size_t size;
	float N; /*number of documents in collections*/

	int *doc_len; /*dynamic array for each document length*/
				  /*document n'd length is sotred in index n*/
	size_t size_doc;

	float avg_doc_len;
}Array;
/*****************************************************************************/
/*basic small tool*/
int get_string_size(char *string);
void display(Dictionary *head_w);
void display_pair(pair *head_p, int total_pair);
int mygetchar();

/*function that doing most jobs*/
Array* scan_data(char *file_name, int *total_pair_p, int *total_word_p, Array *text);
void stage1(Array *text, int total_pair, int total_word);
void stage2and3(Array *text, int *total_term);
void stage2(Array *text, char (*query)[999], int num_query, int *total_term, int (*key_doc_ind));
int binary_search(Array *text, char *word, int total_term);
void stage3(Array *text, char (*query)[999], int num_query, int (*key_doc_ind));
void best3(float (*doc_score), doc_pair (*top3), float N);
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

	/*this is the end*/
	printf("Ta da-da-daaah...");
}
/*****************************************************************************/
void stage2and3(Array *text, int *total_term)
{
	int character;
	char query[20][999] = {{0}};
	int i = 0, j = 0; /*index for iterm and character*/

	while ((character = mygetchar())!= EOF)
	{
		if (character == '\n') 
		{
			/*add \0 to last string*/
			query[i][j] = '\0';	
			
	 		/*if change to next line, perform stage2 and stage3*/
	 		int key_doc_ind[20]={0};
	 		stage2(text, query, i, total_term, key_doc_ind);
	 		stage3(text, query, i, key_doc_ind);

	 		/*reinitialize thing back to normal*/
	 		i = 0;
	 		j = 0;

	 	}
	 	else if (character == 32) 
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

void stage3(Array *text, char (*query)[999], int num_query, int (*key_doc_ind))
{
	printf("Stage 3 Output\n");
	int que_ind = 0;
	float *doc_score = calloc((text->N + 1), (text->N + 1) * sizeof(float));
	doc_pair top3[3] = {0};

	/*calculate score for each document given query*/
	for (que_ind = 0; que_ind <= num_query; que_ind++)
	{
		/*target word at which position in input file*/
		/*note this one stars from 1*/
		/*so in array index sould be -1*/
		int key_ind = key_doc_ind[que_ind]; 

		//float score = 0;
		if (key_ind != 0) /*if target word is indexed*/
		{
			key_ind -= 1;

			/*get current word's <d, fdt> data*/
			pair *d_fdt = malloc((text->word_d[key_ind].ft) * sizeof(pair));
			d_fdt = text->word_d[key_ind].d_fdt; 
			int ft = text->word_d[key_ind].ft; /*number of total pair that word has*/

			/*calculate every pair*/
			int pair_ind = 0;

			for (pair_ind = 0; pair_ind < ft; pair_ind++)
			{
				/*calculate Ld*/
				int doc_ind = d_fdt[pair_ind].d; /*index for document*/

				float d = text->doc_len[doc_ind]; /*document d's length*/
				float avgd = text->avg_doc_len;
				float Ld = K*((1-B) + B*d/avgd);

				/*calculate score in document d for current word*/
				float fdt = d_fdt[pair_ind].fdt;
				float N = text->N;
				float score_q = ( ((1+K)*fdt/(Ld + fdt)) * log((N + 0.5) / ft) / log(2) );

				/*store score into the score array*/
				doc_score[doc_ind] += score_q;
			}
			free(d_fdt);
		}	
	}

	/*sorting the best match*/
	best3(doc_score, top3, text->N);

	/*this is stage 3 output*/
	int topN = 0;
	for (topN = 0; topN < 3; topN++)
	{
		if (top3[topN].score != 0)
		{
			printf("    document %3d: score %2.3f\n", top3[topN].index, top3[topN].score);
		}
	}

	/*end of this stage*/
	free(doc_score);
	printf("\n");
}

void best3(float (*doc_score), doc_pair (*top3), float N)
{
	int doc_ind;
	for (doc_ind = 1; doc_ind <= N; doc_ind ++)
	{
		float score = doc_score[doc_ind];

		if (score >= top3[0].score) /*if biggest*/
		{
			top3[2].index = top3[1].index;
			top3[2].score = top3[1].score;
			top3[1].index = top3[0].index;
			top3[1].score = top3[0].score;
			top3[0].index = doc_ind;
			top3[0].score = score;
		}
		else if (score >= top3[1].score) /*if second biggest*/
		{
			top3[2].index = top3[1].index;
			top3[2].score = top3[1].score;
			top3[1].index = doc_ind;
			top3[1].score = score;
		}
		else if (score >= top3[2].score) /*if third biggest*/
		{
			top3[2].index = doc_ind;
			top3[2].score = score;
		}
	}
}

void stage2(Array *text, char (*query)[999], int num_query, int *total_term, int (*key_doc_ind))
{
	printf("Stage 2 Output\n");
	int i; /*index for query*/
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
			printf("    \"%s\" is in term %d", word, term_index);
		}
		printf("\n");

		/*store data for stage3*/
		key_doc_ind[i] = term_index;
	}
	printf("\n");
}

int binary_search(Array *text, char *target, int total_term)
{
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

void stage1(Array *text, int total_pair, int total_word)
{
	printf("Stage 1 Output\n");
	printf("index has %d terms and %d (d,fdt) pairs\n", total_word, total_pair);

	printf("term 1 is \"%s\":\n", text->word_d[0].word);
	display_pair(text->word_d[0].d_fdt, (text->word_d[0].ft));

	printf("term 2 is \"%s\":\n", text->word_d[1].word);
	display_pair(text->word_d[1].d_fdt, (text->word_d[1].ft));

	printf("term %d is \"%s\":\n", total_word - 1, text->word_d[total_word - 2].word);
	display_pair(text->word_d[total_word - 2].d_fdt, (text->word_d[total_word - 2].ft));

	printf("term %d is \"%s\":\n", total_word, text->word_d[total_word - 1].word);
	display_pair(text->word_d[total_word - 1].d_fdt, (text->word_d[total_word - 1].ft));

	printf("\n");
}

Array* scan_data(char *file_name, int *total_pair_p, int *total_word_p, Array *text)
{
	FILE *collection = fopen(file_name, "r");

	/*if open successfully*/
	char string[MAXCHAR+1];
	int next_is_ft = 0;
	int num_terms = 0; /*count <d,fdt> pair for current word*/
	int total_word = 0; /*total word in the file*/
	int total_pair = 0; /*total pairs in the file*/
	int index_for_pair = 0;
	float total_document_length = 0;

	/*initialize dynamic array for word*/
	text->word_d = (Dictionary *)malloc(2 * sizeof(Dictionary));
	text->size = 2;
	text->N = 0;

	/*initialize dynamic array for <d, fdt>*/
	text->doc_len = (int *)calloc(2, 2 * sizeof(int));
	text->size_doc = 2;


	/*scanning data */
	while ((fscanf(collection, "%s", string))!= EOF)
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

			/*create space for word*/
			if (total_word > text->size)
			{
				text->size *= 2;
				text->word_d = (Dictionary *)realloc(text->word_d, text->size * sizeof(Dictionary));
			}
				
			/*store word and length of the word into the dynamic array*/
			text->word_d[total_word-1].word = (char*)malloc(length+1);
			memcpy(text->word_d[total_word-1].word, str, length+1);
			text->word_d[total_word-1].length = length;

			/*create link to previous one*/


			/* flag next number is ft*/
			next_is_ft = 1;

			/*count for <d, fdt> pair in current word*/
			num_terms = 1;
			index_for_pair = 0;
		}   

		/*if it is a number*/
		else if (isdigit(str[0]))
		{
			/*store how ft of this word*/
			if (next_is_ft) 
			{
				/*flag next_number is not ft*/
				next_is_ft = 0;

				/*strore ft*/
				text->word_d[total_word-1].ft = atoi(str);

				/*count total number of pairs*/
				total_pair += atoi(str);

				/*allocate correct length for the <d, fdt> pair*/
				text->word_d[total_word-1].d_fdt = malloc(atoi(str)*sizeof(pair));
			}

			/*store each <d, fdt> pari*/
			else 
			{
				if (num_terms %2 != 0) /*if this is d*/
				{
					int d = atoi(str);
					/*when num_pair is odd number again, this is a new pair*/
					text->word_d[total_word-1].d_fdt[index_for_pair].d = d;

					/*create dynamic array to store length of each document*/
					if (d > text->size_doc)
					{
						text->doc_len = (int *)realloc(text->doc_len, (d + 1) * sizeof(int));

						/*initial the extra memory that has been given*/
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
					int doc_number = text->word_d[total_word-1].d_fdt[index_for_pair].d;
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

	/*calculate average document length*/
	text->avg_doc_len = total_document_length / text->N;
	/*returning data*/
	*total_word_p = total_word;
	*total_pair_p = total_pair;
	return text;
}

/*****************************************************************************/
void display_pair(pair *d_fdt, int total_pair)
{
	int i;
	printf("    ");
	for (i = 0;;i++)
	{
		if (i < total_pair - 1 && i < 9)
			printf("%d,%d; ", d_fdt[i].d, d_fdt[i].fdt);
		else
			break;
	}
	if (i == 9)
		printf("%d,%d; ...\n", d_fdt[i].d, d_fdt[i].fdt);
	else
		printf("%d,%d\n", d_fdt[i].d, d_fdt[i].fdt);
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

int mygetchar() 
{
	int c;
	while ((c=getchar())=='\r') {
	}
	return c;
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
