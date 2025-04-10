#include <iostream>
#include <fstream>
#include <iomanip>
#include <complex>
#include <vector>
#include <iterator>
#include <cmath>

#include <math.h>
#include <float.h>

using namespace std;

#define JT_DEBUG_LOGGING 1
#undef  JT_DEBUG_LOGGING

extern "C" int Linac_FFX(double *pI, double *pQ, int n, double *pIQ)
{
register
	int	i, j;
        double  dphi = 1.25*M_PI;
        double  phi  = 0;

        complex <double> d0(1,0);
        complex <double> d1(1,0);

	short   sh = 0;

#ifdef JT_DEBUG_LOGGING
	ofstream *pOf[1];
	pOf[0] = new ofstream("LinacFF_IQTable.TXT");
        cout << "LN FF - - -" << endl;
#endif

	i = j = 0;
        do { 
          d0 = complex <double>(*(pI+i), *(pQ+i));
          d1 = polar(abs(d0), arg(d0)+phi ); phi += dphi;

          /* For fast FF table, Table.v, don't output Q values! */
	  /* Just advance the phase by 1.25*M_PI                */
          *(pIQ+j) = (short)floor(real(d1)*32763+0.5);

#ifdef JT_DEBUG_LOGGING
///          *pOf[0] << setw(4) << i << " ";
///          *pOf[0] << setprecision(8) << setw(16) << (double)abs(d1) << " ";
///          *pOf[0] << setprecision(8) << setw(16) << (double)arg(d1) << " " << endl;
          *pOf[0] << setw(4) << j << " ";
          *pOf[0] << setprecision(8) << setw(16) << *(pIQ+j) << endl;
#endif

          j++;

          d0 = complex <double>(*(pI+i), *(pQ+i));
          d1 = polar(abs(d0), arg(d0)+phi ); phi += dphi;
          *(pIQ+j) = (short)floor(real(d1)*32763+0.5);

#ifdef JT_DEBUG_LOGGING
          *pOf[0] << setw(4) << j << " ";
          *pOf[0] << setprecision(8) << setw(16) << *(pIQ+j) << endl;
#endif

          j++; if( !(j%8) ) phi = 0;

          i++;
          } while( i < n );

#ifdef JT_DEBUG_LOGGING
	pOf[0]->close();
        delete pOf[0];
#endif

    return 0;
}

