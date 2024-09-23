// Including neccessary header files for local use
#include <iostream>
#include <vector> // Required for std::vector
#include "sqlDatabaseAPI.h"
#include <chrono> // Required for std::chrono::milliseconds()
#include <thread> // Required for std::this_thread::std::sleep_for()
#include <iomanip> // Required for std::setw
#include <cctype> // toupper_case and tolower_case

// Function's declaration
void main_layout();
void folder_layout();
void create_folder();
void fetch_todo(int status);
void in_folder(std::string id);
int io_Validation(int layout_ID, int status_);
void toinsert_todo_api();
void specific_todo(std::string todo_, int which_);
void postdelayed_(std::string message, int milliseconds);

// Global constant variable's defintion   
const int DONE_TODO = 0;
const int UNDONE_TODO = 1;

// The main function, which is called by operating system
int main(){
	int status_code = db_onCreate();
	if(status_code == 0) main_layout();
	return 0;
}

// Function's definition
void main_layout(){
	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB TO-DO LIST" << std::endl;	
	std::cout << "[1] Folders" << std::endl << "[2] Done" << std::endl << "[3] Undone" << std::endl << "[4] Add to-do list" << std::endl << "[0] Exit" << std::endl << "=> ";
    io_Validation(1, -1);
}

//
void folder_layout(){
	std::vector<std::string> folders_name;
	folders_name = Retrieve_folders();
	
	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB FOLDERS" << std::endl;
	if(!folders_name.empty()){
	    int i = 0, j = 1;
	    while(j < folders_name.size()){
		    std::cout << "[" << folders_name[i] << "] " << folders_name[j] << std::endl;
		    i += 2; j += 2;
		}	
	}
	
	std::cout << std::endl << "[C] Create Folders" << std::endl << "[D] Delete Folder" << std::endl << "[B] Back" << std::endl << "=> ";
	if(io_Validation(2, -1) == 1){
		std::string delete_by_id;
		int j = 0;
	
	    std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB DELETE" << std::endl;
    	std::cout << "Enter ID: ";
	    std::cin >> delete_by_id;
	    system("cls");
	
	    while(j < folders_name.size()){
	    	if(delete_by_id == folders_name[j]){
	    		if(::delete_folder_and_todo(delete_by_id, FOLDER_TABLE_DB_ID, folders_name[j+1]) == 0) std::cout << "Deleted!";
	            else std::cout << "Not Deleted!";
	            break;
			}
			j += 2;  
		}
		folder_layout();
	}
}

// 
void create_folder(){
	std::string frName; // camel case variable name
	std::vector<std::string> cr_folders = ::Retrieve_folders();
	
	system("cls");
	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB CREATE FOLDER" << std::endl;
	std::cout << "Folder Name: ";
	/*
	In C++, you can convert characters to uppercase or lowercase using the standard library functions from the <cctype> header. Here are some examples demonstrating how to do this:

Converting a Single Character
You can use toupper() to convert a character to uppercase and tolower() to convert a character to lowercase.

cpp

Copy
#include <iostream>
#include <cctype> // for toupper and tolower

int main() {
    char ch;

    std::cout << "Enter a character: ";
    std::cin >> ch;

    char upper = std::toupper(ch);
    char lower = std::tolower(ch);

    std::cout << "Uppercase: " << upper << std::endl;
    std::cout << "Lowercase: " << lower << std::endl;

    return 0;
}
Converting a String
To convert an entire string, you can iterate through each character and apply toupper() or tolower().

cpp

Copy
#include <iostream>
#include <string>
#include <cctype>

int main() {
    std::string str;

    std::cout << "Enter a string: ";
    std::getline(std::cin, str);

    // Convert to uppercase
    for (char& ch : str) {
        ch = std::toupper(ch);
    }
    std::cout << "Uppercase: " << str << std::endl;

    // Convert back to lowercase
    for (char& ch : str) {
        ch = std::tolower(ch);
    }
    std::cout << "Lowercase: " << str << std::endl;

    return 0;
}
	*/
	std::cin.ignore();
	getline(std::cin, frName);
	
	// A lambda function
	auto str_tolower_case = [](std::string lowercase_folder_name)-> std::string {
		for(char& ch : lowercase_folder_name) ch = std::tolower(ch);
		return lowercase_folder_name;
	};
	
	/*
	to_Uppercase and to_Lowercase
	*/
	if(!cr_folders.empty()){
		bool chk = 1;
	    for(int j = 0; j < cr_folders.size(); j++){
		    if(str_tolower_case(cr_folders[j]) == str_tolower_case(frName)){
		        std::cout << "The name is taken!";
		        chk = 0;
		        break;
		    }
	    }
	    if(chk) ::create_folder_(frName);
    }else ::create_folder_(frName);
}

//
void fetch_todo(int status){
	std::vector<std::vector<std::string>> todolst;
	std::string input;
	
	if(status == DONE_TODO){ 
	    std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB DONE" << std::endl;
	    todolst = ::todo_lists(status);
    }else{
    	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB UNDONE" << std::endl;
    	todolst = ::todo_lists(status);
	}
	
	if(!todolst.empty()){
		int j = 0;
		std::cout << std::setw(5) << "ID" << std::setw(25) << "DATE" << std::setw(45) << "TITTLE" << std::setw(25) << "FOLDER" << std::setw(25) << "STATUS" << std::endl;
	    for(int i = 0; i < todolst.size(); i++) std::cout << std::setw(5) << todolst[i][j] << std::setw(25) << todolst[i][j+1] << std::setw(45) << todolst[i][j+2] << std::setw(25) << todolst[i][j+4] << std::setw(25) << todolst[i][j+5] << std::endl;	
	}else std::cout << "No Data Found!" << std::endl;
	
	std::cout << "[D] Delete Todo\n[B] Back\n=> ";
	if(io_Validation(3, status) == 1){
		std::string delete_by_id;
		std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB DELETE" << std::endl;
	    std::cout << "Enter ID: ";
	    std::cin >> delete_by_id;
	    system("cls");

        /* :delete_folder_and_todo(id, TODO_TABLE_DB_ID, NULL)
		std::string: Can accept nullptr, but handle it carefully.
		int, float, bool: Cannot accept NULL directly; use sentinel values or alternatives to indicate "no value."
		
		* delete_folder_and_todo(id, TODO_TABLE_DB_ID, nullptr)
		 Error: terminate called after throwing an instance of 'std::logic_error'
	   	    what():  basic_string::_M_construct null not valid
			* When you pass a nullptr to a function expecting a std::string, 
			the constructor of std::string attempts to create a string from the null pointer, 
			which is undefined behavior in C++. 
		*/
	    for(int i = 0; i < todolst.size(); i++){
	    	if(delete_by_id == todolst[i][0]){
	    		if(::delete_folder_and_todo(delete_by_id, TODO_TABLE_DB_ID, "") == 0) std::cout << "Deleted!";
	            else std::cout << "Not Deleted!";
			}
		}
	    fetch_todo(status);
	}
}

//
void in_folder(std::string id){
	std::vector<std::vector<std::string>> todo_infolder = ::in_folder_(id);
	std::string io;
	
	/*
	* * if(todo_infolder != {})
	In C++, using if (object != {}) can lead to confusion because {} is often interpreted as an initializer for an empty object, 
	and whether this comparison is valid depends on the type of object.
	
	Primitive Types: Use direct comparisons to specific values.
	Containers(vector, list): Use .empty() method.
	Custom Classes: Implement logic to check if the object is "empty" or valid.
	std::optional: Use the implicit conversion to check if it has a value.
	
	#include <optional>
    std::optional<int> maybeValue; // Declare an optional integer    
    if (!maybeValue) {} // Check if it has a value, true
    maybeValue = 42;// Assign a value
    if (maybeValue) {} // true
    maybeValue.reset(); // Resetting the optional to empty
	
	* Using if (object != {}) is generally not advisable due to ambiguity, 
	so it's better to use clear and type-specific checks.
	*/

	if(!todo_infolder.empty()){
		std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB " << todo_infolder[0][4] << std::endl;
		int j = 0;
	    std::cout << std::setw(5) << "ID" << std::setw(25) << "DATE" << std::setw(45) << "TITTLE" << std::setw(25) << "FOLDER" << std::setw(25) << "STATUS" << std::endl;
	    for(int i = 0; i < todo_infolder.size(); i++) std::cout << std::setw(5) << todo_infolder[i][j] << std::setw(25) << todo_infolder[i][j+1] << std::setw(45) << todo_infolder[i][j+2] << std::setw(25) << todo_infolder[i][j+4] << std::setw(25) << todo_infolder[i][j+5] << std::endl; 
	}else std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB" << std::endl << "No Data Found!" << std::endl;;
	
	std::cout << "[D] Delete Todo\n[B] Back\n=> ";
	if(io_Validation(4, -1) == 1){
	    std::string delete_by_id;
	    std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB DELETE" << std::endl;
	    std::cout << "Enter ID: ";
	    std::cin >> delete_by_id;
	    system("cls");
	    
	    for(int i = 0; i < todo_infolder.size(); i++){
	    	if(delete_by_id == todo_infolder[i][0]){
	    		if(::delete_folder_and_todo(delete_by_id, TODO_TABLE_DB_ID, "") == 0) std::cout << "Deleted!";
	            else std::cout << "Not Deleted!";
			}
		}
	    in_folder(id);
	}
}

// 
int io_Validation(int layout_ID, int status_){
	// variables for user input handling
	std::string input;
	std::cin >> input;
	
	switch(layout_ID){
	    case 4:
	    // specifictodo layout, input validation
	        if(input == "d"){
		        system("cls");
		        return 1;
	        }else if(input == "b"){
		        system("cls");
		        folder_layout();
	        }else break;
		    break;	
		case 1:
		// main layout input validation
	        if(input == "1"){
		        system("cls");
		        folder_layout();
	        }else if(input == "2"){
	        	system("cls");
		        fetch_todo(DONE_TODO);
	        }else if(input == "3"){
		        system("cls");
		        fetch_todo(UNDONE_TODO);
	        }else if(input == "4"){
		        system("cls");
		        toinsert_todo_api();
		        system("cls");
		        main_layout();
	        }else break;
			break;
		case 2:
		// folder layout, input validation
	        if(input == "c"){
		        create_folder();
		        system("cls");
		        main_layout();
	        }else if(input == "d"){
	        	system("cls");
	        	return 1;
	        }else if(input == "b"){
		        system("cls");
		        main_layout();
	        }else if(input.size() <= 2){
	        	system("cls");
		        in_folder(input);
	        }else break;
			break;
	    case 3:
	    // fetch_todo layout, input validation
	        if(input == "d"){
		        system("cls");
		        return 1;
	        }else if(input == "b"){
		        system("cls");
		        main_layout();
		    }else if(input.size() <= 5){
		    	system("cls");
		    	specific_todo(input, status_);
			}
			break;
		    default: break;
	}
	return 0;
}

//
void toinsert_todo_api(){
	std::string io;
	std::vector<std::string> json_;
	std::vector<std::string> header_ = {"Date", "Tittle", "Description", "Folder ID"};
	std::vector<std::string> folders;
	folders = Retrieve_folders();
	
	/*
	[](std::string id)->bool{
		int j = 0;
		while(j < folders.size()){
			if(folders[j] == id){
				return true;
			}  
			j+=2;
		}
		return false;
	};
	** A "vector is not captured" means that the vector isn't accessible in the current scope, often due to scope or capture issues. 
	Ensuring proper access through capturing or sharing directives can resolve this problem. 
		
	auto is_available = [&folders](std::string id)->bool{
	    int j = 0;
		while(j < folders.size()){
			if(folders[j] == id){
				std::cout << j;
				return true;
			}  
			j+=2;
		}
		std::cout << j << " : " << folders[j] << std::endl;
		
		return false;
	};*/

	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB ADD TODO LIST" << std::endl;
	if(folders.empty()) postdelayed_("No Folder Found!", 1500);
	else{
		// user input handling
		std::cin.ignore();
	    for(int i = 0; i < header_.size(); i++){
			if(i == 3){
		        std::cout << std::endl << "\xDB\xDB AVAILABLE FOLDERS" << std::endl;
		        int j = 0; 
				while(j < folders.size()){
		        	std::cout << "ID: " << folders[j] << " Folder Name: " << folders[j+1] << std::endl;
					j +=2; 
				}
					
				std::cout << std::endl << header_[i] << ": ";
				bool chk = false;
				do{
		            getline(std::cin, io);
		            if(io.empty()) continue;
		            for(int is_available = 0; is_available < folders.size(); is_available++){
		            	if(io == folders[is_available]){
		            		chk = true;
		            		io = folders[is_available + 1];
		            		break;
						}
					}
				}while(!chk);
			}else{
//				std::cin.ignore();
				do{
				    std::cout << header_[i] << ": ";    	
		            getline(std::cin, io);
	            }while(io.empty());
			}
			json_.push_back(io); //
	    }
	    io = "undone";
	    json_.push_back(io);
	    ::toinsert_todo_(json_);
    }   
	
	/*
	::chrono
	The C++ standard library introduced a date and time library in C++20, which includes features for managing dates, times, and durations. 
	Overview of its key components:
	
	* Chrono Library:
	The std::chrono namespace provides facilities for working with time durations, clocks, and time points.
	Key types include:
	std::chrono::duration: Represents a time duration.
	std::chrono::time_point: Represents a point in time.
	std::chrono::system_clock: Represents the system clock.
	
	* Calendar Functions:
	The library includes calendar functionality, allowing for operations on dates.
	You can manipulate dates using the std::chrono::year, std::chrono::month, and std::chrono::day types.
	
	* Date and Time Formatting:
	Functions to format and parse date and time strings.
	*/
	
	/*
	** time_point and system_clock 
	
	* time_point
	Definition: A time_point represents a specific point in time. 
	It is defined as a duration since a certain epoch (a starting point in time).
	    * Template: The std::chrono::time_point class template takes two parameters:
	    A clock type (e.g., std::chrono::system_clock, std::chrono::steady_clock).
	    A duration type (e.g., std::chrono::seconds, std::chrono::milliseconds).
	    * Usage:
	    You can create a time_point to represent the current time or any specific moment.
	    Common operations include comparing time points, calculating the difference between them, and converting them to other time representations.
	
	* system_clock
	Definition: std::chrono::system_clock is one of the clocks provided by the C++ standard library. 
	It represents the system-wide real-time clock.
	    * Characteristics:
		It can be used to obtain the current time as a time_point.
		The time it represents is based on the system's real-time clock and can be affected by adjustments to the system time.
		
	* Key Functions:
	now(): Returns the current time as a time_point.
	to_time_t(): Converts a time_point to a time_t, which is a C-style representation of time.
	from_time_t(): Converts a time_t back to a time_point.
	*/
}

//
void postdelayed_(std::string message, int milliseconds){
	std::cout << message;
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	return;
}

//
void specific_todo(std::string todo_, int which_){
	std::vector<std::vector<std::string>> todo_s;
	std::cout << "\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB SPECIFIC TODO" << std::endl;
	
	/*
	Ternary Operator: A concise, single-line expression that evaluates and returns a value based on a condition. 
	If-Else Statement: A clearer, more explicit structure that allows for additional logic and can be easier to understand at a glance.
	Both approaches are valid; the choice between them often depends on personal or team coding style preferences and the complexity of the logic being implemented.
	*/ 
	which_ == DONE_TODO ? todo_s = todo_lists(DONE_TODO) : todo_s = todo_lists(UNDONE_TODO);
	
	if(!todo_s.empty()){
		bool chk_id = false;
		for(int i = 0; i < todo_s.size(); i++){
			if(todo_s[i][0] == todo_){
				chk_id = true;
				break;
			}
		}
		
		if(chk_id){
		    todo_s.push_back(fetch_specific_todo(todo_));
			if(!todo_s.empty()){
				int size_of_vec = todo_s.size();
				std::cout << "[" << todo_s[size_of_vec-1][0] << "] ID" << std::endl << "    DATE: " << todo_s[size_of_vec-1][1] << std::endl << "    TITTLE: " << todo_s[size_of_vec-1][2] << std::endl << "    FOLDER NAME: " << todo_s[size_of_vec-1][4] << std::endl << "    STATUS: " << todo_s[size_of_vec-1][5] << std::endl << "    DESCRIPTION: " << todo_s[size_of_vec-1][3] << std::endl;  
			}
			
			std::string io_;
			std::cout << "[d] done\n" << "[u] undone\n";
			std::cin >> io_;
			
			if(io_ == "d" && todo_s[todo_s.size()-1][5] == "undone") done_or_undone(todo_s[todo_s.size()-1][0], DONE_TODO);
			else if (io_ == "u" && todo_s[todo_s.size()-1][5] == "done") done_or_undone(todo_s[todo_s.size()-1][0], UNDONE_TODO);
		}
	}else std::cout << "No Todo Found!";
}
