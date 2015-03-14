#include "cli_parser.h"

#include <boost/filesystem.hpp>
#include <ostream>
#include <vector>

#include <boost/regex.hpp>

#ifdef USE_LOG4CXX
#  include "log4cxx/logger.h"
#endif

namespace po = boost::program_options;

CliParser::CliParser() :
	main_options_descriptions("Main options"),
	computer_options_descriptions("Computer options")
{
	this->main_options_descriptions.add_options()
		("help,h",
			po::value< std::vector< std::string > >()->zero_tokens()->multitoken(),
			"Produce help message")
		("input-image,i",
			po::value< std::string >(&(this->input_image))->required(),
			"Input image (required)")
		("output-image,o",
			po::value< std::string >(&(this->output_image))->required(),
			"Ouput image (required)")
		;

	this->computer_options_descriptions.add_options()
		("computer,c",
		 po::value< std::string >()->required(),
		 "Features computers")
		;
}

int CliParser::parse_argv(int argc, char ** argv)
{
#ifdef USE_LOG4CXX
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));
	LOG4CXX_INFO(logger, "Parsing command line options");
#endif

	po::variables_map vm;

	try {
		po::parsed_options recognized_main_options = 
			po::command_line_parser(argc, argv).options(main_options_descriptions).allow_unregistered().run();

		std::vector< po::basic_option< char > >::iterator it;

		po::store(recognized_main_options, vm);

		// Handling --help before notify() in order to allow ->required()
		// http://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options#answer-5517755
		if (vm.count("help")) {
			this->need_help = vm["help"].as< std::vector< std::string > >();
			return 0;
		}

		vm.notify();

		std::vector<std::string> unrecognized_options = po::collect_unrecognized(recognized_main_options.options, po::include_positional);

		po::parsed_options recognized_plugin_options = 
			po::command_line_parser(unrecognized_options).options(this->computer_options_descriptions).allow_unregistered().run();

		std::vector< std::vector< std::string > > computers_options;

		for ( it = recognized_plugin_options.options.begin() ; it < recognized_plugin_options.options.end(); it++ ) {
			if((*it).string_key.compare("computer") == 0)
			{
				this->computers.push_back((*it).value.front()); // The first & only option passed to --computer
				this->computers_options.push_back( std::vector< std::string >() );
			} else {
				std::vector< std::string >::iterator it2;
				for ( it2 = (*it).original_tokens.begin(); it2 < (*it).original_tokens.end(); it2++ ) {
					this->computers_options.back().push_back(*it2);
				}
			}
		}

		/*
		for(int i = 0; i < computers.size(); ++i)
		{
			std::cout << computers.at(i) << " ->";
			for(int j = 0; j < computers_options.at(i).size(); ++j)
			{
				std::cout << " " << computers_options.at(i).at(j);
			}
			std::cout << std::endl;
		}
		*/

	} catch(po::error &err) {
#ifdef USE_LOG4CXX
		LOG4CXX_FATAL(logger, err.what());
#endif
		return -1;
	}

#ifdef USE_LOG4CXX
	LOG4CXX_INFO(logger, "Input image: " << this->input_image);
	LOG4CXX_INFO(logger, "Output image: " << this->output_image);
#endif

	return 1;
}

const std::vector<std::string> CliParser::get_modules_needing_help() const
{
	return this->need_help;
}

const std::string CliParser::get_input_image() const
{
	return this->input_image;
}

const std::string CliParser::get_output_image() const
{
	return this->output_image;
}

const std::vector<std::string> CliParser::get_computers() const
{
	return this->computers;
}

const std::vector< std::vector< std::string > > CliParser::get_computers_options() const
{
	return this->computers_options;
}

void CliParser::print_main_usage(std::ostream &os) const
{
	os << "Usage: ./features_computer.sh [options]" << std::endl;
	os << this->main_options_descriptions;
	os << this->computer_options_descriptions;
}
