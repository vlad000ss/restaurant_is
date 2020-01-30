/**
* @file admsys.c
* @author Sokolovskii Vladislav
* @date 20 Jan 2020
* @brief This file contains all function that are used in the admin mode.
* @details Contained functions enable admin to add a dish to the menu, set its name, price, type and description(optional).  Also, admin can delete all information about a particular dish from the dish table, temporary remove a dish from the menu and retrieve it later, display income over a particular period of time (day, month, year).
*/
#include "smartsys.h"

/*
This function provides interactivity between user and the system. This fucntion calls other functions
according to the user's input.
*/
int admin_interface(MYSQL* connect)
{
	int option;
	
	main_menu(NULL, admin_mode);
	do{
		get_option(&option);
		
		switch(option){
			case dish_menu:
				display_dish_menu(connect, all);
				return_to_main_menu(admin_mode);
				break;
			case add_dish_menu:
				add_dish(connect);
				return_to_main_menu(admin_mode);
				break;
			case del_dish_menu:
				delete_dish_menu(connect);
				return_to_main_menu(admin_mode);
				break;
			case pause_resume_dish:
				do{
					printf("PAUSE - 1 / RESUME - 0: \n");
					get_option(&option);
					if(option == ACTIVE || option == PAUSE)
						break;
				}
				while(true);
				un_pause_dish(connect, option);
				return_to_main_menu(admin_mode);
				break;
			case show_income_per:
				show_income(connect);
				return_to_main_menu(admin_mode);
				break;
			case end_admin_session:
				printf("Have a nice day!\n");
				return SUCCESS;
		}
	}
	while(true);
	
	return UNSUCCESS;
}
/*
This function user for information about a dish to insert it into the dish table,
after every successful adding of a dish user can choose if they want to continue or end the adding process.
*/
int add_dish(MYSQL* connect){
	unsigned dish_id, price_value;
	char answer;
	char query[MAXLEN], name_str[MAXSTR], type_str[MAXSTR], description[MAXLEN], price_str[MAXSTR];
	
	do{
		dish_id = unique_random_num(connect, 100, MAXID, "dish", "dish_id");
		
		printf("Name of the dish: ");
		fgets(name_str, MAXSTR, stdin);
		remove_new_line(name_str);
		
		do{
		printf("Price: ");
		fgets(price_str, MAXSTR, stdin);
		remove_new_line(price_str);
		}while(format_check(number, price_str) != SUCCESS);
		price_value = atoi(price_str);
		
		do{
		printf("Type(drink, main, snack, first, dessert): ");
		fgets(type_str, MAXSTR, stdin);
		remove_new_line(type_str);
		}while(format_check(type, type_str) != SUCCESS);
		
		printf("You can add detailed description of a dish: ");
		fgets(description, MAXLEN, stdin);
		remove_new_line(description);
		
		snprintf(query, MAXLEN, "INSERT INTO dish VALUES(%d, '%s', %d, '%s', '%s', %d, NOW())", dish_id, name_str, price_value, type_str, description, ACTIVE);
		if(!mysql_query(connect, query)){
			printf("Dish was successfully added!\n");
			printf("Would you like to add another dish? Y/N\n");
			yes_no_answer(&answer);
			if(answer == 'y' || answer == 'Y')
				continue;
			else
				break;
		}
		else
			return UNSUCCESS;
	}while(true);
	
	return SUCCESS;
}
/*
This function asks user for an id of the dish they want to delete, after every successful deleting of
a dish user can choose if they want to continue deleting dishes. Return SUCCESS if deleting process
didn't cause any errors. Otherwise returns UNSUCCESS.
*/
int delete_dish_menu(MYSQL* connect)
{
	unsigned id;
	char query[MAXLEN], id_str[MAXSTR], answer;
	display_dish_menu(connect, all);
	do{
		do{
			printf("ID of the dish you want to delete:\n");
			fgets(id_str, MAXSTR, stdin);
			remove_new_line(id_str);
			if(format_check(number, id_str) != SUCCESS)
				continue;
			id = atoi(id_str);
			if(null_selected(connect, id, "dish", "dish_id")){
				printf("Dish with this ID does not exist.\n");
				continue;
			}
			else
				break;
		}while(true);
		
		snprintf(query, MAXLEN, "DELETE FROM dish WHERE dish_id = %d", id);
		if(!mysql_query(connect, query)){
			printf("Dish was successfully deleted!\n");
			printf("Would you like to delete another dish? Y/N\n");
			scanf("%c", &answer);
			getchar();
			if(answer == 'y' || answer == 'Y')
				continue;
			else
				break;
		}
		else
			return UNSUCCESS;
	}while(true);
	
	return SUCCESS;
}
/*
This function sets attribute 'pause' to true when user wants temporary remove a dish from the menu
or to false when user wants to resume previously paused dish. Return value is SUCCESS if a dish
was successfully un/paused. Otherwise return value is UNSUCCESS.
*/
int un_pause_dish(MYSQL* connect, int mode)
{
	unsigned id;
	char answer;
	char query[MAXLEN], id_str[MAXSTR];
	
	if(mode == PAUSE){
		if(mysql_query(connect, "SELECT dish_id, name, price FROM dish ORDER BY add_date DESC"))
			return UNSUCCESS;
		printf("* ID **************************************************************************\n");
		print_results(connect, 3);
		printf("*******************************************************************************\n");
	}
	else if(mode == ACTIVE){
		if(null_selected(connect, PAUSE, "dish", "pause")){
			printf("All dishes are avaliable.\n");
			return SUCCESS;
		}
		if(mysql_query(connect, "SELECT dish_id, name, price FROM dish WHERE pause = 1 ORDER BY add_date DESC"))
			return UNSUCCESS;
		printf("* ID ****************************** Paused dishes *****************************\n");
		print_results(connect, 3);
		printf("*******************************************************************************\n");
	}
	
	do{
		do{
			printf("Please, enter ID of the dish:\n");
			fgets(id_str, MAXSTR, stdin);
			remove_new_line(id_str);
			if(format_check(number, id_str) != SUCCESS)
				continue;
			id = atoi(id_str);
			if(null_selected(connect, id, "dish", "dish_id")){
				printf("Dish with this ID doesn't exist.\n");
				continue;
			}
			else
				break;
		}while(true);
			snprintf(query, MAXLEN, "UPDATE dish SET pause = %d WHERE dish_id = %d", mode, id);
			if(mysql_query(connect, query))
				return UNSUCCESS;
				
		if(mode == PAUSE){
			printf("Dish was successfully paused!\n");
			printf("Would you like to temporary remove another dish? Y/N\n");
		}
		else if(mode == ACTIVE){
			printf("Dish was successfully returned to the today's menu!\n");
			printf("Would you like to resume another dish? Y/N\n");
		}
		yes_no_answer(&answer);
		if(answer == 'y' || answer == 'Y')
			continue;
		else
			break;
	}while(true);
	
	return SUCCESS;
}
/*
This function asks user to enter the time period to count income and print out obtained income
during given period of time.
*/
void show_income(MYSQL* connect)
{
	int option;
	char date[DATELEN + 1], query[MAXLEN];
	
	printf("See income per?\nDAY - 1, MONTH - 2, YEAR - 3\n");
	do{
		get_option(&option);
		if(option == 1 || option == 2 || option == 3)
			break;
	}while(true);
	
	switch(option){
		case 1:
			do{
				printf("Enter date in the format YYYY-MM-DD:\n");
				fgets(date, DATELEN + 1, stdin);
				remove_new_line(date);
				if(format_check(date_format, date) != SUCCESS)
					continue;
				else
					break;
			}while(true);
			snprintf(query, MAXLEN, "SELECT SUM(total_price) FROM meal WHERE date LIKE '%s%%' AND paid = true", date);
			break;
		case 2:
			do{
				printf("Enter year and month in the format YYYY-MM:\n");
				fgets(date, DATELEN, stdin);
				remove_new_line(date);
				if(format_check(date_format, date) != SUCCESS)
					continue;
				else
					break;
			}while(true);
			snprintf(query, MAXLEN, "SELECT SUM(total_price) FROM meal WHERE date LIKE '%s%%' AND paid = true", date);
			break;
		case 3:
			do{
				printf("Enter the year YYYY:\n");
				fgets(date, DATELEN, stdin);
				remove_new_line(date);
				if(format_check(date_format, date) != SUCCESS)
					continue;
				else
					break;
			}while(true);
			snprintf(query, MAXLEN, "SELECT SUM(total_price) FROM meal WHERE date LIKE '%s%%' AND paid = true", date);
			break;
	}
	if(mysql_query(connect, query))
		printf("HUY\n");
	result = mysql_store_result(connect);
	row = mysql_fetch_row(result);
	printf("Income:\n\t%s czk\n", row[0]);
	
	mysql_free_result(result);
}
