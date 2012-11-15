#ifndef FEATURESCOMPUTERLOADER_H
#define FEATURESCOMPUTERLOADER_H

#include "FeaturesComputer.hpp"
#include <string>
#include <stdexcept>

class FeaturesComputerLoader
{
private:
	void* module;
	FeaturesComputer* computer;

public:
	FeaturesComputerLoader(const std::string computer_name);
	~FeaturesComputerLoader();

	FeaturesComputer* operator->()
	{
		return this->computer;
	}
};

class FeaturesComputerLoadingException : public std::runtime_error
{
public:
	FeaturesComputerLoadingException ( const std::string &err ) :
		std::runtime_error (err) {}
};

#endif /* FEATURESCOMPUTERLOADER_H */
