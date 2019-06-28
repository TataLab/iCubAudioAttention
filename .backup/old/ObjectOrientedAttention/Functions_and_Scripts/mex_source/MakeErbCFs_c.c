#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mex.h"
#include "matrix.h"


/*=======================
 * Input arguments
 *=======================
 */
#define IN_Low        prhs[0]
#define IN_High       prhs[1]
#define IN_Number     prhs[2]


/*=======================
 * Output arguments
 *=======================
 */
#define OUT_cf      plhs[0] 



void MakeErbCFs(double mincf, double maxcf,double numchans, double* cf_HZ)
{
void ErbRateToHz(double , double*);
void HzToErbRate(double , double*);
double mincf_ERB,  maxcf_ERB, Delta;
mwSize i;

HzToErbRate(mincf, &mincf_ERB);
HzToErbRate(maxcf, &maxcf_ERB);

Delta=(maxcf_ERB-mincf_ERB)/(numchans-1);

    
    for (i=0; i<numchans; i++) {
      
	 ErbRateToHz(Delta * i+ mincf_ERB, &cf_HZ[i]);
	
    }
	
}


void ErbRateToHz(double x,double* y)
{

*y=(pow(10,x/21.4)-1)/0.00437;// (10^(x/21.4)-1)/0.00437;
}




void HzToErbRate(double x,double* y)
{

*y=(21.4*log10(0.00437*x+1));
}




/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    double Low_Limit;              /* input scalar */
    double High_Limit;              /* input scalar 2 */
	double N_Filter;              /* input scalar 2 */
    double *cfs=0;              /* output matrix */

    /* check for proper number of arguments */
    if(nrhs!=3) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs","Three inputs required.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs","One output required.");
    }
    /* make sure the first input argument is scalar */
    if( !mxIsDouble(IN_Low) || 
         mxIsComplex(IN_Low) ||
         mxGetNumberOfElements(IN_Low)!=1 ) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notScalar","First input must be a scalar.");
    }
    
    /* make sure the second input argument is scalar */
    if( !mxIsDouble(IN_High) || 
         mxIsComplex(IN_High) ||
         mxGetNumberOfElements(IN_High)!=1 ) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notScalar","Second input must be a scalar.");
    }
	
	/* make sure the third input argument is scalar */
    if( !mxIsDouble(IN_Number) || 
         mxIsComplex(IN_Number) ||
         mxGetNumberOfElements(IN_Number)!=1 ) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notScalar","Third input must be a scalar.");
    }
    
       
    /* get the value of the scalar inputs  */
    Low_Limit = mxGetScalar(IN_Low);
    High_Limit = mxGetScalar(IN_High);
	N_Filter = mxGetScalar(IN_Number);

    

    /* create the output matrix */
    OUT_cf = mxCreateDoubleMatrix(1,N_Filter,mxREAL);

    /* get a pointer to the real data in the output matrix */
    cfs = mxGetPr(OUT_cf);

    /* call the computational routine */
    MakeErbCFs(Low_Limit,High_Limit,N_Filter,cfs);
}

