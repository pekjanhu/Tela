package
global(DrawString,SetFont,font)
{

SetFont("7x14");

function SetFont(fontstr)
/* SetFont("fontspec") sets the font for subsequent
   DrawString functions. Currently, only two fonts are
   recognized: "7x14" and "10x20". */
{
	if (!streq(fontstr,"7x14") && !streq(fontstr,"10x20")) {
		format("SetFont: unknown font `` - unchanged\n",fontstr);
		return
	};
	load(sformat("font-``.hdf",fontstr));
};

function [d;g,b] = DrawString(s,x,y; col)
/* [d] = DrawString("string",x,y) draws specified string
   at (x,y) to integer matrix d. The matrix d represents a
   grayscale image with entries in 0..255. The x coordinate
   grows to the right and the y coordinate grows downward.

   [d] = DrawString("string",x,y,col) draws the string using
   specified grayscale (0..255), default black (=0).

   [r,g,b] = DrawString(..) draws to three matrices
   simultaneously, representing 24-bit color image. In this
   case the color may also be a 3-component vector. */
global(font)
{
	if (isdefined(g) && isdefined(b)) {
		if (!isarray(g) || !isarray(b) || !isint(g) || !isint(b)) {
			format("*** [r,g,b]=DrawString(...): r,g,b must be int arrays\n");
			return
		};
		if (isdefined(col)) {
			if (!isint(col)) {format("*** DrawString: color must be int\n"); return};
			if (length(col) == 3) {
				[d] = DrawString(s,x,y,col[1]);
				[g] = DrawString(s,x,y,col[2]);
				[b] = DrawString(s,x,y,col[3]);
			} else {
				[d] = DrawString(s,x,y,col);
				[g] = DrawString(s,x,y,col);
				[b] = DrawString(s,x,y,col);
			}
		} else {
			[d] = DrawString(s,x,y);
			[g] = DrawString(s,x,y);
			[b] = DrawString(s,x,y);
		}
	} else {
		if (!isarray(d) || !isint(d)) {format("*** [d]=DrawString(...): d must be int array\n"); return};
		if (!isstring(s)) {format("*** [d]=DrawString(s,...): s must be string\n"); return};
		fgcolor = 0;
		if (isdefined(col)) {
			if (!isint(col)) {format("*** DrawString: color must be int\n"); return};
			fgcolor = limit(col,0,255);
		};
		[ymax,xmax] = size(d);
		[c256,h1,w1] = size(font);
		for (i=1; i<=length(s); i++) {
			J = limit(y-h1:y-1,1,ymax); I = limit(x+(i-1)*w1:x+(i-1)*w1+w1-1,1,xmax);
			if (length(I)<=0 || length(J)<=0) continue;
			ch = s[i];
			bm = font[ch-15,:,:];
			d[J,I] = d[J,I]*(bm==0) + fgcolor*(bm!=0);
		};
	}
};

}
