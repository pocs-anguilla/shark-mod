#define BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION
#include <shark/Core/Images/WriteImage.h>
#include <shark/Core/Images/ReadImage.h>

#define BOOST_TEST_MODULE Core_ImageReorder
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
using namespace shark;
using namespace std;

struct ImageFixture {
	ImageFixture()
	: shapeRGBA({32,26,4}), shapeRGB({32,26,3}), shapeGray({32,26,1})
	, dataRGBA(32*26*4,0), dataARGB(32*26*4,0), dataRGB(32*26*3,0), dataGray(32*26){
		//red and green channels are 0
		for(std::size_t i = 0; i != 32; ++i){
			for(std::size_t j = 0; j != 26; ++j){
				dataRGBA[(i*26+j)*4+2] = 1.0;//blue channel is maximal
				dataARGB[(i*26+j)*4+3] = 1.0;
				double alpha = 0.0;
				if( i >= 16)
					alpha += 2.0/3.0;
				if(j >= 13)
					alpha += 1.0/3.0;
				dataRGBA[(i*26+j)*4+3] = alpha;//alpha channel are 4 squares
				dataARGB[(i*26+j)*4+0] = alpha;
			}
		}
		
		//the jpeg image looks similar, jus without alpha channel
		for(std::size_t i = 0; i != 32; ++i){
			for(std::size_t j = 0; j != 26; ++j){
				double RG = 1.0;
				if( i < 16 && j >= 13)
					RG = 0.84;
				if( i >= 16 && j < 13)
					RG = 0.61;
				if( i >= 16 && j >= 13)
					RG = 0.0;
		
				dataRGB[(i*26+j)*3+0] = RG;
				dataRGB[(i*26+j)*3+1] = RG;
				dataRGB[(i*26+j)*3+2] = 1;
			}
		}
		
		//pgm grayscale image
		for(std::size_t i = 0; i != 32; ++i){
			for(std::size_t j = 0; j != 26; ++j){
				dataGray[i*26+j] = 1.0;
				if( i < 16 && j >= 13)
					dataGray[i*26+j] = 0.847;
				if( i >= 16 && j < 13)
					dataGray[i*26+j] = 0.639;
				if( i >= 16 && j >= 13)
					dataGray[i*26+j] = 0.071;
			}
		}
	}

	Shape shapeRGBA;
	Shape shapeRGB;
	Shape shapeGray;
	RealVector dataRGBA;
	RealVector dataARGB;
	RealVector dataRGB;
	RealVector dataGray;
};


BOOST_FIXTURE_TEST_SUITE (Core_ReadImage_Tests, ImageFixture )


///////////////////////////////////PNG/////////////////////////////////////

//we check only that round tripping write& read works
BOOST_AUTO_TEST_CASE( Core_Write_PNG_RGBA){
	std::vector<unsigned char> buffer = image::writePNG<double>(dataRGBA, shapeRGBA, PixelType::RGBA);
	std::pair<blas::vector<double>, Shape> result = image::readPNG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGBA[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGBA[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGBA[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGBA.size());
	
	for(std::size_t i = 0; i != dataRGBA.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataRGBA[i], 1.0/255);
	}
}

//roundtrip ARGB->RGBA
BOOST_AUTO_TEST_CASE( Core_Write_PNG_ARGB){
	std::vector<unsigned char> buffer = image::writePNG<double>(dataARGB, shapeRGBA, PixelType::ARGB);
	std::pair<blas::vector<double>, Shape> result = image::readPNG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGBA[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGBA[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGBA[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGBA.size());
	
	for(std::size_t i = 0; i != dataRGBA.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataRGBA[i], 1.0/255);
	}
}

BOOST_AUTO_TEST_CASE( Core_Write_PNG_RGB){
	std::vector<unsigned char> buffer = image::writePNG<double>(dataRGB, shapeRGB, PixelType::RGB);
	std::pair<blas::vector<double>, Shape> result = image::readPNG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGB[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGB[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGB[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGB.size());
	
	for(std::size_t i = 0; i != dataRGB.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataRGB[i], 1.0/255);//less or equal off-by-one
	}
}

BOOST_AUTO_TEST_CASE( Core_Write_PNG_Luma){
	std::vector<unsigned char> buffer = image::writePNG<double>(dataGray, shapeGray, PixelType::Luma);
	std::pair<blas::vector<double>, Shape> result = image::readPNG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeGray[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeGray[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeGray[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataGray.size());
	
	for(std::size_t i = 0; i != dataGray.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataGray[i], 1.0/255);
	}
}


///////////////////////////////////JPEG/////////////////////////////////////

BOOST_AUTO_TEST_CASE( Core_Write_JPEG_RGB){
	std::vector<unsigned char> buffer = image::writeJPEG<double>(dataRGB, shapeRGB, PixelType::RGB);
	std::pair<blas::vector<double>, Shape> result = image::readJPEG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGB[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGB[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGB[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGB.size());
	
	double distance = norm_2(result.first - dataRGB) / shapeRGB.numElements();
	BOOST_CHECK_SMALL(distance, 5.0/255);
}

BOOST_AUTO_TEST_CASE( Core_Write_JPEG_Luma){
	std::vector<unsigned char> buffer = image::writeJPEG<double>(dataGray, shapeGray, PixelType::Luma);
	std::pair<blas::vector<double>, Shape> result = image::readJPEG<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeGray[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeGray[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeGray[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataGray.size());
	
	for(std::size_t i = 0; i != dataGray.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataGray[i], 10.0/255);
	}
}

///////////////////////////////////PGM/////////////////////////////////////

BOOST_AUTO_TEST_CASE( Core_Write_PGM_Luma){
	std::vector<unsigned char> buffer = image::writePGM<double>(dataGray, shapeGray, PixelType::Luma);
	std::pair<blas::vector<double>, Shape> result = image::readPGM<double>(buffer);

	BOOST_REQUIRE_EQUAL(result.second[0], shapeGray[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeGray[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeGray[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataGray.size());
	
	for(std::size_t i = 0; i != dataGray.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataGray[i], 1.0/255);
	}
}



///////////////////////////////////WRITE IMAGE/////////////////////////////////////

BOOST_AUTO_TEST_CASE( Core_Write_Image_PNG){
	image::writeImageToFile<double>("Test/test_output/writer_test.png", dataRGB, shapeRGB, PixelType::RGB);
	std::pair<blas::vector<double>, Shape> result = image::readImageFromFile<double>("Test/test_output/writer_test.png");

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGB[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGB[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGB[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGB.size());
	
	for(std::size_t i = 0; i != dataRGB.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataRGB[i], 1.0/255);//less or equal off-by-one
	}
}

BOOST_AUTO_TEST_CASE( Core_Write_Image_PGM){
	image::writeImageToFile<double>("Test/test_output/writer_test.pgm", dataGray, shapeGray, PixelType::Luma);
	std::pair<blas::vector<double>, Shape> result = image::readImageFromFile<double>("Test/test_output/writer_test.pgm");

	BOOST_REQUIRE_EQUAL(result.second[0], shapeGray[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeGray[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeGray[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataGray.size());
	
	for(std::size_t i = 0; i != dataGray.size(); ++i){
		BOOST_CHECK_SMALL(result.first[i] - dataGray[i], 1.0/255);
	}
}

BOOST_AUTO_TEST_CASE( Core_Write_Image_JPEG){
	image::writeImageToFile<double>("Test/test_output/writer_test.jpeg", dataRGB, shapeRGB, PixelType::RGB);
	std::pair<blas::vector<double>, Shape> result = image::readImageFromFile<double>("Test/test_output/writer_test.jpeg");

	BOOST_REQUIRE_EQUAL(result.second[0], shapeRGB[0]);
	BOOST_REQUIRE_EQUAL(result.second[1], shapeRGB[1]);
	BOOST_REQUIRE_EQUAL(result.second[2], shapeRGB[2]);
	BOOST_REQUIRE_EQUAL(result.first.size(), dataRGB.size());
	
	double distance = norm_2(result.first - dataRGB) / shapeRGB.numElements();
	BOOST_CHECK_SMALL(distance, 5.0/255);
}


BOOST_AUTO_TEST_SUITE_END()
