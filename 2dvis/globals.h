#ifdef MAIN
#define EXTERN
#define INIT(a) =a
#else
#define EXTERN extern
#define INIT(a)
#endif

EXTERN int *nummer;
EXTERN int columns;
EXTERN int *bcode;
EXTERN int bond_mode;
EXTERN int atom_mode INIT(1);
EXTERN int col_mode INIT(0);
EXTERN int stat_bond INIT(0);
EXTERN int scene_type INIT(0);
EXTERN int endian_byte_swap INIT(0);
EXTERN int text INIT(0);
EXTERN int eng_mode;
EXTERN int eng_minmax INIT(0);
EXTERN int qp;
EXTERN int radectyp INIT(0);
EXTERN int x_res;
EXTERN int y_res;
EXTERN int natoms;
EXTERN int nunits;

EXTERN short int *sorte;

EXTERN double *masse;
EXTERN double *x;
EXTERN double *y;
#ifndef TWOD
EXTERN double *z;
#endif
EXTERN double *vx;
EXTERN double *vy;
#ifndef TWOD
EXTERN double *vz;
#endif
EXTERN double *pot;
EXTERN double *kin;

EXTERN unsigned short base_port INIT(31913);

EXTERN float maxx INIT(-1000);
EXTERN float minx INIT(1000);
EXTERN float maxy INIT(-1000);
EXTERN float miny INIT(1000);
#ifndef TWOD
EXTERN float minz INIT(1000);
EXTERN float maxz INIT(-1000);
#endif
EXTERN float maxp INIT(-1000);
EXTERN float minp INIT(1000);
EXTERN float maxk INIT(-1000);
EXTERN float mink INIT(1000);

EXTERN float scalex;
EXTERN float scaley;
#ifndef TWOD
EXTERN float scalez;
#endif
EXTERN float scalepot;
EXTERN float scalekin;
EXTERN float radius INIT(.3);
EXTERN float offspot;
EXTERN float offskin;
EXTERN float *potarray;
EXTERN float *kinarray;
EXTERN float *ux;
EXTERN float *uy;

EXTERN char *paramfilename;
EXTERN char uvfname[255];

