// xintf.t - define functions xwidget and xpanel
// PJ 5.11.1994

package global(xwidget,xpanel)
{

Tboolean = "boolean";
Tinteger = "integer";
Tstring = "string";
Tnotype = "notype";

function y = xwidget(column,kind,label...)
/* xwidget(n,"kind","label"[,"option","value"]) creates an X widget
   for function xpanel. The "kind" argument may be one of "panel", "button",
   "menu", "smenu", "text", "consttext", "exitpanel", "exitbutton",
   "cancelpanel", "cancelbutton".
   The "label" argument is the text string that appears on the widget,
   however, in case of menu and smenu it is the list of menu title and menu
   items separated by vertical bar. Options are passed directly to the X
   window system without checking their validity. Safe options names are
   at least "background", "foreground", "width" and "font".

   xwidget returns an object that contains the argument info packed in
   a certain way. You should not modify the objects returned by xwidget
   but to pass them directly to xpanel.

   See also: xpanel. */
global
{
	y = 0;
	if (isint(column)) {
		if (column<=0) {format("*** xwidget: column argument must be positive\n"); return;};
	} else {
		format("*** xwidget: column argument must be integer\n");
		return;
	};
	if (kind=="panel" || kind=="button")
		type = Tboolean
	else if (kind=="applypanel" || kind=="applybutton" || kind=="exitpanel" || kind=="exitbutton"
			   || kind=="cancelpanel" || kind=="cancelbutton" || kind=="consttext")
		type = Tnotype
	else if (kind=="text" || kind=="smenu")
		type = Tstring
	else if (kind=="menu")
		type = Tinteger
	else {
		format("*** xwidget: unknown kind '%s'\n",kind);
		return;
	};
	if (!isstring(label)) {
		format("*** xwidget: second (label) argument should be a string\n");
		return;
	};
	if (Nargin() mod 2 != 0) {
		format("*** xwidget: must be even number of option specification args\n");
		return;
	};
	options = "";
	for (i=1; i<=Nargin(); i+=2) {
		options = #(options," ",argin(i),"=");
		if (isint(argin(i+1)) && isscalar(argin(i+1))) {
			options = #(options, sformat("``",argin(i+1)));
		} else if (isstring(argin(i+1))) {
			options = #(options, "\"", argin(i+1), "\"");
		} else {
			format("*** xwidget: option value must be string or integer\n");
			return;
		};
	};
	y = strmat("xwidget",kind,type,label,sformat("``",column),options);
};


function [...] = xpanel(...)
/* [var1,var2,...,varN] = xpanel(xwidget(...),xwidget(...),...,xwidget(...))
   creates a graphical user interface dialog panel for X Window System.
   For each xwidget that is modifiable (all widgets except cancel and exit
   buttons and panels, and consttexts, are modifiable), there must be an
   argument present in the output arglist. The initial value of the output
   argument must conform to the type of widget: menus require integers, smenus
   require strings (one of the menu items), text widgets require strings,
   and panels and buttons require integers (on/off value).

   See also: xwidget. */
global
{
	N = Nargin();
	fp = fopen("x.dat","w");
	argoutindices = izeros(N);
	for ({i=1;j=1}; i<=N; i++) {
		argoutindices[i] = j;
		y = argin(i);
		if (rank(y)!=2) return;
		if (!isint(y)) return;
		if (!strstarteq(y[1,:],"xwidget")) return;
		kind = y[2,:];
		type = y[3,:];
		label = y[4,:];
		column = y[5,:];
		options = y[6,:];
		if (strstarteq(type,Tboolean) || strstarteq(type,Tinteger)) {
			if (!isscalar(argout(j)) || !isint(argout(j))) {format("*** xpanel: outarg #`` should be int\n",i); return;};
		} else if (strstarteq(type,Tstring)) {
			if (!isstring(argout(j))) {format("*** xpanel: outarg #`` should be string\n",i); return;};
		};
		fformat(fp, "`` `` ",column,kind);
		if (strstarteq(type,Tboolean)) {
			if (argout(j))
				fformat(fp,"on ")
			else
				fformat(fp,"off ");
			j++;
		} else if (strstarteq(type,Tinteger)) {
			fformat(fp,"`` ",argout(j));
			j++;
		} else if (strstarteq(type,Tstring)) {
			fformat(fp,"\"``\" ",argout(j));
			j++;
		} else
			fformat(fp,"off ");
		fformat(fp,"\"``\"``\n",label,options);
	};
	fclose(fp);
	data = strmat2(run("xpanel -title 'Tela xpanel' x.dat"));
	if (length(data) < 2) return;	// user typed cancelbutton
	[N,dummy] = size(data);
	for (i=1; i<=N; i++) {
		spec = data[i,:];
		for (j=1; j<=length(spec); j++) if (spec[j]==' ') break;
		widgetindex = evalexpr(spec[1:j-1]);
		outindex = argoutindices[widgetindex];
		SetArgOut(outindex,evalexpr(spec[j+1:length(spec)]))
	};
};

}
