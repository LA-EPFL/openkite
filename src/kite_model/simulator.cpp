#include "simulator.h"

using namespace casadi;

//void Simulator::controlCallback(const openkite::aircraft_controls::ConstPtr &msg)
//{
//    controls(0) = msg->thrust;
//    controls(1) = msg->elevator;
//    controls(2) = msg->rudder;
//    controls(3) = msg->ailerons;
//}

void Simulator::controlCallback(const sensor_msgs::JoyConstPtr &msg)
{
    controls(0) = msg->axes[0]; // Thrust
    controls(1) = msg->axes[1]; // Elevator
    controls(2) = msg->axes[2]; // Rudder
    controls(3) = msg->axes[3]; // Aileron
}

Simulator::Simulator(const ODESolver &odeSolver, const ros::NodeHandle &nh)
{
    m_odeSolver = std::make_shared<ODESolver>(odeSolver);
    m_nh        = std::make_shared<ros::NodeHandle>(nh);

    /** define dimensions first given solver object */
    controls = DM::zeros(m_odeSolver->dim_u());
    state    = DM::zeros(m_odeSolver->dim_x());

    /** initialize subscribers and publishers */
    int broadcast_state;
    m_nh->param<int>("broadcast_state", broadcast_state, 1);

    /** initialize subscribers and publishers */
    m_nh->param<bool>("simulate_tether", sim_tether, false);

    std::vector<double> initial_value;
    m_nh->getParam("init_state", initial_value);
    initialize(DM(initial_value));
    ROS_INFO_STREAM("Simulator initialized at: " << initial_value);

    //pose_pub  = m_nh->advertise<geometry_msgs::PoseStamped>("/sim/kite_pose", 100);
    state_pub   = m_nh->advertise<sensor_msgs::MultiDOFJointState>("/sim/kite_state", 1);
    tether_pub   = m_nh->advertise<geometry_msgs::Vector3Stamped>("/sim/tether_force", 1);

    control_sub = m_nh->subscribe("/sim/set/kite_controls", 100, &Simulator::controlCallback, this);

    msg_state.twist.resize(1);
    msg_state.transforms.resize(1);
    msg_state.wrench.resize(1);

    msg_state.header.frame_id = "kite_sim";
}

void Simulator::simulate()
{
    Dict p = m_odeSolver->getParams();
    double dt = p["tf"];

    /* Make sure thrust is not negative */
    if (static_cast<double>(controls(0)) < 0.0)
        controls(0) = 0.0;

    /* Get airspeed */
    DM airspeed_evaluated = m_NumericAirspeed(DMVector{state, controls})[0];
    airspeed = airspeed_evaluated.nonzeros()[0];

    /* Get specific nongravitational force before solving (altering) the state */
    DM specNongravForce_evaluated = m_NumericSpecNongravForce(DMVector{state, controls})[0];
    std::vector<double> specNongravForce_evaluated_vect = specNongravForce_evaluated.nonzeros();
    specNongravForce = specNongravForce_evaluated_vect;

    if (sim_tether) {
        /* Get tether force vector */
        DM specTethForce_evaluated = m_NumericSpecTethForce(DMVector{state, controls})[0];
        std::vector<double> specTethForce_evaluated_vect = specTethForce_evaluated.nonzeros();
        specTethForce = specTethForce_evaluated_vect;
    }

    state = m_odeSolver->solve(state, controls, dt);
    //std::cout << "length : " << DM::norm_2(state(Slice(6,9))) << "\n";
    //std::cout << "State: " << state << "\n";
}

void Simulator::publish_state()
{
    /** State message */
    std::vector<double> state_vec = state.nonzeros();

    msg_state.header.stamp = ros::Time::now();

    msg_state.twist[0].linear.x = state_vec[0];
    msg_state.twist[0].linear.y = state_vec[1];
    msg_state.twist[0].linear.z = state_vec[2];

    msg_state.twist[0].angular.x = state_vec[3];
    msg_state.twist[0].angular.y = state_vec[4];
    msg_state.twist[0].angular.z = state_vec[5];

    msg_state.transforms[0].translation.x = state_vec[6];
    msg_state.transforms[0].translation.y = state_vec[7];
    msg_state.transforms[0].translation.z = state_vec[8];

    msg_state.transforms[0].rotation.w = state_vec[9];
    msg_state.transforms[0].rotation.x = state_vec[10];
    msg_state.transforms[0].rotation.y = state_vec[11];
    msg_state.transforms[0].rotation.z = state_vec[12];

    /* Using wrench/force as for acceleration */
    msg_state.wrench[0].force.x = specNongravForce[0];
    msg_state.wrench[0].force.y = specNongravForce[1];
    msg_state.wrench[0].force.z = specNongravForce[2];

    /* Using wrench/torque for airspeed */
    msg_state.wrench[0].torque.x = airspeed;

    state_pub.publish(msg_state);
}

void Simulator::publish_pose()
{
    DM pose = getPose();
    std::vector<double> pose_vec = pose.nonzeros();

    geometry_msgs::PoseStamped msg_pose;
    msg_pose.header.stamp = ros::Time::now();

    msg_pose.pose.position.x = pose_vec[0];
    msg_pose.pose.position.y = pose_vec[1];
    msg_pose.pose.position.z = pose_vec[2];

    msg_pose.pose.orientation.w = pose_vec[3];
    msg_pose.pose.orientation.x = pose_vec[4];
    msg_pose.pose.orientation.y = pose_vec[5];
    msg_pose.pose.orientation.z = pose_vec[6];

    pose_pub.publish(msg_pose);
}

void Simulator::publish_tether_force() {

    msg_state.header.stamp = ros::Time::now();

    msg_tether.vector.x = specTethForce[0];
    msg_tether.vector.y = specTethForce[1];
    msg_tether.vector.z = specTethForce[2];

    tether_pub.publish(msg_tether);
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "simulator");
    ros::NodeHandle n("~");

    /** create a kite object */
    std::string kite_params_file;
    n.param<std::string>("kiteparams_path", kite_params_file, "_kiteparams_.yaml");
    std::cout << "Simulator: Using kite parameters from: " << kite_params_file << "\n";
    KiteProperties kite_props = kite_utils::LoadProperties(kite_params_file);
    n.param<double>("windFrom_deg", kite_props.Wind.WindFrom_deg, 180.0);
    n.param<double>("windSpeed", kite_props.Wind.WindSpeed, 0.0);

    std::cout << "Simulator: Wind from " << kite_props.Wind.WindFrom_deg << " deg at " << kite_props.Wind.WindSpeed << " m/s.\n";

    bool simulate_tether;
    n.param<bool>("simulate_tether", simulate_tether, false);
    if (simulate_tether)
        std::cout << "[Simulator]: Tether ON\n";

    AlgorithmProperties algo_props;
    algo_props.Integrator = RK4;
    algo_props.sampling_time = 0.02;
    KiteDynamics kite = KiteDynamics(kite_props, algo_props, simulate_tether, false);

    int broadcast_state;
    n.param<int>("broadcast_state", broadcast_state, 1);

    /** create an integrator instance */
    double node_rate;
    n.param<double>("node_rate", node_rate, 200);

    double sim_speed;
    n.param<double>("simulation_speed", sim_speed, 1.0);

    /** cast to seconds and round to ms */
    double dt = (1.0/node_rate);
    dt = std::roundf(sim_speed * dt * 1000) / 1000;

    Dict params({{"tf", dt}, {"tol", 1e-6}, {"method", CVODES}});
    Function ode = kite.getNumericDynamics();
    auto dynamics = kite.getAeroDynamicForces();

    ODESolver odeSolver(ode, params);

    Simulator simulator(odeSolver, n);
    simulator.sim_tether = simulate_tether;
    simulator.setNumericAirspeed(kite.getNumericAirspeed());
    simulator.setNumericSpecNongravForce(kite.getNumericSpecNongravForce());
    simulator.setNumericSpecTethForce(kite.getNumericSpecTethForce());

    ros::Rate loop_rate(node_rate); /** 50 Hz */

    while (ros::ok())
    {
        if(simulator.is_initialized())
        {
            simulator.simulate();
            if(broadcast_state)
                simulator.publish_state();
            else
                simulator.publish_pose();

            if (simulator.sim_tether) {
                simulator.publish_tether_force();
            }

            if (-simulator.getState().nonzeros()[8] < 0) {
                std::cout << "\n----------------------------------\n"
                             "Hit the ground. End of simulation.\n"
                          << "----------------------------------\n";
                return 0;
            }
        }

        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}
