package
global(SetArrayStarts,
	   aref1,aset1,aref2,aset2,aref3,aset3,aref4,aset4)
{

// This provides a simple mechanism for referencing arrays
// with indices that start from not 1 but an arbitrary base.
// Restriction: the index starts are not per array but global.
	
offset_1 = 0;
offset_2 = 0;
offset_3 = 0;
offset_4 = 0;
	
function SetArrayStarts(c1; c2,c3,c4)
// SetArrayStarts sets the first index values for subsequent
// aref1, aref2, ... and aset1, aset2, ... calls.
// For example, if you want to index 2D arrays starting from
// -3 and -5, you first call SetArrayStarts(-3,-5) and then
// can refer to matrices using aref(a,i,j). For instance
// aref(a,-3,-5) yields the first element of a.
// See aref1, aref2, aref3, aref4, aset1, aset2, aset3, aset4.
global
{
	offset_1 = 1-c1;
	if (isdefined(c2)) offset_2 = 1-c2;
	if (isdefined(c3)) offset_2 = 1-c3;
	if (isdefined(c4)) offset_2 = 1-c4;
};

function y = aref1(a,i) global {
	y = a[i+offset_1]
};

function y = aref2(a,i,j) global {
	y = a[i+offset_1,j+offset_2]
};

function y = aref3(a,i,j,k) global {
	y = a[i+offset_1,j+offset_2,k+offset_3]
};

function y = aref4(a,i,j,k,l) global {
	y = a[i+offset_1,j+offset_2,k+offset_3,l+offset_4]
};

function [y;] = aset1(i,value) global {
	y[i+offset_1] = value
};

function [y;] = aset2(i,j,value) global {
	y[i+offset_1,j+offset_2] = value
};

function [y;] = aset3(i,j,k,value) global {
	y[i+offset_1,j+offset_2,k+offset_3] = value
};

function [y;] = aset4(i,j,k,l,value) global {
	y[i+offset_1,j+offset_2,k+offset_3,l+offset_4] = value
};

};
