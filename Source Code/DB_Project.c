#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <my_global.h>
#include <mysql.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>

#define QUERY_LEN 1024
/* struct to store variables */
typedef struct user_info {
	char username[20];
	char password[20];
	char re_password[20];
	int uid;
} User;

/* function to clear the screen */
/* Remark: This function works for UNIX only. */
void clear_screen(){
	const char* CLEAR_SCREEN_ANSI="\e[1:1H\e[2J";
	write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

/* function to terminate the application */
void terminate() {
    printf("Thank you for using our application!\n");
    printf("Bye!\n");
    exit(1);
}

/* function to show the sign up page */
void sign_up(MYSQL *conn) {
	char username[20], password[20], re_password[20];
	MYSQL_RES *result;
	// fgets(username, 20, stdin);
    MYSQL_ROW row;
    printf("Please enter your username (limit 20 characters):\n");
    scanf("%s", &username);
    printf("Please enter your password (limit 20 characters):\n");
    scanf("%s", &password);
    printf("Please re-enter your password (limit 20 characters):\n");
    scanf("%s", &re_password);
    /* check if the two password entered are consistent */
    while(1) {
    	if(strcmp(password, re_password) != 0) {
		printf("Password created failed: The two password you entered are inconsistent! Please reset your password.\n");
		printf("Please enter your password (limit 20 characters):\n");
    	scanf("%s", &password);
    	printf("Please re-enter your password (limit 20 characters):\n");
    	scanf("%s", &re_password);
    	} else {
    		break;
    	}
    }
    if(strcmp(password, re_password) != 0) {
		printf("Password created failed: the two password you entered are inconsistent! Please reset your password.\n");
		printf("Please enter your password (limit 20 characters):\n");
    	scanf("%s", &password);
    	printf("Please re-enter your password (limit 20 characters):\n");
    	scanf("%s", &re_password);
    }

    char query[QUERY_LEN] = "\0";
    time_t timer = time(NULL);
    
    int UID,i,num_fields ;
    sprintf(query,"SELECT count(*) FROM xuhui.user" );
    mysql_query(conn, query);
    result = mysql_store_result(conn);
    num_fields = mysql_num_fields(result);
    row = mysql_fetch_row(result);
    UID = atoi(row[0]);
    sprintf(query, "INSERT INTO xuhui.user (UID, Username, Password, RegisterDate) VALUES ('%d', '%s', '%s', '%s')", UID, username, password, ctime(&timer));
    mysql_query(conn, query);

    sprintf(query, "SELECT UID FROM xuhui.user WHERE Username = '%s'", username);
    mysql_query(conn, query);

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    
    clear_screen();
    printf("Your username is: %s\t and your user id is: %d\n.", username, UID);
    printf("Your user id is used for log in, please remember it, thank you for registration!");
    start(conn);
}

/* function to show the log in page */
void log_in(MYSQL *conn) {
	char username[20], password[20], input_password[20];
	MYSQL_RES *result;
	MYSQL_ROW row;
	char query[QUERY_LEN] = "\0";
	User user;
	while(1) {
	    printf("Enter username:\n");
	    scanf("%s", &username);
        printf("Enter your password:\n");
        scanf("%s",&input_password);
	    sprintf(query, "SELECT UID, Username, Password FROM xuhui.user WHERE Username = '%s'", username);
	    mysql_query(conn, query);
	    result = mysql_store_result(conn);
	    row = mysql_fetch_row(result);
	    if(row != NULL) {
	    	strcpy(password, row[2]);
	    	while(strcmp(password, input_password) != 0 ) {
	    		printf("Wrong password!\n");
                printf("Please re_enter your password (limit 20 characters):\n");
		    	scanf("%s", input_password);
            }
	   		user.uid = atoi(row[0]);
			strcpy(user.username, username);
			strcpy(user.password, password);
	   		break;
	   	} else {
	   		printf("Sorry! No such user!\n");
	   		start(conn);
	   	}
    }
    clear_screen();
	main_menu(conn, &user);
}

/* function to show the start page */
void start(MYSQL *conn) {
    int start_option;
    printf("Welcome!\n");
    printf("Please choose an option (just enter the number before the operation):\n");
    printf("1. Sign Up\n");
    printf("2. Log in\n");
    printf("3. Exit\n");
    printf("Please make a choice: \n");
    scanf("%d", &start_option);
    while(1) {
    	switch(start_option) {
    	case 1: 
    		// clear_screen(); 
    		sign_up(conn);
    		break;
    	case 2:
    		// clear_screen(); 
    		log_in(conn);
    		break;
    	case 3:	
    		terminate();
    		break;
    	default:
    		printf("This is not a valid option, please re-enter your choice!\n");
    		scanf("%d", start_option);
        }
    }   
}

/* function to show the view page */
void view(MYSQL *conn, User *user) {
    int op;
    MYSQL_RES *result;
    char query[QUERY_LEN] = "\0";
    sprintf(query, "SELECT TID, Title, TAuthor FROM xuhui.topic");
    mysql_query(conn, query);
    result = mysql_store_result(conn);
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    int i;
    //printf("%d\n", num_fields);
    printf("You can input \"0\" to BACKWARD at any time.\n");
    printf("TID\t\tTopic\t\tAuthor\n");
    while((row = mysql_fetch_row(result))) {
        for(i = 0; i < num_fields; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");
            //puts(row[i]);
            printf("\t");
        }
        printf("\n");
    }
	printf("Please make a choice: \n");
    scanf("%d", &op);
	User user2;
	strcpy(user2.username, user->username);
	strcpy(user2.password, user->password);
	user2.uid = user->uid;
    if(op != 0) {
        clear_screen();
        content(conn, op, &user2);
    } else {
        clear_screen();
        main_menu(conn, &user2);
    }
}

/* function to show the main menu page */
void main_menu(MYSQL *conn, User *user) {
    printf("1. View Posts\n" );
    printf("2. Post\n" );
    printf("3. Exit\n");
    int op;
    scanf("%d", &op);
    switch(op) {
        case 1:
            view(conn, user);
            break;
        case 2:
            post(conn, user);
            break;
        case 3:
            terminate();
            break;
        default:
            printf("Wrong input.\n");
    } 
}

/* function to show the add comment page */
void add_comment(MYSQL *conn, User *user, char *comment, int op) {
    MYSQL_ROW row;
    MYSQL_RES *result;
    char query[QUERY_LEN] = "\0";
    sprintf(query, "SELECT count(*) as times FROM xuhui.contains LEFT JOIN xuhui.Comment on contains.CID = Comment.CID where contains.TID = %d", op);
    mysql_query(conn, query);
    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    int times = atoi(row[0]);
    char cid[20] = "\0";
    sprintf(cid, "%d_%d", op, times+1);
    time_t timer = time(NULL);
    sprintf(query, "INSERT INTO xuhui.Comment VALUES ('%s', '%s', '%s', '%s')",cid, user->username, ctime(&timer), comment);
    mysql_query(conn, query);
    sprintf(query, "INSERT INTO xuhui.contains VALUES (%d, '%s')", op, cid);
    mysql_query(conn, query);
    sprintf(query, "INSERT INTO xuhui.writes VALUES(%d, '%s')", user->uid, cid);
	User user2;
	strcpy(user2.username, user->username);
	strcpy(user2.password, user->password);
	user2.uid = user->uid;
    clear_screen();
    content(conn, op, &user2);
}

/* function to show the content page */
void content(MYSQL *conn, int op, User *user) {
    char query[QUERY_LEN] = "\0";
    int num_fields;
    MYSQL_RES *result; MYSQL_ROW row;

    sprintf(query, "SELECT * FROM xuhui.topic where TID = %d", op);

    mysql_query(conn, query);
    result = mysql_store_result(conn);
    num_fields = mysql_num_fields(result);
    int i, index= 0, j;
    while((row = mysql_fetch_row(result))) {
        for(i = 0; i < num_fields; i++) {
            printf("%s", row[i]?row[i]:"NULL");
            printf("\t\t");
        }
    }
    printf("\n============================================================================\n");
	//problem
    sprintf(query, "SELECT Comment.CAuthor, Comment.CDate, Comment.CContent FROM xuhui.contains LEFT JOIN xuhui.Comment ON contains.CID = Comment.CID WHERE contains.TID = %d", op);
    MYSQL_RES *tmp_result;
    MYSQL_ROW tmp_row;
    mysql_query(conn, query);
    tmp_result = mysql_store_result(conn);
    num_fields = mysql_num_fields(tmp_result);
    while((tmp_row =  mysql_fetch_row(tmp_result))) {
    	for(j = 0 ; j < num_fields; j++) {
    		printf("%s", tmp_row[j] ? tmp_row[j] : "NULL");
            printf("\t");
    	}
        printf("\n------------------------------------------------------------------------\n");
    }
	User user2;
	strcpy(user2.username, user->username);
	strcpy(user2.password, user->password);
	user2.uid = user->uid;
    char comment[50] = "\0";
    // char *p = comment;
    printf("Input b to BACKWARD or just input your comment (limit 50 characters):\n");
    // scanf("%s", &comment);
	fgets(comment, 50, stdin);
    if(strcmp(comment, "b\n") == 0) {
        clear_screen();
        view(conn, &user2);
    } else {
       	add_comment(conn, &user2, comment, op);
    }
}

/* function to show the post page */
void post(MYSQL *conn, User *user) {
	char title[50]; 
	int number = 0; 
	MYSQL_RES *result; 
	MYSQL_ROW row;
	/* First fgets doesn't work for no reason */
	printf("Enter Topic Title (limit 50 characters):\n");
    fgets(title, 50, stdin);
    fgets(title, 50, stdin);
	char content[200];
    printf("\nEnter Topic Content (limit 200 characters):\n");
	fgets(content, 200, stdin);    
	// scanf("%s", &content);
    char query[QUERY_LEN]="\0";

    sprintf(query, "SELECT count(*) FROM xuhui.topic");
    mysql_query(conn, query);
    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    number = atoi(row[0]);
	time_t timer = time(NULL);	
   	sprintf(query, "INSERT INTO xuhui.topic VALUES ('%d', '%s', '%s', '%s', '%s')", number+1, title, user->username, ctime(&timer), content);
    mysql_query(conn, query);
    clear_screen();	
    view(conn, user);
}

/* function to print the error message and terminate the application */
void finish_with_error(MYSQL *conn) {
    printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
    mysql_close(conn);
    exit(1);
}


int main(int argc, char **argv) {
	MYSQL *conn = mysql_init(NULL);
    
    /* if mysql_init() fails, print error message */
    if (conn == NULL) {
        printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }
    
    /*
    mysql_real_connect() function is used to establish connection to DB-server;
    its parameters are connection-handler, hostname, username, password, db-name, port-number, socket, and client flag.
    */
    if (mysql_real_connect(conn, "db.cs.dal.ca", "xuhui", "B00687751", "xuhui", 0, NULL, 0) == NULL) {
        finish_with_error(conn);
    }
    start(conn);
    mysql_close(conn);
    exit(0);
}
