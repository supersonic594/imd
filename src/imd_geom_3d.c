/******************************************************************************
*
* imd_geom.c -- Domain decomposition routines for the imd package
*
******************************************************************************/

/******************************************************************************
* $RCSfile$
* $Revision$
* $Date$
******************************************************************************/

#include "imd.h"

/* init_cells determines the size of the cells. The cells generated by
   scaling of the box. The scaling is done such that the distance
   between two of the cell's faces (==height) equals the cutoff.  
   An integer number of cells must fit into the box. If the cell
   calculated by the scaling described above is to small, it is
   enlarged accordingly.

   There must be at least three cells in each direction.  */

void init_cells( void )

{
  int i, j, k, l;
  vektor hx, hy, hz; 
  real det, tmp;
  vektor cell_scale;
  ivektor next_cell_dim, cell_dim_old;
  ivektor cellmin_old, cellmax_old, cellc;
  cell *p, *cell_array_old; 
  real s1, s2, r2_cut2;

#ifdef NPT
  if (ensemble == ENS_NPT_ISO) {

#ifdef MPI
    if (0 == myid )
#endif
    printf("actual_shrink=%f limit_shrink=%f limit_growth=%f\n",
            actual_shrink.x, limit_shrink.x, limit_growth.x );
  }
  else if (ensemble == ENS_NPT_AXIAL) {

#ifdef MPI
    if (0 == myid ) {
#endif
      printf("actual_shrink.x=%f limit_shrink.x=%f limit_growth.x=%f\n",
              actual_shrink.x,   limit_shrink.x,   limit_growth.x );
      printf("actual_shrink.y=%f limit_shrink.y=%f limit_growth.y=%f\n",
              actual_shrink.y,   limit_shrink.y,   limit_growth.y );
      printf("actual_shrink.z=%f limit_shrink.z=%f limit_growth.z=%f\n",
              actual_shrink.z,   limit_shrink.z,   limit_growth.z );
#ifdef MPI
    };
#endif
  };

#endif

  /* Calculate smallest possible cell (i.e. the height==cutoff) */
  /* This needs a better way to do operations on 3d vectors */

  /* Height x */
  s1 = SPROD(box_x,box_y)/SPROD(box_y,box_y);
  s2 = SPROD(box_x,box_z)/SPROD(box_z,box_z);
  
  hx.x = box_x.x - s1 * box_y.x - s2 * box_z.x;
  hx.y = box_x.y - s1 * box_y.y - s2 * box_z.y;
  hx.z = box_x.z - s1 * box_y.z - s2 * box_z.z;
  
  /* Height y */
  s1 = SPROD(box_y,box_x)/SPROD(box_x,box_x);
  s2 = SPROD(box_y,box_z)/SPROD(box_z,box_z);

  hy.x = box_y.x - s1 * box_x.x - s2 * box_z.x;
  hy.y = box_y.y - s1 * box_x.y - s2 * box_z.y;
  hy.z = box_y.z - s1 * box_x.z - s2 * box_z.z;
  
  /* Height z */
  s1 = SPROD(box_z,box_x)/SPROD(box_x,box_x);
  s2 = SPROD(box_z,box_y)/SPROD(box_y,box_y);
  
  hz.x = box_z.x - s1 * box_x.x - s2 * box_y.x;
  hz.y = box_z.y - s1 * box_x.y - s2 * box_y.y;
  hz.z = box_z.z - s1 * box_x.z - s2 * box_y.z;
  
  /* Scaling factors box/cell */
#ifdef EAM
  /* increase the size of the cell by 2 for EAM */
  r2_cut2 = MAX( r2_cut, 2 * eam_r2_cut ); 
#else
  r2_cut2 = r2_cut;
#endif /* EAM */ 

  cell_scale.x = sqrt( r2_cut2 / SPROD(hx,hx) );
  cell_scale.y = sqrt( r2_cut2 / SPROD(hy,hy) );
  cell_scale.z = sqrt( r2_cut2 / SPROD(hz,hz) );
#ifdef NPT
  /* the NEXT cell array for a GROWING system; 
     needed to determine when to recompute the cell division */
  if ((ensemble == ENS_NPT_ISO) || (ensemble == ENS_NPT_AXIAL)) {
    next_cell_dim.x = (int) ( (1.0 + cell_size_tolerance) / cell_scale.x );
    next_cell_dim.y = (int) ( (1.0 + cell_size_tolerance) / cell_scale.y );
    next_cell_dim.z = (int) ( (1.0 + cell_size_tolerance) / cell_scale.z );
    cell_scale.x   /= (1.0 - cell_size_tolerance);
    cell_scale.y   /= (1.0 - cell_size_tolerance);
    cell_scale.z   /= (1.0 - cell_size_tolerance);
  };
#endif
  /* set up the CURRENT cell array dimensions */
  global_cell_dim.x = (int) ( 1.0 / cell_scale.x );
  global_cell_dim.y = (int) ( 1.0 / cell_scale.y );
  global_cell_dim.z = (int) ( 1.0 / cell_scale.z );

#ifdef MPI
  if (0 == myid )
#endif
  printf("Minimal cell size: \n\t ( %f %f %f ) \n\t ( %f %f %f ) \n\t ( %f %f %f )\n",
    box_x.x * cell_scale.x, box_x.y * cell_scale.x, box_x.z * cell_scale.x,
    box_y.x * cell_scale.y, box_y.y * cell_scale.y, box_y.z * cell_scale.y,
    box_z.x * cell_scale.z, box_z.y * cell_scale.z, box_z.z * cell_scale.z);

#ifdef MPI
  /* cpu_dim must be a divisor of global_cell_dim */
  if (0 != (global_cell_dim.x % cpu_dim.x))
     global_cell_dim.x = ((int)(global_cell_dim.x/cpu_dim.x))*cpu_dim.x;
  if (0 != (global_cell_dim.y % cpu_dim.y))
     global_cell_dim.y = ((int)(global_cell_dim.y/cpu_dim.y))*cpu_dim.y;
  if (0 != (global_cell_dim.z % cpu_dim.z))
     global_cell_dim.z = ((int)(global_cell_dim.z/cpu_dim.z))*cpu_dim.z;
#ifdef NPT
  /* cpu_dim must be a divisor of next_cell_dim */
  if ((ensemble == ENS_NPT_ISO) || (ensemble == ENS_NPT_AXIAL)) {
    if (0 != (next_cell_dim.x % cpu_dim.x))
       next_cell_dim.x = ((int)(next_cell_dim.x/cpu_dim.x))*cpu_dim.x;
    if (0 != (next_cell_dim.y % cpu_dim.y))
       next_cell_dim.y = ((int)(next_cell_dim.y/cpu_dim.y))*cpu_dim.y;
    if (0 != (next_cell_dim.z % cpu_dim.z))
       next_cell_dim.z = ((int)(next_cell_dim.z/cpu_dim.z))*cpu_dim.z;
  };
#endif
#endif

  /* Check if cell array is large enough */
#ifdef MPI
  if ( 0 == myid ) {
    if (global_cell_dim.x < cpu_dim.x) error("global_cell_dim.x < cpu_dim.x");
    if (global_cell_dim.y < cpu_dim.y) error("global_cell_dim.y < cpu_dim.y");
    if (global_cell_dim.z < cpu_dim.z) error("global_cell_dim.z < cpu_dim.z");
#endif
    if (global_cell_dim.x < 3)         error("global_cell_dim.x < 3");
    if (global_cell_dim.y < 3)         error("global_cell_dim.y < 3");
    if (global_cell_dim.z < 3)         error("global_cell_dim.z < 3");
#ifdef MPI
  };
#endif

#ifdef NPT

  if ((ensemble == ENS_NPT_ISO) || (ensemble == ENS_NPT_AXIAL)) {  
    /* if system grows, the next cell division should have more cells */
#ifdef MPI
    if (next_cell_dim.x == global_cell_dim.x) next_cell_dim.x += cpu_dim.x;
    if (next_cell_dim.y == global_cell_dim.y) next_cell_dim.y += cpu_dim.y;
    if (next_cell_dim.z == global_cell_dim.z) next_cell_dim.z += cpu_dim.z;
#else
    if (next_cell_dim.x == global_cell_dim.x) next_cell_dim.x += 1;
    if (next_cell_dim.y == global_cell_dim.y) next_cell_dim.y += 1;
    if (next_cell_dim.z == global_cell_dim.z) next_cell_dim.z += 1;
#endif
  };

  if (ensemble == ENS_NPT_ISO) {

    /* factor by which a cell can grow before a change of
       the cell division becomes worthwhile */
    /* getting more cells in at least one direction is enough */
    limit_growth.x = cell_scale.x * next_cell_dim.x;
    tmp            = cell_scale.y * next_cell_dim.y;
    limit_growth.x = MIN( limit_growth.x, tmp );
    tmp            = cell_scale.z * next_cell_dim.z;
    limit_growth.x = MIN( limit_growth.x, tmp );
    /* factor by which a cell can safely shrink */
    limit_shrink.x = cell_scale.x * global_cell_dim.x;
    tmp            = cell_scale.y * global_cell_dim.y;
    limit_shrink.x = MAX( limit_shrink.x, tmp );
    tmp            = cell_scale.z * global_cell_dim.z;
    limit_shrink.x = MAX( limit_shrink.x, tmp );
    limit_shrink.x = limit_shrink.x * (1.0 - cell_size_tolerance);

  }
  else if (ensemble == ENS_NPT_AXIAL) {

    /* factors by which a cell can grow before a change of
       the cell division becomes worthwhile */
    limit_growth.x = cell_scale.x * next_cell_dim.x;
    limit_growth.y = cell_scale.y * next_cell_dim.y;
    limit_growth.z = cell_scale.z * next_cell_dim.z;
    /* factor by which a cell can safely shrink */
    limit_shrink.x = cell_scale.x*global_cell_dim.x*(1.0-cell_size_tolerance);
    limit_shrink.y = cell_scale.y*global_cell_dim.y*(1.0-cell_size_tolerance);
    limit_shrink.z = cell_scale.z*global_cell_dim.z*(1.0-cell_size_tolerance);
 
 };

#endif /* NPT */

  /* If an integer number of cells does not fit exactly into the box, the
     cells are enlarged accordingly */
  cell_scale.x = 1.0 / global_cell_dim.x;
  cell_scale.y = 1.0 / global_cell_dim.y;
  cell_scale.z = 1.0 / global_cell_dim.z;

#ifdef MPI
  if (0 == myid )
#endif
  printf("Actual cell size: \n\t ( %f %f %f ) \n\t ( %f %f %f ) \n\t ( %f %f %f )\n",
    box_x.x * cell_scale.x, box_x.y * cell_scale.x, box_x.z * cell_scale.x,
    box_y.x * cell_scale.y, box_y.y * cell_scale.y, box_y.z * cell_scale.y,
    box_z.x * cell_scale.z, box_z.y * cell_scale.z, box_z.z * cell_scale.z);

#ifdef MPI
  if (0==myid)
#endif
  printf("Global cell array dimensions: %d %d %d\n",
          global_cell_dim.x,global_cell_dim.y,global_cell_dim.z);

  /* keep a copy of cell_dim & Co., so that we can redistribute the atoms */
  cell_dim_old = cell_dim;
  cellmin_old  = cellmin;
  cellmax_old  = cellmax;

#ifdef MPI
  /* this thest should be obsolete now */
  if (0==myid) {
     if ( 0 !=  global_cell_dim.x % cpu_dim.x ) 
        error("cpu_dim.x no divisor of global_cell_dim.x");
     if ( 0 !=  global_cell_dim.y % cpu_dim.y ) 
        error("cpu_dim.y no divisor of global_cell_dim.y");
     if ( 0 !=  global_cell_dim.z % cpu_dim.z ) 
        error("cpu_dim.z no divisor of global_cell_dim.z");
  };

  cell_dim.x = global_cell_dim.x / cpu_dim.x + 2;  
  cell_dim.y = global_cell_dim.y / cpu_dim.y + 2;
  cell_dim.z = global_cell_dim.z / cpu_dim.z + 2;

  cellmin.x = 1;
  cellmin.y = 1;
  cellmin.z = 1;

  cellmax.x = cell_dim.x - 1;
  cellmax.y = cell_dim.y - 1;
  cellmax.z = cell_dim.z - 1;
    
  if (0==myid) 
    printf("Local cell array dimensions (incl buffer): %d %d %d\n",
	   cell_dim.x,cell_dim.y,cell_dim.z);
#else

  cell_dim.x = global_cell_dim.x;
  cell_dim.y = global_cell_dim.y;
  cell_dim.z = global_cell_dim.z;

  cellmin.x = 0;
  cellmin.y = 0;
  cellmin.z = 0;

  cellmax.x = cell_dim.x;
  cellmax.y = cell_dim.y;
  cellmax.z = cell_dim.z;

  printf("Local cell array dimensions: %d %d %d\n",
	 cell_dim.x,cell_dim.y,cell_dim.z);
#endif

  /* To determine the cell into which a given particle belongs, we
     have to transform the cartesian coordinates of the particle into
     the coordinate system that is spanned by the vectors of the box
     edges. This yields coordinates in the interval [0..1] that are
     multiplied by global_cell_dim to get the cells index.

     Here we calculate the transformation matrix. */

  /* Calculate inverse of coordinate matrix */

  /* Determinant first */
  det = box_x.y * box_y.z * box_z.x +
        box_z.z * box_y.x * box_z.y +
	box_x.x * box_y.y * box_z.z -
        box_x.z * box_y.y * box_z.x -
        box_x.x * box_y.z * box_z.y -
	box_x.y * box_y.x * box_z.z;
#ifdef MPI
  if ( 0 == myid )
#endif
  if ( 0 == det) error("Box Edges are parallel.");

  /* Inverse second */
  ibox_x.x = ( box_y.y * box_z.z - box_y.z * box_z.y ) / det;
  ibox_x.y = ( box_x.z * box_z.y - box_x.y * box_z.z ) / det;
  ibox_x.z = ( box_x.y * box_y.z - box_x.z * box_y.y ) / det;

  ibox_y.x = ( box_y.z * box_z.x - box_y.x * box_z.z ) / det;
  ibox_y.y = ( box_x.x * box_z.z - box_x.z * box_z.x ) / det;
  ibox_y.z = ( box_x.z * box_y.x - box_x.x * box_y.z ) / det;

  ibox_z.x = ( box_y.x * box_z.y - box_y.y * box_z.x ) / det;
  ibox_z.y = ( box_x.y * box_z.x - box_x.x * box_z.y ) / det;
  ibox_z.z = ( box_x.x * box_y.y - box_x.y * box_y.x ) / det;

  /* Transpose */
  tbox_x.x = ibox_x.x;
  tbox_x.y = ibox_y.x;
  tbox_x.z = ibox_z.x;

  tbox_y.x = ibox_x.y;
  tbox_y.y = ibox_y.y;
  tbox_y.z = ibox_z.y;

  tbox_z.x = ibox_x.z;
  tbox_z.y = ibox_y.z;
  tbox_z.z = ibox_z.z;

  /* save old cell_array (if any), and allocate new one */
  cell_array_old = cell_array;
  cell_array = (cell *) malloc(
		     cell_dim.x * cell_dim.y * cell_dim.z * sizeof(cell));
#ifdef MPI
  if ( 0 == myid )
#endif
  if (NULL == cell_array) error("Can't allocate memory for cells");

  /* Initialize cells */
  for (i=0; i < cell_dim.x; ++i)
    for (j=0; j < cell_dim.y; ++j)
      for (k=0; k < cell_dim.z; ++k) {

	p = PTR_3D_V(cell_array, i, j, k, cell_dim);
	p->n_max=0;
#ifdef MPI
            /* don't alloc data space for buffer cells */
        if ((0!=i) && (0!=j) && (0!=k) &&
            (i != cell_dim.x-1) &&
            (j != cell_dim.y-1) &&
            (k != cell_dim.z-1))
#endif

#ifdef MONOLJ
            alloc_cell(p, initsz);
#else
	    alloc_cell(p, CSTEP);
#endif
  };

  /* on the first invocation we have to set up the MPI process topology */
#ifdef MPI
  if (cell_array_old == NULL) setup_mpi_topology();
#endif

  /* redistribute atoms */
  if (cell_array_old != NULL) {
    for (j=cellmin_old.x; j < cellmax_old.x; j++)
      for (k=cellmin_old.y; k < cellmax_old.y; k++)
        for (l=cellmin_old.z; l < cellmax_old.z; l++) {
          p = PTR_3D_V(cell_array_old, j, k, l, cell_dim_old);
          for (i = p->n - 1; i >= 0; i--) {
#ifdef MPI
            cellc = local_cell_coord(p->ort X(i),p->ort Y(i),p->ort Z(i));
            /* strangly, some atoms get into buffer cells; 
               we push them back into the real cells, 
               so that we don't lose them  */
            if (cellc.x == 0) cellc.x++;
            if (cellc.y == 0) cellc.y++;
            if (cellc.z == 0) cellc.z++;
            if (cellc.x == cellmax.x) cellc.x--;
            if (cellc.y == cellmax.y) cellc.y--;
            if (cellc.z == cellmax.z) cellc.z--;
#else
            cellc = cell_coord(p->ort X(i),p->ort Y(i),p->ort Z(i));
#endif
            move_atom( cellc, p, i );
          };
          alloc_cell( p, 0 );  /* free old cell */
    };
    free(cell_array_old);
  };

#ifdef NPT

  if ((ensemble == ENS_NPT_ISO) || (ensemble == ENS_NPT_AXIAL)) {

    revise_cell_division = 0;
    cells_too_small      = 0;
    actual_shrink.x      = 1.0;
    actual_shrink.y      = 1.0;
    actual_shrink.z      = 1.0;

  };

  if (ensemble == ENS_NPT_ISO) {

#ifdef MPI
    if (0 == myid)
#endif
      printf("actual_shrink=%f limit_shrink=%f limit_growth=%f\n",
              actual_shrink.x, limit_shrink.x, limit_growth.x );
  }
  else if (ensemble == ENS_NPT_AXIAL) {

#ifdef MPI
    if (0 == myid ) {
#endif
      printf("actual_shrink.x=%f limit_shrink.x=%f limit_growth.x=%f\n",
              actual_shrink.x,   limit_shrink.x,   limit_growth.x );
      printf("actual_shrink.y=%f limit_shrink.y=%f limit_growth.y=%f\n",
              actual_shrink.y,   limit_shrink.y,   limit_growth.y );
      printf("actual_shrink.z=%f limit_shrink.z=%f limit_growth.z=%f\n",
              actual_shrink.z,   limit_shrink.z,   limit_growth.z );
#ifdef MPI
    };
#endif
  };

#endif /* NPT */

}


/*
*
* cell_coord gives the (integral) cell_coorinates of a position
*
*/

ivektor cell_coord(real x, real y, real z)

{
  ivektor coord;
  vektor ort;

  /* Yes, this _IS_ ugly */
  ort.x = x;
  ort.y = y;
  ort.z = z;

  /* Map positions to boxes */
  coord.x = (int) TRUNC( global_cell_dim.x * SPROD(ort,tbox_x) );
  coord.y = (int) TRUNC( global_cell_dim.y * SPROD(ort,tbox_y) );
  coord.z = (int) TRUNC( global_cell_dim.z * SPROD(ort,tbox_z) );

  /* Roundoff errors put atoms slightly out of the simulation cell */
  /* Great mess, needs more investigation */
  coord.x = coord.x <   0                ?                    0 : coord.x;
  coord.x = coord.x >= global_cell_dim.x ? global_cell_dim.x -1 : coord.x;
  coord.y = coord.y <   0                ?                    0 : coord.y;
  coord.y = coord.y >= global_cell_dim.y ? global_cell_dim.y -1 : coord.y;
  coord.z = coord.z <   0                ?                    0 : coord.z;
  coord.z = coord.z >= global_cell_dim.z ? global_cell_dim.z -1 : coord.z;

  return(coord);

}


/* map vektor back into simulation box */

vektor back_into_box(vektor pos)
{
  vektor d;
  int i;

  i = FLOOR(SPROD(pos,tbox_x));
  d.x  = pos.x - i *  box_x.x;
  d.y  = pos.y - i *  box_x.y;
  d.z  = pos.z - i *  box_x.z;

  i = FLOOR(SPROD(pos,tbox_y));
  d.x -= i *  box_y.x;
  d.y -= i *  box_y.y;
  d.z -= i *  box_y.z;

  i = FLOOR(SPROD(pos,tbox_z));
  d.x -= i *  box_z.x;
  d.y -= i *  box_z.y;
  d.z -= i *  box_z.z;

  return(d);

}

