/*
 * Copyright (c) 2011, Lloyd Hilaiel <lloyd@hilaiel.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "./src/api/yajl_tree.h"

static unsigned char fileData[65536];

int maina(void)
{
    size_t rd;
    yajl_val node;
    char errbuf[1024];

    /* null plug buffers */
    fileData[0] = errbuf[0] = 0;

    /* read the entire config file */
    rd = fread((void *) fileData, 1, sizeof(fileData) - 1, stdin);

    /* file read error handling */
    if (rd == 0 && !feof(stdin)) {
        fprintf(stderr, "error encountered on file read\n");
        return 1;
    } else if (rd >= sizeof(fileData) - 1) {
        fprintf(stderr, "config file too big\n");
        return 1;
    }

    /* we have the whole config file in memory.  let's parse it ... */
    node = yajl_tree_parse((const char *) fileData, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (node == NULL) {
        fprintf(stderr, "parse_error: ");
        if (strlen(errbuf)) fprintf(stderr, " %s", errbuf);
        else fprintf(stderr, "unknown error");
        fprintf(stderr, "\n");
        return 1;
    }

    /* ... and extract a nested value from the config file */
    {
	const char * path[] = { "wallet", (const char *) 0 };
        yajl_val v = yajl_tree_get(node, path, yajl_t_array);


	if(v) {
		yajl_val *objs = v->u.array.values;
		int i = 0;
		for(i; i < v->u.array.len; i++)
		{
			if(objs[i]) { /*A complete JSON object*/
				char **keys = objs[i]->u.object.keys; /*keys for JSON obj*/
				yajl_val *vals = objs[i]->u.object.values; /*values for JSON obj*/

				int j=0;
				for (j; j< objs[i]->u.object.len; j++) {
					printf("Object key : %s\n", keys[j]);
					if(YAJL_IS_INTEGER(vals[j])) {
						printf("Object value : %lli\n", YAJL_GET_INTEGER(vals[j]));
					}else if(YAJL_IS_DOUBLE(vals[j])) {
						printf("Object value : %lf\n", YAJL_GET_DOUBLE(vals[j]));
					}
					else {
						printf("Object value : %s\n", YAJL_GET_STRING(vals[j]));
					}
				}		
				
			}
		}
	}       
    }

    yajl_tree_free(node);

    return 0;
}
