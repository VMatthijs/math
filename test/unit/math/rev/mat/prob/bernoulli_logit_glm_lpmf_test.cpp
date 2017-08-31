#include <stan/math/rev/mat.hpp>
#include <gtest/gtest.h>
#include <stan/math/prim/scal/fun/value_of.hpp>
#include <chrono>

using stan::math::var;
using Eigen::Dynamic;
using Eigen::Matrix;

typedef std::chrono::high_resolution_clock::time_point TimeVar;
#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

TEST(ProbDistributionsBernoulliLogitGLM, glm_matches_bernoulli_logit_doubles) {
  Matrix<int,Dynamic,1> n(3,1);
  n << 1, 0, 1;
  Matrix<double,Dynamic,Dynamic> x(3,2);
  x << -12, 46, -42,
       24, 25, 27; 
  Matrix<double,Dynamic,1> beta(2,1);
  beta << 0.3, 2;
  Matrix<double,Dynamic,1> alpha(3,1);
  alpha << 10, 23, 13;
  Matrix<double,Dynamic,1> theta(3,1);
  theta = x * beta + alpha;
  
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf(n, x, beta, alpha)));
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf<true>(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf<true>(n, x, beta, alpha)));
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf<false>(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf<false>(n, x, beta, alpha)));
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf<true, Matrix<int,Dynamic,1> >(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf<true, Matrix<int,Dynamic,1> >(n, x, beta, alpha)));
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf<false, Matrix<int,Dynamic,1> >(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf<false,  Matrix<int,Dynamic,1>>(n, x, beta, alpha)));
  EXPECT_FLOAT_EQ((stan::math::bernoulli_logit_lpmf<Matrix<int,Dynamic,1> >(n, theta)),
                  (stan::math::bernoulli_logit_glm_lpmf< Matrix<int,Dynamic,1> >(n, x, beta, alpha)));
          
}

TEST(ProbDistributionsBernoulliLogitGLM, glm_matches_bernoulli_logit_vars) {
  
  
  Matrix<int,Dynamic,1> n(3,1);
  n << 1, 0, 1;
  Matrix<var,Dynamic,Dynamic> x(3,2);
  x << -12, 46, -42,
       24, 25, 27;
  Matrix<var,Dynamic,1> beta(2,1);
  beta << 0.3, 2;
  Matrix<var,Dynamic,1> alpha(3,1);
  alpha << 10, 23, 13;
  Matrix<var,Dynamic,1> theta(3,1);
  theta = x * beta + alpha;
  
  
  var lp = stan::math::bernoulli_logit_lpmf(n, theta);
  lp.grad();
  
  stan::math::recover_memory();

  
  Matrix<int,Dynamic,1> n2(3,1);
  n2 << 1, 0, 1;
  Matrix<var,Dynamic,Dynamic> x2(3,2);
  x2 << -12, 46, -42,
       24, 25, 27; 
  Matrix<var,Dynamic,1> beta2(2,1);
  beta2 << 0.3, 2;
  Matrix<var,Dynamic,1> alpha2(3,1);
  alpha2 << 10, 23, 13;


  var lp2 = stan::math::bernoulli_logit_glm_lpmf(n2, x2, beta2, alpha2);
  lp2.grad();
  
  EXPECT_FLOAT_EQ(lp.val(),
                  lp2.val());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_FLOAT_EQ(beta[i].adj(), beta2[i].adj());
  }    
  for (size_t j = 0; j < 3; j++) {
    EXPECT_FLOAT_EQ(alpha[j].adj(), alpha2[j].adj());
    for (size_t i = 0; i < 2; i++) {
      EXPECT_FLOAT_EQ(x(j,i).adj(), x2(j,i).adj());
    }
  }        
}


TEST(ProbDistributionsBernoulliLogitGLM, glm_matches_bernoulli_logit_speed) {
  int R = 3000;
  int C = 20;  
  
  Matrix<int,Dynamic,1> n(R,1);
  for (size_t i = 0; i < R; i++) {
    n[i] = rand()%2;
  }
  Matrix<double,Dynamic,Dynamic> xreal =  Eigen::MatrixXd::Random(R,C);
  Matrix<double,Dynamic,1> betareal =  Eigen::MatrixXd::Random(C,1);
  Matrix<double,Dynamic,1> alphareal  =  Eigen::MatrixXd::Random(R,1);
  Matrix<var,Dynamic,1> beta = betareal;
  Matrix<var,Dynamic,1> theta(R,1);
  theta = xreal * beta + alphareal;
  
  TimeVar t1 = timeNow();
  var lp = stan::math::bernoulli_logit_lpmf(n, theta);
  lp.grad();
  TimeVar t2 = timeNow();
  stan::math::recover_memory();
  
  Matrix<var,Dynamic,1> beta2 = betareal;
  
  TimeVar t3 = timeNow();
  var lp2 = stan::math::bernoulli_logit_glm_lpmf(n, xreal, beta2, alphareal);
  lp2.grad();
  TimeVar t4 = timeNow();
  
  std::cout << "Existing Primitives:" << std::endl << duration(t2-t1) << std::endl  << "New Primitives:" << std::endl << duration(t4-t3) << std::endl;    
}
