#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cctype>

using namespace std;

int main(int argc, char *argv[]){
	std::string input_type;

	if(argc < 4)
		return 1;

	ifstream in_file(argv[1]);
	if(in_file.is_open()){
		vector<string> types;
		while(in_file.good()){
			getline(in_file, input_type);
			types.push_back(input_type);
		}
		types.pop_back();
		vector<string> types_upper(types);
		for(unsigned int ii=0; ii < types.size(); ii++)
			transform(types_upper[ii].begin(), types_upper[ii].end(), types_upper[ii].begin(), ::toupper);

		//Open two files - header & impl
		ofstream out_header(argv[2]);
		ofstream out_impl(argv[3]);

		//First create the enumeration type
		out_header << "#ifndef " << argv[1] << "_H" << endl;
		out_header << "#define " << argv[1] << "_H" << endl << endl;
		out_header << "enum streamType {" << endl;
		for(unsigned int ii=0; ii < types_upper.size(); ii++){
			out_header << "\tSTREAM_";
			out_header << types_upper[ii];
		       	if(ii < types_upper.size() - 1) out_header << "," << endl;
		}
		out_header << "};" << endl << endl;

		//Then include prototypes for stringToStreamType and getConversionFunc
		out_header << "streamType stringToStreamType(std::string in_string);" << endl << endl;
		out_header << "int getStreamTypeLength(streamType in_type);" << endl << endl;
		out_header << "typedef void (*convFunc)(void*,void*,void*);" << endl << endl;
		out_header << "convFunc getConversionFunc(streamType from_type, streamType to_type);" << endl << endl;
		out_header << "#endif" << endl;
		out_header.close();

		//Now work on the implementation stuff, starting with appropriate headers
		out_impl << "#include <algorithm>" << endl;
		out_impl << "#include \"" << argv[2] << "\"" << endl << endl;

		//Now create all of the N^2 conversion functions
		for(unsigned int ii=0; ii < types_upper.size(); ii++){
			for(unsigned int jj=0; jj < types_upper.size(); jj++){
				out_impl << "void " << types_upper[ii] << "_to_" << types_upper[jj] << "(void *start, void *end, void *result){" << endl;
				out_impl << "\tstd::copy((" << types[ii] << "*)start, (" << types[ii] << "*)end, (" << types[jj] << "*)result);" << endl;
				out_impl << "}" << endl << endl;
			}
		}

		//Now implement the functions listed in the header
		out_impl << "streamType stringToStreamType(std::string in_string){" << endl;
		out_impl << "\tstd::transform(in_string.begin(), in_string.end(), in_string.begin(), ::toupper)" << endl;
		for(unsigned int ii=0; ii < types_upper.size(); ii++){
			out_impl << "\tif(in_string == \"" << types_upper[ii] << "\") return STREAM_" << types_upper[ii] << ";" << endl;
		}
		out_impl << "\treturn STREAM_" << types_upper[0] << ";" << endl;
		out_impl << "}" << endl << endl;

		//getStreamTypeLength
		out_impl << "int getStreamTypeLength(streamType in_type){" << endl;
		for(unsigned int ii=0; ii < types_upper.size(); ii++)
			out_impl << "\tif(in_type == STREAM_" << types_upper[ii] << ") return sizeof(" << types[ii] << ");" << endl;
		out_impl << "}" << endl << endl;

		//getConversionFunc
		out_impl << "convFunc getConversionFunc(streamType from_type, streamType to_type){" << endl;
		for(unsigned int ii=0; ii < types_upper.size(); ii++)
			for(unsigned int jj=0; jj < types_upper.size(); jj++)
				out_impl << "\tif(from_type == STREAM_" << types_upper[ii] << " && to_type == STREAM_" << types_upper[jj] << ") return &" << types_upper[ii] << "_to_" << types_upper[jj] << ";" << endl;
		out_impl << "}" << endl;
		out_impl.close();
	}

}
