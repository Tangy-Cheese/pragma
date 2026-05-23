#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mkl_lapacke.h>

/* First part of code taken from band_utility.c template */

/* Define structure that holds band matrix information */
struct band_mat{
  long ncol;        /* Number of columns in band matrix */
  long nbrows;      /* Number of rows (bands in original matrix) */
  long nbands_up;   /* Number of bands above diagonal */
  long nbands_low;  /* Number of bands below diagonal */
  double *array;    /* Storage for the matrix in banded format */
  /* Internal temporary storage for solving inverse problem */
  long nbrows_inv;  /* Number of rows of inverse matrix */
  double *array_inv;/* Store the inverse if this is generated */
  int *ipiv;        /* Additional inverse information */
};
/* Define a new type band_mat */
typedef struct band_mat band_mat;

/* Initialise a band matrix of a certain size, allocate memory,
   and set the parameters.  */
int init_band_mat(band_mat *bmat, long nbands_lower, long nbands_upper, long n_columns) {
  bmat->nbrows = nbands_lower + nbands_upper + 1;
  bmat->ncol   = n_columns;
  bmat->nbands_up = nbands_upper;
  bmat->nbands_low= nbands_lower;
  bmat->array      = (double *) malloc(sizeof(double)*bmat->nbrows*bmat->ncol);
  bmat->nbrows_inv = bmat->nbands_up*2 + bmat->nbands_low + 1;
  bmat->array_inv  = (double *) malloc(sizeof(double)*(bmat->nbrows+bmat->nbands_low)*bmat->ncol);
  bmat->ipiv       = (int *) malloc(sizeof(int)*bmat->ncol);
  if (bmat->array==NULL||bmat->array_inv==NULL) {
    return 0;
  }  
  /* Initialise array to zero */
  long i;
  for (i=0;i<bmat->nbrows*bmat->ncol;i++) {
    bmat->array[i] = 0.0;
  }
  return 1;
};

/* Finalise function: should free memory as required */
void finalise_band_mat(band_mat *bmat) {
  free(bmat->array);
  free(bmat->array_inv);
  free(bmat->ipiv);
}

/* Get a pointer to a location in the band matrix, using
   the row and column indexes of the full matrix.           */
double *getp(band_mat *bmat, long row, long column) {
  int bandno = bmat->nbands_up + row - column;
  if(row<0 || column<0 || row>=bmat->ncol || column>=bmat->ncol ) {
    printf("Indexes out of bounds in getp: %ld %ld %ld \n",row,column,bmat->ncol);
    exit(1);
  }
  return &bmat->array[bmat->nbrows*column + bandno];
}

/* Retrun the value of a location in the band matrix, using
   the row and column indexes of the full matrix.           */
double getv(band_mat *bmat, long row, long column) {
  return *getp(bmat,row,column);
}

/* Set an element of a band matrix to a desired value based on the pointer
   to a location in the band matrix, using the row and column indexes
   of the full matrix.           */
double setv(band_mat *bmat, long row, long column, double val) {
  *getp(bmat,row,column) = val;
  return val;
}

/* Solve the equation Ax = b for a matrix a stored in band format
   and x and b real arrays                                          */
int solve_Ax_eq_b(band_mat *bmat, double *x, double *b) {
  /* Copy bmat array into the temporary store */
  int i,bandno;
  for(i=0;i<bmat->ncol;i++) {
    for (bandno=0;bandno<bmat->nbrows;bandno++) {
      bmat->array_inv[bmat->nbrows_inv*i+(bandno+bmat->nbands_low)] = bmat->array[bmat->nbrows*i+bandno];
    }
    x[i] = b[i];
  }

  long nrhs = 1;
  long ldab = bmat->nbands_low*2 + bmat->nbands_up + 1;
  int info = LAPACKE_dgbsv( LAPACK_COL_MAJOR, bmat->ncol, bmat->nbands_low, bmat->nbands_up, nrhs, bmat->array_inv, ldab, bmat->ipiv, x, bmat->ncol);
  return info;
}

// Printing the values in the order we want.
void PrintCurrent(FILE* file, double t, double dx, int N, double* U, double* V) {

    for (int i = 0; i < N; i++) { // Over all gridpoints we prpytestint in this format

        fprintf(file, "%lf %lf %lf %lf\n", t, i * dx, U[i], V[i]);

    }

}


// Using scanf to read inputparams file and assign each row to each parameter.
void InputParameters(const char* filename, double* L, int* N, double* t_f, double* t_d, double* U_lb, double* U_rb, double* V_lb, double* V_rb) {
   
    // Opening file
    FILE* file = fopen(filename, "r");

    fscanf(file, "%lf", L); // File is in column format
    fscanf(file, "%d", N);
    fscanf(file, "%lf", t_f);
    fscanf(file, "%lf", t_d);
    fscanf(file, "%lf", U_lb);
    fscanf(file, "%lf", U_rb);
    fscanf(file, "%lf", V_lb);
    fscanf(file, "%lf", V_rb);
    fclose(file);

}


// Using scanf to read through the initial conditions and set them for each point.
void Initialise(const char* filename, int N, double* U, double* V) {
   
    // Opening file
    FILE* file = fopen(filename, "r");

    for (int i = 0; i < N; i++) { // Looping over all points to extract the initial conditions of U and V
       
        fscanf(file, "%lf %lf", &U[i], &V[i]); // File is in this format
 
    }

    fclose(file);
}


// Von Neumann boundary conditions using finite differences.
void BoundaryConditions(double* U, double* V, int N, double U_lb, double U_rb, double V_lb, double V_rb, double dx) {
     
    U[0] = U[1] - dx * U_lb; // Left U
    V[0] = V[1] - dx * V_lb; // Left V

    U[N - 1] = U[N - 2] + dx * U_rb; // Right U
    V[N - 1] = V[N - 2] + dx * V_rb; // Right V

}


// Implicit solver using functions from bandutility.c
void Linear(double* U, double* V, int N, double dt, double dx) {

   
    double k = dt / (dx * dx); // Diffusion term
    double c = 1 + 2 * k;   // Coefficient of banded matrix

    // Initialise the matrix
    struct band_mat bmat;
    // Checking the initialisation
    if (!init_band_mat(&bmat, 1, 1, N)) {

        printf("Failed to initialise band matrix\n");
        exit(1);

    }

    // Memory for N vectors
    double* rhs_U = (double*)malloc(N * sizeof(double));
    double* rhs_V = (double*)malloc(N * sizeof(double));
   
    // Checking memory allocation
    if (!(rhs_U && rhs_V)) {

        printf("Memory allocation failed for RHS vectors\n");
        finalise_band_mat(&bmat);
        free(rhs_U);
        free(rhs_V);
        exit(1);

    }

    // Set up the banded matrix and the right side vectors
    for (int i = 0; i < N; i++) {

        setv(&bmat, i, i, c);
        if (i > 0) setv(&bmat, i, i - 1, -k);    
        if (i < N - 1) setv(&bmat, i, i + 1, -k);
        rhs_U[i] = U[i];
        rhs_V[i] = V[i];
   
    }

    // Solving the system of equations
    int result_U = solve_Ax_eq_b(&bmat, U, rhs_U);
    int result_V = solve_Ax_eq_b(&bmat, V, rhs_V);

    // Checking the solution
    if (result_U || result_V) {

        printf("Error solving linear system\n");
        free(rhs_U);
        free(rhs_V);
        finalise_band_mat(&bmat);
        exit(1);

    }

    // Free any memory after used
    free(rhs_U);
    free(rhs_V);
    finalise_band_mat(&bmat);

}


// Explicit solver using finite differences.
void NonLinear(double* U, double* V, int N, double dt, double dx) {
   
    for (int i = 1; i < N-1; i++) {

        // Discretising of pdes to obtain U[i+1]
        U[i] += 2 * dt * U[i] * (1 - 2 * (U[i] * U[i] + V[i] * V[i]));
        V[i] += 2 * dt * V[i] * (1 - 2 * (U[i] * U[i] + V[i] * V[i]));

    }

}

int main() {

    // Define the output file
    const char* outp = "output.txt";

    // Opening the output file
    FILE* output = fopen(outp, "w");

    // Define the parameters
    int N;
    double L, t_f, t_d, U_lb, U_rb, V_lb, V_rb;

    // Defining all the files used
    const char* init = "init_cond.txt";
    const char* params = "input_params.txt";

    // Setting all the parameters from the right files using the functions
    InputParameters(params, &L, &N, &t_f, &t_d, &U_lb, &U_rb, &V_lb, &V_rb);
   
    double* U = malloc(N * sizeof(double)); // Storing memory for U and V
    double* V = malloc(N * sizeof(double));

    // Checking the memory allocation
    if (!(U && V)) {

      printf("Memory allocation failed\n");
      return 1;

    }

    double dx = L / (N - 1); // Line incrememnt.
    double dt = t_d / 100 ; // Timestep
   
    Initialise(init, N, U, V);

    // Slowly ramp up the time to the final time
    for (double current_t = 0.0, next_print_t = 0.0; current_t <= t_f; current_t += dt) { // And as soon as the time gets to the increment of next_t + N*t_d, then it PrintCurrents and adds another increment for current_t to get to.
     
      if (current_t >= next_print_t) { // Note due to floats >= is safer than just ==. But in theory it printcurrent should be activated at ==.
         
          PrintCurrent(output, current_t, dx, N, U, V);
          next_print_t += t_d;

      }

      BoundaryConditions(U, V, N, U_lb, U_rb, V_lb, V_rb, dx);
      Linear(U, V, N, dt, dx);
      NonLinear(U, V, N, dt, dx);

    }

    fclose(output); // Closes the file when done.  
    free(U); // Freeing up memory
    free(V);

    return 0;

}
