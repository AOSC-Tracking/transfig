/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1985 Supoj Sutantavibul
 * Copyright (c) 1991 Micah Beck
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * The X Consortium, and any party obtaining a copy of these files from
 * the X Consortium, directly or indirectly, is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.  This license includes without
 * limitation a license to do the foregoing actions under any patents of
 * the party supplying this software to the X Consortium.
 */

#ifdef NFSS
static char		*texfontfamily[] = {
			"\\familydefault",	/* default */
			"\\rmdefault",		/* roman */
			"\\rmdefault",		/* bold */
			"\\rmdefault",		/* italic */
			"\\sfdefault",		/* sans serif */
			"\\ttdefault"		/* typewriter */
		};

static char		*texfontseries[] = {
			"\\mddefault", 		/* default */
			"\\mddefault",		/* roman */
			"\\bfdefault",		/* bold */
			"\\mddefault",		/* italic */
			"\\mddefault", 		/* sans serif */
			"\\mddefault"		/* typewriter */
		};

static char		*texfontshape[] = {
			"\\updefault",		/* default */
			"\\updefault",		/* roman */
			"\\updefault",		/* bold */
			"\\itdefault",		/* italic */
			"\\updefault",		/* sans serif */
			"\\updefault"		/* typewriter */
		};
#endif

static char		*texfontnames[] = {
  			"rm",			/* default */
			"rm",			/* roman */
			"bf",			/* bold */
			"it",			/* italic */
			"sf", 			/* sans serif */
			"tt"			/* typewriter */
		};

/* The selection of font names may be site dependent.
 * Not all fonts are preloaded at all sizes.
 */

static char		texfontsizes[] = {
                       11,            /* default */
                       5, 5, 5, 5,    /* 1-4: small fonts */
                       5,			/* five point font */
                       6, 7, 8,	/* etc */
                       9, 10, 11,
                       12, 12, 14,
                       14, 14, 17,
                       17, 17, 20,
                       20, 20, 20, 20, 25,
                       25, 25, 25, 29,
                       29, 29, 29, 29,
                       34, 34, 34, 34,
                       34, 34, 34, 41,
                       41, 41
  			};

#define MAXFONTSIZE 	42

#ifdef NFSS
#define TEXFAMILY(F)	(texfontfamily[((F) <= MAX_FONT) ? (F) : (MAX_FONT-1)])
#define TEXSERIES(F)	(texfontseries[((F) <= MAX_FONT) ? (F) : (MAX_FONT-1)])
#define TEXSHAPE(F)	(texfontshape[((F) <= MAX_FONT) ? (F) : (MAX_FONT-1)])
#endif
#define TEXFONT(F)	(texfontnames[((F) <= MAX_FONT) ? (F) : (MAX_FONT-1)])


#define TEXFONTSIZE(S)	(texfontsizes[((S) <= MAXFONTSIZE) ? round(S)\
				      				: (MAXFONTSIZE-1)])
#define TEXFONTMAG(T)	TEXFONTSIZE(T->size*(rigid_text(T) ? 1.0 : mag))

