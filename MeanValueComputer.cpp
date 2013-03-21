#include "FeaturesComputer.hpp"

#include <itkMeanImageFilter.h>
#include <itkDivideImageFilter.h>
#include <itkComposeImageFilter.h>

#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

typedef typename itk::Image< typename OutputImageType::PixelType::ValueType, 3 > FloatingPointImageType;
typedef typename itk::DivideImageFilter< FloatingPointImageType, FloatingPointImageType, FloatingPointImageType > RescaleImageFilterType;
typedef typename itk::ComposeImageFilter< FloatingPointImageType, OutputImageType > ComposeVectorImageFilterType;

class MeanValueComputer : public FeaturesComputer
{
private:
	boost::program_options::options_description options;
	unsigned int radius;
	bool normalization;

public:
	MeanValueComputer():
		options("MeanValueComputer")
	{
		options.add_options()
			("radius,r",
			 po::value< unsigned int >(&this->radius)->default_value(2),
			 "Radius of the mean filter")
			("normalize,n",
			 "Enables normalization (default: disabled)")
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

		this->normalization = vm.count("normalize") > 0;

		typedef itk::MeanImageFilter< InputImageType, FloatingPointImageType >  MeanFilterType;

		MeanFilterType::Pointer meanFilter = MeanFilterType::New();

		meanFilter->SetInput(input_image);

		InputImageType::SizeType indexRadius;
		indexRadius.Fill(this->radius);
		meanFilter->SetRadius( indexRadius );

		meanFilter->Update();

		FloatingPointImageType::Pointer fp_output_image = meanFilter->GetOutput();

		if(this->normalization) {
			RescaleImageFilterType::Pointer rescaler = RescaleImageFilterType::New();
			rescaler->SetInput(meanFilter->GetOutput());
			rescaler->SetConstant(255.0);
			rescaler->Update();

			fp_output_image = rescaler->GetOutput();
		}

		ComposeVectorImageFilterType::Pointer vectorComposer = ComposeVectorImageFilterType::New();
		vectorComposer->SetInput(0, fp_output_image);

		vectorComposer->Update();

		return vectorComposer->GetOutput();
	}
};

extern "C" FeaturesComputer* create() {
	return new MeanValueComputer;
}

