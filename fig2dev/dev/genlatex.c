/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 by Micah Beck
 * Parts Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-1999 by Brian V. Smith
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 *
 */

/* 
 *	genlatex.c : LaTeX driver for fig2dev
 *
 *	Author: Frank Schmuck, Cornell University 6/88
 * 	Converted from fig2latex 5/89 by Micah Beck
 * 	Color, rotated text and ISO-chars added by Herbert Bauer 11/91
 *      Arc boxes added by C. Scott Ananian 6/99
 *
 */
#if defined(hpux) || defined(SYSV) || defined(SVR4)
#include <sys/types.h>
#endif
#include <sys/file.h>
#include "fig2dev.h"
#include "object.h"
#include "texfonts.h"

extern double rad2deg;
extern void unpsfont();

static put_box();
static put_arc_box();
static single_line();
static put_solidline();
static put_dashline();
static put_dotline();
static put_quarter();

#define rint(a) floor((a)+0.5)     /* close enough? */

/* 
 *  Installation dependent constants:
 *
 *  THINDOT	latex command for generating a dot if line width = \thinlines
 *  THICKDOT	latex command for generating a dot if line width = \thicklines
 *  MIN_LEN	shortest slanted line that latex can produce; shorter lines will
 *		we translated into a sequence of dots generated by \multiput.
 *  THICK_LDOT	latex command for generating the dot for making short slanted
 *		lines if line width = \thinlines
 *  THIN_LDOT	...  if line width = \thicklines
 */
#ifdef NFSS
#define THICKDOT	"\\SetFigFont{10}{12}{\\rmdefault}{\\mddefault}{\\updefault}."
#define THINDOT	"\\SetFigFont{7}{8.4}{\\rmdefault}{\\mddefault}{\\updefault}."
#else
#define THICKDOT	"\\SetFigFont{10}{12}{rm}."
#define THINDOT	"\\SetFigFont{7}{8.4}{rm}."
#endif
double	THIN_XOFF =	(0.1/72.0);
double	THIN_YOFF =	(0.7/72.0);
double	THICK_XOFF =	(0.4/72.0);
double	THICK_YOFF =	(0.6/72.0);
#ifdef NFSS
#define THICK_LDOT	"\\SetFigFont{7}{8.4}{\\rmdefault}{\\mddefault}{\\updefault}."
#define THIN_LDOT	"\\SetFigFont{5}{6}{\\rmdefault}{\\mddefault}{\\updefault}."
#else
#define THICK_LDOT	"\\SetFigFont{7}{8.4}{rm}."
#define THIN_LDOT	"\\SetFigFont{5}{6}{rm}."
#endif
double	THIN_LXOFF =	(0.1/72.0);
double	THIN_LYOFF =	(0.7/72.0);
double	THICK_LXOFF =	(0.4/72.0);
double	THICK_LYOFF =	(0.6/72.0);
#define MIN_LEN		(13.0/72.0/1200.0*80.0)	/* 13  points */

/*
 *  other constants and macros
 */
#define TOP		840
#define THINLINES	1
#define THICKLINES	2

#define MAXCIRCLEDIA	80
#define MAXCIRCLERAD	((MAXCIRCLEDIA-0.5)/(2*72.27))

#define	SWAP(x,y)	{tmp=x; x=y; y=tmp;}
#define TRANS(x,y)		(*translate_coordinates)(&x,&y)
#define TRANS2(x1,y1,x2,y2)	(*translate_coordinates)(&x1,&y1); \
				(*translate_coordinates)(&x2,&y2)
#define TRANSD(x,y)		(*translate_coordinates_d)(&x,&y)
#ifndef MIN
#define	MIN(x,y)	(((x) <= (y))? (x): (y))
#endif
#ifndef MAX
#define	MAX(x,y)	(((x) >= (y))? (x): (y))
#endif
#define	ABS(x)		(((x) >= 0)? (x): -(x))
#define round4(x)	((round(10000.0*(x))/10000.0))
#define round6(x)	((round(1000000.0*(x))/1000000.0))

char		thindot [] = THINDOT;
char		thickdot[] = THICKDOT;
char		thin_ldot [] = THIN_LDOT;
char		thick_ldot[] = THICK_LDOT;

static	int	verbose = 0;
double		dash_mag = 1.0;
int		thick_width = 2;
double		tolerance = 2.0;
double		arc_tolerance = 1.0;
int		(*translate_coordinates)() = NULL;
int		(*translate_coordinates_d)() = NULL;
double		unitlength;
static int	cur_thickness = -1;
double		ldot_diameter = 1.0/72.0;
char		*dot_cmd = thindot;
char		*ldot_cmd = thin_ldot;
double		dot_xoffset;
double		dot_yoffset;
double		ldot_xoffset;
double		ldot_yoffset;

extern char *ISOtoTeX[];

static
translate1(xp, yp)
  int	*xp, *yp;
{
	*xp = *xp + 1;
	*yp = *yp + 1;
	}

static
translate2(xp, yp)
  int	*xp, *yp;
{
	*xp = *xp + 1;
	*yp = (double)(TOP - *yp -1);
	}

static
translate1_d(xp, yp)
  double	*xp, *yp;
{
	*xp = *xp + 1.0;
	*yp = *yp + 1.0;
	}

static
translate2_d(xp, yp)
  double	*xp, *yp;
{
	*xp = *xp + 1.0;
	*yp = (double)TOP - *yp -1.0;
	}

void
genlatex_option(opt, optarg)
  char opt, *optarg;
{
    int i;

    switch (opt) {
	case 'a':
	    fprintf(stderr, "warning: latex option -a obsolete");
	    break;

	case 'd':
	    dash_mag = atof(optarg);	/* set dash magnification */
	    break;


	case 'f':		/* set default text font */
	    for ( i = 1; i <= MAX_FONT; i++ )
		if ( !strcmp(optarg, texfontnames[i]) ) break;

	    if ( i > MAX_FONT)
			{
			  fprintf(stderr,
						 "warning: non-standard font name %s ignored\n", optarg);
			}
		 else
			{
			  texfontnames[0] = texfontnames[i];
#ifdef NFSS
			  texfontfamily[0] = texfontfamily[i];
			  texfontseries[0] = texfontseries[i];
			  texfontshape[0] = texfontshape[i];
#endif
			}
	    break;

	case 'l':		/* set thin/thick line threshold */
	    thick_width = atoi(optarg);
	    break;

	case 'v':
	    verbose = 1;		/* verbose mode */
	    break;

	case 's':
	case 'm':
	case 'L':
	    break;

	default:
	    put_msg(Err_badarg, opt, "latex");
	    exit(1);
	}
}

void
genlatex_start(objects)
  F_compound	*objects;
{
	int tmp;

	texfontsizes[0] = texfontsizes[1] = 
		TEXFONTSIZE(font_size?font_size:DEFAULT_FONT_SIZE);

 	unitlength = mag/objects->nwcorner.x;
	dash_mag /= unitlength*80.0;

	translate_coordinates = translate2;
	translate_coordinates_d = translate2_d;

	TRANS2(llx, lly, urx, ury);
	if (llx > urx) SWAP(llx, urx)
	if (lly > ury) SWAP(lly, ury)

	/* LaTeX start */

	/* print any whole-figure comments prefixed with "%" */
	if (objects->comments) {
	    fprintf(tfp,"%%\n");
	    print_comments("% ",objects->comments, "");
	    fprintf(tfp,"%%\n");
	}

	fprintf(tfp, "\\setlength{\\unitlength}{%lisp}%%\n",
				(long) (round(4736286.72*unitlength)));
	/* define the SetFigFont macro */
	define_setfigfont(tfp);
	fprintf(tfp, "\\begin{picture}(%d,%d)(%d,%d)\n",
	 				 urx-llx, ury-lly, llx, lly);
}

int
genlatex_end()
{
	/* LaTeX ending */
	fprintf(tfp, "\\end{picture}\n");

	/* all ok */
	return 0;
}

static
set_linewidth(w)
  int	w;
{
	int		latex_w;

	if (w == 0) return;
	/* latex only knows thin lines or thick lines */
	latex_w = (w >= thick_width*THICK_SCALE)? THICKLINES: THINLINES;
	if (latex_w != cur_thickness) {
	    cur_thickness = latex_w;
	    if (cur_thickness == THICKLINES) {
		fprintf(tfp, "\\thicklines\n");
		dot_cmd = thickdot;
		dot_xoffset = round4(THICK_XOFF/unitlength);
		dot_yoffset = round4(THICK_YOFF/unitlength);
		ldot_cmd = thick_ldot;
		ldot_xoffset = round4(THICK_LXOFF/unitlength);
		ldot_yoffset = round4(THICK_LYOFF/unitlength);
		}
	    else {
		fprintf(tfp, "\\thinlines\n");
		dot_cmd = thin_ldot;
		dot_xoffset = round4(THIN_XOFF/unitlength);
		dot_yoffset = round4(THIN_YOFF/unitlength);
		ldot_cmd = thin_ldot;
		ldot_xoffset = round4(THIN_LXOFF/unitlength);
		ldot_yoffset = round4(THIN_LYOFF/unitlength);
		}
	    }
	}

void
genlatex_line(l)
  F_line	*l;
{
	F_point		*p, *q;
	int		x, y, llx, lly, urx, ury, arrow;

	if (verbose) 
	    fprintf(tfp, "%%\n%% Fig POLYLINE object\n%%\n");

	/* print any comments prefixed with "%" */
	print_comments("% ",l->comments, "");

	set_linewidth(l->thickness);
	set_color(l->pen_color);

	p = l->points;
	q = p->next;

	if (q == NULL) { /* A single point line */
	    x = p->x; y = p->y;
	    TRANS(x, y);
	    fprintf(tfp, "\\put(%3d,%3d){\\makebox(%.4f,%.4f){%s}}\n",
	      x, y, dot_xoffset, dot_yoffset, dot_cmd);
	    reset_color(l->pen_color);
	    return;
	    }

	/* a box, perhaps with rounded corners */
	if (l->type == T_BOX || l->type == T_ARC_BOX) {
	    x = p->x; y = p->y;
	    TRANS(x, y);
	    llx = urx = x;
	    lly = ury = y;
	    while (q != NULL) {
		x = q->x; y = q->y;
		TRANS(x, y);
		if (x < llx) llx = x;
		if (y < lly) lly = y;
		if (x > urx) urx = x;
		if (y > ury) ury = y;
		q = q->next;
		}
	    if (l->type == T_ARC_BOX)
	      put_arc_box (llx, lly, urx, ury, l->radius, l->style, l->style_val);
	    else
	      put_box (llx, lly, urx, ury, l->style, l->style_val);
	    reset_color(l->pen_color);
	    return;
	    }

	while (q != NULL) {
	    arrow = 0;
	    if (l->for_arrow  &&  q->next == NULL)
		arrow = 1;
	    if (l->back_arrow  &&  p == l->points)
		arrow = (arrow)? 2: -1;
	    single_line(p->x, p->y, q->x, q->y, arrow, l->style, l->style_val);
	    p = q;
	    q = q->next;
	    }

	if (l->fill_style != UNFILLED)
		fprintf(stderr, "Line area fill not implemented\n");
	reset_color(l->pen_color);
	}

static
single_line (x1, y1, x2, y2, arrow, style, val)
  int	x1, y1, x2, y2, arrow, style;
  double	val;
{
	int    dx, dy, sx, sy;
	double l, m, deviation;

	TRANS2(x1, y1, x2, y2);
	dx = x2-x1;
	dy = y2-y1;
	/*** compute direction vector ***/
	get_slope(dx, dy, &sx, &sy, arrow);
	/*** compute line length in x-direction ***/
	if (sx == 0) {
	    l = (double)abs(dy);
	} else {
	    m = (double)abs(sy) / (double)abs(sx);
	    l = ((double)abs(dx) + m*(double)abs(dy)) / (1.0 + m*m);
	    deviation = fabs(l-abs(dx)) + fabs(m*l-abs(dy));
	    if (deviation > tolerance)
		fprintf(stderr,
		  "Not a LaTeX slope (%d, %d), deviation %.1f pixels\n",
		  dx, dy, deviation);
	}
	l = round4(l);
	/*** output letex command ***/
	switch (style) {
	    case SOLID_LINE:
		put_solidline(x1, y1, sx, sy, l, arrow, val);
		break;
	    case DASH_LINE:
		put_dashline(x1, y1, sx, sy, l, arrow, val);
		break;
	    case DOTTED_LINE:
		put_dotline(x1, y1, sx, sy, l, arrow, val);
		break;
	    }
	}


/*
 * draw arc box
 */
static put_arc_box (llx, lly, urx, ury, radius, style, val)
int	llx, lly, urx, ury, radius, style;
double	val;
{
        int     radius2= 2*radius;
	double  swidth = (double)(urx-llx-radius2);
	double  sheight= (double)(ury-lly-radius2);
	int (*put_line)();

	switch (style) {
	    case SOLID_LINE:
	        put_line = put_solidline;
		break;
	    case DASH_LINE:
		fprintf(stderr, "Dashed arc boxes approximated with solid arcs\n");
	        put_line = put_dashline;
		break;
	    case DOTTED_LINE:
		fprintf(stderr, "Dotted arc boxes approximated with solid arcs\n");
	        put_line = put_dotline;
		break;
	    }
	fprintf(tfp, 
		"\\put(%3d,%3d){\\oval(%3d,%3d)[bl]}\n"
		"\\put(%3d,%3d){\\oval(%3d,%3d)[tl]}\n"
		"\\put(%3d,%3d){\\oval(%3d,%3d)[br]}\n"
		"\\put(%3d,%3d){\\oval(%3d,%3d)[tr]}\n",
		llx+radius,lly+radius,radius2,radius2,
		llx+radius,ury-radius,radius2,radius2,
		urx-radius,lly+radius,radius2,radius2,
		urx-radius,ury-radius,radius2,radius2);
	put_line (llx+radius, lly, 1, 0, swidth, 0, val);
	put_line (llx+radius, ury, 1, 0, swidth, 0, val);
	put_line (llx, lly+radius, 0, 1, sheight, 0, val);
	put_line (urx, lly+radius, 0, 1, sheight, 0, val);
	return;
	}

/*
 * draw box
 */
static
put_box (llx, lly, urx, ury, style, val)
  int	llx, lly, urx, ury, style;
  double	val;
{
	int	dlen;

	switch (style) {
	    case SOLID_LINE:
		fprintf(tfp, "\\put(%3d,%3d){\\framebox(%d,%d){}}\n",
		  llx, lly, urx-llx, ury-lly);
		break;
	    case DASH_LINE:
		dlen = round(val*dash_mag);
		fprintf(tfp, "\\put(%3d,%3d){\\dashbox{%d}(%d,%d){}}\n",
		  llx, lly, dlen, urx-llx, ury-lly);
		break;
	    case DOTTED_LINE:
		put_dotline (llx, lly, 1, 0, (double)(urx-llx), 0, val);
		put_dotline (llx, ury, 1, 0, (double)(urx-llx), 0, val);
		put_dotline (llx, lly, 0, 1, (double)(ury-lly), 0, val);
		put_dotline (urx, lly, 0, 1, (double)(ury-lly), 0, val);
		break;
	    }
	return;
	}

/*
 * draw a solid line given latex slope
 */
static
put_solidline (x, y, sx, sy, l, arrow, val)
  int	x, y, sx, sy, arrow;
  double	l;
  double  val; /* unused */
{
	double	cosine;		/* cosine of line angle */
	double	dx, dy;
	int	x2, y2, n;

	if (sx) {
	    cosine = (double)abs(sx) / sqrt((double)(sx*sx)+(double)(sy*sy));
	    x2 = (sx >= 0)? x + round(l): x - round(l);
	    y2 = y + round( ((sx>=0)? l: -l) * (double)sy / (double)sx);
	    }
	else {
	    cosine = 1.0;
	    x2 = x;
	    y2 = (sy >= 0)? y + round(l): y - round(l);
	    }
	if (sx == 0  ||  sy == 0  ||  (l/cosine)*unitlength >= MIN_LEN) {
	    switch (arrow) {
	    case 0:  /* simple line */
		fprintf(tfp, "\\put(%3d,%3d){\\line(%2d,%2d)", x, y, sx,sy);
		break;
	    case 1:  /* forward arrow */
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d)", x, y, sx,sy);
		break;
	    case -1: /* backward arrow */
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d)", x2, y2, -sx,-sy);
		break;
	    case 2:  /* double arrow */
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){  0}}\n", x,y,-sx,-sy);
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d)", x, y, sx, sy);
		break;
		}
	    if (l == floor(l))
		fprintf(tfp, "{%3.0f}}\n", l);
	    else
		fprintf(tfp, "{%7.3f}}\n", l);
	    }
	else {
	    n = 2 * (l/cosine) / (ldot_diameter/unitlength);
	    fprintf(stderr, "Line too short; will do %d dots\n", n);
	    dx = l / (double)n;
	    if (sx < 0) dx = -dx;
	    dy = dx * (double)sy / (double)sx;
	    fprintf(tfp, 
	      "\\multiput(%3d,%3d)(%.5f,%.5f){%d}{\\makebox(%.4f,%.4f){%s}}\n",
	      x, y, dx, dy, n+1, ldot_xoffset, ldot_yoffset, ldot_cmd);
	    if (arrow == 1  ||  arrow == 2)  /* forward arrow */
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x2,y2, sx,sy);
	    if (arrow == -1  ||  arrow == 2) /* backward arrow */
		fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x,y, -sx,-sy);
	    }
	}

/*
 * draw a dashed line given latex slope
 */
static
put_dashline (x, y, sx, sy, l, arrow, val)
  int	x, y, sx, sy, arrow;
  double	l;
  double	val;
{
	double	cosine;		/* cosine of line angle */
	double	nd;		/* number of dashes and gaps fitting on line */
	int	n;		/* nd rounded to the nearest odd integer */
	double	dl;		/* actual x-length of each dash */
	double	dg;		/* actual x-length of each gap */
	double	dx, dy;		/* step between dashes */
	int	x2, y2;

	if (sx) {
	    cosine = (double)abs(sx) / sqrt((double)(sx*sx)+(double)(sy*sy));
	    x2 = (sx >= 0)? x + round(l): x - round(l);
	    y2 = y + round( ((sx>=0)? l: -l) * (double)sy / (double)sx );
	    }
	else {
	    cosine = 1.0;
	    x2 = x;
	    y2 = (sy >= 0)? y + round(l): y - round(l);
	    }
	/*** compute number of dashes, length of dashes and gaps ***/
	nd = l / (val*dash_mag*cosine);
	n = (int) (rint((nd + 1.0)/2.0)*2 - 1);
	dl = l / (double)n;
	if (sx  &&  sy  &&  (dl/cosine)*unitlength < MIN_LEN) {
	    fprintf(stderr, "Dash too small; using larger dash\n");
	    dl = MIN_LEN/unitlength * cosine;
	    nd = l / dl;
	    n = (int) (rint((nd + 1.0)/2.0)*2 - 1);
	    }
	if (2*dl >= l  ||  (sx  &&  sy  &&  (l/cosine)*unitlength < MIN_LEN)) {
	    fprintf(stderr, "Dashed line too short; drawing solid line\n");
	    put_solidline (x, y, sx, sy, l, arrow, (double) 0.0);
	    return;
	    }
	dg = (l - (n/2+1)*dl) / (double)(n/2);
	if (sx) {
	    dx = dl+dg;
	    if (sx < 0) dx = -dx;
	    dy = dx * (double)sy / (double)sx;
	    }
	else {
	    dx = 0.0;
	    dy = dl+dg;
	    if (sy < 0) dy = -dy;
	    }
	/*** draw dashed line ***/
	fprintf(tfp, "\\multiput(%3d,%3d)(%.5f,%.5f){%d}{\\line(%2d,%2d){%7.3f}}\n",
	    x, y, dx, dy, n/2+1, sx, sy, dl);
	/*** draw arrow heads ***/
	if (arrow == 1  ||  arrow == 2)
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x2, y2, sx, sy);
	if (arrow == -1  ||  arrow == 2)
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x, y, -sx, -sy);
	}

/*
 * draw a dotted line given latex slope
 */
static
put_dotline (x, y, sx, sy, l, arrow, val)
  int	x, y, sx, sy, arrow;
  double	l;
  double	val;
{
	double	cosine;		/* cosine of line angle */
	double	nd;		/* number of dots fitting on line */
	int	n;		/* nd rounded to the nearest integer */
	double	dx, dy;		/* step between dashes */
	int	x2, y2;


	cosine = (sx)? (double)abs(sx) / sqrt((double)(sx*sx)+(double)(sy*sy)): 1.0;
	/*** compute step width ***/
	nd = l / (3*val*cosine);
	n = rint(nd);
	if (n == 0)
		n = 1;		/* sanity check */
	if (sx) {
	    dx = l / (double)n;
	    if (sx < 0) dx = -dx;
	    dy = dx * (double)sy / (double)sx;
	    }
	else {
	    dx = 0.0;
	    dy = l / (double)n;
	    if (sy < 0) dy = -dy;
	    }
	/*** draw arrow heads ***/
	if (arrow == 1  ||  arrow == 2) {
	    /* forward arrow */
	    if (sx) {
		x2 = (sx >= 0)? x + round(l): x - round(l);
		y2 = y + round( ((sx>=0)? l: -l) * (double)sy / (double)sx );
		}
	    else {
		x2 = x;
		y2 = (sy >= 0)? y + round(l): y - round(l);
		}
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x2, y2, sx, sy);
	    n--;
	    }
	if (arrow == -1  ||  arrow == 2) {
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%2d,%2d){0}}\n", x, y, -sx, -sy);
	    x = round(x + dx);
	    y = round(y + dy);
	    n--;
	    }
	/*** draw dotted line ***/
	fprintf(tfp, "\\multiput(%3d,%3d)(%.5f,%.5f){%d}{\\makebox(%.4f,%.4f){%s}}\n",
	    x, y, dx, dy, n+1, dot_xoffset, dot_yoffset, dot_cmd);
	}

void
genlatex_spline(s)
  F_spline	*s;
{
	fprintf(stderr, "Can't generate spline; omitting object\n");
	}

void
genlatex_ellipse(e)
  F_ellipse	*e;
{
	int  x, y, d, dx, dy;

	if (verbose) 
	    fprintf(tfp, "%%\n%% Fig ELLIPSE\n%%\n");

	/* print any comments prefixed with "%" */
	print_comments("% ",e->comments, "");

	set_color(e->pen_color);

	set_linewidth(e->thickness);
	switch (e->style) {
	    case SOLID_LINE:
		break;
	    case DASH_LINE:
		fprintf(stderr, "Dashed circles and elipses not supported\n");
		break;
	    case DOTTED_LINE:
		fprintf(stderr, "Dotted circles and elipses not supported\n");
		break;
	    }

	x = e->center.x;
	y = e->center.y;
	TRANS(x, y);
	if ((e->type == T_CIRCLE_BY_RAD || e->type == T_CIRCLE_BY_DIA)
			&& e->radiuses.x*unitlength <= MAXCIRCLERAD) {

	    d = 2 * e->radiuses.x;
	    if (e->fill_style == BLACK_FILL)
	    	fprintf(tfp, "\\put(%3d,%3d){\\circle*{%d}}\n", x, y, d);
	    else {
	      	fprintf(tfp, "\\put(%3d,%3d){\\circle{%d}}\n", x, y, d);
		if (e->fill_style != UNFILLED)
			fprintf(stderr, "Circle area fill not implemented\n");
	    }

	} else {	    
	    dx = 2 * e->radiuses.x;
	    dy = 2 * e->radiuses.y;
	    fprintf(tfp, "\\put(%3d,%3d){\\oval(%d,%d)}\n", x, y, dx, dy);
	    if (e->fill_style != UNFILLED)
		fprintf(stderr, "Ellipse area fill not implemented\n");
	}
	reset_color(e->pen_color);

      }

void
genlatex_text(t)
  F_text	*t;
{
	int   	x, y;
	char	*tpos;
	unsigned char	*cp;

	if (verbose) 
	    fprintf(tfp, "%%\n%% Fig TEXT object\n%%\n");

	/* print any comments prefixed with "%" */
	print_comments("% ",t->comments, "");

	x = t->base_x;
	y = t->base_y;
	TRANS(x, y);

	switch (t->type) {

	    case T_LEFT_JUSTIFIED:
	    case DEFAULT:
	    	tpos = "[lb]";
		break;

	    case T_CENTER_JUSTIFIED:
	    	tpos = "[b]";
		break;

	    case T_RIGHT_JUSTIFIED:
	    	tpos = "[rb]";
		break;

	    default:
		fprintf(stderr, "Text incorrectly positioned\n");
	    	tpos = "[lb]";	/* make left in this case */
	    }

	/* smash is used to position text at baseline */
	unpsfont(t);

	fprintf(tfp, "\\put(%3d,%3d){", x, y);

#ifdef DVIPS
        if(t->angle)
          fprintf(tfp, "\\rotatebox{%.1f}{", t->angle*180/M_PI);
#endif

	fprintf(tfp, "\\makebox(0,0)%s{\\smash{", tpos);

        { int texsize;
          double baselineskip;

	  texsize = TEXFONTMAG(t);
	  baselineskip = (texsize * 1.2);

#ifdef NFSS
 	  fprintf(tfp, "\\SetFigFont{%d}{%.1f}{%s}{%s}{%s}",
				 texsize, baselineskip,
				 TEXFAMILY(t->font),TEXSERIES(t->font),TEXSHAPE(t->font));
#else
 	  fprintf(tfp, "\\SetFigFont{%d}{%.1f}{%s}",
		texsize, baselineskip, TEXFONT(t->font));
#endif
	}

	set_color(t->color);

	if (!special_text(t))

		/* this loop escapes characters "$&%#_{}" */
		/* and deleted characters "~^\" */
		for(cp = (unsigned char*)t->cstring; *cp; cp++) {
	      	    if (strchr("$&%#_{}", *cp)) (void)fputc('\\', tfp);
	      	    if (strchr("~^\\", *cp))
			fprintf(stderr,
				"Bad character in text object '%c'\n" ,*cp);
		    else
			(void)fputc(*cp, tfp);
	      	}
	else 
		for(cp = (unsigned char*)t->cstring; *cp; cp++) {
#ifdef I18N
		    extern Boolean support_i18n;
		    if (support_i18n && (t->font <= 2))
			fputc(*cp, tfp);
		    else
#endif
		    if (*cp >= 0xa0)
			 fprintf(tfp, "%s", ISOtoTeX[(int)*cp-0xa0]);
		else
		    fputc(*cp, tfp);
		}

	reset_color(t->color);

#ifdef DVIPS
        if(t->angle)
             fprintf(tfp, "}");
#endif
 	fprintf(tfp, "}}}\n");
	}

void
genlatex_arc(a)
  F_arc	*a;
/*
 *  Approximates an arc by a sequence of quarter ovals.
 *
 *  Example:
 *
 *	Arc with center at (0,0) and radius 10 from +45 degree to +225 degree
 *	(arc from p1 = (7.07, 7.07) to p2 = (-7.07, -7.07) counterclockwise).
 *	This arc is approximated by three quarter ovals, one for each quadrant
 *	through which the arc goes:
 *
 *	 1. quarter oval from p1 to the intersection of arc and y-axis,
 *	    i.e., from (7.07, 7.07) to (0, 10) in quadrant 0
 *
 *		\put(0, 7.07){\oval(14.14, 5.86)[tr]}
 *
 *	 2. quarter oval from intersection arc/y-axis to intersection arc/x-axis
 *	    i.e., from (0, 10) to (-10, 0) in quadrant 1
 *
 *		\put(0, 0){\oval(20,20)[tl]}
 *
 *	 3. quarter oval from p1 to the intersection of arc and y-axis,
 *	    i.e., from (-10, 0) to (-7.07, -7.07) in quadrant 2
 *
 *		\put(-7.07, 0){\oval(5.86, 14.14)[bl]}
 */
{
	F_pos		p1, p2, pq[4];
	double		cx, cy;
	double		v1x, v1y, v2x, v2y;
	double		r, angle1, angle2;
	int		q1, q2;
	int		p1_arrow, p2_arrow;
	static char	*ad1[4] = { " 0,-1", " 1, 0", " 0, 1", "-1, 0" };
	static char	*ad2[4] = { "-1, 0", " 0,-1", " 1, 0", " 0, 1" };

	if (verbose) 
	    fprintf(tfp, "%%\n%% Fig ARC object\n%%\n");

	/* print any comments prefixed with "%" */
	print_comments("% ",a->comments, "");

	set_linewidth(a->thickness);
	set_color(a->pen_color);
	switch (a->style) {
	    case SOLID_LINE:
		break;
	    case DASH_LINE:
		fprintf(stderr, "Dashed arcs not supported\n");
		break;
	    case DOTTED_LINE:
		fprintf(stderr, "Dotted arcs not supported\n");
		break;
	}
	if (a->direction == 1) {
	    p1 = a->point[0];
	    p2 = a->point[2];
	    p1_arrow = (a->back_arrow != NULL);
	    p2_arrow = (a->for_arrow != NULL);
	} else {
	    p1 = a->point[2];
	    p2 = a->point[0];
	    p1_arrow = (a->for_arrow != NULL);
	    p2_arrow = (a->back_arrow != NULL);
	}
	cx = a->center.x;
	cy = a->center.y;
	TRANS2(p1.x, p1.y, p2.x, p2.y);
	TRANSD(cx, cy);
	/*** compute vectors and angles from arc center to p1, p2 ***/
	v1x = (double)p1.x - cx;
	v1y = (double)p1.y - cy;
	v2x = (double)p2.x - cx;
	v2y = (double)p2.y - cy;
	angle1 = atan2(v1y, v1x) * rad2deg;
	angle2 = atan2(v2y, v2x) * rad2deg;
	if (angle1 < 0.0)
	    angle1 += 360.0; 
	if (angle2 < 0.0)
	    angle2 += 360.0; 
	/* compute arc radius */
	r = sqrt(v1x*v1x+v1y*v1y);
	/*** compute intersection of arc with x and y axis (origin at cx, cy) */
	pq[0].x = round(cx);
	pq[0].y = round(cy + r);
	pq[1].x = round(cx - r);
	pq[1].y = round(cy);
	pq[2].x = round(cx);
	pq[2].y = round(cy - r);
	pq[3].x = round(cx + r);
	pq[3].y = round(cy);
	/*** compute in which quadrants p1 and p2 are located ***/
	q1 = (int)(angle1/90.0);
	q2 = (int)(angle2/90.0);
	if (fabs(angle1 - 90.0*q1) > arc_tolerance 
	 || fabs(angle2 - 90.0*q2) > arc_tolerance)
	    fprintf(stderr, "Approximating arc by ovals\n");
	/*** Draw arc ***/
	if (a->type == T_OPEN_ARC && a->thickness != 0 && p1_arrow)
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%s){0}}\n", p1.x, p1.y, ad1[q1]);
	while (q1 != q2) {
	    put_quarter(p1, pq[q1], q1);
	    p1 = pq[q1];
	    q1 = (q1 + 1) % 4;
	    }
	put_quarter(p1, p2, q1);
	if (a->type == T_OPEN_ARC && a->thickness != 0 && p2_arrow)
	    fprintf(tfp, "\\put(%3d,%3d){\\vector(%s){0}}\n", p2.x, p2.y, ad2[q2]);

	if (a->fill_style != UNFILLED)
		fprintf(stderr, "Arc area fill not implemented\n");
	reset_color(a->pen_color);
	}

static
put_quarter(p1, p2, q)
  F_pos	p1, p2;
  int	q;
/*
 *  Draw quarter oval from p1 to p2 in quadrant q
 */
{
	char	*opt;
	int	px, py, dx, dy;

	dx = 2*ABS(p1.x - p2.x);
	dy = 2*ABS(p1.y - p2.y);
	if (dx == 0  &&  dy == 0)
	    return;
	switch (q) {
	    case 0:
		px = MIN(p1.x, p2.x);
		py = MIN(p1.y, p2.y);
		opt = "tr";
		break;
	    case 1:
		px = MAX(p1.x, p2.x);
		py = MIN(p1.y, p2.y);
		opt = "tl";
		break;
	    case 2:
		px = MAX(p1.x, p2.x);
		py = MAX(p1.y, p2.y);
		opt = "bl";
		break;
	    case 3:
		px = MIN(p1.x, p2.x);
		py = MAX(p1.y, p2.y);
		opt = "br";
		break;
	    }
	fprintf(tfp, "\\put(%3d,%3d){\\oval(%3d,%3d)[%s]}\n", px, py, dx, dy, opt);
	}

#define  MAXCOLORS 32

/* need this for communication between color routines. Sorry */
static int lastcolor=-1;

set_color(col)
int col;
{
   static char *colors[] = {
   "0,0,0",    /* black */
   "0,0,1",    /* blue */
   "0,1,0",    /* green */
   "0,1,1",    /* cyan */
   "1,0,0",    /* red */
   "1,0,1",    /* magenta */
   "1,1,0",    /* yellow */
   "1,1,1",    /* white */
   "1,.84,0",	/* gold */
   "0,0,.56",	/* dk blue */
   "0,0,.69",	/* md blue */
   "0,0,.82",	/* lt blue */
   "0,.56,0",	/* dk green */
   "0,.69,0",	/* md green */
   "0,.82,0",	/* lt green */
   "0,.56,.56",	/* dk cyan */
   "0,.69,.69",	/* md cyan */
   "0,.82,.82",	/* lt cyan */
   ".56,0,0",	/* dk red */
   ".69,0,0",	/* md red */
   ".82,0,0",	/* lt red */
   ".56,0,.56",	/* dk magenta */
   ".69,0,.69",	/* md magenta */
   ".82,0,.82",	/* lt magenta */
   ".5,.17,0",	/* dk brown */
   ".63,.25,0",	/* md brown1 */
   ".75,.38,0",	/* md brown1 */
   ".88,.44,0",	/* lt brown */
   "1,.5,.5",	/* dk pink */
   "1,.63,.63",	/* md pink1 */
   "1,.75,.75",	/* md pink2 */
   "1,.88,.88",	/* lt pink */
   };
   

#ifdef DVIPS
   if (col != -1) {
       /* we do not support nested colors, although LaTeX would */
       if (lastcolor == -1)
	   fprintf(tfp, "{");
       /* switch to (other) color */
	if (col < NUM_STD_COLS)
	    fprintf(tfp, "\\color[rgb]{%s}",colors[col]);
	else
	    fprintf(tfp, "\\color[rgb]{%.3f,%.3f,%.3f}",
				user_colors[col-NUM_STD_COLS].r/256.0,
				user_colors[col-NUM_STD_COLS].g/256.0,
				user_colors[col-NUM_STD_COLS].b/256.0);
       lastcolor = col;
   }
#endif
   return;
}

reset_color(col)
int col;
{
#ifdef DVIPS
    if (col != -1 && col < NUM_STD_COLS + MAX_USR_COLS) {
       /* end using the last color */
       fprintf(tfp, "}%%\n");
    }
    /* make sure the color indicator is reset */
    lastcolor = -1;
#endif
   return;
}


struct driver dev_latex = {
     	genlatex_option,
	genlatex_start,
	genlatex_arc,
	genlatex_ellipse,
	genlatex_line,
	genlatex_spline,
	genlatex_text,
	genlatex_end,
	EXCLUDE_TEXT
};
