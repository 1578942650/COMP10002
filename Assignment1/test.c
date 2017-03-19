int row;
int count = 1;
for (row = 1; row < data_row; row++)
{
	int same = row_comp(row, row+1, sorting_key, argc, text);
	if (same == 0)
	{
		count += 1;
	}
	else
	{
            /**if not identical, print out last row's data**//
            
            //print column data except last column
        int num_key = (argc -1) - 1;
        int comparing = 1;//indicate whether we need to compare column
        for (j = 0; j < num_key ; j++)
        {
                /*print column header:      
                    if difference occur at a column, 
                    the following columnshould print with out comparing,
                    otherwise we omit the same column*/
                    
                //current row's column data
            char *text1 = text[row][sorting_key[j] - 1];
                //last row's column data
            char *text2 = text[row - 1][sorting_key[j] - 1];
                
            if (strcmp(text1, text2) == 0 && comparing)
            {
                    /*if the same column data as last row 
                    and no difference so far, we do nothing.*/
                continue;
            }
            else
            {
                comparing = 0; //differences column has made
                    //print space
                or ( i = 0; i < j*4; i++) printf(" ");
                printf("%s\n", text[row][sorting_key[j]-1]);
        	}
    	}
}