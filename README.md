# ImageFeatureComputer

ImageFeatureComputer is an attempt to build a tool to compute any kind of image features. It has been especially designed to compute texture features (Haralick, local binary patterns...), but you can accomodate it to your needs as long as you need to transform a 2D/3D grayscale image to an image of the same size where each pixel is described by an array of floating point values.

Currently, it does not support a lot of features, but it manages the images IO (based on the [ITK](http://www.itk.org/) Segmentation and Registration Toolkit) and provides an interface to easily build new features computers as independant libraries.

Available computers are:

* Haralick: computes moving Haralick texture features (using [this](https://github.com/Sigill/ITK_Haralick) library).
* MeanValue: computes a blurred image.
* Coordinates: describes each pixel by its coordinates in the image.

It currently only works on Unix and Linux systems. The library loading part has only been designed for this operating system, but with some more work, it should also support other operating systems.

## How to build

Run CMake, and during the "configure" step, set the `ITK_DIR` variable to the `lib/cmake/ITK-X.Y/` directory of your ITK installation, and the `ITK_Haralick_DIR` variable to the `lib/cmake/` directory of your [ITK_Haralick]((https://github.com/Sigill/ITK_Haralick)) installation.

Then, build the project.

## How to use

    $ ./features_computer.sh -h
    Usage: ./image_features_computer.sh [options]
    Main options:
      -h [ --help ] arg         Produce help message
      -i [ --input-image ] arg  Input image (required)
      -o [ --output-image ] arg Ouput image (required)
    Computer options:
      -c [ --computer ] arg Features computers

Each feature computer also provides its own help. You just have to provide the name of the computer to the `--help` option. For example, for the Haralick computer:

    $ ./features_computer.sh -h Haralick
    HaralickComputer:
      -p [ --posterization ] arg Posterization level (required)
      -w [ --window ] arg        Window radius (required)
      -o [ --offset ] arg        Offset (required)

To process an image, you have to specify the input and output images, and for each feature computer, its associated options: 

    ./features_computer.sh -i input.bmp -o output.mha -c Haralick -p 16 -w 7,7,1 --offset 1,0,0 -c Haralick -p 16 -w 7,7,1 --offset 0,1,0

Multiple an different computers can be used at the same time, computed features will be concatenated in the output image (you have to use image format that support vector images, like [MetaImage](http://www.itk.org/Wiki/ITK/MetaIO/Documentation)).

If you want to implement your own computer, take example on the MeanValue or Coordinates computer.

A tool to remove some features from an image is also provided:

    $ ./channel_cutter -h
    Usage: ./channel_cutter [options]
    Main options:
      -h [ --help ]             Produce help message
      -i [ --input-image ] arg  Input image (required)
      -o [ --output-image ] arg Ouput image (required)
      -k [ --keep ] arg         Channels to keep (1-based)
      -r [ --remove ] arg       Channels to remove (1-based)

## License

This tool is released under the terms of the MIT License. See the LICENSE.txt file for more details.