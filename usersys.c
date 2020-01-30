/**
* @file usersys.c
* @author Sokolovskii Vladislav
* @date 20 Jan 2020
* @brief This file contains all functions that are used in the user mode.
* @details Contained in this file functions enable user to log in make an order, edit it and show previous orders of the user.
*/
#include "smartsys.h"

bool order_is_confirmed = false;
int current_user_id;


/*
This function provides interactivity between user and the system. This fucntion calls other functions
according to the user's input.
*/
int user_interface(MYSQL* connect)
{
	int option, meal_id, total;
	bool order_status;
	
	if(log_in(connect, 0) != SUCCESS)
		return EXIT_FAILURE;
	
	do{
		get_option(&option);
		switch(option){
			case show_menu:
				display_dish_menu(connect, all);
				return_to_main_menu(visitor_mode);
				break;
			case create_meal:
				display_dish_menu(connect, all);
				meal_id = create_order(connect);
				order_is_confirmed = false;
				if(meal_id == UNSUCCESS)
					return EXIT_FAILURE;
					
				total = display_order(connect, meal_id);
				sub_menu(order_making);
				do{
					get_option(&option);
					switch(option){
						case make_order:
							if(confirm_order(connect, meal_id, total) == SUCCESS){
								option = back;
								break;
							}
							sub_menu(order_making);
							get_option(&option);
							break;
						case edit_order:
							if(edit_meal(connect, meal_id) == SUCCESS){
								printf("Your meal has been changed!\n");
								total = display_order(connect, meal_id);
								sub_menu(order_making);
								continue;
							}
					}
				}while(option != back);
			
				if(!order_is_confirmed)
					mysql_query(connect, "DELETE FROM meal WHERE date IS NULL");
				return_to_main_menu(visitor_mode);
				break;
			case show_prev_meal:{
				int prev_meal_id;
				
				prev_meal_id = display_prev_orders(connect);
				if(prev_meal_id == UNSUCCESS){
					return_to_main_menu(visitor_mode);
					continue;
				}
				display_order(connect, prev_meal_id);
				return_to_main_menu(visitor_mode);
				break;
			}
		}
	}while(option != end_usr_session);
	
	if(order_is_confirmed)
		pay(connect ,&order_status, meal_id);
	if(order_status == PAID)
		printf("Have a nice day!\n");
	
	return SUCCESS;
}
/*
This function asks for user's id and password in the system, if user gives incorrect data
three times function returns UNSUCCESS (zero value). Otherwise function calls a welcome_screen function
and returns SUCCESS (non-zero value).
*/
int log_in(MYSQL* connect, bool payment)
{
	unsigned id;
	char pass[MAXSTR], name[MAXSTR], id_str[MAXSTR];
	char query[MAXLEN];
	
	for(unsigned try = 0; try < MAX_TRIES; try++){
		do{
			printf("id: ");
			fgets(id_str, MAXSTR, stdin);
			remove_new_line(id_str);
			if(format_check(number, id_str) != SUCCESS)
				continue;
			break;
		}while(true);
		
		printf("password: ");
		fgets(pass, MAXSTR, stdin);
		remove_new_line(pass);
		
		snprintf(query, MAXLEN, "SELECT name FROM visitor WHERE visitor_id = %s AND pass = '%s'", id_str, pass);

		if(mysql_query(connect, query))
			return UNSUCCESS;
		result = mysql_store_result(connect);
		row = mysql_fetch_row(result);
		
		if(mysql_num_rows(result) == 0){
			printf("ID or password is incorrect!\n");
			continue;
		}
		else{
			if(!payment){
				current_user_id = atoi(id_str);
				strcpy(name, row[0]);
				main_menu(name, visitor_mode);
			}
			else{
				if(current_user_id != atoi(id_str)){
					printf("ID or password is incorrect!\n");
					continue;
				}
			}
			mysql_free_result(result);
			
			return SUCCESS;
		}
	}
	mysql_free_result(result);
	
	return UNSUCCESS;
 }
/*
 This function creates a new row in the meal table and asks user what dishes do they want to
 include in this particular order then user is asked if they want to continue choosing,
 in the case of positive feedback cycle repeats, otherwise the process of making
 an order will be ended. If no error occured during the process the function will return order id,
 otherwise the function return value will be UNSUCCESS(zero value).
*/
 int create_order(MYSQL* connect){
	 int dish_id, meal_id, portion_num;
	 char query[MAXLEN], answer;

	 meal_id = unique_random_num(connect, 99, MAXID, "meal", "meal_id");
	 printf("%d\n", current_user_id);
	 snprintf(query, MAXLEN, "INSERT INTO meal VALUES(%d, %d, NULL, NULL, 0)", meal_id, current_user_id);

	 if(mysql_query(connect, query))
		 return UNSUCCESS;
	 fill_order(connect, meal_id);
	 
	 return meal_id;
 }
/*
This function asks user for id of a dish they want to add to their meal as long as user does not choose option N
*/
 int fill_order(MYSQL* connect, int meal_id)
 {
	char query[MAXLEN], id_str[MAXSTR], portion[MAXSTR], answer;
	 
	do{
		do{
			printf("Enter ID of a dish you want to add to your meal:\n");
			fgets(id_str, MAXSTR, stdin);
			remove_new_line(id_str);
			if(format_check(number, id_str) != SUCCESS)
				continue;
			if(null_selected(connect, atoi(id_str), "dish", "dish_id")){
				printf("Dish with this ID does not exist.\n");
				continue;
			}
			break;
		}while(true);
		
		snprintf(query, MAXLEN, "INSERT INTO dish_order VALUES(%s, %d, 1)", id_str, meal_id);
		if(mysql_query(connect, query))
			return UNSUCCESS;
			
		do{
			printf("Number of portions: ");
			fgets(portion, MAXSTR, stdin);
			remove_new_line(portion);
			if(format_check(number, portion) != SUCCESS)
				continue;
			break;
		}while(true);
		
		snprintf(query, MAXLEN, "UPDATE dish_order SET number = %s WHERE dish_id = %s AND meal_id = %d", portion, id_str, meal_id);
		if(mysql_query(connect, query))
			return UNSUCCESS;
			
		printf("Something else? Y/N\n");
		yes_no_answer(&answer);
		if(answer == 'y' || answer == 'Y')
			continue;
		else
			break;
	}while(true);
	
	return SUCCESS;
 }
/*
This function asks user for id of a dish to delete it from their meal
*/
int delete_dish_meal(MYSQL* connect, int meal_id)
{
	char id_str[MAXSTR], query[MAXLEN];
	
	do{
		do{
			printf("Enter id of a dish you want to delete.\n");
			fgets(id_str, MAXSTR, stdin);
			remove_new_line(id_str);
			if(format_check(number, id_str) != SUCCESS)
				continue;
			break;
		}while(true);
			
		snprintf(query, MAXLEN, "SELECT * FROM dish_order WHERE dish_id = %s AND meal_id = %d", id_str, meal_id);
		if(mysql_query(connect, query))
			return UNSUCCESS;
		result = mysql_store_result(connect);
		if(mysql_num_rows(result) == 0){
			printf("There is no dish with this id in your meal.\n");
			mysql_free_result(result);
			continue;
		}
		mysql_free_result(result);
		break;
	}while(true);
	
	snprintf(query, MAXLEN, "DELETE FROM dish_order WHERE meal_id = %d AND dish_id = %s", meal_id, id_str);
	if(mysql_query(connect, query))
		return UNSUCCESS;
	printf("The dish was successfully deleted.\n");
	
	return SUCCESS;
}
/*
 This function takes meal_id as an argument and displays all the dishes that were included into
 this meal, their id and price. After the list of included dishes will be printed a total order amount.
 If no error occured return value is the total amount of the order, otherwise funtion return UNSUCCESS.
*/
 int display_order(MYSQL* connect, int order_id)
 {
	unsigned total;
	char query[MAXLEN];
	
	snprintf(query, MAXLEN, "SELECT dish.dish_id, dish_order.number, name, price FROM dish JOIN dish_order ON dish.dish_id = dish_order.dish_id WHERE meal_id = %d", order_id);
	if(mysql_query(connect, query))
		return UNSUCCESS;
	printf("* ID ****************************** Your order ********************************\n");
	print_results(connect, 4);
	printf("*******************************************************************************\n");
	snprintf(query, MAXLEN, "SELECT SUM(dish.price * dish_order.number) FROM dish JOIN dish_order ON dish.dish_id = dish_order.dish_id WHERE meal_id = %d", order_id);
	if(mysql_query(connect, query))
		return UNSUCCESS;
	result = mysql_use_result(connect);
	row = mysql_fetch_row(result);
	total = atoi(row[0]);
	printf("Total: %d czk\n\n", total);
	mysql_free_result(result);
	 
	return total;
 }
/*
This function displays all the previous orders that user havs ever had at the restaurant
*/
int display_prev_orders(MYSQL* connect)
{
	char query[MAXLEN], prev_meal_id[MAXSTR];
	 
	snprintf(query, MAXLEN, "SELECT meal_id, date, total_price FROM meal WHERE visitor_id = %d AND paid = 1", current_user_id);

	if(mysql_query(connect, query))
		return UNSUCCESS;
	result = mysql_store_result(connect);
	if(mysql_num_rows(result) == 0){
		printf("You have no previous meals.\n");
		return UNSUCCESS;
	}
	if(mysql_query(connect, query))
		return UNSUCCESS;
		
	printf("*******************************************************************************\n");
	print_results(connect, 3);
	printf("*******************************************************************************\n");
	do{
		printf("Enter id of meal you want to see:\n");
		fgets(prev_meal_id, MAXSTR, stdin);
		remove_new_line(prev_meal_id);
		if(format_check(number, prev_meal_id) != SUCCESS)
			continue;
		if(null_selected(connect, atoi(prev_meal_id), "meal", "meal_id")){
			printf("Meal with this id does not exist.\n");
			continue;
		}
		break;
	}while(true);
	
	return atoi(prev_meal_id);
 }
/*
 This function takes meal_id and total order amount as arguments and sets the when an order
 was made and total order amount. If no error occured return value is SUCCESS, otherwise
 the function returns UNSUCCESS.
*/
int confirm_order(MYSQL* connect, int meal_id, int total)
{
	char query[MAXLEN];
	
	snprintf(query, MAXLEN, "UPDATE meal SET total_price = %d, date = NOW() WHERE meal_id = %d", total, meal_id);
	if(mysql_query(connect, query))
		return UNSUCCESS;
	else{
		printf("Your meal is being prepared!\n");
		order_is_confirmed = true;
		return SUCCESS;
	}
}
/*
This function provides sub-menu of edinting an order list
*/
int edit_meal(MYSQL* connect, int meal_id)
{
	int option;
	 
	sub_menu(editing);
	get_option(&option);
	while(option != back){
		switch(option){
			case add_to_meal:
				display_dish_menu(connect, all);
				fill_order(connect, meal_id);
				break;
			case del_from_meal:
				delete_dish_meal(connect, meal_id);
				break;
		}
		sub_menu(editing);
		get_option(&option);
	}
	 
	return SUCCESS;
}
/*
This function displays a sub menu defined by mode value.
*/
void sub_menu(int mode)
{
	switch(mode){
		case order_making:
			printf("*******************************************************************************\n");
			printf("* 1. Confirm my meal                                                          *\n");
			printf("* 2. Edit my meal                                                             *\n");
			printf("* 3. Back                 						      *\n");
			printf("*******************************************************************************\n");
			break;
		case editing:
			printf("*******************************************************************************\n");
			printf("* 1. Add a dish to my meal                                                    *\n");
			printf("* 2. Delete a dish from my meal                                               *\n");
			printf("* 3. Back                 						      *\n");
			printf("*******************************************************************************\n");
			
	}
}
/*
 This function simulates payment process, changes the value order_status to PAID if user enters correct id and password.
 otherwise user's account will be blocked.
*/
void pay(MYSQL* connect, bool* order_status, int meal_id)
{
	char query[MAXLEN];
	
	printf("Enter your id and password to pay, please\n");
	if(log_in(connect, 1) == SUCCESS){
		*order_status = PAID;
		snprintf(query, MAXLEN, "UPDATE meal SET paid = 1 WHERE meal_id = %d", meal_id);
		mysql_query(connect, query);
	}
	else
		printf("Your profile is blocked!\nPlease contact the manager.\n");
 }
