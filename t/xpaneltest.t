// ---------------------- Xpanel test example -----------------------

if (!isfunction(xpanel)) source("xintf");

graphkind = "sin";
linetype = 1;
linethickness = "1";
title = "Daily Telagraph";
subtitle = "Items modifiable by Xpanel";
comment = "This is the 'Comment String' of the graph";
exiting = off;

counter = 0;

repeat

    x = -4*pi:0.05:4*pi;
    y = evalexpr(#(graphkind,"(x)"));
    plot(x,y,"linetype",linetype,"linewidth",linethickness,
         "toplabel",title,"subtitle",subtitle,"comment",comment);
     [graphkind,linetype,linethickness,title,subtitle,comment,exiting] =
        xpanel(xwidget(2,"smenu","Function|sin|cos|tan|exp|sinh|cosh|tanh"),
			   xwidget(1,"menu","Line type|Solid|Dashed|Dotted|Dot-Dash|Long-Dots|Double-Dots|Long-Dash"),
			   xwidget(2,"text","Line thickness:"),
			   xwidget(2,"text","Title:"),
			   xwidget(2,"text","Subtitle:"),
			   xwidget(2,"text","Comment:","width",300),
			   ///xwidget(2,"cancelpanel","Cancel"),
			   xwidget(1,"panel","Exit"),
			   xwidget(1,"exitpanel","OK"));

    counter++;

until exiting || (counter > 10);

if (counter > 10) format("You have been TERMINATED (counter>10).\n");
