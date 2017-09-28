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

#include "Model.hh"

namespace n3 {
	
	const std::string IntegerLiteral::TYPE = "http://www.w3.org/2001/XMLSchema#integer";
	const std::string StringLiteral::TYPE  = "http://www.w3.org/2001/XMLSchema#string";
	const std::string BooleanLiteral::TYPE = "http://www.w3.org/2001/XMLSchema#boolean";
	const std::string DoubleLiteral::TYPE  = "http://www.w3.org/2001/XMLSchema#double";
	const std::string DecimalLiteral::TYPE = "http://www.w3.org/2001/XMLSchema#decimal";

	const std::string RDF::NS    = std::string("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	const URIResource RDF::type  = URIResource("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
	const URIResource RDF::first = URIResource("http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
	const URIResource RDF::rest  = URIResource("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest");
	const URIResource RDF::nil   = URIResource("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");

	const std::string XSD::NS    = std::string("http://www.w3.org/2001/XMLSchema#");

	const std::string LOG::NS             = std::string("http://www.w3.org/2000/10/swap/log#");
	const URIResource LOG::implies        = URIResource("http://www.w3.org/2000/10/swap/log#implies");
	const URIResource LOG::reverseImplies = URIResource("http://www.w3.org/2000/10/swap/log#reverseImplies"); // TODO does not exist in log:

	const std::string OWL::NS             = std::string("http://www.w3.org/2002/07/owl#");
	const URIResource OWL::sameAs         = URIResource("http://www.w3.org/2002/07/owl#sameAs");

	const BooleanLiteral BooleanLiteral::VALUE_TRUE  = BooleanLiteral("true");
	const BooleanLiteral BooleanLiteral::VALUE_FALSE = BooleanLiteral("false");
	const BooleanLiteral BooleanLiteral::VALUE_1     = BooleanLiteral("1");
	const BooleanLiteral BooleanLiteral::VALUE_0     = BooleanLiteral("0");

	std::ostream &operator<<(std::ostream &out, const N3Node &n)
	{
		return n.print(out);
	}

}
