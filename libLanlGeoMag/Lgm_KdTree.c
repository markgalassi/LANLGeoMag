#include "Lgm/Lgm_KdTree.h"
#include "Lgm/quicksort.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**
 *   Store given N-dimensional data into a D-dimensional KD-tree data structure.
 *
 *   Given arrays of positions and data, this routine recursively partitions
 *   the data into a kdtree data structure. 
 *
 *      \param[in]      Points      An array of position vectors in D-dimensional space. ObjectPoints[d][n] is the dth component of the nth point.
 *      \param[in]      Objects     An array of objects in D-dimensional space. Objects[n] is the nth pointer to an object.
 *      \param[in]      N           Number of points.
 *      \param[in]      D           Number of dimensions.
 *
 *      \returns        returns a pointer to the a KdTree structure. User is
 *                      responsible to freeing this with Lgm_FreeKdTree( )
 *
 *      \author         Mike Henderson
 *      \date           2013
 *
 */
Lgm_KdTree *Lgm_KdTree_Init( double **Positions, void **Objects, unsigned long int N, int D ) {


    int                 d;
    unsigned long int   j;
    Lgm_KdTreeNode      *t;
    Lgm_KdTree          *kt;

    /*
     * Initially dump all of the data into a single node.
     */
    t = Lgm_CreateKdTreeRoot( D );

    t->nData      = N;
    t->nDataBelow = t->nData;
    t->Data       = (Lgm_KdTreeData *) calloc( t->nData, sizeof( Lgm_KdTreeData ) );


    /*
     *  Loop over number of data points. Also keep track of ranges in each dimension.
     */
    for (j=0; j<t->nData; j++){

        t->Data[j].Position = (double *) calloc( D, sizeof(double) );

        t->Data[j].Id = j;
        for (d=0; d<D; d++) {
            if (Positions[d][j] < t->Min[d] ) t->Min[d] = Positions[d][j];
            if (Positions[d][j] > t->Max[d] ) t->Max[d] = Positions[d][j];
            t->Data[j].Position[d] = Positions[d][j];
        }
        //t->Data[j].Object = Objects[j];

    }
    for (d=0; d<D; d++) t->Diff[d] = t->Max[d] - t->Min[d];




    kt = (Lgm_KdTree *) calloc( 1, sizeof( Lgm_KdTree) );

    kt->kNN_Lookups   = 0;
    kt->SplitStrategy = LGM_KDTREE_SPLIT_MAXRANGE;
    
    Lgm_KdTree_SubDivideVolume( t, kt );
    kt->Root = t;

    kt->PQ = Lgm_pQueue_Create( 5000 );

    return( kt );
    

}



/**
 *  Create a root-level node for an kdtree.
 *
 *
 *      \returns        void
 *
 *      \author         Mike Henderson
 *      \date           2013
 *
 */
Lgm_KdTreeNode *Lgm_CreateKdTreeRoot( int D ) {

    int              d;
    Lgm_KdTreeNode  *Node;

    /*
     * allocate
     */
    Node = (Lgm_KdTreeNode *) calloc( 1, sizeof( Lgm_KdTreeNode ) );
    Node->nData  = 0;
    Node->D      = D;
    Node->d      = -1; // no split dimension yet
    Node->Parent = NULL;
    Node->Level  = KDTREE_ROOT_LEVEL;

    Node->Min    = (double *) calloc( D, sizeof( double ) );
    Node->Max    = (double *) calloc( D, sizeof( double ) );
    Node->Diff   = (double *) calloc( D, sizeof( double ) );

    for ( d=0; d<D; d++ ) {
        Node->Min[d]  =  9e99;
        Node->Max[d]  = -9e99;
        Node->Diff[d] =  9e99;
    }

    return( Node );

}



/**
 *  Recursively subdivide volume. Stop when the number of data points per node
 *  is lower than the threshold given by KDTREE_MAX_DATA_PER_NODE.
 *
 *    \param[in]     Node in the kdtree to subdivide.
 *
 *    \returns       void
 *
 *    \author        Mike Henderson
 *    \date          2013
 *
 */
void Lgm_KdTree_SubDivideVolume( Lgm_KdTreeNode *t, Lgm_KdTree *kt ) {

    int                 Level, q, d, D, NewLevel;
    unsigned long int   j, nLeft, nRight;
    double             *Pos;
    double               V, c, Diff, MaxDiff;
    unsigned long int   *Idx;


    /*
     *  get dimension and set new Level
     */
    D        = t->D;
    Level    = t->Level;
    NewLevel = Level + 1;


    /*
     * Determine dimension to split on.
     * Could do;
     */
    if        ( kt->SplitStrategy == LGM_KDTREE_SPLIT_SEQUENTIAL ) {

        /*
         * Split sequential on each dimesion
         */
        q = Level%D;

    } else if ( kt->SplitStrategy == LGM_KDTREE_SPLIT_RANDOM ) {

        /*
         * Split randomly on each dimesion
         */
        q = (int)( rand()/(double)RAND_MAX*D );

    } else if ( kt->SplitStrategy == LGM_KDTREE_SPLIT_MAXRANGE ) {

        /*
         * Split on dimesion with largest range of values
         */
        MaxDiff = -1.0; q = 0;
        for (d=0; d<D; d++){
            if ( t->Diff[d] > MaxDiff ) {
                MaxDiff = t->Diff[d];
                q = d;
            }
        }
        //printf("Level = %d Split on %d dimension. MaxDiff = %g\n", Level, q, MaxDiff);

    } else {

        printf(" Unknown value of SplitStrategy, got %d (?)\n", kt->SplitStrategy );

    }



    /*
     * Sort the data in the q-dimension.
     */
    Pos = (double *) calloc( t->nData, sizeof( double ) );
    Idx = (unsigned long int *) calloc( t->nData, sizeof( unsigned long int ) );
    for ( j=0; j<t->nData; j++ ) {
        Idx[j] = j;
        Pos[j] = t->Data[j].Position[q];
    }
    quicksort2uli( t->nData, Pos-1, Idx-1 );


    /*
     *  Create new left and right nodes.
     */
    t->Left  = (Lgm_KdTreeNode *) calloc( 1, sizeof( Lgm_KdTreeNode ) );
    t->Right = (Lgm_KdTreeNode *) calloc( 1, sizeof( Lgm_KdTreeNode ) );



    /*
     * Assign first half of the sorted data to the left, and the rest to the right node.
     */
    nRight = t->nData/2;
    nLeft  = t->nData - nRight;
    //V = 0.5*(Pos[nLeft] + Pos[nRight]);
    //V = Pos[nLeft];
    //printf("Split at: %g %g %g\n", Pos[nLeft], Pos[nRight], V );

    t->Left->Data       = (Lgm_KdTreeData *) calloc( nLeft, sizeof( Lgm_KdTreeData ) );
    t->Left->Min        = (double *) calloc( D, sizeof( double ) );
    t->Left->Max        = (double *) calloc( D, sizeof( double ) );
    t->Left->Diff       = (double *) calloc( D, sizeof( double ) );
    t->Left->nData      = nLeft;
    t->Left->nDataBelow = nLeft;
    t->Left->Level      = NewLevel;
    t->Left->Left       = NULL;
    t->Left->Right      = NULL;
    t->Left->D          = D;

/*
    for ( d=0; d<D; d++ ){
        t->Left->Min[d]  = t->Min[d];
        t->Left->Max[d]  = t->Max[d];
        t->Left->Diff[d] = t->Diff[d];
    }
    t->Left->Min[q]  = Pos[0];
    t->Left->Max[q]  = Pos[nLeft-1];
    t->Left->Diff[q] = t->Left->Max[q] - t->Left->Min[q];
*/
    for (d=0; d<D; d++) {
        t->Left->Min[d] =  9e99;
        t->Left->Max[d] = -9e99;
    }
    for (j=0; j<nLeft; j++){
        t->Left->Data[j].Position = (double *) calloc( D, sizeof(double) );
        t->Left->Data[j].Id = t->Data[ Idx[j] ].Id;
        for (d=0; d<D; d++) {
            c = t->Data[ Idx[j] ].Position[d];
            if ( c < t->Left->Min[d] ) t->Left->Min[d] = c;
            if ( c > t->Left->Max[d] ) t->Left->Max[d] = c;
            t->Left->Data[j].Position[d] = c;
        }
        t->Left->Data[j].Object = t->Data[ Idx[j] ].Object;
    }
    for (d=0; d<D; d++) t->Left->Diff[d] = t->Left->Max[d] - t->Left->Min[d];









    t->Right->Data       = (Lgm_KdTreeData *) calloc( nRight, sizeof( Lgm_KdTreeData ) );
    t->Right->Min        = (double *) calloc( D, sizeof( double ) );
    t->Right->Max        = (double *) calloc( D, sizeof( double ) );
    t->Right->Diff       = (double *) calloc( D, sizeof( double ) );
    t->Right->nData      = nRight;
    t->Right->nDataBelow = nRight;
    t->Right->Level      = NewLevel;
    t->Right->Left       = NULL;
    t->Right->Right      = NULL;
    t->Right->D          = D;
/*
    for ( d=0; d<D; d++ ){
        t->Right->Min[d]  = t->Min[d];
        t->Right->Max[d]  = t->Max[d];
        t->Right->Diff[d] = t->Diff[d];
    }
    t->Right->Min[q]  = Pos[nLeft];
    t->Right->Max[q]  = Pos[t->nData-1];
    t->Right->Diff[q] = t->Right->Max[q] - t->Right->Min[q];
*/
    for (d=0; d<D; d++) {
        t->Right->Min[d] =  9e99;
        t->Right->Max[d] = -9e99;
    }
    for (j=0; j<nRight; j++){
        t->Right->Data[j].Position = (double *) calloc( D, sizeof(double) );
        t->Right->Data[j].Id = t->Data[ Idx[j+nLeft] ].Id;
        for (d=0; d<D; d++) {
            c = t->Data[ Idx[j+nLeft] ].Position[d];
            if ( c < t->Right->Min[d] ) t->Right->Min[d] = c;
            if ( c > t->Right->Max[d] ) t->Right->Max[d] = c;
            t->Right->Data[j].Position[d] = c;
        }
        t->Right->Data[j].Object = t->Data[ Idx[j+nLeft] ].Object;
    }
    for (d=0; d<D; d++) t->Right->Diff[d] = t->Right->Max[d] - t->Right->Min[d];
    
//for (d=0; d<D; d++)
//printf("Left:  npnts: %ld   q: %d  Min[%d] Max[%d] = %g %g\n", nLeft, q, d, d, t->Left->Min[d], t->Left->Max[d] );
//printf("\n");
    //printf("Left:  npnts: %ld   Min[%d] Max[%d] = %g %g\n", nLeft, q, q, t->Left->Min[q], t->Left->Max[q] );
    //if ( nLeft < 4 ) {
    //    for (j=0; j<nLeft; j++){
    //        printf(" ( %g, %g, %g )", t->Left->Data[j].Position[0], t->Left->Data[j].Position[1], t->Left->Data[j].Position[2]);
    //    }
    //    printf("\n");
    //}
//for (d=0; d<D; d++)
//printf("Right:  npnts: %ld   q: %d  Min[%d] Max[%d] = %g %g\n", nRight, q, d, d, t->Right->Min[d], t->Right->Max[d] );
//printf("\n");
    //printf("Right: npnts: %ld   Min[%d] Max[%d] = %g %g\n", nRight, q, q, t->Right->Min[q], t->Right->Max[q] );
    //if ( nRight < 4 ) {
    //    for (j=0; j<nRight; j++){
    //        printf(" ( %g, %g, %g )", t->Right->Data[j].Position[0], t->Right->Data[j].Position[1], t->Right->Data[j].Position[2]);
    //    }
     //   printf("\n");
    //}
    //printf("\n");


    free( Pos );
    free( Idx );


    /*
     * Zero the Data count in this node.
     * Free memory allocated to Parent data field -- its not a leaf anymore.
     */
    for (j=0; j<t->nData; j++) free( t->Data[j].Position );
    free( t->Data ); t->Data = NULL;
    t->nDataBelow = t->nData;
    t->nData = 0;


    /*
     *  Subdivide if there are too many objects in a node
     */
    if ( ( t->Left->Level  < KDTREE_MAX_LEVEL ) && ( t->Left->nData  > KDTREE_MAX_DATA_PER_NODE ) ) Lgm_KdTree_SubDivideVolume( t->Left, kt );
    if ( ( t->Right->Level < KDTREE_MAX_LEVEL ) && ( t->Right->nData > KDTREE_MAX_DATA_PER_NODE ) ) Lgm_KdTree_SubDivideVolume( t->Right, kt );


    return;

}


/**
 *  Finds the k Nearest Neighbors (kNN) of a query point q given that the set
 *  of data is stored as a KdTree. 
 *
 *    \param[in]     q          Query position (D-dimensional) . I.e. the point we want to find NNs for.
 *    \param[in]     Root       Root node of KdTree.
 *    \param[in]     K          Number of NNs to find.
 *    \param[in]     MaxDist2   Threshold distance^2 beyond which we give up on finding
 *                              NNs.  (i.e. could find them, but we arent interested
 *                              because they'd be too far from our query point to be
 *                              useful).
 *    \param[out]    Kgot       Number of NNs (within MaxDist2) that we actually found.
 *    \param[out]    kNN        List of kNN Data items. Sorted (closest first).
 *
 *    \returns       KDTREE_KNN_SUCCESS         Search succeeded.
 *                   KDTREE_KNN_TOO_FEW_NNS     Search terminated because we couldnt find K NNs that were close enough.
 *                   KDTREE_KNN_NOT_ENOUGH_DATA KdTree doesnt contain enough data points.
 *
 *    \author        Mike Henderson
 *    \date          2013
 *
 */
int Lgm_KdTree_kNN( double *q_in, int D, Lgm_KdTree *KdTree, int K, int *Kgot, double MaxDist2, Lgm_KdTreeData *kNN ) {

    int                  k, d, done;
    Lgm_KdTreeNode      *Root, *p;
    double              *q, dist;
    Lgm_pQueue          *PQ;
    Lgm_pQueue_Node     New, TopItem;


    PQ = KdTree->PQ;

    q = (double *) calloc( D, sizeof(double) );



    Root = KdTree->Root;
    *Kgot = 0;


    /*
     * Check to see if there are enough points.
     * Bailout with error flag -1 if not.
     */
    if ( Root->nDataBelow < K ) return( KDTREE_KNN_NOT_ENOUGH_DATA );


    /*
     *  Add Root Node to the Priority Queue.
     */
    for (d=0; d<D; d++) q[d] = q_in[d];
    dist = Lgm_KdTree_MinDist( Root, q );
    if ( dist > MaxDist2 ) return(0);


    /*
     * Create Priority Queue.
     */
    //PQ = Lgm_pQueue_Create( 500 );

    New.key  = dist;  // lowest dist is highest priority
    New.Data = (void *)Root;
    Lgm_pQueue_Insert( &New, PQ );




    /*
     *  Process items on the Priority Queue until we are done.
     */
    k    = 0;       // havent found any NNs yet.
    done = FALSE;
    while( !done ) {

        //Lgm_KdTree_PrintPQ( PQ ); //only for debugging


        /*
         * Pop the highest priority item off of the Priority Queue.
         * This is the closest object to the query point. Note that
         * it could be a cell or it could be a point.
         */

        if ( Lgm_pQueue_Pop( &TopItem, PQ ) ) {

            dist = TopItem.key;
            p    = (Lgm_KdTreeNode *)TopItem.Data;

            if ( p->nData == 0 ) {

                /*
                 * If the object is node, then descend one level closer to
                 * the leaf node that is closest to the query point. This also
                 * adds all nodes we encounter along the way to the PQ. Ignore
                 * any nodes that is farther than MaxDist2 away from query point.
                 */
                Lgm_KdTree_DescendTowardClosestLeaf( p, PQ, q, MaxDist2 );

            } else {

                /*
                 * Since a point is now the closest object, it must be one of
                 * the kNN.  But, if its too far away, then we are done with
                 * the search -- we couldnt find K NNs close enough to the
                 * query point
                 */
                if ( dist > MaxDist2 ) {
                    //Lgm_pQueue_Destroy( PQ );
                    free( q );
                    return( KDTREE_KNN_TOO_FEW_NNS );
                }

                /*
                 * Otherwise, add this point as the next NN and continue.
                 */
                kNN[ k   ]       = p->Data[0];
                kNN[ k++ ].Dist2 = dist; // save the dist2 into the data struct
                *Kgot = k;

            } 

        } else {

            // The priority queue is empy -- there is nothing more to search.
            done = TRUE;

        }

        if ( k >= K ) done = TRUE;


        //Lgm_KdTree_PrintPQ( &PQ ); //only for debugging

    }


    //Lgm_KdTree_PrintPQ( &PQ ); //only for debugging

    ++(KdTree->kNN_Lookups);


    /*
     *  return success
     */
    //Lgm_pQueue_Destroy( PQ );
    free( q );
    return( KDTREE_KNN_SUCCESS );

}












/**
 *  This routine computes the minimum distance between a point and a KdTree
 *  node (each node maintains a region of D-dimensional space). Basically, we
 *  compute the closest distance between the query point and any point on the
 *  hyper-rectangle defined in the node.
 *
 *      \param[in]      Node    Pointer to a node in the kdtree
 *      \param[in]      q       The D-dimensional query point.
 *
 *      \returns        The minimum distance between the point and the node.
 *
 *      \author         Mike Henderson
 *      \date           2013
 *
 */
double  Lgm_KdTree_MinDist( Lgm_KdTreeNode *Node, double *q ) {

    int     d, D;
    double  distance2, delta;

    D = Node->D;

    /*
     * The region in each node is defined by the Min[] and Max[] arrays.
     * E.g., Min[0]/Max[0] is the bounds in the zeroeth dimension, etc.
     */

    for ( distance2=0.0, d=0; d<D; d++ ) {
        if      ( q[d] < Node->Min[d] ) { delta = Node->Min[d] - q[d]; distance2 += delta*delta; }
        else if ( q[d] > Node->Max[d] ) { delta = q[d] - Node->Max[d]; distance2 += delta*delta; }
    }

    //printf("Lgm_KdTree_MinDist(): distance2 = %g\n", distance2);
    return( distance2 );

}

/**
 *   Descend to the leaf node that is closest to the query point. This routine
 *   is used by Lgm_KdTree_kNN().
 *
 *      \param[in]      Node        Pointer to a node in the kdtree
 *      \param[in]      PQ          The "priority queue"
 *      \param[in]      q           The D-dim query point.
 *      \param[in]      MaxDist2    The maximum distance (squared) to care
 *                                  about. Square distances beyond this value are ignored.
 *
 *      \returns        pointer one level closer to leaf node that is closest to query point.
 *
 *      \author         Mike Henderson
 *      \date           2013
 *
 */
void Lgm_KdTree_DescendTowardClosestLeaf( Lgm_KdTreeNode *Node, Lgm_pQueue *PQ, double *q, double MaxDist2 ) {

    double              dL, dR;
    Lgm_KdTreeNode     *L, *R;
    Lgm_pQueue_Node     New;


    L = Node->Left;
    R = Node->Right;



    if ( L && R ) {

        /*
         * This is not a leaf node -- should be no points contained in this
         * node. There are both left and right subtrees defined.  Throw both on
         * the queue for further processing. Make sure the closest one is
         * searched first.
         */

        dL = Lgm_KdTree_MinDist( L, q );
        dR = Lgm_KdTree_MinDist( R, q );
        if ( dL < dR ) {
            if (dL < MaxDist2) { New.key  = dL; New.Data = (void *)L; Lgm_pQueue_Insert( &New, PQ ); }
//            if (dR < MaxDist2) { New.key  = dR; New.Data = (void *)R; Lgm_pQueue_Insert( &New, PQ ); }
        } else {
            if (dR < MaxDist2) { New.key  = dR; New.Data = (void *)R; Lgm_pQueue_Insert( &New, PQ ); }
//            if (dL < MaxDist2) { New.key  = dL; New.Data = (void *)L; Lgm_pQueue_Insert( &New, PQ ); }
        }

    } else if ( L ) {

        /*
         * This is not a leaf node -- should be no points contained in this
         * node. Only left subtree defined.  Throw both on the queue for
         * further processing. 
         */
        dL = Lgm_KdTree_MinDist( L, q );
        if (dL < MaxDist2) { New.key  = dL; New.Data = (void *)L; Lgm_pQueue_Insert( &New, PQ ); }

    } else if ( R ) {

        /*
         * This is not a leaf node -- should be no points contained in this
         * node. Only right subtree defined.  Throw both on the queue for
         * further processing. 
         */
        dR = Lgm_KdTree_MinDist( R, q );
        if (dR < MaxDist2) { New.key  = dR; New.Data = (void *)R; Lgm_pQueue_Insert( &New, PQ ); }

    }


    return;
}

/**
 *   Prints the contents of the priority queue. For dubugging only. This routine
 *   is used by Lgm_KdTree_kNN().
 *
 *      \param[in]  PQ          The "priority queue".
 *
 *      \returns        void
 *
 *      \author         Mike Henderson
 *      \date           2013
 *
 */
void Lgm_KdTree_PrintPQ( Lgm_pQueue *PQ ) {

    long int         i=0;
    double           dist;
    Lgm_pQueue_Node  *p;
    Lgm_KdTreeNode   *N;



    printf("---- Contents of Priority Queue  ---------\n");
    for ( i=1; i<PQ->HeapSize; i++ ) {

        p = &(PQ->HeapArray[i]);
        

        dist = p->key;
        N    = (Lgm_KdTreeNode *)p->Data;

        if ( N ) {
        if ( N->nData > 0 ) {
            printf("Point: MinDist2 = %g\n", dist );
        } else {
            printf("Node: MinDist2 = %g\n", dist );
        }
        }

    }

    printf("\n\n");
}


