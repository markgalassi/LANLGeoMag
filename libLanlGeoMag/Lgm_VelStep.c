#include "Lgm/Lgm_VelStepInfo.h"




Lgm_VelStepInfo *Lgm_InitVelInfo( ) {
    Lgm_VelStepInfo  *vInfo = (Lgm_VelStepInfo *) calloc( 1, sizeof(*vInfo) );
    return( vInfo );
}

void Lgm_FreeVelInfo(  Lgm_VelStepInfo *vInfo ) {
    free( vInfo );
}



int Lgm_ModMid2( Lgm_Vector *u, Lgm_Vector *Vel0, Lgm_Vector *v, double H, int n, double sgn, 
         int (*Velocity)(Lgm_Vector *, Lgm_Vector *, Lgm_VelStepInfo *), Lgm_VelStepInfo *Info ) {

    int           m;
    double        h2, h, Speed;
    Lgm_Vector    z0, z1, z2, Vel;


    /*
     *  Set stepsize, h and 2*h.
     */
    h  = sgn * H/(double)n;
    h2 = 2.0 * h;


    /*
     *  Set initial point, z0.
     */
    z0 = *u;


    /*
     *  Do initial Euler step to get z1. We get Vel0 from the arg list (its computed once).
     */
    z1.x = z0.x + h*Vel0->x; 
    z1.y = z0.y + h*Vel0->y; 
    z1.z = z0.z + h*Vel0->z;
    

    /*
     *  Do general step to get z2 -> zn. This is midpoint formula.
     */
    for ( m = 1; m < n; ++m ) {

        if ( (*Velocity)(&z1, &Vel, Info) == 0 ) {
            // bail if Velocity eval had issues.
            printf("Lgm_ModMid2(): Velocity evaluation during midpoint phase (m = %d and z1 = %g %g %g) returned with errors (returning with 0)\n", m, z1.x, z1.y, z1.z );
            return(0);
        }
        ++(Info->Lgm_nVelEvals);
        Speed = Lgm_Magnitude(&Vel);
        if ( Speed < 1e-16 ) {
            // bail if Speed is too small
            printf("Lgm_ModMid2(): Speed is too small during midpoint phase (m = %d and z1 = %g %g %g Bmag = %g) is too small (returning with 0).\n", m, z1.x, z1.y, z1.z, Speed );
            return(0); 
        }
        z2.x = z0.x + h2*Vel.x; 
        z2.y = z0.y + h2*Vel.y; 
        z2.z = z0.z + h2*Vel.z;
        z0 = z1;
        z1 = z2;

    }


    /*
     *  Do final Euler step to get z(n+1).
     */
    if ( (*Velocity)(&z1, &Vel, Info) == 0 ) {
        // bail if B-field eval had issues.
        printf("Lgm_ModMid2(): Velocity evaluation during final Euler step (z1 = %g %g %g) returned with errors (returning with 0)\n", z1.x, z1.y, z1.z );
        return(0);
    }
    ++(Info->Lgm_nVelEvals);
    Speed = Lgm_Magnitude(&Vel);
    if ( Speed < 1e-16 ) {
        // bail if Speed is too small
        printf("Lgm_ModMid2(): Speed too small during final Euler step (z1 = %g %g %g Bmag = %g) is too small (returning with 0).\n", z1.x, z1.y, z1.z, Speed );
        return(0);
    }
    z2.x = z1.x + h*Vel.x; 
    z2.y = z1.y + h*Vel.y; 
    z2.z = z1.z + h*Vel.z;



    /*
     *   The final answer for zn is the average of z(n-1) and z(n+1). 
     */
    v->x = 0.5 * (z0.x + z2.x); 
    v->y = 0.5 * (z0.y + z2.y); 
    v->z = 0.5 * (z0.z + z2.z);
    

    return(1);

}




/*
 *
 *
 *  This routine is basically the one in Num. Rec.  The bulk of the code here is
 *  trying to figure out what the optimal value should be for the next step.  It
 *  uses a method developed by Deuflhard, 1985 based on information theory.
 *  However, Shampine, 1987 points out that a much simpler heuristic often does
 *  alot better. The problem with Deuflhard's method is apparently twofold: first
 *  Shampine claims that the information theory is not (necessarily) valid for any
 *  given problem (and when it comes down to it we really do have only one problem
 *  to solve). And second, even if it was applicable, Shampine claims that it is
 *  "the answer" to the wrong question -- i.e. it doesnt address the real
 *  problem.  Well....  We'll stick with this one for a while, but Shampine's
 *  approach sounds better, and the stats he shows indicate that significantly
 *  fewer function evals are needed...
 *
 *
 */
int Lgm_VelStep( Lgm_Vector *u, Lgm_Vector *u_scale, 
          double Htry, double *Hdid, double *Hnext, 
          double eps, double sgn, double *s, int *reset,
          int (*Velocity)(Lgm_Vector *, Lgm_Vector *, Lgm_VelStepInfo *), Lgm_VelStepInfo *Info ){


    Lgm_Vector        u0, Vel0, v, uerr, e;
    int               q, k, kk, km=0, n;
    int               reduction, done;
    double            h2, sss, n2, f, H, err[LGM_VELSTEP_KMAX+1], Speed;
    double            eps1, max_error=0.0, fact, red=1.0, scale=1.0, work, workmin;
    /*
     *  For years, we ran with this seq;
     *
     *      static int     Seq[] = { 0, 1, 2, 4, 6, 8, 12, 18, 24, 32, 48, 64 };
     *
     *  but, some tests show this is more efficient;
     *
     *      static int     Seq[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 18, 24, 32, 48, 64, 128, 256 };
     *  
     *  Ratio of required func calls (for the test I did) was about 1.6
     */
    static int     Seq[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 18, 24, 32, 48, 64, 128, 256 };


    if ( fabs(Htry) < 1e-16 ) {
        printf("Lgm_VelStep2(): Requested stepsize is very small (returning with -1). Htry = %g\n", Htry );
        return(-1);
    }


    if ( *reset ) Info->Lgm_nVelEvals = 0;

    if ( ( eps != Info->Lgm_VelStep_eps_old ) || ( *reset ) ){

        Info->Lgm_VelStep_eps_old = eps;

        *Hnext  = -1.0;
        eps1    = LGM_VELSTEP_SAFE1*eps;

        /*
         *  Compute work to get to column k of the tableau.
         *  Here we assume work is just the number of function
         *  evals needed to get to column k.
         */
        Info->Lgm_VelStep_A[1] = Seq[1] + 1;
        for (k=1; k <= LGM_VELSTEP_KMAX; ++k) Info->Lgm_VelStep_A[k+1] = Info->Lgm_VelStep_A[k] + Seq[k+1];


        /*
         *  Compute Deuflhard's correction factors.
         */
        for (q=2; q <= LGM_VELSTEP_KMAX; ++q) {
            for (k=1; k < q; ++k) {
                Info->Lgm_VelStep_alpha[k][q] = pow( eps1, (Info->Lgm_VelStep_A[k+1] - Info->Lgm_VelStep_A[q+1]) 
                / ( (2.0*k+1.0)*(Info->Lgm_VelStep_A[q+1] - Info->Lgm_VelStep_A[1] + 1.0)) );
            }
        }

        /*
         *  Compute optimal row for convergence.
         */
        for (Info->Lgm_VelStep_kopt=2; Info->Lgm_VelStep_kopt < LGM_VELSTEP_KMAX; Info->Lgm_VelStep_kopt++) 
            if (Info->Lgm_VelStep_A[Info->Lgm_VelStep_kopt+1] > Info->Lgm_VelStep_alpha[Info->Lgm_VelStep_kopt-1][Info->Lgm_VelStep_kopt] * Info->Lgm_VelStep_A[Info->Lgm_VelStep_kopt]) break;
            Info->Lgm_VelStep_kmax = Info->Lgm_VelStep_kopt;

    }




    H = Htry;
    u0 = *u;
    if ( (*Velocity)(&u0, &Vel0, Info) == 0 ) {
        // bail if Velocity eval had issues.
        printf("Lgm_VelStep2(): Velocity evaluation at u0 = %g %g %g returned with errors (returning with -1)\n", u0.x, u0.y, u0.z );
        return(-1);
    }
    ++(Info->Lgm_nVelEvals);
    Speed = Lgm_Magnitude(&Vel0);
    if ( Speed < 1e-16 ) {
        // bail if B-field magnitude is too small
        printf("Lgm_VelStep2(): Speed too small at u0 = %g %g %g (Speed = %g) (returning with -1).\n", u0.x, u0.y, u0.z, Speed );
        return(-1);
    }
//printf("u0 = %g %g %g\n", u0.x, u0.y, u0.z );
//printf("Vel0 = %g %g %g\n", Vel0.x, Vel0.y, Vel0.z );

    if ( (*s != Info->Lgm_VelStep_snew) || (H != *Hnext) || ( *reset ) ) {
        Info->Lgm_VelStep_snew = 0.0;
        *s   = 0.0;
        Info->Lgm_VelStep_kopt = Info->Lgm_VelStep_kmax;
        Info->Lgm_VelStep_FirstTimeThrough = TRUE;
    }
    reduction = FALSE;
    done      = FALSE;


    while ( !done ) {

        for (k=1; k<=Info->Lgm_VelStep_kmax; ++k) {

            Info->Lgm_VelStep_snew = *s + H;
            if ( fabs(Info->Lgm_VelStep_snew - *s) < 1e-16 ) { 
                if (Info->VerbosityLevel > 1) {
                    //printf("H = %g\n", H);
                    fprintf(stderr, "step size underflow\n");
                    fprintf(stderr, "Htry, Hdid, Hnext = %g %g %g\n", Htry, *Hdid, *Hnext);
                    }
                Info->Lgm_VelStep_FirstTimeThrough=TRUE;
                Info->Lgm_VelStep_eps_old = -1.0;
                //printf("HOW DID I GET HERE? u0 = %g %g %g    v = %g %g %g    H, Htry, s  = %g %g %g    \n", u0.x, u0.y, u0.z, v.x, v.y, v.z, H, Htry, *s );
                return(-1);
            }

            n  = Seq[k];
            n2 = (double)(n*n);
            h2 = H*H;
            if ( !Lgm_ModMid2( &u0, &Vel0, &v, H, n, sgn, Velocity, Info ) ) return(-1); // bail if Lgm_ModMid2() had issues.
            sss = h2/n2;
            Lgm_VelPolFunExt( k-1, sss, &v, u, &uerr, Info );
            
            if (k !=  1){

                e.x = fabs( uerr.x/u_scale->x );
                e.y = fabs( uerr.y/u_scale->y );
                e.z = fabs( uerr.z/u_scale->z );
                max_error = 1.0e-30;
                if (e.x > max_error) max_error = e.x;
                if (e.y > max_error) max_error = e.y;
                if (e.z > max_error) max_error = e.z;
                max_error /= eps;
                km = k-1;
                err[km] = pow( max_error/LGM_VELSTEP_SAFE1, 1.0/(2.0*km + 1.0) );
            }


            if ( (k != 1) && ((k >= Info->Lgm_VelStep_kopt-1) || Info->Lgm_VelStep_FirstTimeThrough )) {

                if ( max_error < 1.0 ) {

                    /*
                     *  We've converged! Bailout and go home...
                     */
                    done = TRUE;

                } else if ( (k == Info->Lgm_VelStep_kmax)||(k == Info->Lgm_VelStep_kopt+1) ) {

                    red = LGM_VELSTEP_SAFE2/err[km];

                } else if ( (k == Info->Lgm_VelStep_kopt) && (Info->Lgm_VelStep_alpha[Info->Lgm_VelStep_kopt-1][Info->Lgm_VelStep_kopt] < err[km]) ) {

                    red = 1.0/err[km];

                } else if ( (k == Info->Lgm_VelStep_kmax) && (Info->Lgm_VelStep_alpha[km][Info->Lgm_VelStep_kmax-1] < err[km]) ) {

                    red = LGM_VELSTEP_SAFE2 * Info->Lgm_VelStep_alpha[km][Info->Lgm_VelStep_kmax-1]/err[km];

                } else if ( Info->Lgm_VelStep_alpha[km][Info->Lgm_VelStep_kopt] < err[km] ) {

                    red = Info->Lgm_VelStep_alpha[km][Info->Lgm_VelStep_kopt-1]/err[km];

                }

            }


        }

        if (!done) {
            red = (red < LGM_VELSTEP_REDMIN) ? red : LGM_VELSTEP_REDMIN;
            red = (red > LGM_VELSTEP_REDMAX) ? red : LGM_VELSTEP_REDMAX;
            H *= red;
            reduction = TRUE;
        }

    }

    *s     = Info->Lgm_VelStep_snew;
    *Hdid  = H;
    *reset = FALSE;
    Info->Lgm_VelStep_FirstTimeThrough = FALSE;
    workmin = 1e99;
    for (kk=1; kk<=km; ++kk) {
        fact = (err[kk] > LGM_VELSTEP_SCLMAX) ? err[kk] : LGM_VELSTEP_SCLMAX;

        work = fact*Info->Lgm_VelStep_A[kk+1];
        if (work < workmin){
            scale   = fact;
            workmin = work;
            Info->Lgm_VelStep_kopt    = kk+1;
        }

    }

    *Hnext = H/scale;
    if ( (Info->Lgm_VelStep_kopt >= k) && (Info->Lgm_VelStep_kopt != Info->Lgm_VelStep_kmax) && !reduction ) {
        f = scale/Info->Lgm_VelStep_alpha[Info->Lgm_VelStep_kopt-1][Info->Lgm_VelStep_kopt];
        fact = (f > LGM_VELSTEP_SCLMAX) ? f : LGM_VELSTEP_SCLMAX;
        if ( Info->Lgm_VelStep_A[Info->Lgm_VelStep_kopt+1]*fact <= workmin){
            *Hnext = H/fact;
            Info->Lgm_VelStep_kopt++;
        }
    }



    return(1);

}

void Lgm_VelRatFunExt( int k, double x_k, Lgm_Vector *u_k, Lgm_Vector *w, Lgm_Vector *dw, Lgm_VelStepInfo *Info ) {

    int      i, j;
    double   yy, v, ddy=0.0, c, b1, b, fx[LGM_VELSTEP_JMAX];
    double   y_k[3], y[3], dy[3];
//    static double   d[LGM_VELSTEP_JMAX][LGM_VELSTEP_JMAX], x[LGM_VELSTEP_JMAX];

    /*
     *  (x_k, y_k) is the kth "data point" in the sequence.
     */
    y_k[0] = u_k->x; y_k[1] = u_k->y; y_k[2] = u_k->z;

    Info->Lgm_VelStep_BS_x[k] = x_k;
    // next line is not needed -- just debugginh...
    Info->Lgm_VelStep_BS_u[k] = *u_k;
    if (k == 0) {
        for (i=0; i<3; ++i) y[i] = dy[i] = Info->Lgm_VelStep_BS_d[i][0] = y_k[i];
    } else {
        for (j=0; j<k; ++j) fx[j+1] = Info->Lgm_VelStep_BS_x[k-j-1] / x_k;
        for (i=0; i<3; ++i) {
            v = Info->Lgm_VelStep_BS_d[i][0];
            c = yy = Info->Lgm_VelStep_BS_d[i][0] = y_k[i];
            for (j=1; j<=k; ++j) {
                b1 = fx[j] * v;
                b  = b1 - c;
                if (b != 0.0) {
                    b    = (c - v)/b;
                    ddy  = c * b;
                    c    = b1 * b;
                } else{
                    ddy = v;
                }
                if (j != k) v = Info->Lgm_VelStep_BS_d[i][j];
                Info->Lgm_VelStep_BS_d[i][j]  = ddy;
                yy      += ddy;
            }
            dy[i] = ddy;
            y[i]  = yy;
        }
    }
//printf("Lgm_VelRatFunExt:\n");
//for (j=0; j<=k; j++){
//printf("k = %d  x,y = %.15lf    %.15lf %.15lf %.15lf\n", k, Info->Lgm_VelStep_BS_x[j], Info->Lgm_VelStep_BS_u[j].x, Info->Lgm_VelStep_BS_u[j].y, Info->Lgm_VelStep_BS_u[j].z);
//}

    /*
     *  (x_k, u_k) are the input "data points". The rational function should go through
     *  all of these points. w is the rational function evaluated at x_k = 0.0 -- i.e.
     *  an estimate of what the sequence of modified midpoint estimates would converge to
     *  for infinitely small step sizes. dw is the error in this estimate.
     *
     */
    w->x  = y[0];    w->y  = y[1];    w->z  = y[2];
    dw->x = dy[0];   dw->y = dy[1];   dw->z = dy[2];
}


void Lgm_VelPolFunExt( int k, double x_k, Lgm_Vector *u_k, Lgm_Vector *w, Lgm_Vector *dw, Lgm_VelStepInfo *Info ) {

    int             k1, j;
    double          q, f1, f2, c[LGM_VELSTEP_JMAX];
    double          y_k[3], y[3], dy[3], delta;

    /*
     *  (x_k, y_k) is the kth "data point" in the sequence.
     */
    y_k[0] = u_k->x; y_k[1] = u_k->y; y_k[2] = u_k->z;

    Info->Lgm_VelStep_BS_x[k] = x_k;
    Info->Lgm_VelStep_BS_u[k] = *u_k; // line not needed -- just debugginh...

    for (j=0; j<3; j++) y[j] = dy[j] = y_k[j];

    if (k == 0) {
        for (j=0; j<3; j++) Info->Lgm_VelStep_BS_d[j][0] = y_k[j];
    } else {
        for (j=0; j<3; j++) c[j] = y_k[j];

        for (k1=1; k1<k; k1++) {
            delta = 1.0/(Info->Lgm_VelStep_BS_x[k-k1] - x_k);
//printf("delta = %g\n", delta);
            f1 = x_k*delta;
            f2 = Info->Lgm_VelStep_BS_x[k-k1]*delta;
            for ( j=0; j<3; j++){
                q = Info->Lgm_VelStep_BS_d[j][k1];
                Info->Lgm_VelStep_BS_d[j][k1] =  dy[j];
                delta = c[j]-q;
                dy[j] = f1*delta;
                c[j] = f2*delta;
                y[j] += dy[j];
            }
        }
        for (j=0; j<3; j++) Info->Lgm_VelStep_BS_d[j][k] = dy[j];
    }


//printf("Lgm_VelPolFunExt:\n");
//for (j=0; j<=k; j++){
//printf("k = %d  x,y = %.15lf    %.15lf %.15lf %.15lf\n", k, Info->Lgm_VelStep_BS_x[j], Info->Lgm_VelStep_BS_u[j].x, Info->Lgm_VelStep_BS_u[j].y, Info->Lgm_VelStep_BS_u[j].z);
//}
    /*
     *  (x_k, u_k) are the input "data points". The polynomial function should go through
     *  all of these points. w is the polynomial function evaluated at x_k = 0.0 -- i.e.
     *  an estimate of what the sequence of modified midpoint estimates would converge to
     *  for infinitely small step sizes. dw is the error in this estimate.
     *
     */
    w->x  = y[0];    w->y  = y[1];    w->z  = y[2];
//printf("k = %d  w = %.15lf %.15lf %.15lf\n", w->x, w->y, w->z );
    dw->x = dy[0];   dw->y = dy[1];   dw->z = dy[2];
}


