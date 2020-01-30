/**
* @file smartsys.c
* @author Sokolovskii Vladislav
* @date 20 Jan 2020
* @brief This is the main file of the program containing the main function and some functions that both modes (admin/user mode) share.
* @details The file includes the functions that are used in both modes or function that can work differently depending on the current work mode. Also, there are global variables that are necessary for the connection to the database.
*/
#include "smartsys.h"

//mysql server variables
char* server = "localhost";
char* user = "root";
char* password = "04072010";
char* database = "rest_IS";
unsigned int port = 3306;
static char* unix_socket = NULL;
unsigned int flag = 0;

int main(int argc, char* argv[])
{
	//connection to the data base
	MYSQL* connect = mysql_init(NULL);
	if(connect == NULL)
		return EXIT_FAILURE;
	if(!mysql_real_connect(connect, server, user, password, database, port, unix_socket, flag))
		return EXIT_FAILURE;
	
	//arguments check
	int mode = args_check(argc, argv);
	char answer;
	
	//call an interface
	switch(mode){
		case installation:
			install_db(connect);
			break;
		case visitor_mode:
			printf("If you want to create a new account type Y/N :\n");
			yes_no_answer(&answer);
			if(answer == 'Y' || answer == 'y')
				reg(connect);
			user_interface(connect);
			break;
		case admin_mode:
			admin_interface(connect);
			break;
		case help:
			print_usage_msg();
			break;
	}
	mysql_close(connect);
	return EXIT_SUCCESS;
}
/*
This function checks arguments from the command line and returns a number
corresponding to one of the behaviour indexes.
 */
int args_check(int argc, char** argv)
{
	if(argc == 1){
		printf("Try --help for more information.\n");
		return UNSUCCESS;
	}
	if(!(strcmp(argv[1], "admin")))
		return admin_mode;
	else if(!(strcmp(argv[1], "order")))
		return visitor_mode;
	else if(!(strcmp(argv[1], "--help"))){
		return help;
	}
	else if(!(strcmp(argv[1], "--install")))
		return installation;
	else
		printf("Try --help for more information.\n");
		
	return idle;
}
 /*
 This function prints out the main menu for visitors and admin
 */
 void main_menu(char* name, int mode){
	 if(mode == visitor_mode){
		 if(first_visit == true)
			 printf("\n********************************* Welcome, %s *******************************\n", name);
		 printf("*******************************************************************************\n");
		 printf("* 1. Display today's menu                                                     *\n");
		 printf("* 2. Make an order            						      *\n");
		 printf("* 3. Display my previous meal                                                 *\n");
		 printf("* 4. End my session                                                           *\n");
		 printf("*******************************************************************************\n");
	 }
	 else if(mode == admin_mode){
		 if(first_visit == true)
			 printf("\n********************************* Welcome to IS *******************************\n");
		 printf("*******************************************************************************\n");
		 printf("* 1. Display today's menu                                                     *\n");
		 printf("* 2. Add a dish to the menu                                                   *\n");
		 printf("* 3. Delete a dish from the menu                                              *\n");
		 printf("* 4. Pause/Resume a dish                                                      *\n");
		 printf("* 5. Show income                                                              *\n");
		 printf("* 6. Exit                                                                     *\n");
		 printf("*******************************************************************************\n");
	 }
	 first_visit = false;
}
void return_to_main_menu(int mode)
{
	printf("Press enter to return.\n");
	getchar();
	main_menu(NULL, mode);
}
/*
This function displays all the avaliable offers from the menu
*/
 void display_dish_menu(MYSQL* connect, int type)
 {
	printf("* ID ********************************* Menu ***********************************\n");
	
	if(type == snack || type == all){
		printf("************************************* Snacks **********************************\n");
		if(!mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE type = 'snack' AND pause <> true"))
			print_results(connect, 3);
	}
	if(type == first || type == all){
		printf("********************************** First courses ******************************\n");
		if(!mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE type = 'first' AND pause <> true"))
			print_results(connect, 3);
	}
	if(type == main_dish || type == all){
		printf("*********************************** Main dishes *******************************\n");
		if(!mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE type = 'main' AND pause <> true"))
			print_results(connect, 3);
	}
	if(type == dessert || type == all){
		printf("************************************* Desserts ********************************\n");
		if(!mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE type = 'dessert' AND pause <> true"))
			print_results(connect, 3);
	}
	if(type == drink || type == all){
		printf("************************************** Drinks *********************************\n");
		if(!mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE type = 'drink' AND pause <> true"))
			print_results(connect, 3);
	printf("*******************************************************************************\n");
	}
}
/*
This functions prints out every column of selected row/s
*/
 void print_results(MYSQL* connect, int cols_num)
 {
	unsigned desc_length = 0;
	result = mysql_store_result(connect);
	
	while((row = mysql_fetch_row(result))){
		for(size_t i = 0; i < cols_num; i++){
			if(i == 0)
				printf("*");
				
			if(i == cols_num - 1)
				printf("%*s czk *\n", MENUWIDTH - desc_length, row[i]);
			else
				printf(" %s ", row[i]);
				
			if(i == 0)
				printf(" |");
			desc_length += strlen(row[i]);
			if(cols_num == 4 && i == 2)
				desc_length += 2;
		}
		desc_length = 0;
	}
}
/*
This function takes interval, table name and name of the numerical attribute that must be unique
and generates attribute value that will be unique within concrete table. If no error occured
the function returns generated number. Otherwise return value is equal to zero UNSUCCESS.
*/
int unique_random_num(MYSQL* connect, int lower, int upper, char* table_name, char* attribute_name)
{
	unsigned num;
	char query[MAXLEN];
	do{
		num = (rand() % (upper - lower + 1) + lower);
		snprintf(query, MAXLEN, "SELECT %s FROM %s WHERE %s = %d",attribute_name, table_name, attribute_name, num);
		if(!mysql_query(connect, query)){
			result = mysql_store_result(connect);
			if((row = mysql_fetch_row(result))){
				mysql_free_result(result);
				continue;
			}
			else{
				mysql_free_result(result);
				break;
			}
		}
		else
			return UNSUCCESS;
	}while(true);
	return num;
}
/*
This function expects a numerical value on input. The function will ask a user to enter
one of the options as long as the entered value does not satisfy.
*/
void get_option(int* option)
{
	char option_str[MAXSTR];
	
	do{
		fgets(option_str, MAXSTR, stdin);
		if(!isdigit(option_str[0])){
			printf("Please, choose one of the options.\n");
			continue;
		}
		else
			break;
	}while(true);
	*option = atoi(option_str);
}
/*
This function asks user for a one-valued answer YES or NO, as long as user inputs different option
the function will ask him for YES/NO answer.
*/
void yes_no_answer(char* answer)
{
	char answer_str[MAXSTR];

	do{
		fgets(answer_str, MAXSTR, stdin);
		*answer = answer_str[0];
		if(*answer == 'y' || *answer == 'Y' || *answer == 'n' || *answer == 'N')
			return;
		else{
			printf("Please, choose one of the options Y or N.\n");
			continue;
		}
	}while(true);
}
/*
This function checks if given string corresponds to the concrete format. If unexpected characted was detected the function
returns BAD_FORMAT (-1), otherwise return value is equal to SUCCESS.
*/
int format_check(int format, char* string)
{
	switch(format){
		case number:
			for(size_t i = 0; i < strlen(string); i++){
				if(!isdigit(string[i])){
					printf("This value must include only numbers, try again please.\n");
					return BAD_FORMAT;
				}
			}
			break;
		case type:
			if(!strcmp("drink", string) || !strcmp("main", string) || !strcmp("snack", string) ||
			  (!strcmp("dessert", string) || !strcmp("first", string)))
				;
			else{
				printf("Please, choose one of the suggested options.\n");
				return BAD_FORMAT;
			}
			break;
		case date_format:
			for(size_t i = 0; i < strlen(string); i++){
				if(isdigit(string[i]) && (i != 4 && i != 7))
					continue;
				else if(!isdigit(string[i]) && string[i] == '-' && (i == 4 || i == 7))
					continue;
				else
					return BAD_FORMAT;
			}
	}
	return SUCCESS;
}
/*
This function checks if row with given parameters exists in a table. If there is no such row
the function returns true, otherwise return value is equal to false
*/
bool null_selected(MYSQL* connect, unsigned id, char* table_name, char* attribute_name)
{
	char query[MAXLEN];
	
	snprintf(query, MAXLEN, "SELECT * FROM %s WHERE %s = %d", table_name, attribute_name, id);
	
	if(!mysql_query(connect, query)){
		result = mysql_store_result(connect);
		if(mysql_num_rows(result) != 0)
			return false;
	}
	return true;
}
/*
This function removes new line character from a string if there is one
*/
void remove_new_line(char* string)
{
	size_t length;
	
	length = strlen(string);
	
	if(string[length - 1] == '\n')
		string[length - 1] = 0;
}

/*

*/
void install_db(MYSQL* connect)
{
	mysql_query(connect, "CREATE TABLE visitor (visitor_id INT PRIMARY KEY, pass VARCHAR(32) NOT NULL, name VARCHAR(32) NOT NULL)");

	mysql_query(connect, "CREATE TABLE dish (dish_id INT PRIMARY KEY, name VARCHAR(64), price INT NOT NULL, type VARCHAR(64), description VARCHAR(256), pause BOOLEAN, add_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");


	mysql_query(connect, "CREATE TABLE meal (meal_id INT PRIMARY KEY, visitor_id INT, total_price INT, date TIMESTAMP DEFAULT CURRENT_TIMESTAMP, paid BOOLEAN, FOREIGN KEY(visitor_id) REFERENCES visitor(visitor_id))");

	mysql_query(connect, "CREATE TABLE dish_order (dish_id INT, meal_id INT, number INT, FOREIGN KEY(dish_id) REFERENCES dish(dish_id) ON DELETE CASCADE, FOREIGN KEY(meal_id) REFERENCES meal(meal_id) ON DELETE CASCADE, PRIMARY KEY(dish_id, meal_id))");
}

void reg(MYSQL* connect)
{
	int unique_id;
	char password[MAXSTR], conf_pass[MAXSTR], query[MAXLEN], name[MAXSTR];
	
	printf("\n*********************************** Registration ********************************\n");
	unique_id = unique_random_num(connect, 0, MAXID, "visitor", "visitor_id");
	printf("Your unique ID is automatically generated, it will be required for log in.\n");
	printf("Your ID is %d\n", unique_id);
	printf("Your name: ");
	fgets(name, MAXSTR, stdin);
	remove_new_line(name);
	
	do{
		printf("Make up a reliable password: ");
		fgets(password, MAXSTR, stdin);
		remove_new_line(password);
		printf("Enter your password again, please: ");
		fgets(conf_pass, MAXSTR, stdin);
		remove_new_line(conf_pass);
		if(strcmp(password, conf_pass)){
			printf("Passwords do not match, please try again.\n");
			continue;
		}
		snprintf(query, MAXLEN, "INSERT INTO visitor VALUES(%d, '%s', '%s')", unique_id, password, name);
		if(mysql_query(connect, query)){
			printf("%s\n", mysql_error(connect));
			return;
		}
		break;
	}while(true);
	
	printf("You were successfully registred!\n");
}

void print_usage_msg()
{
	printf("This program can be used as a primitive information system for a modern restaurant where visitors can make an order through the information system.\n\n");
	printf("The administrator can display the menu, add, delete, pause and resume dishes, show income for a particular day, month or year.\n\n");
	printf("A visitor can display the menu, make an order, edit the order, display previous orders and \"pay\".\n\n");
	printf("Before using the program:\n");
	printf("1. Download MySQL and log in as a root user.\n");
	printf("2. Type the following command: \nCREATE DATABASE rest_IS; (without quotes)\n");
	printf("3. Open smartsys.c file and change the value of password variable to the password you use to log in MySQL (line 13)\n");
	printf("4. Execute the make file:\nmake\n");
	printf("5. Exit MySQL and type the following command:\n./smartsys --install to create all needed tables.\n");
	printf("6. Now you can use the program!\n\n");
	printf("To run the program in visitor mode:\n./smartsys order\n\n");
	printf("To run program in admin mode:\n./smartsys admin\n\n");
}
