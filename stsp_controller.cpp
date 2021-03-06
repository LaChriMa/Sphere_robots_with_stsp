
#include <stdio.h>
#include <cmath>
#include <assert.h>
#include <selforg/controller_misc.h>
#include <ode_robots/simulation.h>
#include <vector>
#include <fstream>

#include "stsp_controller.h"

using namespace std;
using namespace lpzrobots;

STSPController::STSPController(const OdeConfig& odeconfig, int modeNum) 
  : AbstractController("STPController", "1.0"), odeconfig(odeconfig)
  {
	this->modeNum=modeNum;
};

void STSPController::init(int sensornumber, int motornumber, RandGen* randGen){
     srand(time(0));
     
     number_sensors = sensornumber;
     number_motors = motornumber;
     cout<< " Number of motors: "  << number_motors << endl;
     cout<< " Number of sensors: "  << number_sensors << endl;

     addParameterDef("eps", &eps, 1e-6);
     addParameterDef("a", &a, 0.4);
     addParameterDef("b", &b, 0);   
     addParameterDef("r", &r, 1., "scaling factor of the sigmoidal function (<=1)");
     addParameter("w_0", &w_0,  "");
     addParameter("z_0", &z_0,  "");
     addParameterDef("T_u", &T_u, 0.3, "");
     addParameterDef("T_phi", &T_phi, 0.6, "");
     addParameterDef("U_max", &U_max, 1., "");
     addParameterDef("gamma", &gamma, 20, "");

     neuron.resize(number_motors);
     for(int i = 0; i<number_motors; i++){
	 	neuron[i].x_old = 1;
	 	neuron[i].x_new = 1;
	 	neuron[i].y_old = 1;
	 	neuron[i].y_new = 1;
	 	neuron[i].u_old = 1;
	 	neuron[i].u_new = 1;
	 	neuron[i].phi_old = 1;
	 	neuron[i].phi_new = 1;
	 	neuron[i].sensor = 1;
	 	addInspectableValue("x"+ itos(i), &neuron[i].x_old, "  ");
	 	addInspectableValue("u"+ itos(i), &neuron[i].u_old, "  ");
	 	addInspectableValue("phi"+ itos(i), &neuron[i].phi_old, "  ");
	 	addInspectableValue("y"+ itos(i), &neuron[i].y_old, "  ");
     	addParameter("n"+itos(i)+":x_old", &neuron[i].x_old, "");
     	addParameter("n"+itos(i)+":y_old", &neuron[i].y_old, "");
     	addParameter("n"+itos(i)+":u_old", &neuron[i].u_old, "");
     	addParameter("n"+itos(i)+":phi_old", &neuron[i].phi_old, "");
     }

	 if(modeNum==0){ w_0=280; z_0=-590; 
			neuron[0].x_old=-22; neuron[1].x_old=20; neuron[2].x_old = 20;
			//neuron[1].phi_old=0.2; neuron[2].phi_old=0.7; 
			//neuron[0].y_old=0; neuron[1].y_old=0; neuron[2].y_old=0.7; 
	 }
	 if(modeNum==1){ w_0=230; z_0=-415; }
	 if(modeNum==2){ w_0=190; z_0=-600; }
	 if(modeNum==3){ w_0=250; z_0=-530; }
	 if(modeNum==4){ w_0=240; z_0=-380; }
	 if(modeNum==5){ w_0=220; z_0=-470; }
	 if(modeNum==6){ w_0=210; z_0=-400; }



};

void STSPController::step(const sensor* sensors, int sensornumber,
                          motor* motors, int motornumber) {
     stepsize = odeconfig.simStepSize*odeconfig.controlInterval;

     for(int i= 0; i< number_motors; i++){
         neuron[i].sensor  = sensors[i];
         neuron[i].u_new   = neuron[i].u_old +
         		     ((U(neuron[i].y_old)-neuron[i].u_old)/T_u)*stepsize; 
         neuron[i].phi_new = neuron[i].phi_old +
         	             ((PHI(neuron[i].y_old,neuron[i].u_old)-neuron[i].phi_old )/T_phi)
         		     *stepsize; 
         neuron[i].x_new   = neuron[i].x_old + 
         		     (- gamma* neuron[i].x_old 
                             + w_0* mtargetInv( neuron[i].sensor ))   
         		     *stepsize;
         for(int j=0; j<number_motors; j++){
	     if (i!=j) {
             neuron[i].x_new += (z_0* neuron[j].u_old* neuron[j].phi_old* neuron[j].y_old)* stepsize;
	     }
         }
         neuron[i].y_new    = y( neuron[i].x_new );
         motors[i]          = mtarget( neuron[i].y_new );
     }
     
     /*** rewriting for next timestep ***/
     for(int i=0; i< number_motors; i++){
         neuron[i].y_old = neuron[i].y_new;
         neuron[i].x_old = neuron[i].x_new;
         neuron[i].u_old = neuron[i].u_new;
         neuron[i].phi_old = neuron[i].phi_new;
     }
};

double STSPController::y(double x){
       return 1. / (1. +exp(a*(b-x)));
};

double STSPController::U(double y){
       return  1.+ (U_max- 1.)* y;
};

double STSPController::PHI(double y, double u){
       return  1.- (u* y)/ U_max;
};

double STSPController::mtarget(double y){
       //target motor value from interval [-1,1
       //the target value is rescaled to the actual radius in the robot file
       return r* ( 2.*y - 1.);
};

double STSPController::mtargetInv(double sensor){
       return sensor/(r*2) + 0.5 ;
};

void STSPController::setRandomPhi(){
     cout << " Changes of phi of each Neuron :   ";
     for( int i=0; i < number_motors ; i++){
	  //random value in [0,1] 
	  neuron[i].phi_old = (double)rand()/(double)RAND_MAX;
	  //neuron[i].phi_old += eps * (double)rand()/(double)RAND_MAX;
	  cout << neuron[i].phi_old << "   ";
     }
     cout << endl;
};

void STSPController::setRandomU(){
     cout << " Changes of u of each Neuron :   ";
     for( int i=0; i < number_motors ; i++){ 
	  //random value in [1,U_max]
	  neuron[i].u_old = (double)rand()/(double)RAND_MAX* ( U_max- 1.)+ 1.;
	  //neuron[i].u_old += eps * (double)rand()/(double)RAND_MAX;
	  cout << neuron[i].u_old << "   ";
     }
     cout << endl;
};

void STSPController::setRandomX(double size){
     cout << " Changes of x of each Neuron :   ";
     for( int i=0; i < number_motors ; i++){ 
	  //random value in [-size/2,size/2]
	  neuron[i].x_old = (double)rand()/(double)RAND_MAX* size - size / 2.;
	  //neuron[i].x_old += eps * (2*(double)rand()/(double)RAND_MAX-1.);
	  cout << neuron[i].x_old << "   ";
     }
     cout << endl;
};

void STSPController::setRandomAll(double size){
	//randomize all internal variables of the neuron
	setRandomU();
	setRandomPhi();
	setRandomX(size);
}

void STSPController::stepNoLearning(const sensor* sensors, int number_sensors,
                                    motor* motors, int number_motors) {

};


