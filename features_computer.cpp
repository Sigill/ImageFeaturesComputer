#include "log4cxx/logger.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/basicconfigurator.h"

#include "datatypes.h"

#include "cli_parser.h"
#include "image_loader.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <dlfcn.h>

#include "FeaturesComputer.hpp"

#include "itkComposeVectorImageFilter.h"

#include "itkImageFileWriter.h"

typedef itk::ComposeVectorImageFilter< OutputImageType, OutputImageType> JoinImageFilterType;
typedef itk::ImageFileWriter< OutputImageType > OutputImageWriter;

int main(int argc, char** argv)
{
	log4cxx::BasicConfigurator::configure(
			log4cxx::AppenderPtr(new log4cxx::ConsoleAppender(
					log4cxx::LayoutPtr(new log4cxx::PatternLayout("\%-5p - [%c] - \%m\%n")),
					log4cxx::ConsoleAppender::getSystemErr()
					)
				)
			);

	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));

	CliParser cli_parser;
	int parse_result = cli_parser.parse_argv(argc, argv);
	if(parse_result <= 0) {
		exit(parse_result);
	}

	InputImageType::Pointer input_image;
	try {
		input_image = ImageLoader::load(cli_parser.get_input_image());
	} catch (ImageLoadingException &ex) {
		LOG4CXX_FATAL(logger, ex.what());
	}

	std::vector< std::string > computers = cli_parser.get_computers();
	std::vector< std::vector< std::string > > computers_options = cli_parser.get_computers_options();

	OutputImageType::Pointer output_image;

	for (int i = 0; i < computers.size(); ++i)
	{
		std::cout << "Running: " << computers.at(i) << std::endl;

		std::ostringstream plugin_file;
		plugin_file << "./lib" << computers.at(i) << ".so";

		void* plug = dlopen(plugin_file.str().c_str(), RTLD_LAZY);
		if (!plug) {
			LOG4CXX_FATAL(logger, "Cannot load library: " << dlerror());
			return 1;
		}

		// reset errors
		dlerror();

		// load the symbols
		create_t* create_plug = (create_t*) dlsym(plug, "create");
		const char* dlsym_error = dlerror();
		if (dlsym_error) {
			LOG4CXX_FATAL(logger, "Cannot load symbol create: " << dlsym_error);
			return 1;
		}

		// create an instance of the class
		FeaturesComputer* p = create_plug();

		p->setLogger(logger);

		// use the class
		OutputImageType::Pointer output = p->compute(input_image, computers_options.at(i));

		if(output_image.IsNull()) {
			output_image = output;
		} else {
			JoinImageFilterType::Pointer joinFilter = JoinImageFilterType::New();
			joinFilter->SetInput1(output_image);
			joinFilter->SetInput2(output);
			joinFilter->Update();

			output_image = joinFilter->GetOutput();
		}

		// destroy the class
		//destroy_plug(p);
		destroy(p);

		// unload the plug library
		dlclose(plug);

		std::cout << "Done" << std::endl;
	}

	OutputImageWriter::Pointer writer = OutputImageWriter::New();
	writer->SetInput(output_image);
	writer->SetFileName(cli_parser.get_output_image());
	writer->Update();
}
