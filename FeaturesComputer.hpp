#ifndef FEATURESCOMPUTER_HPP
#define FEATURESCOMPUTER_HPP

#include "datatypes.h"

#include <vector>

class FeaturesComputer
{
public:
	FeaturesComputer() {}
	virtual ~FeaturesComputer() {}
	virtual OutputImageType::Pointer compute(InputImageType::Pointer input_image, std::vector< std::string > params) = 0;
};

// the types of the class factories
typedef FeaturesComputer* create_t();
void destroy(FeaturesComputer* fc);

#endif /* FEATURESCOMPUTER_HPP */
