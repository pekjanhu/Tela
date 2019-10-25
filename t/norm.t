imax=3;
jmax=3;

/*
function m = norm(u)
local (i,j)
{
  m = 0.9999;
  absu = sqrt(u*u);
  disp absu;
  for (i=1; i<=imax; i++) for (j=1; j<=jmax; j++) {
    m = m + absu[i,j];
  };
};
*/

function m = norm(V)
{
   m = 0.0;
   for (i=1; i<=5; i++) m=m+V[i];
};

