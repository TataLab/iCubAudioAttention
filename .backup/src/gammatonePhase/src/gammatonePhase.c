
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mex.h"
#include "matrix.h"


/*=======================
 * Input arguments
 *=======================
 */
#define IN_x        prhs[0]
#define IN_P        prhs[1]
#define IN_fs       prhs[2]
#define IN_cf       prhs[3]
#define IN_align    prhs[4]
#define IN_hrect    prhs[5]

/*=======================
 * Output arguments
 *=======================
 */
#define OUT_bm      plhs[0] /* Basilar membrane response */
#define OUT_P       plhs[1] /* Last values of filter coefficients */
#define OUT_env     plhs[2] /* Instantaneous Hilbert envelope */
#define OUT_instp   plhs[3] /* Instantaneous phase */
#define OUT_instf   plhs[4] /* Instantaneous frequency */

/*=======================
 * Useful Const
 *=======================
 */
#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-200
#ifndef M_PI
#define M_PI               3.141592653589
#endif

/*=======================
 * Utility functions
 *=======================
 */
#define myMax(x,y)     ( ( x ) > ( y ) ? ( x ) : ( y ) )
#define myMod(x,y)     ( ( x ) - ( y ) * floor ( ( x ) / ( y ) ) )
#define erb(x)         ( 24.7 * ( 4.37e-3 * ( x ) + 1.0 ) )

/*=======================
 * Main Function
 *=======================
 */
 //  [bm, env, instp, instf] = gammatone_c(x, P_in, fs, cf, align, hrect) 
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
	double *x=0, *cf, *bm, *env = 0, *instp = 0, *instf = 0 ,*P_in=0, *P_out=0;
	int i, j, t, fs, nsamples, hrect, ch, Channels, align=0, intshift=0;
	double  xx=0,a, tpt, tptbw, gain, envelopecomptime=0, phasealign=0;
	double p0r, p1r, p2r, p3r, p4r, p0i, p1i, p2i, p3i, p4i;
	double a1, a2, a3, a4, a5, u0r, u0i; /*, u1r, u1i;*/
	double qcos, qsin, oldcs, coscf, sincf, oldphase, dp, dps;

	/*=========================================
	 * Check argument numbers
	 *=========================================
	 */
	if (nrhs < 4) {
		mexPrintf("??? Not enough input arguments.\n");
		return;
	}
	if (nrhs > 6) {
		mexPrintf("??? Too many input arguments.\n");
		return;
	}
	
	/*=========================================
	 * input arguments
	 *=========================================
	 */
	if (nrhs < 6) {
		hrect = 0;
	}
	else {
		hrect = (int)mxGetScalar(IN_hrect);
	}
	
	if (nrhs < 5) {
		align = 0; //default is align=off or zero
	}
	else {
		align = (int)mxGetScalar(IN_align);
	}

	Channels = mxGetN(IN_cf);//number of channels
	x = mxGetPr(IN_x);   /* waveform */
	i = mxGetN(IN_x);
	j = mxGetM(IN_x);
	if (i > 1 && j > 1) {
		mexPrintf("??? Input x must be a vector.\n");
		return;
	}
	nsamples = myMax(i, j);
	fs = (int)mxGetScalar(IN_fs);   /* sampling rate */
	cf = mxGetPr(IN_cf);   /* centre frequency */
	P_in= mxGetPr ( IN_P ); 
   /*=========================================
	* output arguments
	*=========================================
	*/
	OUT_bm = mxCreateDoubleMatrix( nsamples,Channels ,mxREAL);
	bm = mxGetPr(OUT_bm);
	if (nlhs > 1) {
		OUT_env = mxCreateDoubleMatrix(nsamples,Channels, mxREAL);
		env = mxGetPr(OUT_env);
	}
	if ( nlhs > 2 ) {
      OUT_P = mxCreateDoubleMatrix ( 8, Channels, mxREAL );
      P_out = mxGetPr ( OUT_P );
   }
	if (nlhs > 3) {
		OUT_instp = mxCreateDoubleMatrix(nsamples,Channels, mxREAL);
		instp = mxGetPr(OUT_instp);
	}
	if (nlhs > 4) {
		OUT_instf = mxCreateDoubleMatrix(nsamples,Channels, mxREAL);
		instf = mxGetPr(OUT_instf);
	}


	for (ch = 0; ch < Channels; ch++)
	{
		/*=========================================
		 * Initialising variables
		 *=========================================
		 */

		oldphase = 0.0;
		tpt = (M_PI + M_PI) / fs;
		tptbw = tpt * erb(cf[ch]) * BW_CORRECTION;
		a = exp(-tptbw);
		
		if (align)
		{
			//B=1.019*2*M_PI*erb(cf);
			envelopecomptime = 3/(BW_CORRECTION*2*M_PI*erb(cf[ch]));
		}
		else
		{
			envelopecomptime = 0;
		}
        	
		intshift=(int)(floor(envelopecomptime*fs));
        
//         
// 		phasealign=-2*M_PI*cf[ch]*envelopecomptime;
// 		//phasealign=mod(phasealign,2*M_PI);
// 		phasealign=fmod(phasealign,2*M_PI);
//         //phasealign=myMod(phasealign,2*M_PI);
// 		phasealign=phasealign/(2*M_PI*cf[ch]);
	
		
		
		
		/* based on integral of impulse response */
		gain = (tptbw*tptbw*tptbw*tptbw) / 3;

		/* Update filter coefficients */
		a1 = 4.0*a; a2 = -6.0*a*a; a3 = 4.0*a*a*a; a4 = -a*a*a*a; a5 = a*a;
		p0r = 0.0;
 		p1r = P_in[0+ch*8]; p2r =P_in[1+ch*8]; p3r = P_in[2+ch*8]; p4r = P_in[3+ch*8];
  		 //P[0]=p1r;P[1]=p2r;P[2]=p3r;P[3]=p4r;P[4]=p1i;P[5]=p2i;P[6]=p3i;P[7]=p4i;
 	    p0i = 0.0;
 	    p1i = P_in[4+ch*8]; p2i = P_in[5+ch*8]; p3i = P_in[6+ch*8]; p4i = P_in[7+ch*8];

		/*===========================================================
		 * exp(a+i*b) = exp(a)*(cos(b)+i*sin(b))
		 * q = exp(-i*tpt*cf*t) = cos(tpt*cf*t) + i*(-sin(tpt*cf*t))
		 * qcos = cos(tpt*cf*t)
		 * qsin = -sin(tpt*cf*t)
		 *===========================================================
		 */
		coscf = cos(tpt * cf[ch]);
		sincf = sin(tpt * cf[ch]);
		qcos = 1; qsin = 0;   /* t=0 & q = exp(-i*tpt*t*cf)*/
		for (t = 0; t < (nsamples); t++)
		{
			if (t>(nsamples-1))
			{
			xx=0;	
				
			}
            else
            {
             xx=x[t];   
            }   
			
			/* Filter part 1 & shift down to d.c. */
			p0r = qcos*xx + a1*p1r + a2*p2r + a3*p3r + a4*p4r;
			p0i = qsin*xx + a1*p1i + a2*p2i + a3*p3i + a4*p4i;

			/* Clip coefficients to stop them from becoming too close to zero */
			if (fabs(p0r) < VERY_SMALL_NUMBER)
				p0r = 0.0F;
			if (fabs(p0i) < VERY_SMALL_NUMBER)
				p0i = 0.0F;

			/* Filter part 2 */
			u0r = p0r + a1*p1r + a5*p2r;
			u0i = p0i + a1*p1i + a5*p2i;

			/* Update filter results */
			p4r = p3r; p3r = p2r; p2r = p1r; p1r = p0r;
			p4i = p3i; p3i = p2i; p2i = p1i; p1i = p0i;


			 
			if (t>(intshift-1))
			{
			
			
			 
			 
			//bm[t + ch*(nsamples)-ch*(intshift)] = (u0r * qcos + u0i * qsin) * gain
			bm[t + ch*(nsamples)-(intshift)] =(u0r * (qcos * cos(2*M_PI*cf[ch]*phasealign) + qsin* sin(2*M_PI*cf[ch]*phasealign))+ u0i*(qsin*cos(2*M_PI*cf[ch]*phasealign)-qcos*sin(2*M_PI*cf[ch]*phasealign)) )* gain;

			if (1 == hrect && bm[t + ch*(nsamples)-(intshift)] < 0) {
				bm[t + ch*(nsamples)-(intshift)] = 0;  /* half-wave rectifying */
			}

			 if ( nlhs > 1 ) {
      		   P_out[0+ch*8]=p1r;P_out[1+ch*8]=p2r;P_out[2+ch*8]=p3r;P_out[3+ch*8]=p4r;P_out[4+ch*8]=p1i;P_out[5+ch*8]=p2i;P_out[6+ch*8]=p3i;P_out[7+ch*8]=p4i;
    		  }

			/*==========================================
			 * Instantaneous Hilbert envelope
			 * env = abs(u) * gain;
			 *==========================================
			 */
			if (nlhs > 2) {
				env[t + ch*(nsamples)-(intshift)] = sqrt(u0r * u0r + u0i * u0i) * gain;
			}
			/*==========================================
			 * Instantaneous phase
			 * instp = unwrap(angle(u));
			 *==========================================
			 */
			if (nlhs > 3) {
				instp[t + ch*(nsamples)-(intshift)] = atan2(u0i, u0r);
				/* unwrap it */
				dp = instp[t + ch*(nsamples)-(intshift)] - oldphase;
				if (abs(dp) > M_PI) {
					dps = myMod(dp + M_PI, 2 * M_PI) - M_PI;
					if (dps == -M_PI && dp > 0) {
						dps = M_PI;
					}
					instp[t + ch*(nsamples)-(intshift)] = instp[t + ch*(nsamples)-(intshift)] + dps - dp;
				}
				oldphase = instp[t + ch*(nsamples)-(intshift)];
			}
			/*==========================================
			 * Instantaneous frequency
			 * instf = cf + [diff(instp) 0]./tpt;
			 *==========================================
			 */
			if (nlhs > 4 && t > (intshift)) {
				instf[t - 1 + ch*(nsamples)-(intshift)] = cf[ch] + (instp[t + ch*(nsamples)-(intshift)] - instp[t - 1 + ch*(nsamples)-(intshift)]) / tpt;
			}
// 
// 			/*====================================================
// 			 * The basic idea of saving computational load:
// 			 * cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)
// 			 * sin(a+b) = sin(a)*cos(b) + cos(a)*sin(b)
// 			 * qcos = cos(tpt*cf*t) = cos(tpt*cf + tpt*cf*(t-1))
// 			 * qsin = -sin(tpt*cf*t) = -sin(tpt*cf + tpt*cf*(t-1))
// 			 *====================================================
// 			 */
			 
			 } //t>(intshift-1)
			 
			qcos = coscf * (oldcs = qcos) + sincf * qsin;
			qsin = coscf * qsin - sincf * oldcs;
			
			
			
		}//samples for loop


		if (nlhs > 4) {
			//instf[(ch+1)*nsamples - 1] = cf[ch];
            instf[nsamples - 1] = cf[ch];
		}

}//Channels for loop
		return;
	
} //mexFunction
/* end */
