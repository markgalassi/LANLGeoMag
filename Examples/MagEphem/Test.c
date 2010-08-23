#include <Lgm_CTrans.h>
#include <Lgm_MagEphemInfo.h>
const char *sMonth[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };



/*
main(){

    FILE        *fp;

    fp = fopen( "puke.txt", "wb");
    WriteMagEphemHeader( fp, "mgh", "mithril" );
    WriteMagEphemData( fp );

}
*/


void WriteMagEphemHeader( FILE *fp, char *UserName, char *Machine, Lgm_MagEphemInfo *m ){

    int         i, Year, Month, Day, HH, MM, SS;
    char        Str[80];
    long int    CreateDate;
    double      JD, UTC;
    Lgm_CTrans  *c = Lgm_init_ctrans(0);

    int         nAlpha;
    double      a, Alpha[100];

    for (nAlpha=0,a=5.0; a<=90.0; a+=5,++nAlpha) Alpha[nAlpha] = a;


    JD = Lgm_GetCurrentJD(c);
    CreateDate = Lgm_JD_to_Date( JD, &Year, &Month, &Day, &UTC );
    Lgm_UT_to_HMS( UTC, &HH, &MM, &SS );

    /*
     * Write Header
     */
    fprintf( fp, "# Spacecraft:  %s\n", "RBSPA" );
    fprintf( fp, "# Field Model:  %s\n", "T89" );
    fprintf( fp, "# nAlpha:  %d; ", m->nAlpha ); for (i=0; i<m->nAlpha; i++) fprintf(fp, " %g", m->Alpha[i]); fprintf( fp, ";   Units: Degrees\n");
    fprintf( fp, "#\n");
    fprintf( fp, "# File Contents    :  L* values and Phase Space Densities at Constant Mu and K\n");
    fprintf( fp, "# File Created at  :  %02d:%02d:%02d UTC  %s %02d %4d\n", HH, MM, SS, sMonth[Month], Day, Year );
    fprintf( fp, "# File Created by  :  %s\n", UserName );
    fprintf( fp, "# File Created on  :  %s\n", Machine );
    fprintf( fp, "# File Fmt Version :  %s\n", "" );
    fprintf( fp, "#\n");
    fprintf( fp, "# Description of Variables:\n");
    fprintf( fp, "#     Date:           The date. In YYYMMDD format.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     UTC:            Universal Time (Coordinated). In decimal hours.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     DateTime:       The date and time in ISO 8601 compliant format.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Xgeo:            X-component of Geographic position of S/C. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Ygeo:            Y-component of Geographic position of S/C. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Zgeo:            Z-component of Geographic position of S/C. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Xgsm:           X-compontent of GSM position vector. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Ygsm:           Y-compontent of GSM position vector. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Zgsm:           Z-compontent of GSM position vector. In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     B_Mod:          Magnetic field strength given by the field model (in nT)\n");
    fprintf( fp, "#                     In units of nT.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     M_Used:         The Magnetic Dipole Moment that was used to convert\n");
    fprintf( fp, "#                     magnetic flux to L*. In units of nT.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     M_Ref:          A fixed reference magnetic dipole moment for converting\n");
    fprintf( fp, "#                     magnetic flux to L*. In units of nT.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     M_IGRF:         Time-dependant magnetic dipole moment (probably shouldn't\n");
    fprintf( fp, "#                     be used for converting Magnetic Flux to L*, but it\n");
    fprintf( fp, "#                     sometimes is). In units of nT.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     L*0-(nAlpha-1): The nAlpha L* values obtained. L* values are dimensionless quantities.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     Bm0-(nAlpha-1): The nAlpha Bmirror values computed. In units of nT.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     I0-(nAlpha-1):  The nAlpha Integral Invariant values computed.  In units of Re.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#     L-(nAlpha-1):   The nAlpha McIlwain L values computed.  Dimensionless.\n");
    fprintf( fp, "#\n");
    fprintf( fp, "#\n");

    // Column Header
    fprintf( fp, "%51s",  "# +----------------- Date and Time ----------------+" );
    fprintf( fp, " %38s",  " +----- Geographic Coordinates ------+" );
    fprintf( fp, " %38s",  " +--------- GSM Coordinates ---------+" );
    fprintf( fp, " %38s",  " +---------- SM Coordinates ---------+" );
    fprintf( fp, " %38s",  " +------- GEI 2000 Coordinates ------+" );
    fprintf( fp, " %38s",  " +---------- GSE Coordinates --------+" );
    fprintf( fp, " %29s",  " +----- Field Line Type ----+" );
    fprintf( fp, " %38s",  " +---- North Mag. Footpoint GSM -----+" );
    fprintf( fp, " %38s",  " +- North Mag. Footpoint Geographic -+" );
    fprintf( fp, " %38s",  " +-- North Mag. Footpoint Geodetic --+" );
    fprintf( fp, " %38s",  " +---- South Mag. Footpoint GSM -----+" );
    fprintf( fp, " %38s",  " +- South Mag. Footpoint Geographic -+" );
    fprintf( fp, " %38s",  " +-- South Mag. Footpoint Geodetic --+" );
    fprintf( fp, " %38s",  " +----- Minimum |B| point GSM -------+" );

    fprintf(fp, "\n");


    // Column Header
    fprintf( fp, "# %-10s", "Date" );
    fprintf( fp, " %13s", "UTC" );
    fprintf( fp, " %25s", "DateTime" );

    fprintf( fp, " %12s", "Xgeo" );
    fprintf( fp, " %12s", "Ygeo" );
    fprintf( fp, " %12s", "Zgeo" );

    fprintf( fp, " %12s", "Xgsm" );
    fprintf( fp, " %12s", "Ygsm" );
    fprintf( fp, " %12s", "Zgsm" );

    fprintf( fp, " %12s", "Xsm" );
    fprintf( fp, " %12s", "Ysm" );
    fprintf( fp, " %12s", "Zsm" );

    fprintf( fp, " %12s", "Xgei" );
    fprintf( fp, " %12s", "Ygei" );
    fprintf( fp, " %12s", "Zgei" );

    fprintf( fp, " %12s", "Xgse" );
    fprintf( fp, " %12s", "Ygse" );
    fprintf( fp, " %12s", "Zgse" );

    fprintf( fp, " %29s",  "" );

    fprintf( fp, " %12s", "Xgsm" ); // N. Foot
    fprintf( fp, " %12s", "Ygsm" );
    fprintf( fp, " %12s", "Zgsm" );

    fprintf( fp, " %12s", "Xgeo" );
    fprintf( fp, " %12s", "Ygeo" );
    fprintf( fp, " %12s", "Zgeo" );

    fprintf( fp, " %12s", "GeodLat" );
    fprintf( fp, " %12s", "GeodLon" );
    fprintf( fp, " %12s", "GeodHeight" );

    fprintf( fp, " %12s", "Xgsm" ); // S. Foot
    fprintf( fp, " %12s", "Ygsm" );
    fprintf( fp, " %12s", "Zgsm" );

    fprintf( fp, " %12s", "Xgeo" );
    fprintf( fp, " %12s", "Ygeo" );
    fprintf( fp, " %12s", "Zgeo" );

    fprintf( fp, " %12s", "GeodLat" );
    fprintf( fp, " %12s", "GeodLon" );
    fprintf( fp, " %12s", "GeodHeight" );

    fprintf( fp, " %12s", "Xgsm" ); // |B|min GSM
    fprintf( fp, " %12s", "Ygsm" );
    fprintf( fp, " %12s", "Zgsm" );

    fprintf( fp, " %12s", "B_Mod" );
    fprintf( fp, " %12s", "M_Used" );
    fprintf( fp, " %12s", "M_Ref" );
    fprintf( fp, " %12s", "M_IGRF" );
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "L*%d", i ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "L%d", i ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "Bm%d", i ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "I%d", i ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "\n");
    // Units/Format
    fprintf( fp, "# %-10s", "YYYYMMDD" );
    fprintf( fp, " %13s", "Hours" );
    fprintf( fp, " %25s", "YYYY-MM-DDTHH:MM:SS.SSSSZ" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %29s",  "" ); // FL Type

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Deg." );
    fprintf( fp, " %12s", "Deg." );
    fprintf( fp, " %12s", "km" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "Deg." );
    fprintf( fp, " %12s", "Deg." );
    fprintf( fp, " %12s", "km" );

    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );
    fprintf( fp, " %12s", "Re" );

    fprintf( fp, " %12s", "nT" );
    fprintf( fp, " %12s", "nT" );
    fprintf( fp, " %12s", "nT" );
    fprintf( fp, " %12s", "nT" );
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "dimless" ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "dimless" ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "nT" ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { sprintf( Str, "Re" ); fprintf(fp, " %12s", Str ); }
    fprintf(fp, "\n");

    Lgm_free_ctrans(c);

}


void WriteMagEphemData( FILE *fp, Lgm_MagEphemInfo *m ){

    int             i;
    char            Str[128];
    double          GeodLat, GeodLong, GeodHeight, L;
    Lgm_DateTime    DT_UTC;
    Lgm_CTrans      *c = Lgm_init_ctrans(0);
    Lgm_Vector v;

    Lgm_Set_Coord_Transforms( m->Date, m->UTC, c );
    Lgm_Make_UTC( m->Date, m->UTC, &DT_UTC, c );
    Lgm_DateTimeToString( Str, DT_UTC, 0, 4 );
    

    fprintf( fp, "%10ld  ",   m->Date );  // Date
    fprintf( fp, " %13.8lf", m->UTC );  // UTC
    fprintf( fp, " %25s",  Str );       // Date+Time in ISO 8601 format

    Lgm_Convert_Coords( &m->P, &v, GSM_TO_GEO, c );
    fprintf( fp, " %12g", v.x );        // Xgeo
    fprintf( fp, " %12g", v.y );        // Ygeo
    fprintf( fp, " %12g", v.z );        // Zgeo

    fprintf( fp, " %12g", m->P.x );  // Xgsm
    fprintf( fp, " %12g", m->P.y );  // Ygsm
    fprintf( fp, " %12g", m->P.z );  // Zgsm

    Lgm_Convert_Coords( &m->P, &v, GSM_TO_SM, c );
    fprintf( fp, " %12g", v.x );        // Xsm
    fprintf( fp, " %12g", v.y );        // Ysm
    fprintf( fp, " %12g", v.z );        // Zsm

    Lgm_Convert_Coords( &m->P, &v, GSM_TO_GEI2000, c );
    fprintf( fp, " %12g", v.x );        // Xgei
    fprintf( fp, " %12g", v.y );        // Ygei
    fprintf( fp, " %12g", v.z );        // Zgei

    Lgm_Convert_Coords( &m->P, &v, GSM_TO_GSE, c );
    fprintf( fp, " %12g", v.x );        // Xgse
    fprintf( fp, " %12g", v.y );        // Ygse
    fprintf( fp, " %12g", v.z );        // Zgse

    switch ( m->FieldLineType ) {
        case LGM_OPEN_IMF:
                            fprintf( fp, " %29s",  "LGM_OPEN_IMF" ); // FL Type
                            break;
        case LGM_CLOSED:
                            fprintf( fp, " %29s",  "LGM_CLOSED" ); // FL Type
                            break;
        case LGM_OPEN_N_LOBE:
                            fprintf( fp, " %29s",  "LGM_OPEN_N_LOBE" ); // FL Type
                            break;
        case LGM_OPEN_S_LOBE:
                            fprintf( fp, " %29s",  "LGM_OPEN_S_LOBE" ); // FL Type
                            break;
        case LGM_INSIDE_EARTH:
                            fprintf( fp, " %29s",  "LGM_INSIDE_EARTH" ); // FL Type
                            break;
        case LGM_TARGET_HEIGHT_UNREACHABLE:
                            fprintf( fp, " %29s",  "LGM_TARGET_HEIGHT_UNREACHABLE" ); // FL Type
                            break;
        default:
                            fprintf( fp, " %29s",  "UNKNOWN FIELD TYPE" ); // FL Type
                            break;
    }

    if ( (m->FieldLineType == LGM_CLOSED) || (m->FieldLineType == LGM_OPEN_N_LOBE) ) {

        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Pn.x );     // Xgsm   North Foot
        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Pn.y );     // Ygsm
        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Pn.z );     // Zgsm

        Lgm_Convert_Coords( &m->Ellipsoid_Footprint_Pn, &v, GSM_TO_GEO, c );
        fprintf( fp, " %12g", v.x );                    // Xgeo   North Foot
        fprintf( fp, " %12g", v.y );                    // Ygeo
        fprintf( fp, " %12g", v.z );                    // Zgeo

        Lgm_WGS84_to_GEOD( &v, &GeodLat, &GeodLong, &GeodHeight );
        fprintf( fp, " %12g", GeodLat );                // Geod Lat   North Foot
        fprintf( fp, " %12g", GeodLong );               // Geod Long
        fprintf( fp, " %12g", GeodHeight );             // Geod Height

    } else {

        fprintf( fp, " %12g", 0.0 ); 
        fprintf( fp, " %12g", 0.0 ); 
        fprintf( fp, " %12g", 0.0 );

        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );

        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );

    }


    if ( (m->FieldLineType == LGM_CLOSED) || (m->FieldLineType == LGM_OPEN_S_LOBE) ) {

        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Ps.x );     // Xgsm   South Foot
        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Ps.y );     // Ygsm
        fprintf( fp, " %12g", m->Ellipsoid_Footprint_Ps.z );     // Zgsm

        Lgm_Convert_Coords( &m->Ellipsoid_Footprint_Ps, &v, GSM_TO_GEO, c );
        fprintf( fp, " %12g", v.x );                    // Xgeo   South Foot
        fprintf( fp, " %12g", v.y );                    // Ygeo
        fprintf( fp, " %12g", v.z );                    // Zgeo


        Lgm_WGS84_to_GEOD( &v, &GeodLat, &GeodLong, &GeodHeight );
        fprintf( fp, " %12g", GeodLat );                // Geod Lat   South Foot
        fprintf( fp, " %12g", GeodLong );               // Geod Long
        fprintf( fp, " %12g", GeodHeight );             // Geod Height

    } else {
        fprintf( fp, " %12g", 0.0 ); 
        fprintf( fp, " %12g", 0.0 ); 
        fprintf( fp, " %12g", 0.0 );

        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );

        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
    }
    

    if ( m->FieldLineType == LGM_CLOSED ) {
        fprintf( fp, " %12g", m->Pmin.x );          // Xgsm  Pmin
        fprintf( fp, " %12g", m->Pmin.y );          // Ygsm
        fprintf( fp, " %12g", m->Pmin.z );          // Zgsm
    } else {
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
        fprintf( fp, " %12g", 0.0 );
    }

    fprintf( fp, " %12g", m->B );       // B_Mod
    fprintf( fp, " %12g", m->Mused );   // M_Used
    fprintf( fp, " %12g", m->Mref );    // M_Ref
    fprintf( fp, " %12g", m->Mcurr );   // M_IGRF

    // L*'s
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { fprintf(fp, " %12g", m->Lstar[i] ); }

    // McIlwain L (computed from I, Bm, M)
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { 
        L = ( m->I[i] > 0.0 ) ? LFromIBmM_McIlwain(m->I[i], m->Bm[i], m->Mused ) : 0.0;
        fprintf(fp, " %12g", L);
    }

    // Bms's
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { fprintf(fp, " %12g", m->Bm[i] ); }

    // I's
    fprintf(fp, "    ");
    for (i=0; i<m->nAlpha; i++) { fprintf(fp, " %12g", m->I[i] ); }


    fprintf(fp, "\n");

    
    Lgm_free_ctrans( c );
}
