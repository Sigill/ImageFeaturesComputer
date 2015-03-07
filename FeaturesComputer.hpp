#ifndef FEATURESCOMPUTER_HPP
#define FEATURESCOMPUTER_HPP

#include "datatypes.h"

#ifdef USE_LOG4CXX
#  include "log4cxx/logger.h"
#endif

class FeaturesComputer
{
public:
	FeaturesComputer() {}
	virtual ~FeaturesComputer() {}
	virtual OutputImageType::Pointer compute(InputImageType::Pointer input_image, std::vector< std::string > params) = 0;

#ifdef USE_LOG4CXX
	void setLogger(log4cxx::Logger *logger) {
		m_Logger = logger;
	}
#endif

	virtual void print_usage(std::ostream &os) = 0;

protected:
#ifdef USE_LOG4CXX
	log4cxx::LoggerPtr m_Logger;
#endif
};

// the types of the class factories
typedef FeaturesComputer* create_t();

#endif /* FEATURESCOMPUTER_HPP */
