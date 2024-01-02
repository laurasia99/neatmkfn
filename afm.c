/* AFM fonts */
/* https://adobe-type-tools.github.io/font-tech-notes/pdfs/5004.AFM_Spec.pdf */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mkfn.h"

#define TOKLEN		256

static char *afm_charfield(char *s, char *d)
{
	while (*s && !isspace(*s) && *s != ';')
		*d++ = *s++;
	while (isspace(*s) || *s == ';')
		s++;
	*d = '\0';
	return s;
}

static int uwid(int w)
{
	long div = 72000 / mkfn_res;
	return (w < 0 ? w - div / 20 : w + div / 20) * (long) 10 / div;
}

int afm_read(void)
{
	char ln[1024];
	char ch[TOKLEN] = "", pos[TOKLEN] = "";
	char c1[TOKLEN] = "", c2[TOKLEN] = "";
	char wid[TOKLEN] = "", field[TOKLEN] = "";
	char llx[TOKLEN] = "0", lly[TOKLEN] = "0";
	char urx[TOKLEN] = "0", ury[TOKLEN] = "0";
	char fontname[128];
	char *s;
	while (fgets(ln, sizeof(ln), stdin)) {
		if (ln[0] == '#')
			continue;
		if (!strncmp("FontName ", ln, 8)) {
			sscanf(ln, "FontName %s", fontname);
			continue;
		}
		if (!strncmp("StartCharMetrics", ln, 16))
			break;
	}
	while (fgets(ln, sizeof(ln), stdin)) {
		if (ln[0] == '#')
			continue;
		if (!strncmp("EndCharMetrics", ln, 14))
			break;
		s = ln;
		while (*s) {
			/* 5004.AFM Ch 8 'Individual Character Metrics' */
			s = afm_charfield(s, field);
			if (!strcmp("C", field)) {		/* Default char code; decimal; -1 not encoded */
				s = afm_charfield(s, pos);
				continue;
			}
			if (!strcmp("WX", field)) {		/* Character width */
				s = afm_charfield(s, wid);
				continue;
			}
			if (!strcmp("N", field)) {		/* Postscript char name (optional in spec) */
				s = afm_charfield(s, ch);
				continue;
			}
			if (!strcmp("B", field)) {		/* Bounding box llx lly urx ury (optional) */
				s = afm_charfield(s, llx);
				s = afm_charfield(s, lly);
				s = afm_charfield(s, urx);
				s = afm_charfield(s, ury);
				continue;
			}
			if (!strcmp("L", field)) {		/* L successor ligature (optional) */
				s = afm_charfield(s, c1);
				s = afm_charfield(s, c2);
				continue;
			}
			break;
		}
		/* Fields N C WX */
		if (ch[0] && pos[0] && wid[0])
			mkfn_char(ch, atoi(pos), 0, uwid(atoi(wid)),
				uwid(atoi(llx)), uwid(atoi(lly)),
				uwid(atoi(urx)), uwid(atoi(ury)));
	}
	mkfn_header(fontname);
	while (fgets(ln, sizeof(ln), stdin)) {
		if (ln[0] == '#')
			continue;
		if (!strncmp("StartKernPairs", ln, 14))
			break;
	}
	while (fgets(ln, sizeof(ln), stdin)) {
		if (ln[0] == '#')
			continue;
		if (!strncmp("EndKernPairs", ln, 12))
			break;
		if (sscanf(ln, "KPX %s %s %s", c1, c2, wid) == 3)
			mkfn_kern(c1, c2, uwid(atoi(wid)));
	}
	return 0;
}
