function y=norm(A) {y=mean(abs(A))};

function test(A,B,msg) {
	if (max(abs(A-B)) > norm(A)) {
		format("*** Test `` failed: max(abs(A-B))=``  while norm(A)=``\n",msg,max(abs(A-B)),norm(A));
		TestsFailed++;
	}
};

TestsFailed = 0;
n = 25;
A = rand(n,n);
B = inv(A);
test(A**B, eye(n), "real A**B==eye(n)");
test(B**A, eye(n), "real B**A==eye(n)");

b = rand(n);
x = B**b;
test(A**x,b,"real A**x==b");

A = rand(n,n) + 1i*rand(n,n);
B = inv(A);
test(A**B, eye(n), "complex A**B==eye(n)");
test(B**A, eye(n), "complex B**A==eye(n)");

b = rand(n)+1i*rand(n);
x = B**b;
test(A**x,b,"complex A**x==b");


if (TestsFailed>0)
	format("testla.t:   - `` tests failed.\n")
else
    format("testla.t:   - All tests passed.\n");

