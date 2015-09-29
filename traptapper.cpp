///////////////////////////////////////////////////////////////////////////////
// TRAPTAPPER v.1
// by Drew Stevens (ahstevens@gmail.com)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <vector>
#include <sys/stat.h>


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int main( int argc, char* argv[] )
{
	if ( argc != 4 ){
		std::cerr << "Usage: " << argv[0] << " OUTPUT_FILE CONFIG_FILE IMAGE_LIST_FILE";
		return 0;
	}

	/**************************** TESSERACT INIT *****************************/
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if ( api->Init( NULL, "eng" ) ) {
		std::cerr << "ERROR: Could not initialize tesseract." << std::endl;
		exit( 1 );
	}
	
	std::string line;

	/**************************** CONFIG FILE ********************************/
	if ( !file_exists(std::string( argv[2] ) ) )
	{ 
		std::cerr << "ERROR: Could not find config file: " << std::string(argv[2]) << std::endl;
		exit( 1 );
	}

	std::ifstream configFile( argv[2] );
	std::vector< std::vector< std::string > > config;

	// camera model is the first line of config file
	std::string cameraType;
	std::getline( configFile, cameraType );
	boost::erase_all( cameraType, "\n" );

	// read in config parameters
	while ( std::getline( configFile, line ) )
	{
		boost::erase_all(line, "\n");
		config.push_back( split( line, ' ' ) );
	}

	/**************************** OUTPUT FILE ********************************/
	std::ofstream outFile(argv[1]);
	if (!outFile.is_open())
	{
		std::cerr << "ERROR: Could not open output file " << argv[1] << std::endl;
		exit(1);
	}

	// begin header with filepaths
	outFile << "filepath,cameraType";

	// fill remaining headers from config
	std::vector<std::vector<std::string>>::iterator it;
	for (it = config.begin(); it != config.end(); ++it)
	{
		outFile << "," << (*it)[0];

		if ((*it)[0].compare("temperature") == 0 || (*it)[0].compare("temp") == 0)
			outFile << ",temperature_corrected";
	}

	outFile << std::endl;

	/**************************** PROCESS IMAGES *****************************/
	std::ifstream imagelist(argv[3]);
	int imageCount = 0;
	while (std::getline(imagelist, line))
	{
		if (!file_exists(line))
		{
			std::cerr << "ERROR: Could not find image file: " << line << std::endl;
			continue;
		}

		Pix *image = pixRead(line.c_str());
		api->SetImage(image);

		outFile << line << "," << cameraType;

		std::string temp;
		for ( it = config.begin(); it != config.end(); ++it )
		{
			api->SetRectangle( atoi( (*it)[1].c_str() ), 
							   atoi( (*it)[2].c_str() ),
							   atoi( (*it)[3].c_str() ),
							   atoi( (*it)[4].c_str() ) );
			temp = api->GetUTF8Text();
			boost::erase_all( temp, "\n" );

			// if a set precision is specified as the 4th property of a parameter
			if ( (*it).size() == 6 )
			{
				temp.resize( atoi( (*it)[5].c_str() ) );
			}

			// fix misidentified zeroes in dates
			if ( (*it)[0].compare("date") == 0 || (*it)[0].compare("time") == 0 )
			{
				boost::replace_all( temp, "O", "0" );
				boost::replace_all( temp, "o", "0" );
				if ( (*it)[0].compare("time") == 0 )
				{
					temp.replace( 2, 1, std::string(":") );
					temp.replace( 5, 1, std::string(":") );
				}

				boost::replace_all(temp, ".", "/");
			}

			temp.erase(remove_if(temp.begin(), temp.end(), isspace), temp.end());

			outFile << "," << temp;

			// try to correct wonky temperature readouts
			if ( (*it)[0].compare("temperature") == 0 || (*it)[0].compare("temp") == 0 )
			{
				std::string temperature = temp;
				boost::erase_all(temperature, " ");
				int i = 0;
				while ( temperature[i] == '-' || isdigit( temperature[i] ) ) i++;
				temperature.resize( i );
				outFile << "," << temperature;
			}
		}

		outFile << std::endl;
		std::cout << "Processed " << line << std::endl;
		imageCount++;
		pixDestroy(&image);
	}

	// Destroy used object and release memory
	api->End();

	std::cout << "Completed processing " << imageCount << " images." << std::endl;
	std::cout << "Results saved to " << argv[1] << std::endl;

	return 0;
}