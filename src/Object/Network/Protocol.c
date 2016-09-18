/*
*  Protocol.c
*
*  Created on: 25/10/2012.
*  Created by: Prajowal Manandhar
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/

/*
*  SEE HEADER FILE FOR DOCUMENTATION
*/

#include "Protocol.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
* Constructor
*/

unsigned int getPayloadLength(struct CmdHeaderCheckSum header)
{
	return header.length;
}

struct CmdVersion parseVersion(const char *payload)
{
	struct CmdVersion ver;
	memcpy(&ver, payload, sizeof(ver));
	return ver;
}


