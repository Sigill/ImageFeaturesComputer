#include "FeaturesComputer.hpp"

#include <itkImageRegionIteratorWithIndex.h>

#include <iostream>

typedef itk::ImageRegionIteratorWithIndex< OutputImageType >  OutputImageIterator;

class CoordinatesComputer : public FeaturesComputer
{
	virtual OutputImageType::Pointer compute( InputImageType::Pointer input_image, std::vector< std::string > params )
	{
		OutputImageType::Pointer output_image = OutputImageType::New();
		output_image->SetRegions( input_image->GetLargestPossibleRegion() );
		output_image->SetVectorLength(3);
		output_image->Allocate();

		input_image = NULL;

		OutputImageIterator ito(output_image, output_image->GetLargestPossibleRegion());

		OutputImageType::PixelType out_pix;
		OutputImageType::IndexType coords;
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

		return output_image;
	}
};

extern "C" FeaturesComputer* create() {
	return new CoordinatesComputer;
}
