/**
* @file smartsys.h
* @author Sokolovskii Vladislav
* @date 20 Jan 2020
* @brief This header file contains declarations of all functions, enumerations and macroses that program use.
*/
#ifndef SMARTSYS
#define SMARTSYS
	#define smartsys_h
	
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <stdbool.h>
	#include <mysql.h>/**This is the library which supportes MySQL C API and provides access to the database content*/
	#include <time.h>
	
	#define DATELEN 11/**<The length of date in YYYY-MM-DD format + newline character*/
	#define MAXSTR 32/**<Maximum length of a string*/
	#define MAXLEN 256/**<Maximum length of a query*/
	#define MAX_TRIES 5/**<Maximum number of tries to log in*/
	#define SUCCESS 1/**<Non-zero return value indicating successful ending of a function*/
	#define UNSUCCESS 0/**<Zero return value indicating unsuccessful ending of a function*/
	#define MAXID 999/**<Maxim possible id number*/
	#define MENUWIDTH 66/**<The width of the "window" of app*/
	#define ACTIVE 0/**<Zero value indicating that dish is avaliable in the menu*/
	#define PAUSE 1/**<Non zero value indication that dish is paused*/
	#define PAID 1/**<Non zero value indication that the order was paid*/
	#define BAD_FORMAT -1/**<Negative return value indication that given value do not correspond with the pattern*/
	
/**
The ennumeration of possible program behaviours
 */
	enum work_modes{idle,
	visitor_mode = 11, /**<Visitor behavior index*/
	admin_mode = 22,/**<Admin behavior index*/
	installation,/**<Installation behavior index*/
	registration,/**<Registration of a new user behavoir index*/
	help/**<Usage message behavior index*/
	};
/**
The ennumeration of possible string formats
 */
	enum formats{type = 60,/**< Type of a dish format*/
	number,/**<Number format*/
	date_format/**<Date YYYY-MM-DD format*/
	};
/**
The ennumeration of types of the dishes
 */
	enum types_of_dishes{drink = 12,
	first,/**<The first course type*/
	main_dish,/**<The main dish type*/
	dessert,/**<Dessert dish type*/
	snack,/**<Snack dish type*/
	all/**<Dish type that includes all types of dishes*/
	};
/**
The ennumeraton of admin main menu options
 */
	enum admin_main_menu{
	dish_menu = 1,/**<Display the menu option*/
	add_dish_menu,/**<Add a dish to the menu option*/
	del_dish_menu,/**Delete a dish from the menu option*/
	pause_resume_dish,/**<Temporary remove or resume a dish option*/
	show_income_per,/**<Show income option*/
	end_admin_session/**<End session option*/
	};
/**
The ennumeration of user main menu options
 */
	enum user_main_menu{
	show_menu = 1,/**<Display the menu option*/
	create_meal,/**<Make an order option*/
	show_prev_meal,/**<Show previous meals option*/
	end_usr_session/**<End session option*/
	};
/**
The ennumerations of user sub menu options
 */
	enum sub_menu_options{make_order = 1,
	edit_order,/**<Edit the order information option*/
	back,/**<Back to the main menu option*/
	add_to_meal = 1,/**<Add a dish to the meal option*/
	del_from_meal/**<Delete a dish from the meal option*/
	};
/**
The ennumeration of types of user sub menus
 */
	enum sub_menu_modes{
	order_making = 50,/**<Make the order sub menu*/
	editing/**<Edit the order sub menu*/
	};
/**
@brief This function provides interactivity between admin and the system. This driver fucntion calls other functions
according to the user's input.
@pre The program must be executed with 'admin' parameter
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@return This function returns \b SUCCESS (1) if no error occured.  Otherwise returns \b UNSUCCESS (0)
 */
	int admin_interface(MYSQL* connect);
/**
@brief This function checks arguments from the command line and returns a number
corresponding to one of the behaviour indexes.
@param[in] argc - This is the number of arguments passed to the command line
@param[in] argv - This is array of arguments passed to the command line
@return This function returns the index of expected behavior according to the argument passed to the command line
@post One of the possible program flows will be chosen
 */
	int args_check(int argc, char** argv);
/**
@brief This function asks for user's id and password in the system, if given data was found in the database the function returns SUCCESS (1). Otherwise
if user enter bad data five times the function will return UNSUCCESS (0). Also, the function is user of confirmation of a payment.
@param[in] connect - This is a MYSQL pointer which enables the connection to the database
@param[in] payment - This is a boolean value which indicates if the function is used for confirmation of a payment or for log in.
@return This function returns SUCCESS (1) if user gave correct id and password. Otherwise return value is UNSUCCESS (0).
@post Depending on the return value the program will terminate or continue.
 */
	int log_in(MYSQL* connect, bool payment);
/**
@brief This function prints out one type of the main menu (visitor or admin) according to the passed arguments. If the function is called for the first time
in visitor mode the personilized welcome message will be printed.
@param[in] name - This is the name of the visitor.
@param[in] mode - This is the current mode of the program (visitor or admin)
 */
	void main_menu(char* name, int mode);
/**
@brief This function asks user if they want to return to the main menu
@param[in] mode - This is the current mode of the program (visitor or admin)
@post The main_menu function will be called.
 */
	void return_to_main_menu(int mode);
/**
@brief This function assignes a passed char variable to the one of the options 'Y' or 'N' depending on the user's input
@param[in] answer - This is the pointer to the answer variable
 */
	void yes_no_answer(char* answer);
/**
@brief This function prints out all dishes included to the menu in categories according to their type.
@param[in] connect - This is a MYSQL pointer which enables the connection to the database
@param[in] type - This is one of the values of ennumeration of types of the dishes
 */
	void display_dish_menu(MYSQL* connect, int type);
/**
@brief This function enables admin to add a new dish to the menu, admin can choose name, price, dish type and description(optional)
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@return This function return SUCCESS (1) in the case when no error occured. Otherwise return value is UNSUCCESS (0)
@post A new row to the dish table will be added
 */
	int add_dish(MYSQL* connect);
/**
@brief This function enables admin to permanently delete all information about a dish from the dish table
@param[in] connect - This is a MYSQL pointer which enables the connection to the database
@return This function returns SUCCESS (1) in the case when no error occured. Otherwise return value is UNSUCCESS (0)
@post A row from the dish table will be deleted
 */
	int delete_dish_menu(MYSQL* connect);
/**
@brief This function enables a visitor to delete a dish from their order.
@pre edit_meal function must be called before
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] meal_id - This is the unique identificator of the order we edit now
@return This function returns  SUCCESS (1) in the case when no error occured. Otherwise return value is UNSUCCESS (0)
@post A dish from the particular order will be deleted
 */
	int delete_dish_meal(MYSQL* connect, int meal_id);
/**
@brief This function prints out context of every column of selected row/rows
@param[in] connect - This is a MYSQL pointer which enables the connection to the database
@param[in] cols - This is the number of colums to print
 */
	void print_results(MYSQL* connect, int cols);
/**
@brief This function generates a  number on defined interval and makes sure that this number  is unique within a particular table.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] lower - This is the lower bound of the interval
@param[in] upper - This is the upper bound of the interval
@param[in] table_name - This is the name of the table in which the generated number must be unique
@param[in] attribute_name - This is the attribute name that will be assigned to the generated number
 */
	int unique_random_num(MYSQL* connect, int lower, int upper, char* table_name, char* attribute_name);
/**
@brief This function sets attribute 'pause' in the dish table to \b true when user wants to temporary remove a dish from the menu
or to \b false when user wants to resume a previously paused dish.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] mode - This is the chosen behavior index (pause or resume 1/0)
@return Return value is SUCCESS (1) if a dish was successfully un/paused. Otherwise return value is UNSUCCESS (0).
 */
	int un_pause_dish(MYSQL* connect, int mode);
/**
@brief This function inserts a new row to the meal table
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@post fill_order funcion will be called
@return The function returns the id of newly created order
 */
	int create_order(MYSQL* connect);
/**
@brief This function ask user what dishes do they want to include in this particular meal then user is asked if they want to continue choosing,
in the case of positive feedback cycle repeats, otherwise the process of making an order will be ended.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] meal_id - This is the unique identificator of the order we want to fill with dishes
@return This function returns SUCCESS (1) in the case when no error occured. Otherwise return value is UNSUCCESS (0)
 */
	int fill_order(MYSQL* connect, int meal_id);
/**
@brief This function prints out name id and price of every dish that was included to the particular meal. On the end total amount is printed.
@param connect - This is a MYSQL pointer which provides the connection to the database
@param[in] meal_id - This is the unique identificator of the order we want to display
@return This function returns value that is equal to the total amount of the particular order
 */
	int display_order(MYSQL* connect, int order_id);
/**
@brief This function displays id, date and total price of all the previous orders that user has ever had at the restaurant. And asks user for
the id of the meal to display.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@return This function returns the id of the meal to display
@post The display_order function will be called
 */
	int display_prev_orders(MYSQL* connect);
/**
@pre A user chose 'confirm my meal' option in sub menu of making order
@brief This function sets the date and time when the order was made and total order amount.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] meal_id - This is the unique identificator of the order user wants to confirm
@param[in] total - This is the total price of the particular order
@return This function returns the id of the meal user wants to display
@post After finishing the session a user will be asked to pay
 */
	int confirm_order(MYSQL* connect, int meal_id, int total);
/**
@brief This function enables visitor to edit their meal (add or delete a dish)
 @param[in] connect - This is a MYSQL pointer which provides the connection to the database
 @param[in] meal_id - This is the unique identificator of the order user wants to edit
 @return This function return SUCCESS (1) in the case when no error occured. Otherwise return value is UNSUCCESS (0)
 */
	int edit_meal(MYSQL* connect, int meal_id);
/**
@brief This function prints out a type of sub menu according to the passed argument
@param[in] mode - This is the index of particular sub menu
 */
	void sub_menu(int mode);
/**
@brief This function assignes 'option' to the chosen index behavior. The function will ask a user to enter
one of the options as long as the entered value will not satisfy any of the given options.
@param[in] option - This is pointer to the option value, which is used to choose the flow of the program
 */
	void get_option(int* option);
/**
@brief This function simulates payment process and changes the value order_status in meal table to PAID if log_in function returns the SUCCESS (1) value
 @param[in] connect - This is a MYSQL pointer which provides the connection to the database
 @param[in] order_status - This is the value which indicates if order was confirmed or no
 @param[in] meal_id - This is the unique identificator of the order user wants to pay for
 */
	void pay(MYSQL* connect, bool* order_status, int meal_id);
/**
@brief This function asks user over what period of time they want to display the income and displays it.
@param[in] connect - This is a MYSQL pointer which enables the connection to the database
 */
	void show_income(MYSQL* connect);
/**
@brief This function checks if given string corresponds to the chosen format.
@param[in] format - This is a value from formats enumeration representing a particular fomat (date, number, dish_type)
@param[in] string - This is a string to be checked
@return This function returns SUCCESS (1) in the case when the string corresponds to the format. Otherwise return value is UNSUCCESS (0)
 */
	int format_check(int format, char* string);
/**
@brief This function deletes the newline character from the given string
@param[in] string - This is the string from where the newline character must be deleted
 */
	void remove_new_line(char* string);
/**
@brief This function checks if particular attribute value exist in a particular table.
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@param[in] id - This is a numerical attribute value to be checked
@param[in] table_name - This is the name of the table in which a number must be checked
@param[in] attribute_name - This is the name of the attribute of a table
@return This function returns \b false if id value of attribute_name exists in table_name. Otherwise return value is \b true
 */
	bool null_selected(MYSQL* connect, unsigned id, char* table_name, char* attribute_name);
/**
@brief This function provides interactivity between user and the system. This driver fucntion calls other functions
according to the user's input.
@pre The program must be executed with 'order' parameter
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@return This function returns \b SUCCESS (1) if no error occured.  Otherwise returns \b UNSUCCESS (0)
 */
	int user_interface(MYSQL* connect);
/**
@brief This function creates tables that are necessary for proper work of the program
@pre The program must be executed with '--install' parametr
@param[in] connect - This is a MYSQL pointer which provides the connection to the database
@post The program will work correctly
 */
	void install_db(MYSQL* connect);
/**
@brief This function enables user to create an account in the system. The function calls unique_random_number to generate unique user id.
@param[in] correct - This is a MYSQL pointer which provides the connection to the database
@post A new row in visitor table will be created
 */
	void reg(MYSQL* connect);
/**
@brief This function prints out the usage message
@pre The program must be executed with '--help' parameter
 */
	void print_usage_msg(void);
	
	
	static bool first_visit = true;/**<Boolean value which is true only before the fist opening of the menu*/
	
	MYSQL_RES* result;
	MYSQL_ROW row;
	
#endif


