/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <private-lib-core.h>

/*
 * It's either a buflist (.is_direct = 0) or
 * a direct pointer + len (.is_direct = 1)
 */


int
lws_system_get_info(struct lws_context *context, lws_system_item_t item,
		    lws_system_arg_t *arg)
{
	if (!context->system_ops || !context->system_ops->get_info)
		return 1;

	return context->system_ops->get_info(item, arg);
}

const lws_system_ops_t *
lws_system_get_ops(struct lws_context *context)
{
	return context->system_ops;
}


void
lws_system_blob_direct_set(lws_system_blob_t *b, const uint8_t *ptr, size_t len)
{
	b->is_direct = 1;
	b->u.direct.ptr = ptr;
	b->u.direct.len = len;
}

void
lws_system_blob_heap_empty(lws_system_blob_t *b)
{
	b->is_direct = 0;
	lws_buflist_destroy_all_segments(&b->u.bl);
}

int
lws_system_blob_heap_append(lws_system_blob_t *b, const uint8_t *buf, size_t len)
{
	assert(!b->is_direct);

	if (lws_buflist_append_segment(&b->u.bl, buf, len) < 0)
		return -1;

	return 0;
}

size_t
lws_system_blob_get_size(lws_system_blob_t *b)
{
	if (b->is_direct)
		return b->u.direct.len;

	return lws_buflist_total_len(&b->u.bl);
}

int
lws_system_blob_get(lws_system_blob_t *b, uint8_t *buf, size_t *len, size_t ofs)
{
	int n;

	if (b->is_direct) {

		assert(b->u.direct.ptr);

		if (ofs >= b->u.direct.len) {
			*len = 0;
			return 1;
		}

		if (*len > b->u.direct.len - ofs)
			*len = b->u.direct.len - ofs;

		memcpy(buf, b->u.direct.ptr + ofs, *len);

		return 0;
	}

	n = lws_buflist_linear_copy(&b->u.bl, ofs, buf, *len);
	if (n < 0)
		return -2;

	*len = n;

	return 0;
}

int
lws_system_blob_get_single_ptr(lws_system_blob_t *b, const uint8_t **ptr)
{
	if (b->is_direct) {
		*ptr = b->u.direct.ptr;
		return 0;
	}

	if (b->u.bl->next)
		return -1;  /* multipart buflist, no single pointer to it all */

	*ptr = (const uint8_t *)&b->u.bl[1];

	return 0;
}

void
lws_system_blob_destroy(lws_system_blob_t *b)
{
	if (!b)
		return;
	if (!b->is_direct)
		lws_buflist_destroy_all_segments(&b->u.bl);
}

lws_system_blob_t *
lws_system_get_blob(struct lws_context *context, lws_system_blob_item_t type,
		    int idx)
{
	if (idx < 0 ||
	    idx >= (int)LWS_ARRAY_SIZE(context->system_blobs[0]))
		return NULL;

	return &context->system_blobs[type][idx];
}

