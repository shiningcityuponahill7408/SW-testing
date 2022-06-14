#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* func_name(char func_proto[]);
int total_test_cases(FILE* fp, char* text_file_name);
void write_one_case(FILE* cfp, FILE* text, char* func_name);
char* get_output(char* line, int* output_index);

int main(int argc, char* argv[]){

	//char func_proto[200]; // contains the fucntion prototype
	char read_line[200]; // for reading text file
	char write_line[200]; // for writing c source code

	FILE* fp_read_text;
	FILE* fp_make_test_driver;
	
	// get file names
	char text_file_name[200];

	// get text file name
	printf("Enter text file name: ");
	scanf("%s", text_file_name);
	fp_read_text = fopen(text_file_name, "r");
	if(!fp_read_text){
		printf("No such text file\n");
		exit(1);
	}
	
	// get the length of function prototype
	int first_line_length = 0;
	char stop;
	stop = fgetc(fp_read_text);
	
	while( stop != '\n' && stop != 13 ){ // read until meet new line or carriage return
		first_line_length++;
		stop = fgetc(fp_read_text);
	}
	
	// go back to the 0th location of text file
	// because to get the funtion prototype
	fseek(fp_read_text, 0, SEEK_SET);
	
	// this contains the funtion prototype
	char* func_proto = malloc(first_line_length * sizeof(*func_proto) + 1);
	memset(func_proto, 0, first_line_length * sizeof(*func_proto) + 1);
	int i;
	for(i = 0; i <first_line_length; i++){
		func_proto[i] = fgetc(fp_read_text);
	}
	// file pointer is at the last element of the first line, which neither 10 nor 13
	
	// this makes C source file
	fp_make_test_driver = fopen("my_test_driver.c", "w");

	// write header files
	strcpy(write_line, "#include <stdlib.h>\n");
	fwrite(write_line, 1, strlen(write_line), fp_make_test_driver);
	memset(write_line, 0, sizeof(write_line));

	strcpy(write_line, "#include <stdio.h>\n");
	fwrite(write_line, 1, strlen(write_line), fp_make_test_driver);
	memset(write_line, 0, sizeof(write_line));

	strcpy(write_line, "#include <string.h>\n");
	fwrite(write_line, 1, strlen(write_line), fp_make_test_driver);
	memset(write_line, 0, sizeof(write_line));

	// write: function prototype
	fwrite(func_proto, 1, strlen(func_proto), fp_make_test_driver);
	if(!strchr(func_proto, ';')){
		fputc(';', fp_make_test_driver);
	}
	

	// write: int main(){
	strcpy(write_line, "\nint main(){\n");
	fwrite(write_line, 1, strlen(write_line), fp_make_test_driver);
	memset(write_line, 0, sizeof(write_line));
	
	// count the total number of test cases
	const int total_cases = total_test_cases(fp_read_text, text_file_name);
	
	// get the funtion name from function prototype
	char* name_of_funtion = func_name(func_proto);
	
	// want to put file pointer at new line char
	// then fgetc returns the first element of the second line
	stop = fgetc(fp_read_text);
	while(stop != EOF){
		
		if(stop == '\n'){
			break;
		}

		stop = fgetc(fp_read_text);
	}

	// ... write codes
	for(i = 0; i < total_cases; i++){
		write_one_case(fp_make_test_driver, fp_read_text, name_of_funtion);
		fputc('\n', fp_make_test_driver);
	}
	
	// this contains c source file name
	char source_code_name[200];
	printf("Enter the source code name: ");
	scanf("%s", source_code_name);

	// make shell file
	FILE* make_shell_file;
	make_shell_file = fopen("run_my_test.sh", "w");
	fputs("gcc -o a.out ", make_shell_file);
	fputs(source_code_name, make_shell_file);
	fputc(' ', make_shell_file);
	fputs("my_test_driver.c\n", make_shell_file);
	fputs("./a.out", make_shell_file);

	// return 0;}
	strcpy(write_line, "return 0;} \n");
	fwrite(write_line, 1, strlen(write_line), fp_make_test_driver);
	memset(write_line, 0, sizeof(write_line));

	//free malloc
	free(name_of_funtion);
	free(func_proto);

	// close file pointers
	fclose(fp_read_text);
	fclose(fp_make_test_driver);
	fclose(make_shell_file);
	
	return 0;
}

/**
count total number of test cases
**/
int total_test_cases(FILE* fp, char* text_file_name){

	FILE* temp = fopen(text_file_name, "rt");
	
	fseek(temp, ftell(fp), SEEK_SET);

	char line_counter = getc(temp);

	int total = 0;

	while(1){

		if(line_counter == EOF){
			break;
		}

		if(line_counter != EOF && line_counter == '\n' ){ // when encountered a new line
			
			line_counter = getc(temp); // check the next character

			if(line_counter != EOF && line_counter != '\n' && line_counter != 13){
				total++;
			}else{
				break;
			}

		}

		line_counter = getc(temp);
	}

	fclose(temp);

	return total;
}

// return function's name from function prototype
char* func_name(char func_proto[]){
	// assume that the length of the function's name is less than the given prototype
	char* result = malloc( strlen(func_proto) * sizeof(char));
	memset(result, 0, strlen(func_proto) * sizeof(char));
	int space_counter = 0;
	int i, j = 0, k;

	int flag = 0;

	for(i = 0; i < strlen(func_proto); i++){
		
		if(func_proto[i] == '('){ // when encountered "(", loop back

			k = i - 1;

			while(1){ // loop back from (
				if(func_proto[k] == ' ' || func_proto[k] =='*'){ // the end of the function name
					
					if(flag == 1){
						break;
					}

				}else{ // result will be the reverse of the function name
					flag = 1; 
					result[j] = func_proto[k];
					j++;
				}
				k--;
			}
			break;
		}
	}

	// reverse the content to get the correct function name
	char temp;
	for(i = 0; i < strlen(result) / 2; i++){
		temp = result[i];
		result[i] = result[strlen(result) - i - 1];
		result[strlen(result) - i - 1] = temp;
	}

	return result;
}

/**
write test cases 

**/

void write_one_case(FILE* cfp, FILE* text, char* func_name){

	int test_num_length = 0;
	char stop; 
	int word_counter = 0;
	int i;
	
	// write "if( " 
	fputs("if( ",  cfp);

	// write functio name
	fwrite(func_name, 1, strlen(func_name), cfp);

	// this gets the first element of the line
	stop = fgetc(text);
	
	// want to get the line(test case)
	while(stop != EOF){

		if(stop != 10 && stop != 13){ // if character, then count
			word_counter++;
		}
		else{ // if new line, break the loop
			break;
		}

		stop = fgetc(text);
	}
	// current file pointer is at 13(windows) or 10(the others) or EOF

	
	// move back to the first element of the line
	// -1 is needed becasue of fgetc at line:257

	if(stop == EOF){
		fseek(text, word_counter * -1, SEEK_CUR);
	}else{
		fseek(text, -1, SEEK_CUR);
		fseek(text, word_counter * -1, SEEK_CUR);
	}

	// this contains the line
	char* line = malloc(word_counter * sizeof(*line) + 1);
	memset(line, 0, word_counter * sizeof(*line) + 1); // memory adjustion
	// get the whole line
	for(i = 0; i < word_counter; i++){
		line[i] = fgetc(text);
	}
	line[i] = 0;
	// this would be 10 or 13 depending on OS 
	// because fgetc ended at the last character of the line
	stop = fgetc(text); 

	// want to put file pointer at 10 (new line)
	// because at the beginning of this fucntion, fgetc gets the first element of the line 
	while(stop != EOF){
		if(stop != 10){
			stop = fgetc(text);
		}else{
			break;
		}
	}
	
	
	// get the test number
	i = 0;
	while(1){
		if(line[i] == 32){
			break;
		}else{
			test_num_length++;
		}
		i++;
	}
	// get the test number
	char* test_num = malloc(test_num_length * sizeof(*test_num) + 1);
	memset(test_num, 0, test_num_length * sizeof(*test_num) + 1); // memory adjustion
	for(i = 0; i < test_num_length; i++){
		
		test_num[i] = line[i];

	}
	
	// get output
	char* temp_word;
	int output_index = 0;
	temp_word = get_output(line, &output_index);
	
	
	// replace the first and the last space with "("" and ")"
	line[test_num_length] = '(';
	line[output_index - 1] = ')';
	
	// replace space with comma
	for(i = test_num_length; i < output_index; i++){
		if(line[i] == ' '){
			line[i] = ',';
		}
	}
	
	// write input codes
	for(i = test_num_length; i < output_index; i++){
		fputc(line[i], cfp);
	}


	fputs(" == ", cfp);

	for(i = output_index; i < strlen(line); i++){
		fputc(line[i], cfp);
	}

	fputc(')', cfp);

	fputs("  printf(\"test case ", cfp);
	
	fputs(test_num, cfp);

	fputs(": pass \\n\");\n", cfp);

	fputs("else printf(\"test case ", cfp);

	fputs(test_num, cfp);

	fputs(": Fail\\n\" );", cfp);

	free(temp_word);
	free(line);
	free(test_num);
}


/**
return the output

**/
char* get_output(char* line, int *output_index){

	int i = strlen(line) - 1;
	int flag = 0; // if flag is 1, then next space is where it stops
	int output_length_counter = 0;

	while(1){

		if(line[i] != ' '){ // flag = 1 when a character is given
			flag = 1;
			output_length_counter++;
		}
		else if(flag == 1 && line[i] == ' '){
			break;
		}

		i--;
	}
	
	// this contains the output
	char* result = malloc(output_length_counter * sizeof(*result) + 1);
	memset(result, 0, output_length_counter * sizeof(*result) + 1); // mem adjustion
	i++;

	*output_index = i; // record output index

	int j;
	for(j = 0; j < output_length_counter; j++){
		result[j] = line[i];
	}
	
	return result;
}