#include "image_loader.h"

#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"

#include <ostream>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "log4cxx/logger.h"

typedef itk::ImageFileReader< InputImageType > ImageReader;
typedef itk::ImageSeriesReader< InputImageType > ImageSeriesReader;

InputImageType::Pointer ImageLoader::load(const std::string filename)
{
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));

	LOG4CXX_INFO(logger, "Loading image \"" << filename << "\"");

	try
	{
		boost::filesystem::path path(filename);

		if(boost::filesystem::exists(path)) {
			InputImageType::Pointer img;

			if(boost::filesystem::is_directory(path))
			{
				LOG4CXX_DEBUG(logger, path << " is a folder");

				img = loadImageSerie(filename);
			} else {
				LOG4CXX_DEBUG(logger, path << " is a file");

				img = loadImage(filename);
			}

			LOG4CXX_INFO(logger, "Image " << path << " loaded");

			return img;
		} else {
			std::stringstream err;
			err << "\"" << filename << "\" does not exists";

			LOG4CXX_FATAL(logger, err.str());

			throw ImageLoadingException(err.str());
		}
	} catch(boost::filesystem::filesystem_error &ex) {
		std::stringstream err;
		err << filename << " cannot be read (" << ex.what() << ")" << std::endl;
		throw ImageLoadingException(err.str());
	}
}

InputImageType::Pointer ImageLoader::loadImage(const std::string filename)
{
	typename ImageReader::Pointer reader = ImageReader::New();

	reader->SetFileName(filename);

	try {
		reader->Update();
	}
	catch( itk::ExceptionObject &ex )
	{
		std::stringstream err;
		err << "ITK is unable to load the image \"" << filename << "\" (" << ex.what() << ")";

		throw ImageLoadingException(err.str());
	}

	return reader->GetOutput();
}

InputImageType::Pointer ImageLoader::loadImageSerie(const std::string filename)
{
	typename ImageSeriesReader::Pointer reader = ImageSeriesReader::New();

	typename ImageSeriesReader::FileNamesContainer filenames;

	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));

	try
	{
		boost::filesystem::path path(filename);
		boost::regex pattern(".*\\.((?:png)|(?:bmp)|(?:jpe?g))", boost::regex::icase);

		typedef std::vector< boost::filesystem::path > path_list;
		path_list slices;
		std::copy(boost::filesystem::directory_iterator(path), boost::filesystem::directory_iterator(), std::back_inserter(slices));
		std::sort(slices.begin(), slices.end());

		for( path_list::const_iterator it(slices.begin()) ; it != slices.end() ; ++it)
		{
			boost::smatch match;
			if( !boost::regex_match( (*it).filename().string(), match, pattern ) ) continue;

			LOG4CXX_DEBUG(logger, "Loading \"" << boost::filesystem::absolute(*it).string() << "\"");

			filenames.push_back(boost::filesystem::absolute(*it).string());
		}
	}
	catch(boost::filesystem::filesystem_error &ex) {
		std::stringstream err;
		err << filename << " cannot be read (" << ex.what() << ")" << std::endl;

		throw ImageLoadingException(err.str());
	}

	std::sort(filenames.begin(), filenames.end());

	reader->SetFileNames(filenames);

	try {
		reader->Update();
	}
	catch( itk::ExceptionObject &ex )
	{
		std::stringstream err;
		err << "ITK is unable to load the image serie located in \"" << filename << "\" (" << ex.what() << ")";

		throw ImageLoadingException(err.str());
	}

	return reader->GetOutput();
}

