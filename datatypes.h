#ifndef DATATYPES_H
#define DATATYPES_H

#include <itkImage.h>
#include <itkVectorImage.h>

typedef itk::Image< unsigned char, 3 > InputImageType;
typedef itk::VectorImage< float, 3 > OutputImageType;

#endif /* DATATYPES_H */
