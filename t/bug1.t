/*
  This file tests a bug which was in Tela versions <=1.2.
  A buggy Tela produces the result #(31,31), whereas the
  correct answer is #(16,31).

  The bug is very sensitive: e.g. if you replace exp(-z)
  by exp(z), the bug disappears. The module codegen.C screws
  up register allocation because the variable unique_index
  was not always zeroed between statements.

  This bug should be fixed in Tela versions > 1.2.
*/

Nx = 32;
Nz = 31;
z = 1;
dz = 1;
sdf = 0.03+exp(-z)+1*z;
[kx,kz] = grid(1:Nx/2,(1:Nz)+1*dz);
size(kx);
