#!/usr/bin/env python
#
#  structure_maker.py
#  
#  Created by: Attila - Peter Toth
#  Created on: 16/10/2012
#	
#  Copyright (c) 2012 Bitcoin Project Team
#
#  Based on Matthew Mitchell's code of cbitcoin
#  


#  This file makes a structure inheriting Object. May be improved to make structures inheriting a selected structure.

import os,sys
from datetime import datetime

name = raw_input("Enter the name of the new structure (Use CamelCase): ")
author = raw_input("Enter your first and last name: ")
desc = raw_input("Enter a description for the structure: ")
d = datetime.utcnow()
# Make directory

import sys, os
        
pathname = os.path.dirname(sys.argv[0])        
fullpath =  os.path.abspath(pathname)

dir = fullpath + "/../src/Object/" + name
if os.path.exists(dir):
	input = raw_input("This name conflicts with a previous name. Overwrite? (Y/N): ")
	if input != "Y":
		sys.exit()
else:
	os.makedirs(dir)
# Make files
header = open(dir + "/" + name + ".h","w")
source = open(dir + "/" + name + ".c","w")
# Header
header.write("/*\n\
*  "+name+".h\n\
*\n\
*  Created on: "+d.strftime("%d/%m/%Y")+".\n\
*  Created by: "+author+".\n\
*  Modified by: \n\
*  Copyright (c) 2012 Bitcoin Project Team\n\
*/\n\
/**\n\
 @file\n\
 @brief "+desc+" Inherits Object\n\
*/\n\
\n\
#ifndef "+name.upper()+"_H_\n\
#define "+name.upper()+"_H_\n\
\n\
\n\
#include \"../Object.h\"\n\
#include \"../../Constants.h\"\n\
\n\
/**\n\
 @brief Structure for "+name+" objects. @see "+name+".h\n\
*/\n\
typedef struct{\n\
\tObject base; /**< Object base structure */\n\
} "+name+";\n\
\n\
/**\n\
 @brief Creates a new "+name+" object.\n\
 @returns A new "+name+" object.\n\
 */\n\
"+name+" * new"+name+"(void);\n\
\n\
/**\n\
 @brief Initializes a "+name+" object\n\
 @param self The "+name+" object to initialize\n\
 @returns TRUE on success, FALSE on failure.\n\
 */\n\
int init"+name+"("+name+" * self);\n\
\n\
/**\n\
 @brief Gets a "+name+" from an object. Use this to avoid casts.\n\
 @param self The object to obtain the "+name+" from.\n\
 @returns The "+name+" object.\n\
 */\n\
"+name+" * get"+name+"(void * self);\n\
\n\
/**\n\
 @brief Destroys an "+name+" object.\n\
 @param self The "+name+" object to free.\n\
 */\n\
void destroy"+name+"(void * self);\n\
 \n\
#endif /* "+name.upper()+"_H_ */");


# Source
source.write("/*\n\
*  "+name+".h\n\
*\n\
*  Created on: "+d.strftime("%d/%m/%Y")+".\n\
*  Created by: "+author+".\n\
*  Modified by: \n\
*  Copyright (c) 2012 Bitcoin Project Team\n\
*/\n\
\n\
/*\n\
*  SEE HEADER FILE FOR DOCUMENTATION\n\
*/\n\
\n\
#include \""+name+".h\"\n\
\n\
/*\n\
* Constructor\n\
*/\n\
\n\
"+name+" * new"+name+"(){\n\
\t"+name+" * self = malloc(sizeof(*self));\n\
\tif (NULL == self)\n\
\t\treturn NULL;\n\
\tgetObject(self)->destroy = destroy"+name+";\n\
\tif(init"+name+"(self))\n\
\t\treturn self;\n\
\tfree(self);\n\
\treturn NULL;\n\
}\n\
\n\
/*\n\
* Object Getter\n\
*/\n\
\n\
"+name+" * get"+name+"(void * self){\n\
\treturn self;\n\
}\n\
\n\
/*\n\
*  Initialiser\n\
*/\n\
\n\
int init"+name+"("+name+" * self){\n\
\tif(getObject(self) == NULL){\n\
\t\treturn FALSE;\n\
\t}\n\
\tgetObject(self)->referenceCount = 1;\n\
\treturn TRUE;\n\
}\n\
\n\
/*\n\
*  Destructor\n\
*/\n\
\n\
void destroy"+name+"(void * self){\n\
\tdestroyObject(self);\n\
}\n\
\n\
/*\n\
* Functions\n\
*/\n\
\n")
# Close files
header.close()
source.close()
