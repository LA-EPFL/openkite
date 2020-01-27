#include "kite.h"

using namespace casadi;

namespace kite_utils {
    KiteProperties LoadProperties(const std::string &filename) {
        //read YAML config file
        YAML::Node config = YAML::LoadFile(filename);

        //create properties object and fill in with data
        KiteProperties props;

        props.Name = config["name"].as<std::string>();

        props.Geometry.ImuPitchOffset_deg = config["geometry"]["imu_pitch_offs_deg"].as<double>();

        props.Geometry.WingSpan = config["geometry"]["b"].as<double>();
        props.Geometry.MAC = config["geometry"]["c"].as<double>();
        props.Geometry.AspectRatio = config["geometry"]["AR"].as<double>();
        props.Geometry.WingSurfaceArea = config["geometry"]["S"].as<double>();
        //props.Geometry.TaperRatio = config["geometry"]["lam"].as<double>();
        //props.Geometry.HTailsurface = config["geometry"]["St"].as<double>();
        //props.Geometry.TailLeverArm = config["geometry"]["lt"].as<double>();
        //props.Geometry.FinSurfaceArea = config["geometry"]["Sf"].as<double>();
        //props.Geometry.FinLeverArm = config["geometry"]["lf"].as<double>();
        //props.Geometry.AerodynamicCenter = config["geometry"]["Xac"].as<double>();

        props.Inertia.Mass = config["inertia"]["mass"].as<double>();
        props.Inertia.Ixx = config["inertia"]["Ixx"].as<double>();
        props.Inertia.Iyy = config["inertia"]["Iyy"].as<double>();
        props.Inertia.Izz = config["inertia"]["Izz"].as<double>();
        props.Inertia.Ixz = config["inertia"]["Ixz"].as<double>();

        props.Aerodynamics.CL0 = config["aerodynamic"]["CL0"].as<double>();
        //props.Aerodynamics.CL0_tail = config["aerodynamic"]["CL0_tail"].as<double>();
        props.Aerodynamics.CLa_total = config["aerodynamic"]["CLa_total"].as<double>();
        //props.Aerodynamics.CLa_wing = config["aerodynamic"]["CLa_wing"].as<double>();
        //props.Aerodynamics.CLa_tail = config["aerodynamic"]["CLa_tail"].as<double>();
        props.Aerodynamics.e_oswald = config["aerodynamic"]["e_oswald"].as<double>();

        props.Aerodynamics.CD0_total = config["aerodynamic"]["CD0_total"].as<double>();
        //props.Aerodynamics.CD0_wing = config["aerodynamic"]["CD0_wing"].as<double>();
        //props.Aerodynamics.CD0_tail = config["aerodynamic"]["CD0_tail"].as<double>();
        props.Aerodynamics.CYb = config["aerodynamic"]["CYb"].as<double>();
        //props.Aerodynamics.CYb_vtail = config["aerodynamic"]["CYb_vtail"].as<double>();
        props.Aerodynamics.Cm0 = config["aerodynamic"]["Cm0"].as<double>();
        props.Aerodynamics.Cma = config["aerodynamic"]["Cma"].as<double>();
        props.Aerodynamics.Cn0 = config["aerodynamic"]["Cn0"].as<double>();
        props.Aerodynamics.Cnb = config["aerodynamic"]["Cnb"].as<double>();
        props.Aerodynamics.Cl0 = config["aerodynamic"]["Cl0"].as<double>();
        props.Aerodynamics.Clb = config["aerodynamic"]["Clb"].as<double>();

        props.Aerodynamics.CLq = config["aerodynamic"]["CLq"].as<double>();
        props.Aerodynamics.Cmq = config["aerodynamic"]["Cmq"].as<double>();
        props.Aerodynamics.CYr = config["aerodynamic"]["CYr"].as<double>();
        props.Aerodynamics.Cnr = config["aerodynamic"]["Cnr"].as<double>();
        props.Aerodynamics.Clr = config["aerodynamic"]["Clr"].as<double>();
        props.Aerodynamics.CYp = config["aerodynamic"]["CYp"].as<double>();
        props.Aerodynamics.Clp = config["aerodynamic"]["Clp"].as<double>();
        props.Aerodynamics.Cnp = config["aerodynamic"]["Cnp"].as<double>();

        props.Aerodynamics.CLde = config["aerodynamic"]["CLde"].as<double>();
        props.Aerodynamics.CYdr = config["aerodynamic"]["CYdr"].as<double>();
        props.Aerodynamics.Cmde = config["aerodynamic"]["Cmde"].as<double>();
        props.Aerodynamics.Cndr = config["aerodynamic"]["Cndr"].as<double>();
        props.Aerodynamics.Cldr = config["aerodynamic"]["Cldr"].as<double>();
        //props.Aerodynamics.CDde = config["aerodynamic"]["CDde"].as<double>();
        props.Aerodynamics.Cnda = config["aerodynamic"]["Cnda"].as<double>();
        props.Aerodynamics.Clda = config["aerodynamic"]["Clda"].as<double>();

        //props.Tether.length = config["tether"]["length"].as<double>();
        //props.Tether.Ks = config["tether"]["Ks"].as<double>();
        //props.Tether.Kd = config["tether"]["Kd"].as<double>();
        //props.Tether.rx = config["tether"]["rx"].as<double>();
        //props.Tether.ry = config["tether"]["ry"].as<double>();
        //props.Tether.rz = config["tether"]["rz"].as<double>();

        return props;
    }

    time_point get_time() {
        /** OS dependent */
#ifdef __APPLE__
        return std::chrono::system_clock::now();
#else
        return std::chrono::high_resolution_clock::now();
#endif
    }

    bool file_exists(const std::string &filename) {
        struct stat buffer;
        return (stat(filename.c_str(), &buffer) == 0);
    }

    DM read_from_file(const std::string &filename) {
        std::ifstream file(filename, std::ios::in);
        std::vector<double> vec;
        if (!file.fail()) {
            double x;
            while (file >> x) {
                vec.push_back(x);
            }
            return DM({vec});
        } else {
            std::cout << "Could not open : " << filename << " data file \n";
            file.clear();
            return DM({});
        }
    }

    void write_to_file(const std::string &filename, const DM &data) {
        std::ofstream data_file(filename, std::ios::out);
        std::vector<double> vec = data.nonzeros();

        /** solution */
        if (!data_file.fail()) {
            for (std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it) {
                data_file << (*it) << " ";
            }
            data_file << "\n";
        }
        data_file.close();
    }
}


template<typename P, typename PO, typename PA>
void
KiteDynamics::getModel(P &g, P &rho, P &windFrom_deg, P &windSpeed, P &b, P &c, P &AR, P &S, P &Mass, P &Ixx, P &Iyy,
                       P &Izz, P &Ixz, PO &imuPitchOffset, PO &CL0, PO &CLa_tot, P &e_o, PO &CD0_tot, PA &CYb, PO &Cm0,
                       PO &Cma, P &Cn0, PA &Cnb, P &Cl0, PA &Clb, PO &CLq, PO &Cmq, PA &CYr, PA &Cnr, PA &Clr, PA &CYp,
                       PA &Clp, PA &Cnp, PO &CLde, PA &CYdr, PO &Cmde, PA &Cndr, PA &Cldr, PA &Clda, PA &Cnda,

                       casadi::SX &v, casadi::SX &w, casadi::SX &r, casadi::SX &q, casadi::SX &T, casadi::SX &dE,
                       casadi::SX &dR, casadi::SX &dA, casadi::SX &v_dot, casadi::SX &w_dot, casadi::SX &r_dot,
                       casadi::SX &q_dot, casadi::SX &Faero_b, casadi::SX &T_b) {


/** ============================================================================================================ **/
/** Start of model **/

/** -------------------------- **/
/** State variables definition **/
/** -------------------------- **/
    v = SX::sym("v", 3); /**  linear velocity of the CoG in the Body Reference Frame (BRF) [m/s] **/
    w = SX::sym("w", 3); /**  glider angular velocity in BRF [rad/s]                             **/
    r = SX::sym("r", 3); /**  position of the CoG in the Inertial Reference Frame (IRF) [m]      **/
    q = SX::sym("q", 4); /**  body attitude relative to IRF [unit quaternion]                    **/

/** ---------------------------- **/
/** Control variables definition **/
/** ---------------------------- **/
/** @todo: consider more detailed propeller model **/
    T = SX::sym("T");   /** propeller propulsion : applies along X-axis in BRF [N] **/
    dE = SX::sym("dE"); /** elevator deflection [positive causing negative pitch movement (nose down)] [rad] **/
    dR = SX::sym("dR"); /** rudder deflection [positive causing negative yaw movement (nose left)] [rad] **/
    dA = SX::sym("dA"); /** aileron deflection [positive causing negative roll movement (right wing up)] [rad] **/
//SX dF = SX::sym("dF"); /** flaps deflection [reserved, but not used]               **/

    SX WS = SX::sym("WS", 3);                 /** Wind speed (velocity) **/
    SX windDir = (windFrom_deg + 180.0) * M_PI / 180.0;    /** Wind To direction [rad] */
    WS = windSpeed * SX::vertcat({cos(windDir), sin(windDir), 0.0});

    SX q_imu = SX::vertcat({cos(-imuPitchOffset / 2.0),
                            0.0,
                            sin(-imuPitchOffset / 2.0),
                            0.0}); /** IMU to body **/
    SX q_corrected = kmath::quat_multiply(q,
                                          q_imu); /** Rotation from NED to body frame, corrected by IMU pitch offset **/

    SX qWS = kmath::quat_multiply(kmath::quat_inverse(q), SX::vertcat({0, WS}));
    SX qWS_q = kmath::quat_multiply(qWS, q_corrected);
    SX WS_b = qWS_q(Slice(1, 4), 0);            /** Wind velocity in body frame **/

    SX Va = v - WS_b;         /** Apparent velocity in body frame **/
    SX V = SX::norm_2(Va);     /** Apparent speed **/
    SX V2 = SX::dot(Va, Va);   /** Apparent speed **/

    SX ss = asin(Va(1) / (V + 1e-4));       /** side slip angle [rad] (v(3)/v(1)) // small angle assumption **/
    SX aoa = atan2(Va(2), (Va(0) + 1e-4));  /** angle of attack definition [rad] (v(2)/L2(v)) **/
    SX dyn_press = 0.5 * rho * V2;         /** dynamic pressure **/

    SX CD = CD0_tot + pow(CL0 + CLa_tot * aoa, 2) / (pi * e_o * AR); /** total drag coefficient **/

/** ------------------------- **/
/** Dynamic Equations: Forces */
/** ------------------------- **/
    SX LIFT = (CL0 + CLa_tot * aoa) * dyn_press * S +
              (0.25 * CLq * c * S * rho) * V * w(1);
    SX DRAG = CD * dyn_press * S;
    SX SF = (CYb * ss + CYdr * dR) * dyn_press * S +
            0.25 * (CYr * w(2) + CYp * w(0)) * (b * rho * S) * V;

/** Compute transformation between WRF and BRF: qw_b **/
/** qw_b = q(aoa) * q(-ss);                           **/
    SX q_aoa = SX::vertcat({cos(aoa / 2), 0, sin(aoa / 2), 0});
    SX q_ss = SX::vertcat({cos(-ss / 2), 0, 0, sin(-ss / 2)});

    SX qw_b = kmath::quat_multiply(q_aoa, q_ss);
    SX qw_b_inv = kmath::quat_inverse(qw_b);

/** Aerodynamic forces in BRF: Faer0_b = qw_b * [0; -DRAG; SF; -LIFT] * qw_b_inv */
    SX qF_tmp = kmath::quat_multiply(qw_b_inv, SX::vertcat({0, -DRAG, 0, -LIFT}));
    SX qF_q = kmath::quat_multiply(qF_tmp, qw_b);
    Faero_b = qF_q(Slice(1, 4), 0);

    SX Zde = (-CLde) * dE * dyn_press * S;
    SX FdE_tmp = kmath::quat_multiply(kmath::quat_inverse(q_aoa),
                                      SX::vertcat({0, 0, 0, Zde}));
    SX qFdE = kmath::quat_multiply(FdE_tmp, q_aoa);
    SX FdE = qFdE(Slice(1, 4), 0);

    Faero_b = Faero_b + FdE + SX::vertcat({0, SF, 0});

/** Gravitational acceleration in BRF */
    SX qG = kmath::quat_multiply(kmath::quat_inverse(q),
                                 SX::vertcat({0, 0, 0, g}));
    SX qG_q = kmath::quat_multiply(qG, q_corrected);
    SX g_b = qG_q(Slice(1, 4), 0);

/** Propulsion force in BRF */
// const double p1 = -0.007752958684034;
// const double p2 = -0.137115918638887;
// const double p3 = 9.327531598002182;
// normalize to get 0...1
    const double p1 = -0.00083119;
    const double p2 = -0.014700129;
    T_b = SX::vertcat({T * (p1 * V2 + p2 * V + 1.0), 0, 0});
//SX T_b = SX::vertcat({T, 0, 0});

/** Tether force */
//    /** value: using smooth ramp approximation */
//    SX d_ = SX::norm_2(r);
//    /** spring term */
//    /** @todo: put all coefficients in the config */
//    //const double Ks = 15 * Mass;
//    //const double Kd = 10 * Mass;
//    SX Rv = ((d_ - Lt));
//    SX Rs = -Rv * (r / d_);
//    /** damping term */
//    SX qvi = kmath::quat_multiply(q, SX::vertcat({0, v}));
//    SX qvi_q = kmath::quat_multiply(qvi, kmath::quat_inverse(q));
//    SX vi = qvi_q(Slice(1, 4), 0);
//    SX Rd = (-r / d_) * SX::dot(r, vi) / d_;
//    SX R = (Ks * Rs + Kd * Rd) * kmath::heaviside(d_ - Lt, 1);
//
//    /** BRF */
//    SX qR = kmath::quat_multiply(kmath::quat_inverse(q), SX::vertcat({0, R}));
//    SX qR_q = kmath::quat_multiply(qR, q_corrected);
//    SX R_b = qR_q(Slice(1, 4), 0);

/** Total external forces devided by glider's mass (linear acceleration) */
    v_dot = (Faero_b + T_b) / Mass + g_b - SX::cross(w, v);
//    auto v_dot = (Faero_b + T_b + R_b) / Mass + g_b - SX::cross(w, v);

/** ------------------------- */
/** Dynamic Equation: Moments */
/** ------------------------- */
/** Rolling Aerodynamic Moment */
    SX L = (Cl0 + Clb * ss + Cldr * dR + Clda * dA) * dyn_press * S * b +
           (Clr * w(2) + Clp * w(0)) * (0.25 * rho * std::pow(b, 2) * S) * V;

/** Pitching Aerodynamic Moment */
    SX M = (Cm0 + Cma * aoa + Cmde * dE) * dyn_press * S * c +
           Cmq * (0.25 * S * std::pow(c, 2) * rho) * w(1) * V;

/** Yawing Aerodynamic Moment */
    SX N = (Cn0 + Cnb * ss + Cndr * dR + Cnda * dA) * dyn_press * S * b +
           (Cnp * w(0) + Cnr * w(2)) * (0.25 * S * std::pow(b, 2) * rho) * V;

/** Aircraft Inertia Matrix */
    SXVector j_vec{Ixx, Iyy, Izz};
    auto J = SX::diag(SX::vertcat(j_vec));
    J(0, 2) =
            Ixz;
    J(2, 0) =
            Ixz;

/** Angular motion equationin BRF */
/** Moments transformation SRF -> BRF */
    SX T_tmp = kmath::quat_multiply(kmath::quat_inverse(q_aoa),
                                    SX::vertcat({0, L, M, N}));
    SX Trot = kmath::quat_multiply(T_tmp, q_aoa);
    auto Maero = Trot(Slice(1, 4), 0);

/** Moment introduced by tether */
//    //DM tether_arm = DM({rx, ry, rz});
//    SX tether_arm = SX::vertcat({rx, ry, rz});
//    SX Mt = SX::cross(tether_arm, R_b);
//
//    auto w_dot = SX::mtimes(SX::inv(J), (Maero + Mt - SX::cross(w, SX::mtimes(J, w))));
    w_dot = SX::mtimes(SX::inv(J), (Maero - SX::cross(w, SX::mtimes(J, w))));

/** ----------------------------- */
/** Kinematic Equations: Position */
/** ----------------------------- */
/** Aircraft position in the IRF  */
    SX qv = kmath::quat_multiply(q, SX::vertcat({0, v}));
    SX qv_q = kmath::quat_multiply(qv, kmath::quat_inverse(q));
    r_dot = qv_q(Slice(1, 4), 0);

/** ----------------------------- */
/** Kinematic Equations: Attitude */
/** ----------------------------- */
/** Aircraft attitude wrt IRF  */
    double lambda = -5;
    q_dot = 0.5 * kmath::quat_multiply(q, SX::vertcat({0, w})) +
            0.5 * lambda * q_corrected * (SX::dot(q_corrected, q_corrected) - 1);

/** End of dynamics model **/
/** ============================================================================================================ **/
}

KiteDynamics::KiteDynamics(const KiteProperties &KiteProps, const AlgorithmProperties &AlgoProps) {

    /** enviromental constants */
    double g = 9.80665; /** gravitational acceleration [m/s2] [WGS84] */
    double rho = 1.2985; /** standard atmospheric density [kg/m3] [standard Atmosphere 1976] */

    /** --------------------- **/
    /** Wind properties       **/
    /** --------------------- **/
    double windFrom_deg = KiteProps.Wind.WindFrom_deg;
    double windSpeed = KiteProps.Wind.WindSpeed;

    /** --------------------- **/
    /** Geometric parameters  **/
    /** --------------------- **/
    double imuPitchOffset = KiteProps.Geometry.ImuPitchOffset_deg * M_PI / 180.0;

    double b = KiteProps.Geometry.WingSpan;
    double c = KiteProps.Geometry.MAC;
    double AR = KiteProps.Geometry.AspectRatio;
    double S = KiteProps.Geometry.WingSurfaceArea;
    //double lam = KiteProps.Geometry.TaperRatio;
    //double St = KiteProps.Geometry.HTailsurface;
    //double lt = KiteProps.Geometry.TailLeverArm;
    //double Sf = KiteProps.Geometry.FinSurfaceArea;
    //double lf = KiteProps.Geometry.FinLeverArm;
    //double Xac = KiteProps.Geometry.AerodynamicCenter;
    /** @todo: get rid of all magic numbers **/
    //double Xcg = 0.031/c;               /** Center of Gravity (CoG) wrt leading edge [1/c] **/
    //double Vf = (Sf * lf) / (S * b);    /** fin volume coefficient []                      **/
    //double Vh = (lt * St) / (S * c);    /** horizontal tail volume coefficient []          **/

    /** --------------------------- **/
    /** Mass and inertia parameters **/
    /** --------------------------- **/
    double Mass = KiteProps.Inertia.Mass;
    double Ixx = KiteProps.Inertia.Ixx;
    double Iyy = KiteProps.Inertia.Iyy;
    double Izz = KiteProps.Inertia.Izz;
    double Ixz = KiteProps.Inertia.Ixz;

    /** ------------------------------- **/
    /** Static aerodynamic coefficients **/
    /** ------------------------------- **/
    double CL0 = KiteProps.Aerodynamics.CL0;
    //double CL0_t = KiteProps.Aerodynamics.CL0_tail;
    double CLa_tot = KiteProps.Aerodynamics.CLa_total;
    //double CLa_w = KiteProps.Aerodynamics.CLa_wing;
    //double CLa_t = KiteProps.Aerodynamics.CLa_tail;
    double e_o = KiteProps.Aerodynamics.e_oswald;

    double CD0_tot = KiteProps.Aerodynamics.CD0_total;
    //double CD0_w = KiteProps.Aerodynamics.CD0_wing;
    //double CD0_t = KiteProps.Aerodynamics.CD0_tail;
    double CYb = KiteProps.Aerodynamics.CYb;
    //double CYb_vt = KiteProps.Aerodynamics.CYb_vtail;
    double Cm0 = KiteProps.Aerodynamics.Cm0;
    double Cma = KiteProps.Aerodynamics.Cma;
    double Cn0 = KiteProps.Aerodynamics.Cn0;
    double Cnb = KiteProps.Aerodynamics.Cnb;
    double Cl0 = KiteProps.Aerodynamics.Cl0;
    double Clb = KiteProps.Aerodynamics.Clb;
    //double dw = CLa_tot / (pi * e_o * AR);             /** downwash acting at the tail [] **/

    double CLq = KiteProps.Aerodynamics.CLq;
    double Cmq = KiteProps.Aerodynamics.Cmq;
    double CYr = KiteProps.Aerodynamics.CYr;
    double Cnr = KiteProps.Aerodynamics.Cnr;
    double Clr = KiteProps.Aerodynamics.Clr;
    double CYp = KiteProps.Aerodynamics.CYp;
    double Clp = KiteProps.Aerodynamics.Clp;
    double Cnp = KiteProps.Aerodynamics.Cnp;

    /** ------------------------------ **/
    /** Aerodynamic effects of control **/
    /** ------------------------------ **/
    double CLde = KiteProps.Aerodynamics.CLde;
    double CYdr = KiteProps.Aerodynamics.CYdr;
    double Cmde = KiteProps.Aerodynamics.Cmde;
    double Cndr = KiteProps.Aerodynamics.Cndr;
    double Cldr = KiteProps.Aerodynamics.Cldr;
    //double CDde = KiteProps.Aerodynamics.CDde;
    double Clda = KiteProps.Aerodynamics.Clda;
    double Cnda = KiteProps.Aerodynamics.Cnda;

    //double CL_daoa = -2 * CLa_t * Vh * dw;
    //double Cm_daoa = -2 * CLa_t * Vh * (lt/c) * dw;

    /** ------------------------------ **/
    /**        Tether parameters       **/
    /** ------------------------------ **/
    //    double Lt = KiteProps.Tether.length;
    //    double Ks = KiteProps.Tether.Ks;
    //    double Kd = KiteProps.Tether.Kd;
    //    double rx = KiteProps.Tether.rx;
    //    double ry = KiteProps.Tether.ry;
    //    double rz = KiteProps.Tether.rz;

    SX v, w, r, q;
    SX T, dE, dR, dA;
    SX v_dot, w_dot, r_dot, q_dot;
    SX Faero_b, T_b;

    /* Info: <General, Lon, Lat> Types for parameter groups, defined in function call */
    getModel<double, double, double>(g, rho, windFrom_deg, windSpeed,
                                     b, c, AR, S,
                                     Mass, Ixx, Iyy, Izz, Ixz,
                                     imuPitchOffset, CL0, CLa_tot, e_o,
                                     CD0_tot, CYb, Cm0, Cma, Cn0, Cnb, Cl0, Clb,
                                     CLq, Cmq, CYr, Cnr, Clr, CYp, Clp, Cnp,
                                     CLde, CYdr, Cmde, Cndr, Cldr, Clda, Cnda,

                                     v, w, r, q,
                                     T, dE, dR, dA,
                                     v_dot, w_dot, r_dot, q_dot,
                                     Faero_b, T_b);

    /** Complete dynamics of the Kite */
    auto state = SX::vertcat({v, w, r, q});
    auto control = SX::vertcat({T, dE, dR, dA});
    auto dynamics = SX::vertcat({v_dot, w_dot, r_dot, q_dot});

    Function dyn_func = Function("dynamics", {state, control}, {dynamics});
    Function specNongravForce_func = Function("spec_nongrav_force", {state, control}, {(Faero_b + T_b) / Mass});

    /** compute dynamics state Jacobian */
    SX d_jacobian = SX::jacobian(dynamics, state);
    Function dyn_jac = Function("dyn_jacobian", {state, control}, {d_jacobian});

    AeroDynamics = Function("Aero", {state, control}, {Faero_b});
    /** define RK4 integrator scheme */
    SX X = SX::sym("X", state.size1());
    SX U = SX::sym("U", control.size1());
    SX dT = SX::sym("dT");

    /** get symbolic expression for RK4 integrator */
    SX sym_integrator = kmath::rk4_symbolic(X, U, dyn_func, dT);
    Function RK4_INT = Function("RK4", {X, U, dT}, {sym_integrator});

    /** CVODES integrator */
    /** @todo: make smarter initialisation of integrator */
    double h = AlgoProps.sampling_time;
    SXDict ode = {{"x",   state},
                  {"p",   control},
                  {"ode", dynamics}};
    Dict opts = {{"tf", h}};
    Function CVODES_INT = integrator("CVODES_INT", "cvodes", ode, opts);

    /** assign class atributes */
    this->State = state;
    this->Control = control;
    this->SymDynamics = dynamics;
    this->SymIntegartor = sym_integrator;
    this->SymJacobian = d_jacobian;

    this->NumDynamics = dyn_func;
    this->NumSpecNongravForce = specNongravForce_func;
    this->NumJacobian = dyn_jac;

    /** return integrator function */
    if (AlgoProps.Integrator == IntType::CVODES)
        this->NumIntegrator = CVODES_INT;
    else
        this->NumIntegrator = RK4_INT;

}

KiteDynamics::KiteDynamics(const KiteProperties &KiteProps, const AlgorithmProperties &AlgoProps,
                           const IdentMode &identMode) {

    /** enviromental constants */
    double g = 9.80665; /** gravitational acceleration [m/s2] [WGS84] */
    double rho = 1.2985; /** standard atmospheric density [kg/m3] [Standard Atmosphere 1976] */

    /** --------------------- **/
    /** Wind properties       **/
    /** --------------------  **/
    double windFrom_deg = KiteProps.Wind.WindFrom_deg;
    double windSpeed = KiteProps.Wind.WindSpeed;

    /** --------------------- **/
    /** Geometric parameters  **/
    /** --------------------- **/
    double b = KiteProps.Geometry.WingSpan;
    double c = KiteProps.Geometry.MAC;
    double AR = KiteProps.Geometry.AspectRatio;
    double S = KiteProps.Geometry.WingSurfaceArea;
    //double lam = KiteProps.Geometry.TaperRatio;
    //double St = KiteProps.Geometry.HTailsurface;
    //double lt = KiteProps.Geometry.TailLeverArm;
    //double Sf = KiteProps.Geometry.FinSurfaceArea;
    //double lf = KiteProps.Geometry.FinLeverArm;
    //double Xac = KiteProps.Geometry.AerodynamicCenter;
    /** @todo: get rid of all magic numbers **/
    //double Xcg = 0.031/c;               /** Center of Gravity (CoG) wrt leading edge [1/c] **/
    //double Vf = (Sf * lf) / (S * b);    /** fin volume coefficient []                      **/
    //double Vh = (lt * St) / (S * c);    /** horizontal tail volume coefficient []          **/

    /** --------------------------- **/
    /** Mass and inertia parameters **/
    /** --------------------------- **/
    double Mass = KiteProps.Inertia.Mass;
    double Ixx = KiteProps.Inertia.Ixx;
    double Iyy = KiteProps.Inertia.Iyy;
    double Izz = KiteProps.Inertia.Izz;
    double Ixz = KiteProps.Inertia.Ixz;

    /** ------------------------------- **/
    /** Static aerodynamic coefficients **/
    /** ------------------------------- **/

    SX params;

    SX v, w, r, q;
    SX T, dE, dR, dA;
    SX v_dot, w_dot, r_dot, q_dot;
    SX Faero_b, T_b;

    if (identMode == LONGITUDINAL) {
        /** LONGITUDINAL IDENTIFICATION PARAMETERS ------------------------------------------------------------------ */

        SX imuPitchOffset = SX::sym("imuPitchOffset");
        SX CL0 = SX::sym("Cl0");            //double CL0 = KiteProps.Aerodynamics.CL0;
        //double CL0_t = KiteProps.Aerodynamics.CL0_tail;
        SX CLa_tot = SX::sym("Cla_tot");    //double CLa_tot = KiteProps.Aerodynamics.CLa_total;
        //double CLa_w = KiteProps.Aerodynamics.CLa_wing;
        //double CLa_t = KiteProps.Aerodynamics.CLa_tail;
        double e_o = KiteProps.Aerodynamics.e_oswald;

        //double dw = CLa_tot / (pi * e_o * AR);             /** downwash acting at the tail [] **/

        SX CD0_tot = SX::sym("CD0_tot");
        //double CD0_w = KiteProps.Aerodynamics.CD0_wing;
        //double CD0_t = KiteProps.Aerodynamics.CD0_tail;
        double CYb = KiteProps.Aerodynamics.CYb;
        //double CYb_vt = KiteProps.Aerodynamics.CYb_vtail;
        SX Cm0 = SX::sym("Cm0");
        SX Cma = SX::sym("Cma");
        double Cn0 = KiteProps.Aerodynamics.Cn0;
        double Cnb = KiteProps.Aerodynamics.Cnb;
        double Cl0 = KiteProps.Aerodynamics.Cl0;
        double Clb = KiteProps.Aerodynamics.Clb;

        SX CLq = SX::sym("CLq");
        SX Cmq = SX::sym("Cmq");
        double CYr = KiteProps.Aerodynamics.CYr;
        double Cnr = KiteProps.Aerodynamics.Cnr;
        double Clr = KiteProps.Aerodynamics.Clr;
        double CYp = KiteProps.Aerodynamics.CYp;
        double Clp = KiteProps.Aerodynamics.Clp;
        double Cnp = KiteProps.Aerodynamics.Cnp;

        /** ------------------------------ **/
        /** Aerodynamic effects of control **/
        /** ------------------------------ **/
        SX CLde = SX::sym("CLde");
        double CYdr = KiteProps.Aerodynamics.CYdr;
        SX Cmde = SX::sym("Cmde");
        double Cndr = KiteProps.Aerodynamics.Cndr;
        double Cldr = KiteProps.Aerodynamics.Cldr;
        //SX CDde = SX::sym("CDde");
        double Clda = KiteProps.Aerodynamics.Clda;
        double Cnda = KiteProps.Aerodynamics.Cnda;

        //double CL_daoa = -2 * CLa_t * Vh * dw;
        //double Cm_daoa = -2 * CLa_t * Vh * (lt/c) * dw;

        params = SX::vertcat({imuPitchOffset,
                              CL0, CLa_tot,
                              CD0_tot, Cm0, Cma,
                              CLq, Cmq,
                              CLde, Cmde
                             }); // 10 longitudinal parameters

        /* Info: <General, Lon, Lat> Types for parameter groups, defined in function call */
        getModel<double, SX, double>(g, rho, windFrom_deg, windSpeed,
                                     b, c, AR, S,
                                     Mass, Ixx, Iyy, Izz, Ixz,
                                     imuPitchOffset, CL0, CLa_tot, e_o,
                                     CD0_tot, CYb, Cm0, Cma, Cn0, Cnb, Cl0, Clb,
                                     CLq, Cmq, CYr, Cnr, Clr, CYp, Clp, Cnp,
                                     CLde, CYdr, Cmde, Cndr, Cldr, Clda, Cnda,

                                     v, w, r, q,
                                     T, dE, dR, dA,
                                     v_dot, w_dot, r_dot, q_dot,
                                     Faero_b, T_b);

    } else if (identMode == LATERAL) {
        /** LATERAL IDENTIFICATION PARAMETERS ----------------------------------------------------------------------- */

        double imuPitchOffset = KiteProps.Geometry.ImuPitchOffset_deg * M_PI / 180.0;

        double CL0 = KiteProps.Aerodynamics.Cl0;                // Identify also in Lateral identification?
        //double CL0_t = KiteProps.Aerodynamics.CL0_tail;
        double CLa_tot = KiteProps.Aerodynamics.CLa_total;      // Identify also in Lateral identification?
        //double CLa_w = KiteProps.Aerodynamics.CLa_wing;
        //double CLa_t = KiteProps.Aerodynamics.CLa_tail;
        double e_o = KiteProps.Aerodynamics.e_oswald;

        //double dw = CLa_tot / (pi * e_o * AR);             /** downwash acting at the tail [] **/

        double CD0_tot = KiteProps.Aerodynamics.CD0_total;      // Identify also in Lateral identification?
        //double CD0_w = KiteProps.Aerodynamics.CD0_wing;
        //double CD0_t = KiteProps.Aerodynamics.CD0_tail;
        SX CYb = SX::sym("CYb");            //double CYb = KiteProps.Aerodynamics.CYb;
        //double CYb_vt = KiteProps.Aerodynamics.CYb_vtail;
        double Cm0 = KiteProps.Aerodynamics.Cm0;
        double Cma = KiteProps.Aerodynamics.Cma;
        //SX Cn0 = SX::sym("Cn0");
        double Cn0 = KiteProps.Aerodynamics.Cn0;
        SX Cnb = SX::sym("Cnb");
        //SX Cl0 = SX::sym("Cl0");
        double Cl0 = KiteProps.Aerodynamics.Cl0;
        SX Clb = SX::sym("Clb");

        double CLq = KiteProps.Aerodynamics.CLq;
        double Cmq = KiteProps.Aerodynamics.Cmq;
        SX CYr = SX::sym("CYr");
        SX Cnr = SX::sym("Cnr");
        SX Clr = SX::sym("Clr");
        SX CYp = SX::sym("CYp");
        SX Clp = SX::sym("Clp");
        SX Cnp = SX::sym("Cnp");

        /** ------------------------------ **/
        /** Aerodynamic effects of control **/
        /** ------------------------------ **/
        double CLde = KiteProps.Aerodynamics.CLde;
        SX CYdr = SX::sym("CYdr");
        double Cmde = KiteProps.Aerodynamics.Cmde;
        SX Cndr = SX::sym("Cndr");
        SX Cldr = SX::sym("Cldr");
        //SX CDde = SX::sym("CDde");
        SX Clda = SX::sym("Clda");
        SX Cnda = SX::sym("Cnda");

        //double CL_daoa = -2 * CLa_t * Vh * dw;
        //double Cm_daoa = -2 * CLa_t * Vh * (lt/c) * dw;
        params = SX::vertcat({params,
                              CYb, Cnb, Clb,
                              CYr, Cnr, Clr, CYp, Clp, Cnp,
                              CYdr, Cndr, Cldr, Clda, Cnda
                             }); // 14 lateral parameters

        /* Info: <General, Lon, Lat> Types for parameter groups, defined in function call */
        getModel<double, double, SX>(g, rho, windFrom_deg, windSpeed,
                                     b, c, AR, S,
                                     Mass, Ixx, Iyy, Izz, Ixz,
                                     imuPitchOffset, CL0, CLa_tot, e_o,
                                     CD0_tot, CYb, Cm0, Cma, Cn0, Cnb, Cl0, Clb,
                                     CLq, Cmq, CYr, Cnr, Clr, CYp, Clp, Cnp,
                                     CLde, CYdr, Cmde, Cndr, Cldr, Clda, Cnda,

                                     v, w, r, q,
                                     T, dE, dR, dA,
                                     v_dot, w_dot, r_dot, q_dot,
                                     Faero_b, T_b);

    }

//    /** ------------------------------ **/
//    /**        Tether parameters       **/
//    /** ------------------------------ **/
//    double Ks = KiteProps.Tether.Ks;
//    double Kd = KiteProps.Tether.Kd;
//    double Lt = KiteProps.Tether.length;
//    double rx = KiteProps.Tether.rx;
// //   double ry = KiteProps.Tether.ry;
//    double rz = KiteProps.Tether.rz;
//
//    params = SX::vertcat({params,
//                                 //                               Ks, Kd, Lt, rx, rz,
//                         }); // 5 Tether parameters

    /** Complete dynamics of the Kite */
    auto state = SX::vertcat({v, w, r, q});
    auto control = SX::vertcat({T, dE, dR, dA});
    auto dynamics = SX::vertcat({v_dot, w_dot, r_dot, q_dot});

    Function dyn_func = Function("dynamics", {state, control, params}, {dynamics});
    Function specNongravForce_func = Function("spec_nongrav_force", {state, control}, {(Faero_b + T_b) / Mass});

    /** compute dynamics state Jacobian */
    SX d_jacobian = SX::jacobian(dynamics, state);
    Function dyn_jac = Function("dyn_jacobian", {state, control, params}, {d_jacobian});

    /** define RK4 integrator scheme */
    SX X = SX::sym("X", state.size1());
    SX U = SX::sym("U", control.size1());
    SX dT = SX::sym("dT");

    /** get symbolic expression for RK4 integrator */
    //SX sym_integrator = kmath::rk4_symbolic(X, U, dyn_func, dT);
    //Function RK4_INT = Function("RK4", {X,U,dT},{sym_integrator});

    /** CVODES integrator */
    /** @todo: make smarter initialisation of integrator */
    //double h = AlgoProps.sampling_time;
    //SX int_parameters = SX::vertcat({control, params});
    //SXDict ode = {{"x", state}, {"p", int_parameters}, {"ode", dynamics}};
    //Dict opts = {{"tf", h}};
    //Function CVODES_INT = integrator("CVODES_INT", "cvodes", ode, opts);

    /** assign class atributes */
    this->State = state;
    this->Control = control;
    this->Parameters = params;
    this->SymDynamics = dynamics;
    //this->SymIntegartor = sym_integrator;
    this->SymJacobian = d_jacobian;

    this->NumDynamics = dyn_func;
    this->NumSpecNongravForce = specNongravForce_func;
    this->NumJacobian = dyn_jac;

    /** return integrator function */
    /**
    if(AlgoProps.Integrator == IntType::CVODES)
        this->NumIntegrator = CVODES_INT;
    else
        this->NumIntegrator = RK4_INT;
        */
}



/** --------------------- */
/** Rigid Body Kinematics */
/** --------------------- */

RigidBodyKinematics::RigidBodyKinematics(const AlgorithmProperties &AlgoProps) {
    algo_props = AlgoProps;

    /** define system dynamics */
    SX r = SX::sym("r", 3);
    SX q = SX::sym("q", 4);
    SX vb = SX::sym("vb", 3);
    SX wb = SX::sym("wb", 3);
    SX u = SX::sym("u", 3);

    /** translation */
    SX qv = kmath::quat_multiply(q, SX::vertcat({0, vb}));
    SX qv_q = kmath::quat_multiply(qv, kmath::quat_inverse(q));
    SX rdot = qv_q(Slice(1, 4), 0);

    /** rotation / with norm correction*/
    double lambda = -10;
    SX qdot = 0.5 * kmath::quat_multiply(q, SX::vertcat({0, wb})) + 0.5 * lambda * q * (SX::dot(q, q) - 1);

    /** velocities */
    SX vb_dot = SX::zeros(3);
    SX wb_dot = SX::zeros(3);

    state = SX::vertcat({vb, wb, r, q});
    SX Dynamics = SX::vertcat({vb_dot, wb_dot, rdot, qdot});
    NumDynamics = Function("RB_Dynamics", {state}, {Dynamics});

    /** Jacobian */
    SX Jacobian = SX::jacobian(Dynamics, state);
    NumJacobian = Function("RB_Jacobian", {state, u}, {Jacobian});

    /** Integrators */
    /** CVODES */
    double h = algo_props.sampling_time;
    SXDict ode = {{"x",   state},
                  {"ode", Dynamics}};
    Dict opts = {{"tf", h}};
    Function CVODES_INT = integrator("CVODES_INT", "cvodes", ode, opts);
    NumIntegartor = CVODES_INT;
}

