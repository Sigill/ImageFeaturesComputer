#include <boost/program_options.hpp>

#include "image_loader.h"

#include "log4cxx/logger.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/basicconfigurator.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkScalarImageToHaralickTextureFeaturesImageFilter.h"

#include <itkImageFileWriter.h>

#include <vector>
#include <boost/regex.hpp>

#include <iostream>

namespace po = boost::program_options;

typedef itk::RescaleIntensityImageFilter< ImageLoader::ImageType, ImageLoader::ImageType > RescaleFilter;
typedef typename itk::Statistics::ScalarImageToHaralickTextureFeaturesImageFilter< ImageLoader::ImageType, double > HaralickFilter;
typedef typename HaralickFilter::OutputImageType HaralickImageType;

typedef itk::ImageFileWriter< HaralickImageType > OutputImageWriter;

class cli_offset {
public :
	cli_offset(unsigned int o1, unsigned int o2, unsigned int o3) : m_offset(3)
	{
		m_offset[0] = o1; m_offset[1] = o2; 	m_offset[2] = o3;
	}

	std::vector< unsigned int > getOffset() { return m_offset; }

	const unsigned int operator[](int i) const { return m_offset[i]; }

private:
	std::vector< unsigned int > m_offset;
};

void validate(boost::any& v, const std::vector<std::string>& values, cli_offset* target_type, int)
{
	static boost::regex r("(\\d+),(\\d+),(\\d+)");

	using namespace boost::program_options;

	// Make sure no previous assignment to 'v' was made.
	validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = validators::get_single_string(values);

	// Do regex match and convert the interesting part to int.
	boost::smatch match;
	if (boost::regex_match(s, match, r)) {
		v = boost::any(cli_offset(boost::lexical_cast<unsigned int>(match[1]), boost::lexical_cast<unsigned int>(match[2]), boost::lexical_cast<unsigned int>(match[3])));
	} else {
		throw invalid_option_value(s);
	}
}

std::ostream &operator<<(std::ostream &out, cli_offset& t)
{
	std::vector<unsigned int> vec = t.getOffset();

	std::copy(vec.begin(), vec.end(), std::ostream_iterator<unsigned int>(out, ", ") );

	return out;
}

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
	unsigned char posterization_level;
	cli_offset window(0, 0, 0), offset(0, 0, 0);

	LOG4CXX_DEBUG(logger, "Parsing command line options");

	po::options_description desc("Command line parameters");
	desc.add_options()
		("help,h",
			"Produce help message")
		("image,i",
			po::value< std::string >(&input_image_path)->required(), "Input image (required)")
		("posterization,p",
			po::value< unsigned char >(&posterization_level)->required(), "Posterization level (required)")
		("window,w",
			po::value< cli_offset >(&window)->required(), "Window radius (required)")
		("offset,o",
			po::value< cli_offset >(&offset)->required(), "Offset (required)")
		;

	po::variables_map vm;

	try {
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		// Handling --help before notify() in order to allow ->required()
		// http://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options#answer-5517755
		if (vm.count("help")) {
			std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
			std::cout << desc;
			return 0;
		}

		po::notify(vm);
	} catch(po::error &err) {
		LOG4CXX_FATAL(logger, err.what());
		return -1;
	}

	LOG4CXX_INFO(logger, "Input image: " << input_image_path);

	ImageLoader::ImageType::Pointer input_image;
	try {
		input_image = ImageLoader::load(input_image_path);
	} catch (ImageLoadingException &ex) {
		LOG4CXX_FATAL(logger, ex.what());
	}

    // Posterize the input image
    typename RescaleFilter::Pointer rescaler = RescaleFilter::New();
    rescaler->SetInput(input_image);
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(posterization_level - 1);
    rescaler->Update();

    // Compute the haralick features
    typename HaralickFilter::Pointer haralickImageComputer = HaralickFilter::New();
    typename HaralickFilter::RadiusType window_radius = {{window[0], window[1], window[2]}};
    haralickImageComputer->SetInput(rescaler->GetOutput());
    haralickImageComputer->SetWindowRadius(window_radius);
    haralickImageComputer->SetNumberOfBinsPerAxis(posterization_level);

    typename HaralickFilter::OffsetType offset1 = {{offset[0], offset[1], offset[2]}};
    typename HaralickFilter::OffsetVectorType::Pointer offsetV = HaralickFilter::OffsetVectorType::New();
    offsetV->push_back(offset1);
    haralickImageComputer->SetOffsets(offsetV);
	haralickImageComputer->Update();

	OutputImageWriter::Pointer writer = OutputImageWriter::New();
	writer->SetFileName("out.mhd");
	writer->SetInput(haralickImageComputer->GetOutput());
	writer->Update();

	LOG4CXX_INFO(logger, "Image written");
}



