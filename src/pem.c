﻿/*
 *  Copyright 2014-2022 The GmSSL Project. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the License); you may
 *  not use this file except in compliance with the License.
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmssl/pem.h>
#include <gmssl/error.h>


int pem_write(FILE* fp, const char* name, const uint8_t* data, size_t datalen)
{
	int ret = 0;
	BASE64_CTX ctx;
	uint8_t* b64 = NULL;
	int len;

	if (!datalen) {
		error_print();
		return -1;
	}

	if (!(b64 = malloc(datalen * 2))) {
		error_print();
		return -1;
	}

	base64_encode_init(&ctx);
	base64_encode_update(&ctx, data, (int)datalen, b64, &len);
	base64_encode_finish(&ctx, b64 + len, &len);

	ret += fprintf(fp, "-----BEGIN %s-----\n", name);
	ret += fprintf(fp, "%s", (char *)b64);
	ret += fprintf(fp, "-----END %s-----\n", name);
	//return ret;
	return 1;
}

int pem_read(FILE *fp, const char *name, uint8_t *data, size_t *datalen, size_t maxlen)
{
	char line[80];
	char begin_line[80];
	char end_line[80];
	int len;
	BASE64_CTX ctx;

	snprintf(begin_line, sizeof(begin_line), "-----BEGIN %s-----\n", name);
	snprintf(end_line, sizeof(end_line), "-----END %s-----\n", name);

	if (feof(fp)) {
		return 0;
	}

	if (!fgets(line, sizeof(line), fp)) {
		if (feof(fp))
			return 0;
		else {
			error_print();
			return -1;
		}
	}

	if (strcmp(line, begin_line) != 0) {
		// FIXME: 这里是不是应该容忍一些错误呢？

		fprintf(stderr, "%s %d: %s\n", __FILE__, __LINE__, line);
		fprintf(stderr, "%s %d: %s\n", __FILE__, __LINE__, begin_line);

		error_print();
		return -1;
	}

	*datalen = 0;

	base64_decode_init(&ctx);

	for (;;) {
		if (!fgets(line, sizeof(line), fp)) {
			if (feof(fp)){
			    //如果END xxx最后一行没有换行符，提前跳出循环
				if (strncmp(line, end_line, strlen(end_line)-1) == 0) {
					break;
				} else {
					error_print();
					return -1;
				}
			}
			else {
				error_print();
				return -1;
			}
		}
		if (strcmp(line, end_line) == 0) {
			break;
		}

		base64_decode_update(&ctx, (uint8_t *)line, (int)strlen(line), data, &len);
		data += len;
		*datalen += len;
	}

	base64_decode_finish(&ctx, data, &len);
	*datalen += len;
	return 1;
}
