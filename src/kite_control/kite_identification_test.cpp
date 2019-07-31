#include "kiteNMPF.h"
#include "integrator.h"

#define  BOOST_TEST_TOOLS_UNDER_DEBUGGER
#define BOOST_TEST_MODULE kite_identification_test
#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include "pseudospectral/chebyshev.hpp"

using namespace casadi;

BOOST_AUTO_TEST_SUITE( kite_identification_test )

BOOST_AUTO_TEST_CASE( first_id_test )
{
    /** Load identification data */
    std::ifstream id_data_file("id_data_state.txt", std::ios::in);
    std::ifstream id_control_file("id_data_control.txt", std::ios::in);
    const int DATA_POINTS = 61;
    DM id_data    = DM::zeros(13, DATA_POINTS);
    DM id_control = DM::zeros(3, DATA_POINTS);

    /** load state trajectory */
    if(!id_data_file.fail())
    {
    for(uint i = 0; i < DATA_POINTS; ++i) {
        for(uint j = 0; j < 13; ++j){
            double entry;
            id_data_file >> entry;
            id_data(j,i) = entry;
        }
    }
    }
    else
    {
        std::cout << "Could not open : id state data file \n";
        id_data_file.clear();
    }

    /** load control data */
    if(!id_control_file.fail())
    {
    for(uint i = 0; i < DATA_POINTS; ++i){
        for(uint j = 0; j < 3; ++j){
            double entry;
            id_control_file >> entry;
            /** put in reverse order to comply with Chebyshev method */
            id_control(j,DATA_POINTS - 1 - i) = entry;
        }
    }
    }
    else
    {
        std::cout << "Could not open : id control data file \n";
        id_control_file.clear();
    }

    /** define kite dynamics */
    std::string kite_params_file = "umx_radian_matlab.yaml";
    KiteProperties kite_props = kite_utils::LoadProperties(kite_params_file);

    AlgorithmProperties algo_props;
    algo_props.Integrator = CVODES;
    algo_props.sampling_time = 0.02;
    std::shared_ptr<KiteDynamics> kite     = std::make_shared<KiteDynamics>(kite_props, algo_props, true);
    std::shared_ptr<KiteDynamics> kite_int = std::make_shared<KiteDynamics>(kite_props, algo_props); //integration model
    Function ode = kite_int->getNumericDynamics();

    /** get dynamics function and state Jacobian */
    Function DynamicsFunc = kite->getNumericDynamics();
    SX X = kite->getSymbolicState();
    SX U = kite->getSymbolicControl();
    SX P = kite->getSymbolicParameters();

    /** state bounds */
    DM LBX = DM::vertcat({2.0, -DM::inf(1), -DM::inf(1), -4 * M_PI, -4 * M_PI, -4 * M_PI, -DM::inf(1), -DM::inf(1), -DM::inf(1),
                          -1.05, -1.05, -1.05, -1.05});
    DM UBX = DM::vertcat({DM::inf(1), DM::inf(1), DM::inf(1), 4 * M_PI, 4 * M_PI, 4 * M_PI, DM::inf(1), DM::inf(1), DM::inf(1),
                          1.05, 1.05, 1.05, 1.05});
    /** control bounds */
    DM LBU = DM::vec(id_control);
    DM UBU = DM::vec(id_control);

    /** parameter bounds */
    YAML::Node config = YAML::LoadFile("umx_radian3.yaml");
    double CL0 = config["aerodynamic"]["CL0"].as<double>();
    double CLa_tot = config["aerodynamic"]["CLa_total"].as<double>();

    double CD0_tot = config["aerodynamic"]["CD0_total"].as<double>();
    double CYb = config["aerodynamic"]["CYb"].as<double>();
    double Cm0 = config["aerodynamic"]["Cm0"].as<double>();
    double Cma = config["aerodynamic"]["Cma"].as<double>();
    double Cnb = config["aerodynamic"]["Cnb"].as<double>();
    double Clb = config["aerodynamic"]["Clb"].as<double>();

    double CLq = config["aerodynamic"]["CLq"].as<double>();
    double Cmq = config["aerodynamic"]["Cmq"].as<double>();
    double CYr = config["aerodynamic"]["CYr"].as<double>();
    double Cnr = config["aerodynamic"]["Cnr"].as<double>();
    double Clr = config["aerodynamic"]["Clr"].as<double>();
    double CYp = config["aerodynamic"]["CYp"].as<double>();
    double Clp = config["aerodynamic"]["Clp"].as<double>();
    double Cnp = config["aerodynamic"]["Cnp"].as<double>();

    double CLde = config["aerodynamic"]["CLde"].as<double>();
    double CYdr = config["aerodynamic"]["CYdr"].as<double>();
    double Cmde = config["aerodynamic"]["Cmde"].as<double>();
    double Cndr = config["aerodynamic"]["Cndr"].as<double>();
    double Cldr = config["aerodynamic"]["Cldr"].as<double>();

    double Lt = config["tether"]["length"].as<double>();
    double Ks = config["tether"]["Ks"].as<double>();
    double Kd = config["tether"]["Kd"].as<double>();
    double rx = config["tether"]["rx"].as<double>();
    double rz = config["tether"]["rz"].as<double>();

    DM REF_P = DM::vertcat({CL0, CLa_tot, CD0_tot, CYb, Cm0, Cma, Cnb, Clb, CLq, Cmq,
                            CYr, Cnr, Clr, CYp, Clp, Cnp, CLde, CYdr, Cmde, Cndr, Cldr});
    DM LBP = REF_P; DM UBP = REF_P;
    LBP = -DM::inf(21);
    UBP = DM::inf(21);

    /*
    LBP[0] = REF_P[0] -  0.1 * fabs(REF_P[0]); UBP[0] = REF_P[0] +  0.1 * fabs(REF_P[0]); // CL0
    LBP[1] = REF_P[1] - 0.05 * fabs(REF_P[1]); UBP[1] = REF_P[1] +  0.1 * fabs(REF_P[1]); // CLa
    LBP[2] = REF_P[2] -  0.1 * fabs(REF_P[2]); UBP[2] = REF_P[2] + 0.25 * fabs(REF_P[2]); // CD0
    LBP[3] = REF_P[3] -  0.5 * fabs(REF_P[3]); UBP[3] = REF_P[3] +  0.5 * fabs(REF_P[3]); // CYb
    LBP[4] = REF_P[4] -  0.5 * fabs(REF_P[4]); UBP[4] = REF_P[4] +  0.5 * fabs(REF_P[4]); // Cm0
    LBP[5] = REF_P[5] -  0.1 * fabs(REF_P[5]); UBP[5] = REF_P[5] + 0.30 * fabs(REF_P[5]); // Cma
    LBP[6] = REF_P[6] -  0.5 * fabs(REF_P[6]); UBP[6] = REF_P[6] +  0.5 * fabs(REF_P[6]); // Cnb
    LBP[7] = REF_P[7] -  0.5 * fabs(REF_P[7]); UBP[7] = REF_P[7] +  0.5 * fabs(REF_P[7]); // Clb
    LBP[8] = REF_P[8] -  0.2 * fabs(REF_P[8]); UBP[8] = REF_P[8] +  0.2 * fabs(REF_P[8]); // CLq
    LBP[9] = REF_P[9] -  0.3 * fabs(REF_P[9]); UBP[9] = REF_P[9] +  0.3 * fabs(REF_P[9]); // Cmq

    LBP[10] = REF_P[10] -  0.3 * fabs(REF_P[10]); UBP[10] = REF_P[10] +  0.3 * fabs(REF_P[10]); // CYr
    LBP[11] = REF_P[11] -  0.5 * fabs(REF_P[11]); UBP[11] = REF_P[11] +  0.5 * fabs(REF_P[11]); // Cnr
    LBP[12] = REF_P[12] -  0.5 * fabs(REF_P[12]); UBP[12] = REF_P[12] +  0.5 * fabs(REF_P[12]); // Clr
    LBP[13] = REF_P[13] -  0.5 * fabs(REF_P[13]); UBP[13] = REF_P[13] +  0.5 * fabs(REF_P[13]); // CYp
    LBP[14] = REF_P[14] -  0.5 * fabs(REF_P[14]); UBP[14] = REF_P[14] +  0.5 * fabs(REF_P[14]); // Clp
    LBP[15] = REF_P[15] -  0.3 * fabs(REF_P[15]); UBP[15] = REF_P[15] +  1.0 * fabs(REF_P[15]); // Cnp
    LBP[16] = REF_P[16] -  0.5 * fabs(REF_P[16]); UBP[16] = REF_P[16] +  0.5 * fabs(REF_P[16]); // CLde
    LBP[17] = REF_P[17] -  0.5 * fabs(REF_P[17]); UBP[17] = REF_P[17] +  0.5 * fabs(REF_P[17]); // CYdr
    LBP[18] = REF_P[18] -  0.5 * fabs(REF_P[18]); UBP[18] = REF_P[18] +  0.5 * fabs(REF_P[18]); // Cmde
    LBP[19] = REF_P[19] -  0.5 * fabs(REF_P[19]); UBP[19] = REF_P[19] +  0.5 * fabs(REF_P[19]); // Cndr
    LBP[20] = REF_P[20] -  0.5 * fabs(REF_P[20]); UBP[20] = REF_P[20] +  0.5 * fabs(REF_P[20]); // Cldr
    */
    // LBP[21] = 2.65;    UBP[21] = 2.75;   // tether length
    // LBP[22] = 150.0;  UBP[22] = 150.0;  // Ks
    // LBP[23] = 0.0;    UBP[23] = 10;   // Kd
    // LBP[24] = 0.0;    UBP[24] = 0.0;   // rx
    // LBP[25] = 0.0;    UBP[25] = 0.0;  // rz

    std::cout << "OK so far \n";

    /** ----------------------------------------------------------------------------------*/
    const int num_segments = 2;
    const int poly_order   = 30;
    const int dimx         = 13;
    const int dimu         = 3;
    const int dimp         = 21;
    const double tf        = 3.0;

    Chebyshev<SX, poly_order, num_segments, dimx, dimu, dimp> spectral;
    SX diff_constr = spectral.CollocateDynamics(DynamicsFunc, 0, tf);
    diff_constr = diff_constr(casadi::Slice(0, num_segments * poly_order * dimx));

    SX varx = spectral.VarX();
    SX varu = spectral.VarU();
    SX varp = spectral.VarP();

    SX opt_var = SX::vertcat(SXVector{varx, varu, varp});

    SX lbg = SX::zeros(diff_constr.size());
    SX ubg = SX::zeros(diff_constr.size());

    /** set inequality (box) constraints */
    /** state */
    SX lbx = SX::repmat(LBX, num_segments * poly_order + 1, 1);
    SX ubx = SX::repmat(UBX, num_segments * poly_order + 1, 1);

    /** control */
    lbx = SX::vertcat({lbx, LBU});
    ubx = SX::vertcat({ubx, UBU});

    /** parameters */
    lbx = SX::vertcat({lbx, LBP});
    ubx = SX::vertcat({ubx, UBP});


    DM Q  = SX::diag(SX({1e3, 1e2, 1e2,  1e2, 1e2, 1e2,  1e1, 1e1, 1e1,  1e1, 1e1, 1e1, 1e1})); //good one as well
    //DM Q = 1e1 * DM::eye(13);
    double alpha = 100.0;


    SX fitting_error = 0;
    SX varx_ = SX::reshape(varx, 13, DATA_POINTS);
    for (uint j = 0; j < DATA_POINTS; ++j)
    {
        SX measurement = id_data(Slice(0, id_data.size1()), j);
        SX error = measurement - varx_(Slice(0, varx_.size1()), varx_.size2() - j - 1);
        fitting_error = fitting_error + (1 / DATA_POINTS) * SX::sum1( SX::mtimes(Q, pow(error, 2)) );
    }

    /** add regularisation */
    fitting_error = fitting_error + alpha * SX::dot(varp - SX({REF_P}), varp - SX({REF_P}));

    /** alternative approximation */
    SX x = SX::sym("x",13);
    SX y = SX::sym("y",13);
    SX cost_function = SX::sum1( SX::mtimes(Q, pow(x - y, 2)) );
    Function IdCost = Function("IdCost",{x,y}, {cost_function});
    SX fitting_error2 = spectral.CollocateIdCost(IdCost, id_data, 0, tf);
    fitting_error2 = fitting_error2 + alpha * SX::dot(varp - SX({REF_P}), varp - SX({REF_P}));

    /** formulate NLP */
    SXDict NLP;
    Dict OPTS;
    DMDict ARG;
    NLP["x"] = opt_var;
    NLP["f"] = fitting_error;
    NLP["g"] = diff_constr;

    OPTS["ipopt.linear_solver"]  = "ma97";
    OPTS["ipopt.print_level"]    = 5;
    OPTS["ipopt.tol"]            = 1e-4;
    OPTS["ipopt.acceptable_tol"] = 1e-4;
    OPTS["ipopt.warm_start_init_point"] = "yes";
    //OPTS["ipopt.max_iter"]       = 20;

    Function NLP_Solver = nlpsol("solver", "ipopt", NLP, OPTS);

    /** set default args */
    ARG["lbx"] = lbx;
    ARG["ubx"] = ubx;
    ARG["lbg"] = lbg;
    ARG["ubg"] = ubg;

    /** provide initial guess from integrator */
    casadi::DMDict props;
    props["scale"] = 0;
    props["P"] = casadi::DM::diag(casadi::DM({0.1, 1/3.0, 1/3.0, 1/2.0, 1/5.0, 1/2.0, 1/3.0, 1/3.0, 1/3.0, 1.0, 1.0, 1.0, 1.0}));
    props["R"] = casadi::DM::diag(casadi::DM({1/0.15, 1/0.2618, 1/0.2618}));
    PSODESolver<poly_order,num_segments,dimx,dimu>ps_solver(ode, tf, props);

    DM x0 = id_data(Slice(0, id_data.size1()), 0);
    DMDict solution = ps_solver.solve_trajectory(x0, LBU, true);
    DM feasible_state = solution.at("x");
    //DM feasible_state = DM::reshape(id_data, 13 * (num_segments * poly_order + 1), 1);
    //DM feasible_state = DM::repmat(id_data(Slice(0, id_data.size1()), 0), (num_segments * poly_order + 1), 1);
    DM feasible_control = (UBU + LBU) / 2;

    //ARG["x0"] = DM::vertcat(DMVector{feasible_state, feasible_control, REF_P});
    ARG["x0"] = DM::vertcat(DMVector{feasible_state, REF_P});
    ARG["lam_g0"] = solution.at("lam_g");
    ARG["lam_x0"] = DM::vertcat({solution.at("lam_x"), DM::zeros(REF_P.size1())});

    int idx_in = num_segments * poly_order * dimx;
    int idx_out = idx_in + dimx;
    ARG["lbx"](Slice(idx_in, idx_out), 0) = x0;
    ARG["ubx"](Slice(idx_in, idx_out), 0) = x0;

    DMDict res = NLP_Solver(ARG);
    DM result = res.at("x");

    DM new_params = result(Slice(result.size1() - varp.size1(), result.size1()));
    std::vector<double> new_params_vec = new_params.nonzeros();

    DM trajectory = result(Slice(0, varx.size1()));
    //DM trajectory = DM::reshape(traj, DATA_POINTS, dimx );
    std::ofstream trajectory_file("estimated_trajectory.txt", std::ios::out);

    if(!trajectory_file.fail())
    {
        for (int i = 0; i < trajectory.size1(); i = i + dimx)
        {
            std::vector<double> tmp = trajectory(Slice(i, i + dimx),0).nonzeros();
            for (uint j = 0; j < tmp.size(); j++)
            {
                trajectory_file << tmp[j] << " ";
            }
            trajectory_file << "\n";
        }
    }
    trajectory_file.close();

    /** update parameter file */
    config["aerodynamic"]["CL0"] = new_params_vec[0];
    config["aerodynamic"]["CLa_total"] = new_params_vec[1];
    config["aerodynamic"]["CD0_total"] = new_params_vec[2];
    config["aerodynamic"]["CYb"] = new_params_vec[3];
    config["aerodynamic"]["Cm0"] = new_params_vec[4];
    config["aerodynamic"]["Cma"] = new_params_vec[5];
    config["aerodynamic"]["Cnb"] = new_params_vec[6];
    config["aerodynamic"]["Clb"] = new_params_vec[7];

    config["aerodynamic"]["CLq"] = new_params_vec[8];
    config["aerodynamic"]["Cmq"] = new_params_vec[9];
    config["aerodynamic"]["CYr"] = new_params_vec[10];
    config["aerodynamic"]["Cnr"] = new_params_vec[11];
    config["aerodynamic"]["Clr"] = new_params_vec[12];
    config["aerodynamic"]["CYp"] = new_params_vec[13];
    config["aerodynamic"]["Clp"] = new_params_vec[14];
    config["aerodynamic"]["Cnp"] = new_params_vec[15];

    config["aerodynamic"]["CLde"] = new_params_vec[16];
    config["aerodynamic"]["CYdr"] = new_params_vec[17];
    config["aerodynamic"]["Cmde"] = new_params_vec[18];
    config["aerodynamic"]["Cndr"] = new_params_vec[19];
    config["aerodynamic"]["Cldr"] = new_params_vec[20];

    // config["tether"]["length"] = new_params_vec[21];
    // config["tether"]["Ks"] = new_params_vec[22];
    // config["tether"]["Kd"] = new_params_vec[23];
    // config["tether"]["rx"] = new_params_vec[24];
    // config["tether"]["rz"] = new_params_vec[25];

    std::ofstream fout("umx_radian_id.yaml");
    fout << config;

    BOOST_CHECK(true);
}


BOOST_AUTO_TEST_SUITE_END()
