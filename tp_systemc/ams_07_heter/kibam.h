#ifndef KIBAM_H
#define KIBAM_H

SCA_TDF_MODULE (kibam)
{
    	sca_tdf::sca_in < double > in_i;
    	sca_tdf::sca_out < double > py1;
    	sca_tdf::sca_out < double > py2;

        double y1, y2;
        double dy1, dy2;
        double dt;  // time step

	double C;
	double k;
	double c;
	double current;

    	void processing ()
    	{
        	current= in_i.read() ;

                dy1 = (-current + k* (y2/(1-c)-y1/c)) * dt;
                dy2 = (-k * (y2/(1-c)-y1/c) ) * dt;
        	y1 += dy1;
        	y2 += dy2;
		py1.write(y1);
		py2.write(y2);
    	}

	void set_attributes()
 	{	
		//dt=in_i.get_T().to_seconds();	
		//dt=1.0;	
		dt=0.01;	
	}
	
    	SCA_CTOR (kibam) {
		y1=2.0*4500.0 ;
		y2=2.0*2700.0 ;
		k=4.5e-5;
		c=0.625;
    	}

};
#endif

