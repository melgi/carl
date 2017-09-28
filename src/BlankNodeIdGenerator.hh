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

#ifndef CARL_BLANKNODEIDGENERATOR_HH
#define CARL_BLANKNODEIDGENERATOR_HH

#include <cstddef>
#include <string>

namespace n3 {
	
	class BlankNodeIdGenerator {
		
		static const std::size_t m_length = 16;
		
		std::string m_prefix;
		unsigned m_c;
		
	public:
		
		BlankNodeIdGenerator() : m_prefix(), m_c(0)
		{
			initialize();
		}
		
		std::string generate()
		{
			return m_prefix + "-" + std::to_string(m_c++);
		}
		
		std::string generate(const std::string &id)
		{
			return m_prefix + "-" + id;
		}
		
		void initialize();
	};

}

#endif /* CARL_BLANKNODEIDGENERATOR_HH */
