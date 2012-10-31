#include <boost/program_options.hpp>

#include "image_loader.h"

#include "log4cxx/logger.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/basicconfigurator.h"

#include <itkVectorImage.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <itkImageFileWriter.h>

namespace po = boost::program_options;

typedef itk::VectorImage< double, 3 > OutputImage;
typedef itk::ImageRegionIteratorWithIndex< OutputImage >  OutputImageIterator;
typedef itk::ImageFileWriter< OutputImage > OutputImageWriter;

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

	LOG4CXX_DEBUG(logger, "Parsing command line options");

	po::options_description desc("Command line parameters");
	desc.add_options()
		("help,h",
			"Produce help message")
		("image,i",
			po::value< std::string >(&input_image_path)->required(), "Input image (required)")
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

	OutputImage::Pointer output_image = OutputImage::New();
	output_image->SetRegions( input_image->GetLargestPossibleRegion() );
	output_image->SetVectorLength(3);
	output_image->Allocate();

	LOG4CXX_INFO(logger, "Output image allocated");

	input_image = NULL;

	OutputImageIterator ito(output_image, output_image->GetLargestPossibleRegion());

	OutputImage::PixelType out_pix;
	OutputImage::IndexType coords;
	out_pix.SetSize(3);
	ito.GoToBegin();
	while( !ito.IsAtEnd() )
	{
		coords = ito.GetIndex();

		out_pix[0] = coords[0];
		out_pix[1] = coords[1];
		out_pix[2] = coords[2];

		ito.Set(out_pix);

		++ito;
	}

	LOG4CXX_INFO(logger, "Coordinates computed");

	OutputImageWriter::Pointer writer = OutputImageWriter::New();
	writer->SetFileName("out.nrrd");
	writer->SetInput(output_image);
	writer->Update();

	LOG4CXX_INFO(logger, "Image written");
}


