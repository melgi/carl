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

#ifndef CARL_COMMAND_LINE_HH
#define CARL_COMMAND_LINE_HH

#include <vector>
#include <string>

#include "Optional.hh"

namespace n3 {
	
	struct CommandLine {
	
		bool error;
		bool help;
		std::vector<std::string> inputs;
		Optional<std::string> output;
		Optional<std::string> base;
		
		static CommandLine parse(int argc, char *argv[]);
	};

}

#endif /* CARL_COMMAND_LINE_HH */
