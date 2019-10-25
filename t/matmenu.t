package
global(matmenu,smatmenu,MenuAnswer)

{

function y = matmenu(title,S)
/* matmenu("title",S) works similarly to menu except that the second
   argument is a string matrix that contains all the choices.
   matmenu returns the integer choice (the row number in S).
   See also: menu, smatmenu. */
global(MenuAnswer)
{
	[nrows,ncols] = size(S);
	k = 1;
	str = izeros(length(S) + 3*nrows + 2);
	for (i=1; i<=nrows; i++) {
		str[k] = ','; k++;
		str[k] = '"'; k++;
		for (j=1; j<=ncols; j++) {
			if (S[i,j]!=0) {
				str[k] = S[i,j];
				k++;
			};
		};
		str[k] = '"'; k++;
	};
	command = tostring(#("MenuAnswer=menu(\"",title,"\"",str[find(str!=0)],");"));
	eval(command);
	y = MenuAnswer;
};

function y = smatmenu(title,S)
/* smatmenu("title",S) works similarly to smenu except that the second
   argument is a string matrix that contains all the choices.
   smatmenu returns the chosen string as does smenu.
   See also: smenu, matmenu. */
global(MenuAnswer)
{
	[nrows,ncols] = size(S);
	k = 1;
	str = izeros(length(S) + 3*nrows + 2);
	for (i=1; i<=nrows; i++) {
		str[k] = ','; k++;
		str[k] = '"'; k++;
		for (j=1; j<=ncols; j++) {
			if (S[i,j]!=0) {
				str[k] = S[i,j];
				k++;
			};
		};
		str[k] = '"'; k++;
	};
	command = tostring(#("MenuAnswer=smenu(\"",title,"\"",str[find(str!=0)],");"));
	eval(command);
	y = MenuAnswer;
};

}
