#ifndef KITE_H
#define KITE_H

#include "casadi/casadi.hpp"
#include "yaml-cpp/yaml.h"
#include "kitemath.h"
#include <chrono>

struct PlaneGeometry
{
    double WingSpan;
    double MAC;
    double AspectRatio;
    double WingSurfaceArea;
    double TaperRatio;
    double HTailsurface;
    double TailLeverArm;
    double FinSurfaceArea;
    double FinLeverArm;
    double AerodynamicCenter;
};

struct PlaneInertia
{
    double Mass;
    double Ixx;
    double Iyy;
    double Izz;
    double Ixz;
};

struct PlaneAerodynamics
{
    double CL0;
    double CL0_tail;
    double CLa_total;
    double CLa_wing;
    double CLa_tail;
    double e_oswald;

    double CD0_total;
    double CD0_wing;
    double CD0_tail;
    double CYb;
    double CYb_vtail;
    double Cm0;
    double Cma;
    double Cn0;
    double Cnb;
    double Cl0;
    double Clb;

    double CLq;
    double Cmq;
    double CYr;
    double Cnr;
    double Clr;
    double CYp;
    double Clp;
    double Cnp;

    double CLde;
    double CYdr;
    double Cmde;
    double Cndr;
    double Cldr;
    double CDde;
};

struct TetherProperties
{
    double length;
    double Ks;
    double Kd;
};

struct KiteProperties
{
    std::string Name;
    PlaneGeometry Geometry;
    PlaneInertia Inertia;
    PlaneAerodynamics Aerodynamics;
    TetherProperties Tether;
};

struct SimpleKinematicKiteProperties
{
    double tether_length;
    double wind_speed;
    double gliding_ratio;
};

struct AlgorithmProperties
{
    IntType Integrator;
    double sampling_time;
};


namespace kite_utils
{
    KiteProperties LoadProperties(const std::string &filename);

    typedef std::chrono::time_point<std::chrono::system_clock> time_point;
    time_point get_time();

}

class KiteDynamics
{
public:
    //constructor
    KiteDynamics(const KiteProperties &KiteProps, const AlgorithmProperties &AlgoProps);
    KiteDynamics(const KiteProperties &KiteProps, const AlgorithmProperties &AlgoProps, const bool &id);
    virtual ~KiteDynamics(){}

    /** public methods */
    casadi::SX getSymbolicState(){return this->State;}
    casadi::SX getSymbolicControl(){return this->Control;}
    casadi::SX getSymbolicParameters(){return this->Parameters;}

    casadi::SX getSymbolicDynamics(){return this->SymDynamics;}
    casadi::SX getSymbolicIntegrator(){return this->SymIntegartor;}
    casadi::SX getSymbolicJacobian(){return this->SymJacobian;}

    casadi::Function getNumericDynamics(){return this->NumDynamics;}
    casadi::Function getNumericIntegrator(){return this->NumIntegrator;}
    casadi::Function getNumericJacobian(){return this->NumJacobian;}

private:
    //state variables
    casadi::SX State;
    //control variables
    casadi::SX Control;
    //parameters
    casadi::SX Parameters;
    //symbolic equations of motion
    casadi::SX SymDynamics;
    //symbolic equations of integartor
    casadi::SX SymIntegartor;
    //symbolic expression for system jacobian
    casadi::SX SymJacobian;

    //numerical dynamics evaluation
    casadi::Function NumDynamics;
    //numerical integral evaluation
    casadi::Function NumIntegrator;
    //numerical jacobian evaluation
    casadi::Function NumJacobian;
};

/** 6-DoF Kinematics of a Rigid Body */
class RigidBodyKinematics
{
public:
    RigidBodyKinematics(const AlgorithmProperties &AlgoProps);
    virtual ~RigidBodyKinematics(){}

    casadi::Function getNumericIntegrator() {return NumIntegartor;}
    casadi::Function getNumericJacobian() {return NumJacobian;}
    casadi::Function getNumericDynamcis() {return NumDynamics;}

private:
    casadi::SX state;
    AlgorithmProperties algo_props;

    /** numerical integrator */
    casadi::Function NumIntegartor;
    /** numerical evaluation of Jacobian */
    casadi::Function NumJacobian;
    /** numerical evaluation of system dynamics */
    casadi::Function NumDynamics;
};

/** Relatively simple kite model : tricycle on a sphere */
class SimpleKinematicKite
{
public:
    SimpleKinematicKite(const AlgorithmProperties &AlgoProps, const SimpleKinematicKiteProperties &KiteProps);
    virtual ~SimpleKinematicKite(){}

    casadi::Function getNumericJacobian(){return NumJacobian;}
    casadi::Function getNumericDynamics(){return NumDynamics;}
    casadi::Function getNumericIntegrator(){return NumIntegrator;}

    casadi::SX getSymbolicDynamics(){return Dynamics;}
    casadi::SX getSymbolicState(){return state;}
    casadi::SX getSymbolicControl(){return control;}

private:
    casadi::SX state;
    casadi::SX control;
    casadi::SX Dynamics;

    casadi::Function NumDynamics;
    casadi::Function NumJacobian;
    casadi::Function NumIntegrator;
};



#endif // KITE_H
