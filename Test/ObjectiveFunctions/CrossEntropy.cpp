#include <shark/ObjectiveFunctions/Loss/CrossEntropy.h>
#include <cmath>

#include <shark/ObjectiveFunctions/ErrorFunction.h>
#include <shark/Models/LinearModel.h>
#include <shark/Core/Random.h>
#include "TestLoss.h"

#define BOOST_TEST_MODULE OBJECTIVEFUNCTIONS_CROSSENTROPY
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


using namespace shark;
using namespace std;

BOOST_AUTO_TEST_SUITE (ObjectiveFunctions_CrossEntropy)

BOOST_AUTO_TEST_CASE( CROSSENTROPY_DERIVATIVES_TWO_CLASSES_SINGLE_INPUT ){
	unsigned int maxTests = 1000;
	for(unsigned int test = 0; test != maxTests; ++test){
		CrossEntropy<unsigned int, RealVector> loss;

		//sample point between -10,10
		RealMatrix testPoint(1,1);
		testPoint(0,0) = random::uni(random::globalRng(), -300.0,300.0);


		//sample label
		unsigned int label = random::coinToss(random::globalRng());
		double calcLabel = label ? 1 : -1;
		UIntVector labelVec(1);
		labelVec(0) = label;
		//the test results
		double valueResult = std::log(1 + std::exp(- calcLabel * testPoint(0,0)));
		RealVector estimatedDerivative = estimateDerivative(loss, testPoint, labelVec);

		//test eval
		
		double value = loss.eval(labelVec,testPoint);
		BOOST_CHECK_SMALL(value-valueResult, 1.e-13);

		//test evalDerivative (first)
		RealMatrix derivative;
		value = loss.evalDerivative(labelVec, testPoint, derivative);
		BOOST_CHECK_SMALL(value - valueResult, 1.e-13);
		BOOST_CHECK_SMALL(norm_2(row(derivative,0) - estimatedDerivative), 1.e-8);
	}
}
BOOST_AUTO_TEST_CASE( CROSSENTROPY_DERIVATIVES_TWO_CLASSES_TWO_INPUT ){
	unsigned int maxTests = 10000;
	for(unsigned int test = 0; test != maxTests; ++test){
		CrossEntropy<unsigned int, RealVector> loss;

		//sample point between -10,10
		RealMatrix testPoint(1,2);
		testPoint(0,0) = random::uni(random::globalRng(), -150.0,150.0);
		testPoint(0,1) = -testPoint(0,0);


		//sample label
		unsigned int label = random::coinToss(random::globalRng());
		UIntVector labelVec(1);
		labelVec(0) = label;
		//the test results
		double valueResult = std::log(1 + std::exp(-2* testPoint(0,label)));
		RealVector estimatedDerivative = estimateDerivative(loss, testPoint, labelVec);

		//test eval
		
		double value = loss.eval(labelVec,testPoint);
		BOOST_CHECK_SMALL(value-valueResult, 1.e-13);

		//test evalDerivative (first)
		RealMatrix derivative;
		value = loss.evalDerivative(labelVec, testPoint, derivative);
		BOOST_CHECK_SMALL(value - valueResult, 1.e-13);
		BOOST_CHECK_SMALL(norm_2(row(derivative,0) - estimatedDerivative), 1.e-8);
	}
}
BOOST_AUTO_TEST_CASE( CROSSENTROPY_DERIVATIVES_MULTI_CLASS ){
	unsigned int maxTests = 1000;
	for(unsigned int test = 0; test != maxTests; ++test){
		CrossEntropy<unsigned int, RealVector> loss;

		//sample point between -10,10
		
		RealMatrix testPoint(1,5);
		double norm = 0;
		for(std::size_t i = 0; i !=5; ++i){
			testPoint(0,i) = random::uni(random::globalRng(), -10.0,10.0);
			norm+=std::exp(testPoint(0,i));
		}


		//sample label
		unsigned int label = random::discrete(random::globalRng(), 0,4);
		UIntVector labelVec(1);
		labelVec(0) = label;
		//the test results
		double valueResult = std::log(norm)-testPoint(0,label);
		RealVector estimatedDerivative = estimateDerivative(loss, testPoint, labelVec);

		//test eval
		
		double value = loss.eval(labelVec,testPoint);
		BOOST_CHECK_SMALL(value-valueResult, 1.e-13);

		//test evalDerivative (first)
		RealMatrix derivative;
		value = loss.evalDerivative(labelVec, testPoint, derivative);
		BOOST_CHECK_SMALL(value - valueResult, 1.e-13);
		BOOST_CHECK_SMALL(norm_2(row(derivative,0) - estimatedDerivative), 1.e-9);

	}
}

BOOST_AUTO_TEST_CASE( CROSSENTROPY_DERIVATIVES_MULTI_CLASS_PROB ){
	unsigned int maxTests = 10;
	for(unsigned int test = 0; test != maxTests; ++test){
		CrossEntropy<RealVector, RealVector> loss;
		
		RealMatrix testPoint(1,5);
		RealMatrix testLabel(1,5);
		double norm = 0;
		for(std::size_t i = 0; i !=5; ++i){
			testPoint(0,i) = random::uni(random::globalRng(), -3.0,3.0);
			norm+=std::exp(testPoint(0,i));
		}
		//sample probability-vector as label
		noalias(row(testLabel,0)) = abs(blas::normal(random::globalRng(), 5, 0.0, 1.0, blas::cpu_tag()));//abs->positivity
		noalias(row(testLabel,0)) /= sum(row(testLabel,0));//sum to one constraint
		double result = std::log(norm) - inner_prod(row(testLabel,0), row(testPoint,0));
		
		RealVector estimatedDerivative = estimateDerivative(loss, testPoint, testLabel);

		//test eval
		
		double value = loss.eval(testLabel,testPoint);
		BOOST_CHECK_SMALL(value-result, 1.e-6);

		//test evalDerivative (first)
		RealMatrix derivative;
		value = loss.evalDerivative(testLabel, testPoint, derivative);
		BOOST_CHECK_SMALL(value - result, 1.e-6);
		BOOST_CHECK_SMALL(max(abs(row(derivative,0) - estimatedDerivative)), 1.e-6);

	}
}

BOOST_AUTO_TEST_SUITE_END()
