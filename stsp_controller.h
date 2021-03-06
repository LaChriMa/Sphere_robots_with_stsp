
#include <stdio.h>
#include <selforg/abstractcontroller.h>
#include <vector>

#include <ode_robots/odeconfig.h>

class STSPController : public AbstractController {
public:


  STSPController(const lpzrobots::OdeConfig& odeconfig, int modeNum=6 );

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0);

  virtual int getSensorNumber() const {return number_sensors;}

  virtual int getMotorNumber() const {return number_motors;}

  virtual void step(const sensor* sensors, int sensornumber,
                    motor* motors, int motornumber);

  virtual void stepNoLearning(const sensor* , int number_sensors,
                              motor* , int number_motors);

  void setRandomPhi();
  void setRandomU();
  void setRandomX( double size );
  void setRandomAll(double size);

  void increaseGamma( double size ){ gamma += size; };
  void increaseW( double size ){ w_0 += size; };
  void increaseZ( double size ){ z_0 += size; };
  void increaseA( double size ){ a += size; };

  /********* STORABLE INTERFACE ******/
  /// @see Storable
  virtual bool store(FILE* f) const {
    Configurable::print(f,"");
    return true;
  }

  /// @see Storable
  virtual bool restore(FILE* f) {
    Configurable::parse(f);
    return true;
  }


protected:
  const lpzrobots::OdeConfig& odeconfig;

  double y(double x);
  double U(double y);
  double PHI(double y, double u);
  double mtarget(double y);
  double mtargetInv(double sensor);

  std::string name;
  double a; 
  double b;
  double stepsize; 
  double eps; 
  int modeNum;

  int number_sensors;
  int number_motors;

  double w_0;
  double z_0;
  double T_u;
  double T_phi;
  double U_max;
  double gamma;
  double r; 


  struct Neuron{
	 double sensor;
	 double x_old;
	 double x_new;
	 double y_old;
	 double y_new;
	 double u_old;
	 double u_new;
	 double phi_old;
	 double phi_new;
  };
  std::vector<Neuron> neuron;

};



