#include "FeaturesComputerLoader.h"

#include <dlfcn.h>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

FeaturesComputerLoader::FeaturesComputerLoader(const std::string name)
{
	void* plug = dlopen(FeaturesComputerLoader::filename(name).c_str(), RTLD_LAZY);

	if (!plug) {
		std::stringstream err;
		err << "Cannot load library " << name << ": " << dlerror();

		throw FeaturesComputerLoadingException(err.str());
	}

	// reset errors
	dlerror();

	this->module = plug;

	// load the create symbols
	create_t* create_plug = (create_t*) dlsym(this->module, "create");
	const char* dlsym_error = dlerror();
	if (dlsym_error) {
		std::stringstream err;
		err << "Cannot load symbol create: " << dlerror();
		dlclose(this->module);

		throw FeaturesComputerLoadingException(err.str());
	}

	// create an instance of the class
	this->computer = create_plug();
}

FeaturesComputerLoader::~FeaturesComputerLoader()
{
	if(this->computer) {
		delete this->computer;
		this->computer = NULL;
	}
	if(this->module) {
		dlclose(this->module);
		this->module = NULL;
	}
}

FeaturesComputer* FeaturesComputerLoader::operator->()
{
	return this->computer;
}

/*
std::vector< std::string > FeaturesComputerLoader::getAvailableModules()
{
	std::vector< std::string > available_modules;

	static const boost::regex computerFilter( "lib.+Computer\\.so" );

	boost::filesystem::path path(".");
	boost::filesystem::directory_iterator end_itr;
	for( boost::filesystem::directory_iterator i( path ); i != end_itr; ++i ) {
		// Skip if not a file
		if( !boost::filesystem::is_regular_file( i->status() ) ) continue;

		boost::smatch match;

		// Skip if no match
		if( !boost::regex_match( i->path().filename().string(), match, computerFilter ) ) continue;

		// File matches, store it
		available_modules.push_back( FeaturesComputerLoader::computerName(i->path().filename().string()) );
	}

	return available_modules;
}
*/

std::string FeaturesComputerLoader::filename(const std::string name)
{
	std::ostringstream f;
	f << "lib" << name << "Computer.so";

	return f.str();
}

std::string FeaturesComputerLoader::computerName(const std::string filename)
{
	boost::regex exp("^lib(.+)Computer\\.so");
	boost::smatch match;
	if (boost::regex_search(filename, match, exp)) {
		return std::string(match[1].first, match[1].second);
	}
}

