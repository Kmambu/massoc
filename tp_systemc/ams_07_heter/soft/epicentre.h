#include <stdio.h>
#include <math.h>

#define M 2

void non_line_eqns(double x[M], void (*function)(double[M],double[M],double[M][M]));
void func(double x[2],double y[2],double deriv[2][2]);
void compute_initial_pos(double *xi, double *yi);
void calcule_epicentre();

int ind[4];
double sensor_date[4];
double sensor_relative_date[4];
double posx[4];
double posy[4];
double dist[2];
volatile int sensor_acquired;
volatile int flag_calcul;

void compute_initial_pos(double *xi, double *yi)
{
  double coef[4];
  
  coef[0]=1.0;
  coef[1]=(1.0-sensor_relative_date[1]/sensor_relative_date[3]);
  coef[2]=(1.0-sensor_relative_date[2]/sensor_relative_date[3]);

  *xi=(   coef[0]*posx[ind[0]]+
	  coef[1]*posx[ind[1]]+
	  coef[2]*posx[ind[2]])/(coef[0]+coef[1]+coef[2]);
  *yi=(   coef[0]*posy[ind[0]]+
	  coef[1]*posy[ind[1]]+
	  coef[2]*posy[ind[2]])/(coef[0]+coef[1]+coef[2]);
}

void remplir_table(int _ind,int cpt){
  int i,j,ind_tmp;
  double sensor_date_tmp;
  printf("remplir table ind : %d et cpt : %d\n",_ind,cpt);
  ind[sensor_acquired]=_ind ;
  sensor_date[sensor_acquired]=(double)(cpt*10.0e-9) ; 
  sensor_acquired++;
  if (sensor_acquired==4) {
    for (j=0; j<4; j++){
      for (i=0; i<3; i++){
	if (sensor_date[i+1] < sensor_date[i]) {
	  sensor_date_tmp=sensor_date[i];
	  ind_tmp=ind[i];
	  sensor_date[i]=sensor_date[i+1];
	  ind[i]=ind[i+1];
	  sensor_date[i+1]=sensor_date_tmp;
	  ind[i+1]=ind_tmp;
	}
      }
    }
    for (i=0;i<4;i++)
      sensor_relative_date[i]=sensor_date[i]-sensor_date[0];
    flag_calcul=1;
  }
}

void calcule_epicentre(){
  double x[2];
  double xinit, yinit;

  posx[0]=SENSOR0_X ; posy[0]=SENSOR0_Y;
  posx[1]=SENSOR1_X ; posy[1]=SENSOR1_Y;
  posx[2]=SENSOR2_X ; posy[2]=SENSOR2_Y;
  posx[3]=SENSOR3_X ; posy[3]=SENSOR3_Y;

  printf("indices %d %d %d %d\n",ind[0], ind[1], ind[2], ind[3]);

  dist[0]=sensor_relative_date[1]*50000/WAVE_SPEED;
  dist[1]=sensor_relative_date[2]*50000/WAVE_SPEED;
	
  compute_initial_pos(&xinit,&yinit);
  x[0]=xinit;
  x[1]=yinit;
  non_line_eqns(x,&func);
  printf("The zero is %d,%d %d,%d\n",(int)x[0],(int)((x[0]-(int)x[0])*10000),(int)x[1],(int)((x[1]-(int)x[1])*10000));
}

void non_line_eqns(double x[M], void (*function)(double[M],double[M],double[M][M]))
{
  int i,j,k,m,p,q;
  double D[M][M],d[M],s[M],fMaxElem,fAcc,tmp;
  int max();
  double diff2zero;

  diff2zero=1.0;
  // x représentent les inconnues, les valeurs pour lesquelles les f(x) sont
  //   nulles
  // d représentent les f(x)
  // D représentent les dérivées de f par rapport aux x
  // s représentent les h(n), ce qui résulte de l'élimination de Gauss
  (*function)(x,d,D);
  while (diff2zero>.00001) {

    for (k=0 ; k<(2-1); k++)
    {
        // search of line with max element
        fMaxElem = fabs( D[k][k] );
        m = k;
        for(i=k+1; i<2; i++)
        {
                if(fMaxElem < fabs(D[i][k]) )
                {
                        fMaxElem = D[i][k];
                        m = i;
                }
        }

        // permutation of base line (index k) and max element line(index m)
        if(m != k)
        {
                for(i=k; i<2; i++)
                {
                        fAcc    = D[k][i];
                        D[k][i] = D[m][i];
                        D[m][i] = fAcc;
                }
                fAcc = d[k];
                d[k] = d[m];
                d[m] = fAcc;
        }

        // triangulation of matrix with coefficients
        for(j=(k+1); j<2; j++) // current row of matrix
        {
                fAcc = - D[j][k] / D[k][k];
                for(i=k; i<2; i++)
                {
                        D[j][i] = D[j][i] + fAcc*D[k][i];
                }
                d[j] = d[j] + fAcc*d[k]; // free member recalculation
        }
    }

    for(k=(2-1); k>=0; k--)
    {
        s[k] = d[k];
        for(i=(k+1); i<2; i++)
        {
                s[k] -= (D[k][i]*s[i]);
        }
        s[k] = s[k] / D[k][k];
    }

    for (i=0;i<2;i++)
      x[i]=x[i]+s[i];
    (*function)(x,d,D);
    diff2zero=fabs(d[1]+d[0]);
  }

}

void func(double x[2],double y[2],double deriv[2][2])
{
  double x0,y0,x1,y1,x2,y2;

  x0=posx[ind[0]]; 	y0=posy[ind[0]];
  x1=posx[ind[1]];	y1=posy[ind[1]];
  x2=posx[ind[2]];	y2=posy[ind[2]];

  y[0]=-(sqrt(pow(x1-x[0],2)+pow(y1-x[1],2))-sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-dist[0]);
  y[1]=-(sqrt(pow(x2-x[0],2)+pow(y2-x[1],2))-sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-dist[1]);

  deriv[0][0]=((x0-x[0])/sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-(x1-x[0])/sqrt(pow(x1-x[0],2)+pow(y1-x[1],2)));
  deriv[0][1]=((x0-x[1])/sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-(y1-x[1])/sqrt(pow(x1-x[0],2)+pow(y1-x[1],2)));

  deriv[1][0]=((x0-x[0])/sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-(x2-x[0])/sqrt(pow(x2-x[0],2)+pow(y2-x[1],2)));
  deriv[1][1]=((y0-x[1])/sqrt(pow(x0-x[0],2)+pow(y0-x[1],2))-(y2-x[1])/sqrt(pow(x2-x[0],2)+pow(y2-x[1],2)));
}

