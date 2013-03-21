#include "FeaturesComputer.hpp"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVectorImageToImageAdaptor.h"
#include "itkComposeImageFilter.h"

#include "itkScalarImageToHaralickTextureFeaturesImageFilter.h"

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

typedef itk::RescaleIntensityImageFilter< InputImageType, InputImageType > RescaleFilter;

typedef typename itk::Statistics::ScalarImageToHaralickTextureFeaturesImageFilter< InputImageType, typename OutputImageType::PixelType::ValueType > HaralickFilter;

typedef itk::VectorImageToImageAdaptor< typename OutputImageType::PixelType::ValueType, OutputImageType::ImageDimension > HaralickFeatureAdaptorType;

typedef typename itk::Image< typename OutputImageType::PixelType::ValueType, 3 > HaralickFeatureImageType;

typedef itk::RescaleIntensityImageFilter< HaralickFeatureAdaptorType, HaralickFeatureImageType > HaralickFeatureImageRescaleFilter;

class cli_offset {
public :
	cli_offset() : m_offset(3)
	{
		m_offset[0] = 0; m_offset[1] = 0; m_offset[2] = 0;
	}

	cli_offset(unsigned int o1, unsigned int o2, unsigned int o3) : m_offset(3)
	{
		m_offset[0] = o1; m_offset[1] = o2; m_offset[2] = o3;
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

typedef itk::ImageRegionIteratorWithIndex< OutputImageType >  OutputImageIterator;

class HaralickComputer : public FeaturesComputer
{
private:
	boost::program_options::options_description options;
	unsigned int posterization_level;
	cli_offset window;
	std::vector< cli_offset > offsets;

public:
	HaralickComputer():
		options("HaralickComputer")
	{
		options.add_options()
			("posterization,p",
				po::value< unsigned int >(&this->posterization_level)->required(), "Posterization level (required)")
			("window,w",
				po::value< cli_offset >(&this->window)->required(), "Window radius (required)")
			("offset,o",
				po::value< std::vector< cli_offset > >(&this->offsets)->required()->multitoken(), "Offset (required)")
			;
	}

	virtual void print_usage(std::ostream &os)
	{
		os << this->options;
	}

	virtual OutputImageType::Pointer compute( InputImageType::Pointer input_image, std::vector< std::string > params )
	{
		po::variables_map vm;

		po::store(po::command_line_parser(params).options(this->options).run(), vm);
		vm.notify();

		// Posterize the input image
		typename RescaleFilter::Pointer rescaler = RescaleFilter::New();
		rescaler->SetInput(input_image);
		rescaler->SetOutputMinimum(0);
		rescaler->SetOutputMaximum(this->posterization_level - 1);
		rescaler->Update();

		LOG4CXX_INFO(m_Logger, "Posterization done.");

		typename HaralickFilter::Pointer haralickImageComputer = HaralickFilter::New();
		haralickImageComputer->SetInput(rescaler->GetOutput());
		haralickImageComputer->SetNumberOfBinsPerAxis(this->posterization_level);

		{
			typename HaralickFilter::RadiusType window_radius =
				{{this->window[0], this->window[1], this->window[2]}};
			haralickImageComputer->SetWindowRadius(window_radius);
		}

		{
			typename HaralickFilter::OffsetVectorType::Pointer offsetV =
				HaralickFilter::OffsetVectorType::New();
			typename HaralickFilter::OffsetType offset;
			std::vector< cli_offset >::const_iterator offsets_it;
			for( offsets_it = this->offsets.begin(); offsets_it < this->offsets.end(); ++offsets_it)
			{
				offset[0] = (*offsets_it)[0];
				offset[1] = (*offsets_it)[1];
				offset[2] = (*offsets_it)[2];

				offsetV->push_back(offset);
			}

			haralickImageComputer->SetOffsets(offsetV);
		}

		haralickImageComputer->Update();

		LOG4CXX_INFO(m_Logger, "Computation of Haralick features done.");

		return haralickImageComputer->GetOutput();
	}
};

extern "C" FeaturesComputer* create() {
	return new HaralickComputer;
}

