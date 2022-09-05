/**
 * @file OsqpEigenOptimization.cpp
 * @author Ivo Vatavuk
 * @copyright Released under the terms of the BSD 3-Clause License
 * @date 2022
 */

#include "OsqpEigenOptimization.hpp"

OsqpEigenOpt::OsqpEigenOpt( const SparseQpProblem &qp_problem, 
                            bool verbosity ) 
  : alpha_(1.0), qp_problem_(qp_problem), 
  n_(qp_problem.A_qp.rows()), 
  m_(qp_problem.A_eq.rows() + qp_problem.A_ieq.rows())
{
  initializeSolver(verbosity);
}

void OsqpEigenOpt::initializeSolver(bool verbosity) 
{
  solver_.settings()->setVerbosity(verbosity);
  solver_.settings()->setAlpha(alpha_);

  solver_.settings()->setAdaptiveRho(false);

  solver_.data()->setNumberOfVariables(n_);
  solver_.data()->setNumberOfConstraints(m_);

  solver_.data()->clearHessianMatrix();
  solver_.data()->setHessianMatrix(qp_problem_.A_qp);
  solver_.data()->setGradient(qp_problem_.b_qp);

  solver_.data()->clearLinearConstraintsMatrix();
  solver_.data()->setLinearConstraintsMatrix(qp_problem_.A_ieq);

  VecNd lower_bound_eq = -qp_problem_.b_eq;
  VecNd upper_bound_eq = -qp_problem_.b_eq;

  VecNd lower_bound_ieq = -inf * VecNd::Ones(qp_problem_.b_ieq.size());
  VecNd upper_bound_ieq = -qp_problem_.b_ieq;

  VecNd lower_bound(lower_bound_eq.size() + lower_bound_ieq.size());
  VecNd upper_bound(upper_bound_eq.size() + upper_bound_ieq.size());

  lower_bound << lower_bound_eq, lower_bound_ieq;
  upper_bound << upper_bound_eq, upper_bound_ieq;

  solver_.data()->setBounds(lower_bound, upper_bound);

  solver_.clearSolver();
  solver_.initSolver();
}

VecNd OsqpEigenOpt::solveProblem()
{
  auto exit_flag = solver_.solveProblem();
  return solver_.getSolution();
}

bool OsqpEigenOpt::checkFeasibility() //Call this after calling solve
{
  return !( (int) solver_.getStatus() == OSQP_PRIMAL_INFEASIBLE );
}

