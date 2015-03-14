#ifndef _CLI_OPTIONS_H
#define _CLI_OPTIONS_H

#include <boost/program_options.hpp>
#include <vector>

class CliParser
{
public:
	CliParser();

	int parse_argv(int argc, char ** argv);

	const std::vector<std::string> get_modules_needing_help() const;
	const std::string get_input_image() const;
	const std::string get_output_image() const;
	const std::vector<std::string> get_computers() const;
	const std::vector< std::vector< std::string > > get_computers_options() const;

	void print_main_usage(std::ostream &os) const;

private:
	boost::program_options::options_description main_options_descriptions;
	boost::program_options::options_description computer_options_descriptions;
	std::vector< std::string > need_help;
	std::string input_image;
	std::string output_image;
	std::vector< std::string > computers;
	std::vector< std::vector< std::string > > computers_options;
};

#endif /* _CLI_OPTIONS_H */
