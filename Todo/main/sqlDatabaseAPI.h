// Including neccessary header files for local use
#include "sqlite3.h"
#include <iostream>
#include <string>
#include <cctype> // toUpper_case and toLower_case
#include <fstream> // Required for ofstream
#include <chrono>
#include <thread>

// Function's declaration
int db_onCreate();
void create_folder_(std::string folder_name);
std::vector<std::string> Retrieve_folders();
std::vector<std::vector<std::string>> in_folder_(std::string id);
int delete_folder_and_todo(std::string id, int which_table, std::string folder_name);
std::vector<std::vector<std::string>> todo_lists(int sign);
void delete_todo_in_folder(std::string folder_name);
void toinsert_todo_(std::vector<std::string> json_);
std::vector<std::string> fetch_specific_todo(std::string id);
void done_or_undone(std::string id, int status);

/*
The "database is locked" error in SQLite typically occurs when:

Another connection holds a write lock: If another process or thread is writing to the database, your connection might be unable to acquire the necessary locks to perform its operation.
Long-running transactions: If a transaction is open for a long time without being committed or rolled back, it can prevent other operations from proceeding.
Multiple threads: If you're using SQLite in a multi-threaded environment, ensure that your database connections are properly managed and that you're using the correct threading mode.
Solutions
Here are some strategies to address the "database is locked" error:

Retry Logic: Implement a retry mechanism with a delay. This allows your application to attempt the operation again after a short pause.
cpp

Copy
const int maxRetries = 5;
for (int attempt = 0; attempt < maxRetries; ++attempt) {
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        break; // Success
    }
    std::cerr << "Error: " << sqlite3_errmsg(db) << ", retrying..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait before retrying
}
Transaction Management: Ensure that transactions are short-lived. Commit or rollback as soon as possible.
Connection Management: If you're using multiple threads, consider using a single connection per thread or use a connection pool.
Database Configuration: If appropriate, you can configure SQLite to use a different locking mode or increase the timeout:
cpp

Copy
sqlite3_busy_timeout(db, 5000); // Set a timeout of 5000 milliseconds (5 seconds)
Close Unused Connections: Make sure to close any database connections that are no longer needed to free up locks.
Debugging
Check Active Connections: Use a tool to check if other connections are holding locks on the database.
Verbose Logging: Enable verbose logging in SQLite to get more insight into when and why locks are being held.
By implementing these strategies, you should be able to mitigate or avoid the "database is locked" error. If you need further assistance or specific examples, let me know!
*/

// Global variables
sqlite3 *db;
sqlite3_stmt *stmt;
const char* DATABASE_NAME = "todolist.db";
const int FOLDER_TABLE_DB_ID = 1;
const int TODO_TABLE_DB_ID = 2;
/*
* Variable:
A variable is a named storage location in memory that can hold a value.
It acts as a reference to that location, allowing you to access and manipulate the stored value.
* Value:
A value is the actual data that is stored in the variable. It can be of various types, such as integers, floats, characters, strings, etc.
Values can change (except for constants) and represent the current state or data in your program.
*/

/*
The sqlite3_busy_timeout function in SQLite sets a timeout for how long the database connection will wait for a lock to clear before returning a "busy" error. Here's how it works regarding your question:

Behavior of sqlite3_busy_timeout
Connection-Specific: The timeout is set on a specific database connection. If you call sqlite3_busy_timeout(db, 5000);, it applies only to that particular db connection.
Not Persistent Across Sessions: If you close the database connection using sqlite3_close(db);, the timeout setting is lost. When you open a new connection to the database, you will need to call sqlite3_busy_timeout again to set the timeout for that new connection.
Example:
Here’s a simple example demonstrating this:
cpp

Copy
sqlite3* db;
if (sqlite3_open("example.db", &db) == SQLITE_OK) {
    // Set busy timeout for this connection
    sqlite3_busy_timeout(db, 5000);

    // Perform database operations...

    sqlite3_close(db); // Close the database connection
}

// Open a new connection
if (sqlite3_open("example.db", &db) == SQLITE_OK) {
    // You need to set the busy timeout again for the new connection
    sqlite3_busy_timeout(db, 5000);
    // Perform operations...
    sqlite3_close(db);
}
Summary
The timeout is specific to the connection and must be set each time you open a new database connection.
It does not carry over between closed and reopened connections.
*/

// Function's definition 
int db_onCreate(){
    char *errmsg = nullptr;
    
    // SQL statement to create a table
    const char *sql = "CREATE TABLE IF NOT EXISTS Created_folders ("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "Folder_Name TEXT NOT NULL);";
    
    const char *sql2 = "CREATE TABLE IF NOT EXISTS Todo_lists ("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "Date TEXT NOT NULL,"
					  "Todo_tittle TEXT NOT NULL,"
					  "Todo_discription TEXT NOT NULL,"
					  "Folder_name TEXT NOT NULL,"
					  "Status TEXT NOT NULL);";
					  
	/* 
	** Single INSERT statement for multiple values
    const char *sql3 = "INSERT INTO Created_folders (Folder_Name) VALUES ('Participation'), ('Training');";
    ** Single INSERT statement for a single value
	const char *sql1 = "INSERT INTO Created_folders VALUES('Participation');";
*/
					   			  
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK){
		std::cerr << "Can't open the database: " << sqlite3_errmsg(db);
		return 1;
	}else{
		sqlite3_busy_timeout(db, 500);
		if(sqlite3_exec(db, sql, 0, 0, &errmsg) != SQLITE_OK){
	        std::cerr << "SQL error: " << errmsg << std::endl;
            sqlite3_free(errmsg);
	    }
	
    	if(sqlite3_exec(db, sql2, 0, 0, &errmsg) != SQLITE_OK){
	        std::cerr << "SQL error: " << errmsg << std::endl;
            sqlite3_free(errmsg);
	    }
	
        sqlite3_close(db); // Close the database
	    return 0;  	
	}
}

// A function
std::vector<std::string> Retrieve_folders(){
	std::vector<std::string> folders;
	
	// open the database 
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK){
		std::cerr << sqlite3_errmsg(db);
		return folders;
	}else{
		sqlite3_busy_timeout(db, 500); // set a timeout inorder to prevent a dead lock while in operation
		
		const char* sql = "SELECT * FROM Created_folders";
        if(sqlite3_prepare(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db);
        }else{
        	// Execute the statement and retrieve data row by column
            while (sqlite3_step(stmt) == SQLITE_ROW) {
    	        // Retrieve the Name column as text
                const unsigned char* frname = sqlite3_column_text(stmt, 1); // second column
                int id = sqlite3_column_int(stmt, 0); // first column
                
                /*
                ** Checking for NULL:
		        The condition if (frname) evaluates to true if frname is a valid pointer (i.e., it points to some data).
		        If frname is NULL, the condition evaluates to false, and the code inside(if statement) will not execute. 
		        This is crucial for preventing dereferencing a null pointer, which would lead to undefined behavior (typically a crash).
                */
                folders.push_back(std::to_string(id));
		        if (frname) {
                    // Convert to constant char pointer and add to the vector
                    folders.push_back(reinterpret_cast<const char*>(frname)); // Implicitly converts cstr to std::string
                }
            }
		}
		sqlite3_finalize(stmt);// Finalize the statement
        sqlite3_close(db);// Close the database
	    return folders;
	}
}

// A function
void create_folder_(std::string folder_name){
	/*
	Cannot directly assign a std::string object to a const char* pointer using the address-of operator (&). 
	Instead, you should use the c_str() member function of the std::string class, 
	which returns a pointer to a null-terminated character array representing the string.
	Example:
	const std::string stmt = "INSERT INTO Created_folders (Folder_Name) VALUES ('" + folder_name + "')";
	const char *sql = stmt.c_str();
	*/
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK){
		std::cerr << "Can't open the database: " << sqlite3_errmsg(db);
		return;
	}else{
		
		/*
		** A busy timeout using sqlite3_busy_timeout(db, 1000);
		it means that SQLite will wait for up to 1000 milliseconds (1 second) for a lock to clear before returning a "database is locked" error. 
		*/
		sqlite3_busy_timeout(db, 500);
		const char* sql = "INSERT INTO Created_folders (Folder_Name) VALUES (?)";
	    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
		    std::cerr << "Error2: " << sqlite3_errmsg(db);
	    }else{
	    	
	        /*
	        ** sqlite3_bind_text(stmt, 1, frname, -1, SQLITE_STATIC)
	
        	* (1): 
	        This is the parameter index. 
	        SQLite uses 1-based indexing for parameters. 
	        So, 1 refers to the first parameter in the SQL statement.
	
        	* (-1):
	        This specifies the length of the string name. When set to -1, 
	        it tells SQLite to compute the length of the string using strlen(frname). 
	        If you know the length beforehand, you can specify it explicitly.
	
	        * (SQLITE_STATIC):
	        This is a destructor option. 
	        It tells SQLite that the memory pointed to by frname is managed by the application and 
	        will not change or be freed while SQLite is using it.
	        SQLITE_STATIC means that SQLite does not need to make a copy of the string;
	        it can use the pointer as is since the string's data will remain valid for 
	        the duration of the prepared statement's execution.
	        */
	        const char *frname = folder_name.c_str();
	        if(sqlite3_bind_text(stmt, 1, frname, -1, SQLITE_STATIC) != SQLITE_OK){
		        std::cerr << "Error3: " << sqlite3_errmsg(db);
	        }else{
	            /*
	            ** std::cerr
				Immediate Feedback: Since std::cerr is unbuffered, it provides immediate feedback, 
				which is crucial for debugging and error reporting. Redirection: You can redirect std::cerr to a file or another output stream if needed, just like std::cout.
		   		Separation of Concerns: Using std::cerr helps in separating error messages from regular output, 
				which can be especially useful in larger applications where you want to log errors separately.
	            
				Conclusion:
				In summary, std::cerr is a convenient way to handle error messages in C++. 
				It's unbuffered nature ensures that error messages are displayed immediately, making it a useful tool for debugging 
				and error reporting in your applications.
	            */
	            if(sqlite3_step(stmt) != SQLITE_DONE){
		            std::cerr << "Error4: " << sqlite3_errmsg(db);
			    }
	        }
        }
        sqlite3_finalize(stmt); // finalize the statement
	    sqlite3_close(db); // close the database
    }	
}

// A function
std::vector<std::vector<std::string>> in_folder_(std::string id){
	std::vector<std::vector<std::string>> infolder;
	auto postdelayed = [](short msec)->void{
		std::this_thread::sleep_for(std::chrono::milliseconds(msec));
	};
	/*
	* In C++, using return {}; 
	is a way to return an empty object of the type expected by the function. 
	This syntax can be particularly useful for returning empty containers like std::vector, std::list, etc.
	
	* return {}; 
	is not appropriate if the function return type is int.
	The syntax return {}; is typically used with types that can be initialized as empty, such as containers (std::vector, std::string, etc.).
	Use return {}; only for types that can be initialized as empty, like containers or classes with default constructors.
	*/
	
	// open the database
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK){
		std::cerr << sqlite3_errmsg(db);
		return infolder;
	}else{
		sqlite3_busy_timeout(db, 500);
		
		const char* sql = "SELECT * FROM Created_folders Where ID = (?)"; // string literal(is as a character array)
		if(sqlite3_prepare(db, sql, -1, &stmt, NULL) != SQLITE_OK){
			std::cerr << sqlite3_errmsg(db);
		}else{
			const char* param = id.c_str();
			if(sqlite3_bind_text(stmt, 1, param, -1, SQLITE_STATIC) != SQLITE_OK){
				std::cerr << sqlite3_errmsg(db);
			}else{
				if(sqlite3_step(stmt) == SQLITE_ROW){
					const unsigned char* fr_name = sqlite3_column_text(stmt, 1); // second column
					sqlite3_stmt* stmt_; 
					
					const char* query = "SELECT * FROM Todo_lists where Folder_name = (?)"; // string literal
				    if(sqlite3_prepare(db, query, -1, &stmt_, NULL) != SQLITE_OK){
					    std::cerr << sqlite3_errmsg(db);
				    }else{
						
					    /*
					    * * reinterpret_cast:
					    is one of C++'s type casting operators used for converting one pointer type to another. 
					    It allows for low-level reinterpreting of the bit pattern of an object, 
					    which can be useful in certain situations, particularly when dealing with raw memory or interfacing with hardware.
						
				    	* Key Features of reinterpret_cast
					    Low-Level Casting:
				 	    It can cast any pointer type to any other pointer type.
					    It does not perform any type checks, so it should be used with caution.
						
					   	Pointer Conversions:
					    Useful for casting between pointer types, like from void* to another pointer type,
					    or from const unsigned char* to const char*.
					 
					    Non-Portable:
					    Since it deals with raw memory, using reinterpret_cast can lead to non-portable code. 
					    The behavior might differ across different architectures or compilers.
					
				    	No Data Conversion:
				    	Unlike static_cast, reinterpret_cast does not convert the actual data or perform any checks on type safety.
				    	It merely reinterprets the bits.
						
				    	Since reinterpret_cast can lead to unsafe code, it should be used judiciously and only when you are certain about the types involved.
				    	Undefined Behavior: If you cast between incompatible types and then attempt to access the data, it may lead to undefined behavior.
					    Alignment Issues: Ensure that the resulting pointer type is correctly aligned for the type it points to.
					
					   	* reinterpret_cast is a powerful tool in C++ for low-level programming, particularly for pointer manipulation. 
					   	However, it should be used carefully to avoid pitfalls related to type safety, portability, and alignment.
					    */
					    const char* folder_name_ = reinterpret_cast<const char*>(fr_name);
					    if(sqlite3_bind_text(stmt_, 1, folder_name_, -1, SQLITE_STATIC) != SQLITE_OK){
						    std::cerr << sqlite3_errmsg(db);
					    }else{
						    while(sqlite3_step(stmt_) == SQLITE_ROW){
							    std::vector<std::string> row; // create a row list
						        // casting all the retrieved data to a string type
							    for(int j = 0; j < sqlite3_column_count(stmt_); j++){
								    if(sqlite3_column_type(stmt_, j) == SQLITE_INTEGER){ // integer column
									    row.push_back(std::to_string(sqlite3_column_int(stmt_, j)));
								    }else if(sqlite3_column_type(stmt_, j) == SQLITE_TEXT){ // text column
									    row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt_, j))); // Implicitly converts cstr to std::string
								    }else row.push_back("");                                         
							    }
							    infolder.push_back(row); // add vector row to the vecotr list infolder 
						    }
						    sqlite3_finalize(stmt_); // finalize the statement
						    sqlite3_finalize(stmt);
                            sqlite3_close(db); // close the database
						    return infolder;
					    }
				    }
				}			    
			}			
	    }
	    sqlite3_finalize(stmt); // finalize the statement
        sqlite3_close(db); // close the database
        return infolder;
   } 
}

// A function
std::vector<std::vector<std::string>> todo_lists(int sign) { 
	std::vector<std::vector<std::string>> todolst;
	/*
	** Error:
	std::vector<std::string> row(6, 0);
	todolst.push_back(row);
	*/
	
	// Open the database
    if (sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return {};
    } else {
    	sqlite3_busy_timeout(db, 500); // it means that SQLite will wait for up to 500 milliseconds (0.5 second) for a lock to clear
    	
    	/*
		* * String Literal:
		"SELECT * From Todo_lists Where Status = (?)" is a string literal.
		It is created at compile time and has a fixed size.
		The memory for string literals is usually allocated in a read-only section, meaning you cannot modify them.

        Data Type: In C and C++, string literals are treated as arrays of characters, 
		typically of type const char[].
		Memory: String literals are stored in a read-only section of memory. 
		Modifying a string literal results in undefined behavior.
		
		* * In summary, 
		string literals are used for text representation, while integer literals represent numerical values. 
		They differ in data type, memory storage, and how they are utilized in programming.
		*/ 
	    const char* sql = "SELECT * From Todo_lists Where Status = (?)"; // string literal(is as a character array)
        if(sqlite3_prepare(db, sql, -1, &stmt, 0) != SQLITE_OK) {
    	    std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
    	    sqlite3_close(db); // close the database
    	    return {};
	    } else {
	    	
	    	/*
			* * String literals (like "SELECT * From Todo_lists Where Status = (?)") are stored in a read-only section of memory.
			no need to (and cannot) delete or free memory allocated for string literals; 
			they remain in memory for the duration of the program.
			
			* const char* sql = "SELECT * From Todo_lists Where Status = (?)";
			If it points to a string literal, you don't need to manage its memory since it’s managed by the compiler.
			
			* char* sql = new char[10];
			If it points to dynamically allocated memory, you must explicitly free that memory when it is no longer needed.
			*/
			const char* argument;
			if(sign == 0) argument = "done";
			else argument = "undone";
	    	if(sqlite3_bind_text(stmt, 1, argument, -1, SQLITE_STATIC) != SQLITE_OK){
	    		std::cerr << "Error: " << sqlite3_errmsg(db);
	    		sqlite3_finalize(stmt); // finalize the statement
	    		sqlite3_close(db); // close the database
	    		return {};
			}else{
				
				/* 
				* * sqlite3_step(stmt) == SQLITE_ROW
			    This function is called to execute the prepared SQL statement referred to by stmt.
			    It progresses the statement to the next row of the result set.
			
			    * * The return value can be:
			    SQLITE_ROW: Indicates that the next row of the result is ready for processing.
			    SQLITE_DONE: Indicates that there are no more rows to process.
			
			    * * while(... == SQLITE_ROW):
			    The loop continues as long as the return value of sqlite3_step(stmt) is equal to SQLITE_ROW, 
			    meaning there are still rows available to process.
			
			    * * Purpose:
			    The purpose of this line is to iterate over all the rows returned by the executed SQL statement. 
			    Inside the loop, you would typically retrieve data from the current row using functions like sqlite3_column_*.
			    */
	            while(sqlite3_step(stmt) == SQLITE_ROW){
		            std::vector<std::string> row; // Create a new row
                    for (int j = 0; j < sqlite3_column_count(stmt); j++) {
                        // Convert each column to string
                        if (sqlite3_column_type(stmt, j) == SQLITE_INTEGER) {
                            row.push_back(std::to_string(sqlite3_column_int(stmt, j))); // Integer column
                        } else if (sqlite3_column_type(stmt, j) == SQLITE_TEXT) {
                            row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, j))); // Text column
                        } else {
                            row.push_back(""); // Handle other types if necessary
                        }
                    }
                    todolst.push_back(row); // Add the row to the list
     	        }
			}	
	    }
	    sqlite3_finalize(stmt); // finalize the statement
	    sqlite3_close(db); // close the database
	    return todolst;
	}
	
	return {};
}

// A function
int delete_folder_and_todo(std::string id, int which_table, std::string folder_name){
	// openning the database 
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK){
		std::cerr << "Unable to open the datbase: " << sqlite3_errmsg(db);
		return 1;
	}else{
		
		/*
		** DELETE FROM (?) where ID = (?)
		(?) the table name is the first parameter
		(?) the value of id is the second parameter
		
		* The table name cannot be replaced with a placeholder (?). 
		it needs to specify directly in the SQL statement.
		*/
		
		// setting a busy time out inorder to prevent like database botlnecks like dead lock 
		sqlite3_busy_timeout(db, 500); // it means that SQLite will wait for up to 500 milliseconds (0.5 second) for a lock to clear
		
		// from which table is the deletion wanted to be proceed
		const char* sql = nullptr;
		
		// ternary operator
		which_table == TODO_TABLE_DB_ID ? sql = "DELETE FROM Todo_lists where ID = (?)" : sql = "DELETE FROM Created_folders where ID = (?)"; // string literal(is as a character array) assigned to a constant character pointer
	
		// prepare the statement
		if(sqlite3_prepare(db, sql, -1, &stmt, NULL) != SQLITE_OK){
	    	std::cerr << "Unable to prepare the statement: " << sqlite3_errmsg(db);
	    	sqlite3_close(db); // close the database connection
	    	return 1;
	    }else{
	    	const char* param = id.c_str();
	    	if(sqlite3_bind_text(stmt, 1, param, -1, SQLITE_STATIC) != SQLITE_OK){
	    		std::cerr << "Unable to bind text: " << sqlite3_errmsg(db);
	    		sqlite3_finalize(stmt); // finalize the statement
				sqlite3_close(db); // close the database connection
	    		return 1;
			}else{
			    if(sqlite3_step(stmt) != SQLITE_DONE){
				    std::cerr << "Error while step: " << sqlite3_errmsg(db);
				    sqlite3_finalize(stmt); // finalize the statement
			        sqlite3_close(db); // close the database connection
				    return 1;
			    }else{
			    	if(which_table == FOLDER_TABLE_DB_ID) delete_todo_in_folder(folder_name);
			    	sqlite3_finalize(stmt); // finalize the statement
			   	    sqlite3_close(db); // close the database connection
				    return 0;
				} 
			}
		}
	}
}

//
void delete_todo_in_folder(std::string folder_name){
	// the database is already opened by the caller function 'delete_folder_and_todo()' the function that called this function 'delete_todo_in_folder()'
	const char* sql_deleting_todo = "DELETE FROM Todo_lists where Folder_name = (?)";
	if(sqlite3_prepare(db, sql_deleting_todo, -1, &stmt, NULL) != SQLITE_OK){
		std::cerr << "prepare2: " << sqlite3_errmsg(db);
	}else{
		const char* sql = folder_name.c_str();
		if(sqlite3_bind_text(stmt, 1, sql, -1, SQLITE_STATIC)){
	   		std::cerr << "Bind2: " << sqlite3_errmsg(db);
	    }else{
	   	    if(sqlite3_step(stmt) != SQLITE_DONE){
	   		    std::cerr << "Step2: " << sqlite3_errmsg(db);
		    }
        }
    }
    /*
    * These functions will just called from where this function is called
	sqlite3_finalize(stmt); // finalize the statement
	sqlite3_close(db); // close the database
	*/
    return;
}

//
void toinsert_todo_(std::vector<std::string> json_){
	// open the database
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK) std::cerr << sqlite3_errmsg(db);
	else{
		// setting a busy time out inorder to prevent like database botlnecks like dead lock 
		sqlite3_busy_timeout(db, 500); // it means that SQLite will wait for up to 500 milliseconds (0.5 second) for a lock to clear
		
		const char* sql = "INSERT INTO Todo_lists (Date, Todo_tittle, Todo_discription, Folder_name, Status) VALUES (?, ?, ?, ?, ?)";
		if(sqlite3_prepare(db, sql, -1, &stmt, NULL)) std::cerr << sqlite3_errmsg(db);
		else{
			bool chk = true;
			int j = 0;
			while(j < json_.size()){
				// const char* param = json_[j].c_str(); // the char* param is reset to an empty state on each iteration
				if(sqlite3_bind_text(stmt, j+1, json_[j].c_str(), -1, SQLITE_STATIC) != SQLITE_OK){
					std::cerr << sqlite3_errmsg(db);
					chk = false;
					break;
				} 
				j++;
			}
			
			if(chk){
				if(sqlite3_step(stmt) != SQLITE_DONE) std::cerr << sqlite3_errmsg(db);
				else {
					sqlite3_finalize(stmt); // finalize the statement
				    sqlite3_close(db); // close the database
				    return;
				}
			}
		}
		sqlite3_finalize(stmt); // finalize the statement
	    sqlite3_close(db); // close the database
	}
}

//
std::vector<std::string> fetch_specific_todo(std::string id){
	// open the database 
	if(sqlite3_open(DATABASE_NAME, &db) != SQLITE_OK) std::cerr << sqlite3_errmsg(db);
	else{
		// setting a busy time out inorder to prevent like database botlnecks like dead lock 
		sqlite3_busy_timeout(db, 5000); // it means that SQLite will wait for up to 500 milliseconds (0.5 second) for a lock to clear
		
		const char* sql = "SELECT * FROM Todo_lists where ID = (?)";
		if(sqlite3_prepare(db, sql, -1, &stmt, NULL) != SQLITE_OK) std::cerr << sqlite3_errmsg(db);
		else {
			const char* param = id.c_str();
			if(sqlite3_bind_text(stmt, 1, param, -1, SQLITE_STATIC) != SQLITE_OK) std::cerr << sqlite3_errmsg(db);
			else{
				if(sqlite3_step(stmt) != SQLITE_ROW) std::cerr << sqlite3_errmsg(db);
				else{
					std::vector<std::string> json_;
					
					/*
					const char* cstr = "Hello, world!";
                    std::string str = cstr; // Valid assignment
                    
                    * it is perfectly fine to assign a const char* to a std::string in C++. Just ensure that the pointer points to a valid null-terminated string.
					Memory Management: The std::string manages its own memory, so you no need to worry about freeing the memory allocated for the string.
					Null-Termination: The const char* should point to a null-terminated 'with out \0' string. If it does not, the behavior is undefined.
					Safety: Since const char* is a pointer to a constant character array[], it cannot be modified the characters it points to. However, the std::string created from it is mutable.
					*/
					
					for(int i = 0; i < 6; i++){
						if(sqlite3_column_type(stmt, i) == SQLITE_INTEGER){
							json_.push_back(std::to_string(sqlite3_column_int(stmt, i)));
						}else if(sqlite3_column_type(stmt, i) == SQLITE_TEXT){
							json_.push_back(reinterpret_cast<const char*> (sqlite3_column_text(stmt, i)));
						}
					}
					sqlite3_finalize(stmt); // finalize the statement
		            sqlite3_close(db); // close the database 
		            return json_;
				}
			}
		}
		sqlite3_finalize(stmt); // finalize the statement
		sqlite3_close(db); // close the database 
		return {};
	}
	return{};
}

//
void done_or_undone(std::string id, int status){	
	/*
	* To alter a row in a database, you typically use the SQL UPDATE statement.
	UPDATE table_name
	SET column1 = value1, column2 = value2, ...
	WHERE condition;
	
	* The ALTER statement in SQL is used to modify an existing database object, such as a table.
	Altering a Table Structure
	Add a Column
	To add a new column to an existing table:

    ALTER TABLE table_name
    ADD column_name column_type;
	*/
	
	// open the database 
	if(sqlite3_open(DATABASE_NAME, &db)) std::cout << sqlite3_errmsg(db);
	else{
		// setting a busy time out inorder to prevent like database botlnecks like dead lock 
		sqlite3_busy_timeout(db, 5000); 
		
		const char* sql = "UPDATE Todo_lists SET Status = (?) WHERE ID = (?);";
		if(sqlite3_prepare(db, sql, -1, &stmt, NULL)) std::cout << sqlite3_errmsg(db);
		else{
			const char* param; // 'it is c type of like c++ string', also it is as a character array, *param (resulting u), param (resulting undone)
			status == 0 ?  param = "done" : param = "undone";
			
			if(sqlite3_bind_text(stmt, 1, param, -1, SQLITE_STATIC)) std::cout << sqlite3_errmsg(db);
			else{
				if(sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC)) std::cout << sqlite3_errmsg(db);
				else{
					if(sqlite3_step(stmt) != SQLITE_DONE) std::cout << sqlite3_errmsg(db);
				}
			}
		}
		sqlite3_finalize(stmt); // finalize the statement
		sqlite3_close(db); // close the database
	}
}
	