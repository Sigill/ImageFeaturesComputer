#include "log4cxx/logger.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/basicconfigurator.h"

#include <boost/program_options.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVectorImageToImageAdaptor.h"
#include "itkComposeImageFilter.h"

#include "datatypes.h"

#include "image_loader.h"

namespace po = boost::program_options;

typedef itk::ImageFileReader< OutputImageType > ImageReader;
typedef itk::ImageFileWriter< OutputImageType > ImageWriter;
typedef itk::VectorImageToImageAdaptor< typename OutputImageType::PixelType::ValueType, OutputImageType::ImageDimension > SingleChannelAdaptor;
typedef itk::ComposeImageFilter< SingleChannelAdaptor, OutputImageType > ChannelComposer;

std::ostream &operator<<(std::ostream &out, std::vector< int >& t)
{
	std::copy(t.begin(), t.end(), std::ostream_iterator<int>(out, ", ") );

	return out;
}

class out_of_range_predicate
{
public:
	out_of_range_predicate(const int l, const int h): lower_bound(l), higher_bound(h) {}
	bool operator()(const int v) { return (v < lower_bound) || (v > higher_bound); }
private:
	int lower_bound, higher_bound;
};

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

	std::string input_image_path;
	std::string output_image_path;
	std::vector< int > channels_to_keep;
	std::vector< int > channels_to_remove;

	po::options_description main_options("Main options");

	main_options.add_options()
		("help,h",
			"Produce help message")
		("input-image,i",
			po::value< std::string >(&input_image_path)->required(),
			"Input image (required)")
		("output-image,o",
			po::value< std::string >(&output_image_path)->required(),
			"Ouput image (required)")
		("keep,k",
			po::value< std::vector< int > >(&channels_to_keep)->multitoken(),
			"Channels to keep")
		("remove,r",
			po::value< std::vector< int > >(&channels_to_remove)->multitoken(),
			"Channels to remove")
		;

	po::variables_map vm;

	try {
		po::parsed_options recognized_main_options = po::command_line_parser(argc, argv).options(main_options).run();

		std::vector< po::basic_option< char > >::iterator it;

		po::store(recognized_main_options, vm);

		// Handling --help before notify() in order to allow ->required()
		// http://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options#answer-5517755
		if (vm.count("help")) {
			std::cerr << "Usage: " << argv[0] << " [options]" << std::endl;
			std::cerr << main_options;
			return 0;
		}

		vm.notify();
	} catch(po::error &err) {
		LOG4CXX_FATAL(logger, err.what());
		return -1;
	}

	LOG4CXX_INFO(logger, "Input image: " << input_image_path);
	LOG4CXX_INFO(logger, "Output image: " << output_image_path);

	ImageReader::Pointer reader = ImageReader::New();
	reader->SetFileName(input_image_path);
	try {
		reader->Update();
	} catch ( itk::ExceptionObject & err ) {
		LOG4CXX_FATAL(logger, "The image located at \"" << input_image_path << "\" is not readable");
		return -1;
	}

	if(channels_to_keep.empty() == channels_to_remove.empty()) { // XOR trick
		LOG4CXX_FATAL(logger, "You need to provide a list of either the channels to keep or the channels to remove (not both)");
		return -1;
	}

	if(!channels_to_remove.empty()) {
		/*
		std::stringstream list;
		list << channels_to_remove;
		LOG4CXX_INFO(logger, "Channels to remove: " << list.str());

		std::vector< int >::iterator it = std::find_if(channels_to_remove.begin(), channels_to_remove.end(), out_of_range_predicate(1, reader->GetOutput()->GetNumberOfComponentsPerPixel() + 1));
		if(it != channels_to_remove.end()) {
			LOG4CXX_FATAL(logger, "Channel #" << *it << " does not exists");
			return -1;
		}
		*/

		for(int i = 1; i <= reader->GetOutput()->GetNumberOfComponentsPerPixel(); ++i) {
			if(std::find(channels_to_remove.begin(), channels_to_remove.end(), i) == channels_to_remove.end()) { // i is not in the list of the channels to be removed
				channels_to_keep.push_back(i);
			}
		}
	}

	{
		std::stringstream list;
		list << channels_to_keep;
		LOG4CXX_INFO(logger, "Channels to keep: " << list.str());

		std::vector< int >::iterator it = std::find_if(channels_to_keep.begin(), channels_to_keep.end(), out_of_range_predicate(1, reader->GetOutput()->GetNumberOfComponentsPerPixel() + 1));
		if(it != channels_to_keep.end()) {
			LOG4CXX_FATAL(logger, "Channel #" << *it << " does not exists");
			return -1;
		}
	}

	
	ChannelComposer::Pointer channelComposer = ChannelComposer::New();
	int i = 0;
	std::vector< int >::iterator it = channels_to_keep.begin();
	for ( ; it != channels_to_keep.end(); ++i, ++it)
	{
		SingleChannelAdaptor::Pointer singleChannelAdaptor = SingleChannelAdaptor::New();
		singleChannelAdaptor->SetExtractComponentIndex(*it -  1);
		singleChannelAdaptor->SetImage(reader->GetOutput());

		channelComposer->SetInput(i, singleChannelAdaptor);
	}

	channelComposer->Update();

	ImageWriter::Pointer writer = ImageWriter::New();
	writer->SetInput(channelComposer->GetOutput());
	writer->SetFileName(output_image_path);
	writer->Update();
}
