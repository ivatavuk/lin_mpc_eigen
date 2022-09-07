#include "EigenLinearMpc.hpp"
#include "matplotlibcpp.hpp"
#include "QpProblem.hpp"
#include "ChronoCall.hpp"

namespace plt = matplotlibcpp;  

Eigen::VectorXd generate_ramp_vec(	uint32_t len, uint32_t ramp_half_period, double ramp_rate );
std::vector<double> eigen2stdVec(	Eigen::VectorXd eigen_vec );

int main()
{

  // define lawnmower reference
  uint32_t n_simulate_steps = 30;
  uint32_t horizon = 100;
  double Q = 10000.0;
  double R = 1.0;
  Eigen::VectorXd Y_d_full = generate_ramp_vec(horizon + n_simulate_steps, 20, 0.1);

  /**                   Define linear system
   * x = [px, dpx]^T
   * u = [ddpx]^T
   * 
   * y = [px]^T
   */
  double T = 0.05;
  Eigen::MatrixXd A(4, 4);
  A <<  1, 0, T, 0,
        0, 1, 0, T,
        0, 0, 1, 0,
        0, 0, 0, 1;

  Eigen::MatrixXd B(4, 2);
  B <<  T*T/2.0,  0,
        0,        T*T/2.0,
        T,        0,
        0,        T;

  Eigen::MatrixXd C(1, 4);
  C <<  1, 1, 0, 0;
  
  Eigen::MatrixXd D = Eigen::MatrixXd::Zero(1, 2);

  EigenLinearMpc::LinearSystem example_system(SparseQpProblem::sparseMatrixFromDense(A), 
                                              SparseQpProblem::sparseMatrixFromDense(B), 
                                              SparseQpProblem::sparseMatrixFromDense(C), 
                                              SparseQpProblem::sparseMatrixFromDense(D));

  
  Eigen::VectorXd x0(4);
  x0 << 0, 0, 0, 0;
  
  VecNd Y_d = Y_d_full.segment(0, horizon);

  VecNd u_lower_bound(2);
  u_lower_bound << -7, 0;
  VecNd u_upper_bound(2);
  u_upper_bound << 7, 0;

  EigenLinearMpc::MPC mpc(example_system, horizon, Y_d, x0, Q, R, u_lower_bound, u_upper_bound);
  VecNd U_sol;
  for(uint32_t i = 0; i < n_simulate_steps; i++)
  {
    if(i == 0)
    {
      std::cout << "First solver initialization:\n";
      ChronoCall(microseconds,
        mpc.initializeSolver();
      );
    }
    else
    { 
      std::cout << "i = " << i << "\n";
      Y_d = Y_d_full.segment(i, horizon);
      x0 = mpc.calculateX(U_sol).segment(0, 2);
      std::cout << "Updating MPC:\n";
      ChronoCall(microseconds,
        mpc.updateSolver(Y_d, x0);
      );
    }
    
    std::cout << "Solving:\n";
    ChronoCall(microseconds,
      U_sol = mpc.solve();
    );

    auto extracted_U = mpc.extractU(U_sol);
    for(auto curr_U : extracted_U)
    {
      plt::plot(curr_U);
      plt::show();
    }
    
    plt::plot(eigen2stdVec(Y_d));
    plt::plot(eigen2stdVec(mpc.calculateY(U_sol))); //Y does not show current point!!
    plt::show();
  }
  
  return 0;
}

Eigen::VectorXd generate_ramp_vec(	uint32_t len, uint32_t ramp_half_period, double ramp_rate )
{
  Eigen::VectorXd lawnmower_vec(len);

  lawnmower_vec(0) = 0.0;
  for(uint32_t i = 1; i < len; i++)
  {
    lawnmower_vec(i) = !((i / ramp_half_period) % 2) ?  lawnmower_vec(i-1) : lawnmower_vec(i-1) + ramp_rate;
  }

  return lawnmower_vec;
}

std::vector<double> eigen2stdVec(	Eigen::VectorXd eigen_vec )
{
  std::vector<double> ret_vec;
  for(uint32_t i = 0; i < eigen_vec.rows(); i++)
    ret_vec.push_back(eigen_vec(i));
  return ret_vec;
}