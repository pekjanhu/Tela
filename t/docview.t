choiceViewing = smenu("Choose viewing method","ASCII","xdvi","Xemacs");

directory = "/usr/local/lib/tela";

choiceDoc = smenu("Choose document to view",
				 "Basic help file",
				 "Builtin functions alphabetically sorted",
				 "Builtin functions sorted by sections");

if (strstarteq(choiceDoc,"Basic")) {
	file = "telahelp";
} else if (strstarteq(choiceDoc,"Builtin functions alpha")) {
	file = "telafuncs";
} else if (strstarteq(choiceDoc,"Builtin function sorted")) {
    file = "telafuncs_sectioned";
};

if (strstarteq(choiceViewing,"ASCII")) {
	system(#("less -Ms ",directory,"/",file,".txt"));
} else if (strstarteq(choiceViewing,"xdvi")) {
	system(#("xdvi -geometry 847x570+0+0 -s 3 ",directory,"/",file,".dvi&"));
} else if (strstarteq(choiceViewing,"Xemacs")) {
	system(#("emacs ",directory,"/",file,".txt&"));
};

