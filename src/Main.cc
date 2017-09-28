//
// Copyright 2017 Giovanni Mels
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <string>
#include <ostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <iomanip>

#include "CommandLine.hh"
#include "Parser.hh"
#include "Uri.hh"
#include "CN3Writer.hh"
#include "Util.hh"
#include "Version.hh"


int main(int argc, char *argv[])
{
	n3::useBinaryStreams();

	std::cin.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
	char outputBuffer[1024*1024];
	std::cout.rdbuf()->pubsetbuf(outputBuffer, sizeof(outputBuffer));
	
	std::cin.tie(nullptr);
	
	n3::CommandLine opt = n3::CommandLine::parse(argc, argv);
	
	if (opt.error || opt.help) {
		std::cerr << "carl version " << CARL_VERSION_STR << std::endl;
		std::cerr << "\nUsage: carl [-b=base-uri] [-o=output-file] [input-files]" << std::endl;
		
		return opt.error ? -1 : 0;
	}
	
	std::unique_ptr<std::ostream> out;
	if (opt.output && *opt.output != "-") {
		out = std::unique_ptr<std::ostream>(new std::ofstream(*opt.output));
		
		if (!*out) {
			std::cerr << "error opening \"" << *opt.output << "\"" << std::endl;
			
			return -1;
		}
	}
	
	n3::TripleSink *s = new n3::CN3Writer(out ? *out : std::cout);
	
	std::unique_ptr<n3::TripleSink> sink(s);
	
	typedef std::chrono::high_resolution_clock Clock;
	
	Clock::time_point start = Clock::now();
	
	sink->start();
	
	for (std::string input : opt.inputs) {
		
		std::string uri;
		
		std::unique_ptr<std::ifstream> in;
		if (input != "-") {
			if (!n3::exists(input)) {
				std::cerr << "\"" << input << "\" not found" << std::endl;
				sink->end();
				
				return -1;
			}
			
			uri = n3::toUri(input);
			in = std::unique_ptr<std::ifstream>(new std::ifstream(input, std::ios_base::in | std::ios_base::binary));
			if (!*in) {
				std::cerr << "error opening \"" << input << "\"" << std::endl;
				sink->end();
				
				return -1;
			}
		} else {
			uri = "file:///dev/stdin";
		}
		
		std::cerr << "translating " << uri << std::endl;
		
		n3::Uri baseUri(opt.base ? *opt.base : uri);
		
		n3::Parser parser(in ? in.get() : &std::cin, baseUri, sink.get());
		try {
			parser.parse();
		} catch (n3::ParseException &e) {
			if (e.line() == -1)
				std::cerr << "parse error: " << e.what() << std::endl;
			else
				std::cerr << "parse error at line " << e.line() << ": " << e.what() << std::endl;
			
			return -1;
		}
	}
	
	sink->end();
	
	Clock::duration d = Clock::now() - start;
	
	double ms = static_cast<double>(1000 * d.count() * Clock::duration::period::num) / static_cast<double>(Clock::duration::period::den);
	
	unsigned count = sink->count();
	
	if (count && ms > 0.0) {
		std::streamsize p = std::cerr.precision();
		std::cerr << "Done: translated " << count << " triples in " << std::fixed << std::setprecision(1) << ms << std::setprecision(0) << " ms (" << (1000.0 * count / ms) << " triples/s)" << std::setprecision(p) <<  std::endl;
	} else
		std::cerr << "Done: translated " << count << " triples" << std::endl;
	
	return 0;

}
