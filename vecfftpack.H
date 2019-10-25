/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#define VECARGS_FORMAL , int lot, int step, int jump

extern "C" {

extern void veccfftf(Tint N, Tcomplex data[], Treal wrk[] VECARGS_FORMAL);
extern void veccfftb(Tint N, Tcomplex data[], Treal wrk[] VECARGS_FORMAL);
extern void veccffti(Tint N, Treal wrk[] VECARGS_FORMAL);

extern void vecrfftf(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void vecrfftb(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void vecrffti(Tint N, Treal wrk[] VECARGS_FORMAL);

extern void vecsint(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void vecsinti(Tint N, Treal wrk[] VECARGS_FORMAL);

extern void veccost(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void veccosti(Tint N, Treal wrk[] VECARGS_FORMAL);

extern void vecsinqf(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void vecsinqb(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void vecsinqi(Tint N, Treal wrk[] VECARGS_FORMAL);

extern void veccosqf(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void veccosqb(Tint N, Treal data[], Treal wrk[] VECARGS_FORMAL);
extern void veccosqi(Tint N, Treal wrk[] VECARGS_FORMAL);

#define VECCFFTF veccfftf
#define VECCFFTB veccfftb
#define VECCFFTI veccffti
#define VECRFFTF vecrfftf
#define VECRFFTB vecrfftb
#define VECRFFTI vecrffti
#define VECSINTI vecsinti
#define VECSINT vecsint
#define VECCOSTI veccosti
#define VECCOST veccost
#define VECSINQF vecsinqf
#define VECSINQB vecsinqb
#define VECSINQI vecsinqi
#define VECCOSQF veccosqf
#define VECCOSQB veccosqb
#define VECCOSQI veccosqi

}
