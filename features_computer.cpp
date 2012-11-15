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

#include "FeaturesComputerLoader.h"

#include "itkComposeVectorImageFilter.h"

#include "itkImageFileWriter.h"

typedef itk::ComposeVectorImageFilter< OutputImageType, OutputImageType> JoinImageFilterType;
typedef itk::ImageFileWriter< OutputImageType > OutputImageWriter;

FeaturesComputer* load_plugin(const std::string name);

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

	if(parse_result == 0) // --help
	{
		std::vector< std::string > modules_needing_help = cli_parser.get_modules_needing_help();

		if(modules_needing_help.empty())
		{
			cli_parser.print_main_usage(std::cout);
		} else {
			std::vector< std::string >::const_iterator it;
			for(it = modules_needing_help.begin(); it < modules_needing_help.end(); ++it)
			{
				FeaturesComputerLoader p(*it);
				p->setLogger(logger);
				p->print_usage(std::cout);
			}
		}
	}
	
	if(parse_result <= 0) { // --help or error
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

		FeaturesComputerLoader p(computers.at(i));

		p->setLogger(logger);

		OutputImageType::Pointer output;

		// use the class
		try {
			output = p->compute(input_image, computers_options.at(i));
		} catch( std::exception &ex) {
			LOG4CXX_FATAL(logger, ex.what());
			return -1;
		}

		if(output_image.IsNull()) {
			output_image = output;
		} else {
			JoinImageFilterType::Pointer joinFilter = JoinImageFilterType::New();
			joinFilter->SetInput1(output_image);
			joinFilter->SetInput2(output);
			joinFilter->Update();

			output_image = joinFilter->GetOutput();
		}

		std::cout << "Done" << std::endl;
	}

	OutputImageWriter::Pointer writer = OutputImageWriter::New();
	writer->SetInput(output_image);
	writer->SetFileName(cli_parser.get_output_image());
	writer->Update();
}

FeaturesComputer* load_plugin(const std::string name)
{

}
