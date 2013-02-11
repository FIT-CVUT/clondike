/*
 * Copyright (C) 2006 by Latchesar Ionkov <lucho@ionkov.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * LATCHESAR IONKOV AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "npfs.h"

typedef struct Nperror Nperror;
struct Nperror {
	char*	ename;
	int	ecode;
};

static pthread_key_t error_key;
static pthread_once_t error_once = PTHREAD_ONCE_INIT;

static void
np_destroy_error(void *a)
{
	Nperror *err;

	err = a;
	free(err->ename);
	free(err);
}

static void
np_init_error_key()
{
	pthread_key_create(&error_key, np_destroy_error);
}

void
np_werror(char *ename, int ecode)
{
	Nperror *err;

	pthread_once(&error_once, np_init_error_key);
	err = pthread_getspecific(error_key);
	if (!err) {
		err = malloc(sizeof(*err));
		if (!err) {
			fprintf(stderr, "not enough memory\n");
			return;
		}

		err->ename = NULL;
		pthread_setspecific(error_key, err);
	}

	free(err->ename);
	if (ename)
		err->ename = strdup(ename);
	else
		err->ename = NULL;

	err->ecode = ecode;
}

void
np_rerror(char **ename, int *ecode)
{
	Nperror *err;

	pthread_once(&error_once, np_init_error_key);
	err = pthread_getspecific(error_key);
	if (err) {
		*ename = err->ename;
		*ecode = err->ecode;
	} else {
		*ename = NULL;
		*ecode = 0;
	}
}

int
np_haserror()
{
	Nperror *err;

	pthread_once(&error_once, np_init_error_key);
	err = pthread_getspecific(error_key);
	if (err)
		return err->ename != NULL;
	else
		return 0;
}

void
np_uerror(int ecode)
{
	char buf[256];

	strerror_r(ecode, buf, sizeof(buf));
	np_werror(buf, ecode);
}

