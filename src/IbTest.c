#include <stdio.h>
#include <stdlib.h>

typedef char String[1000];/*make the size big to avoid errors*/

typedef struct  {
	int id;
	char *name;
} myClass;
/*
int main(void) {

	String message = "not enitialized message\n";

	myClass *myObject = (myClass *)malloc(sizeof(myClass));/*instentiate an object

	int idSize,nameSize;

	myObject->id= 123;

	myObject->name = "Adam";

	idSize = sizeof myObject->id;
	nameSize = sizeof myObject->name;

	sprintf(message,"\nObject Name: myObject\n"
		"Object Members:-\n"
		"id:   size = %d, value = %d\n"
		"name: size = %d, value = %s\n\n",
		idSize,
		myObject->id,
		nameSize,
		myObject->name);

	puts(message);

	return 0;
}
*/
/*Sample Output:-
Object Name: myObject
Object Members:-
id:   size = 4, value = 123
name: size = 4, value = Adam
*/
