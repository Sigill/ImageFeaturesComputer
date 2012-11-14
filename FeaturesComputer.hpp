#ifndef FEATURESCOMPUTER_HPP
#define FEATURESCOMPUTER_HPP

#include "datatypes.h"

#include "log4cxx/logger.h"

class FeaturesComputer
{
public:
	FeaturesComputer() {}
	virtual ~FeaturesComputer() {}
	virtual OutputImageType::Pointer compute(InputImageType::Pointer input_image, std::vector< std::string > params) = 0;

	void setLogger(log4cxx::Logger *logger) {
		m_Logger = logger;
	}

protected:
	log4cxx::LoggerPtr m_Logger;
};

// the types of the class factories
typedef FeaturesComputer* create_t();
void destroy(FeaturesComputer* fc);

#endif /* FEATURESCOMPUTER_HPP */
