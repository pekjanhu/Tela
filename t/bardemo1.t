
oldmode = holdmode();
holdmode(paging);
hold(on);

bar(
	strmat("SS2","SS10/30","SS10/41","RS6K 350","RS6K 370","DEC AXP 400","HP 735"),
	#(19,41.1,63.4,65.4,121,111,150.6),
	"Speed",
	"xlabel","Workstation Model",
	"ylabel","Speed in SPECfp92",
	"toplabel","Comparison of Workstation Speed",
    "subtitle","Compiled from SunWorld",
    "sidelabel","false");

bar(
	1982.:1987,
	#(100,200,-100;
	  120,200, -80;
      140,200, -60;
      160,220, -60;
      200,230, -30;
      400,240, 160),
	strmat("Income","Expenses","Savings"),
	"subtitle","From Kenny Toh's PlotMTV-1.40 manual",
	"xlabel","Year", "ylabel","$");

hold(off);
holdmode(oldmode);
