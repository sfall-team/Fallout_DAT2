#include "stdafx.h"

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2	/* encode string into position and length if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

//unsigned long int textsize = 0;		/* text size counter */
//unsigned long int codesize = 0;		/* code size counter */

BYTE text_buf[N + F - 1];	/* Ring buffer of size N, with extra F-1 bytes to facilitate string comparison */

int match_position;		/* Longest match.  These are set by the InsertNode() procedure. */
int match_length;

int lson[N + 1];		/* left children */
int rson[N + 257];		/* right children */
int dad[N + 1];			/* parents -- These constitute binary search trees. */

void InitTree()
{
	int i;

	for(i = N + 1; i <= N + 256; i++) {
		rson[i] = NIL;
	}

	for(i = 0; i < N; i++) {
		dad[i] = NIL;
	}
}

void InsertNode(int r)
{
	int i;
	int p;
	int cmp;
	BYTE* key;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];

	rson[r] = lson[r] = NIL;
	match_length = 0;

	for( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) {
				p = rson[p];
			}
			else {
				rson[p] = r;
				dad[r] = p;
				return;  
			}
		}
		else {
			if (lson[p] != NIL) {
				p = lson[p];
			}
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}

		for (i = 1; i < F; i++) {
			if ((cmp = key[i] - text_buf[p + i]) != 0) {
				break;
			}
		}

		if (i > match_length) {
			match_position = p;

			if ((match_length = i) >= F)  {
				break;
			}
		}
	}

	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];

	dad[lson[p]] = r;
	dad[rson[p]] = r;

	if (rson[dad[p]] == p) {
		rson[dad[p]] = r;
	}
	else {
		lson[dad[p]] = r;
	}
	
	dad[p] = NIL;	/* remove p */
}

void DeleteNode(int p)
{
	int  q;

	if (dad[p] == NIL) {
		return;		/* not in tree */
	}

	if (rson[p] == NIL) {
		q = lson[p];
	}
	else if (lson[p] == NIL) {
		q = rson[p];
	}
	else {
		q = lson[p];

		if (rson[q] != NIL) {
			do {
				q = rson[q];
			} while (rson[q] != NIL);

			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}

		rson[q] = rson[p];
		dad[rson[p]] = q;
	}

	dad[q] = dad[p];

	if (rson[dad[p]] == p) {
		rson[dad[p]] = q;
	}
	else {
		lson[dad[p]] = q;
	}

	dad[p] = NIL;
}

#define GETBYTE() ((dwInputCurrent < dwInputLength) ? int(buffer[dwInputCurrent++]) & 0xFF : (-1))

void LZSSEncodeBuffer(BYTE* buffer, DWORD dwInputLength, CFile& fileOut)
{
	ASSERT(buffer);

	int i;
	int c;
	int len;
	int r;
	int s;
	int last_match_length;
	int code_buf_ptr;

	BYTE code_buf[17];
	BYTE mask;

	DWORD dwInputCurrent = 0;

	InitTree();

	code_buf[0] = 0;
	code_buf_ptr = mask = 1;
	s = 0;
	r = N - F;

	for(i = s; i < r; i++) {
		text_buf[i] = ' ';
	}

	for(len = 0; len < F && (c = GETBYTE()) != (-1); len++) {
		text_buf[r + len] = c;
	}

	if (len == 0) {
		return;
	}

	for(i = 1; i <= F; i++) {
		InsertNode(r - i);
	}

	InsertNode(r);

	do {
		if (match_length > len) {
			match_length = len;
		}

		if (match_length <= THRESHOLD) {
			match_length = 1;
			code_buf[0] |= mask;
			code_buf[code_buf_ptr++] = text_buf[r];
		}
		else {
			code_buf[code_buf_ptr++] = (BYTE) match_position;
			code_buf[code_buf_ptr++] = (BYTE)(((match_position >> 4) & 0xf0) | (match_length - (THRESHOLD + 1)));
		}

		if ((mask <<= 1) == 0) {
			for (i = 0; i < code_buf_ptr; i++) {
				fileOut.Write(code_buf + i, 1);
			}

			code_buf[0] = 0;
			code_buf_ptr = mask = 1;
		}

		last_match_length = match_length;

		for (i = 0; i < last_match_length && (c = GETBYTE()) != (-1); i++) {
			DeleteNode(s);
			text_buf[s] = c;

			if (s < F - 1) {
				text_buf[s + N] = c;
			}

			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}

		while (i++ < last_match_length) {
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);

			if (--len) {
				InsertNode(r);
			}
		}
	} while(len > 0);

	if (code_buf_ptr > 1) {
		for (i = 0; i < code_buf_ptr; i++) {
			fileOut.Write(code_buf + i, 1);
		}
	}
}

void LZSSDecodeBuffer(BYTE* buffer, DWORD dwInputLength, CArchive& arOut)
{
	ASSERT(buffer);

	int i;
	int j;
	int k;
	int r;
	int c;
	unsigned int flags;

	DWORD dwInputCurrent = 0;

	for (i = 0; i < N - F; i++) {
		text_buf[i] = ' ';
	}

	r = N - F;
	flags = 0;

	for ( ; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if ((c = GETBYTE()) == (-1)) {
				break;
			}

			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */

		if (flags & 1) {
			if ((c = GETBYTE()) == (-1)) {
				break;
			}

			arOut.Write(&c, 1);
			text_buf[r++] = c;
			r &= (N - 1);
		}
		else {
			if ((i = GETBYTE()) == (-1)) {
				break;
			}

			if ((j = GETBYTE()) == (-1)) {
				break;
			}

			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;

			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				arOut.Write(&c, 1);
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}
}
