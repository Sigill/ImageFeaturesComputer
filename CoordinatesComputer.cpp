#include "FeaturesComputer.hpp"

#include <itkImageRegionIteratorWithIndex.h>

#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

typedef itk::ImageRegionIteratorWithIndex< OutputImageType >  OutputImageIterator;

class CoordinatesComputer : public FeaturesComputer
{
private:
	boost::program_options::options_description options;
	unsigned int dimension;

public:
	CoordinatesComputer():
		options("CoordinatesComputer")
	{
		options.add_options()
			("dimension,d",
			 po::value< unsigned int >(&this->dimension)->default_value(3),
			 "Dimensions of the coordinates space: 2 or 3 (default)")
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

		if((this->dimension != 2) && (this->dimension != 3))
		{
			boost::program_options::validation_error err =
				po::validation_error(
					po::validation_error::invalid_option_value, 
					boost::lexical_cast<std::string>(this->dimension), 
					"dimension");
			throw err;
		}

		OutputImageType::Pointer output_image = OutputImageType::New();
		output_image->SetRegions( input_image->GetLargestPossibleRegion() );
		output_image->SetVectorLength(this->dimension);
		output_image->Allocate();

		OutputImageIterator ito(output_image, output_image->GetLargestPossibleRegion());

		OutputImageType::PixelType out_pix;
		OutputImageType::IndexType coords;
		out_pix.SetSize(this->dimension);
		ito.GoToBegin();
		while( !ito.IsAtEnd() )
		{
			coords = ito.GetIndex();

			out_pix[0] = coords[0];
			out_pix[1] = coords[1];
			if(this->dimension == 3)
				out_pix[2] = coords[2];

			ito.Set(out_pix);

			++ito;
		}

		return output_image;
	}
};

extern "C" FeaturesComputer* create() {
	return new CoordinatesComputer;
}
