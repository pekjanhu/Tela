function y=gcf() {y=matlab_call("gcf")};
function y=gca() {y=matlab_call("gca")};

function y=get(obj;name) {
	if (isstr(name))
		y = matlab_call("get",obj,name)
	else 
		y = matlab_call("get",obj)
};

function []=set(obj,name,value) {
	[] = matlab_call("set",obj,name,value)
};

