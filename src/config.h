
/******************************************************************************
*
* IMD -- The ITAP Molecular Dynamics Program
*
* Copyright 1996-2004 Institute for Theoretical and Applied Physics,
* University of Stuttgart, D-70550 Stuttgart
*
******************************************************************************/

/******************************************************************************
*
* config.h -- Configuration rules and constants for the imd Package
*
******************************************************************************/

/******************************************************************************
* $Revision$
* $Date$
******************************************************************************/

/******************************************************************************
*
* Misc. Settings
*
******************************************************************************/

/* double precision is the default */
#ifndef SINGLE
#define DOUBLE
#endif
/* the world record switch */
#ifdef MONOLJ
#define NODBG_DIST
#endif

/* define USE_RUSAGE for using getrusage()-routines   */
/* define USE_CLOCK to use clock() instead of times() */

/******************************************************************************
*
* Interactions
*
******************************************************************************/

/* backwards compatibility for potential files without headers */
#ifdef EAM2
#define DEFAULT_POTFILE_TYPE 2
#else
#define DEFAULT_POTFILE_TYPE 1
#endif

/* we always need PAIR, unless MEAM, TERSOFF, KEATING, STIWEB, UNIAX, 
   or EWALD */
/* Note that PAIR is the default, if no interaction is specified */
#if !(defined(MEAM) || defined(TERSOFF) || defined(STIWEB) || defined(KEATING) || defined(UNIAX) || defined(EWALD))
#ifndef PAIR
#define PAIR
#endif
#endif

/* for EAM2 and TTBP, we also need PAIR */
#if (defined(EAM2) || defined(TTBP))
#ifndef PAIR
#define PAIR 
#endif
#endif

/* shortcut for covalent interactions */
#if defined(MEAM) || defined(KEATING) || defined(TTBP) || defined(TERSOFF) || defined(STIWEB)
#define COVALENT
#endif

/* percentage of r2_cut, in which generated potentials are corrected
   so that the forces go continuously to zero */
#define POT_TAIL 0.05

/* with VEC or MPI or NBL we always use buffer cells */
#if defined(MPI) || defined(VEC) || defined(NBL)
#define BUFCELLS
#endif

#ifdef VEC
/* #define MULTIPOT */
#define N_POT_TAB 128
#define NBLIST
#endif

#ifdef NBL
#define NBLIST
#endif

#ifdef BUFCELLS

/* AR is the default. We could make the default machine dependent */
#define AR  

/* for COVALENT, AR *must* be set */
#ifdef COVALENT
#ifndef AR
#define AR
#endif
#endif

#endif /* BUFCELLS */

/******************************************************************************
*
* Statistical Ensembles
*
******************************************************************************/

#ifdef NPT_axial
#define P_AXIAL
#endif

/* heat transport */
#ifdef NVX
#define RNEMD
#endif
#if defined(NVX) || defined(RNEMD)
#define TRANSPORT
#endif

#ifdef MSQD
#define REFPOS
#endif

#ifdef CORRELATE
#define REFPOS
#endif

#ifdef SHOCK
#define STRESS_TENS
#endif

/******************************************************************************
*
* Architectures
*
******************************************************************************/

#ifdef SX4
#undef USE_SOCKETS
#endif

/******************************************************************************
*
* Constants
*
******************************************************************************/

/* memory allocation increment for potential */
#define PSTEP 50

/* security margin for buffer sizes */
#define CSTEP 10

/* interval for checking MPI buffers */
#define BUFSTEP 100

#ifdef COVALENT
#ifdef MEAM
#define NEIGH_LEN_INIT 12
#else
#define NEIGH_LEN_INIT 4
#endif
#define NEIGH_LEN_INC  2
#endif

/* maximum number of data items an atom can have */
#ifdef TWOD
#define MAX_ATOM_SIZE 20
#else
#ifdef UNIAX
#define MAX_ATOM_SIZE 40
#else
#ifdef CG
#define MAX_ATOM_SIZE 35
#else
#define MAX_ATOM_SIZE 25
#endif
#endif
#endif

/* pressure relaxation */
#define RELAX_FULL     0
#define RELAX_AXIAL    1
#define RELAX_ISO      2

/* number of data items by which an MPI buffer is increased */
#define BUFFER_SIZE_INC 1024 

/* buffer size for serial read_atoms */
#define INPUT_BUF_SIZE 16384

/* output buffer size (in bytes) */
#define OUTPUT_BUF_SIZE 131072

