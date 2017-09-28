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

#ifndef CARL_TOKEN_HH
#define CARL_TOKEN_HH

namespace n3 {

	struct Token {
		
		typedef int Type;
		
		static const Type Eof                          = 0;
		
		static const Type IriRef                       = 1000;
		static const Type PNameNS                      = 1001;
		static const Type PNameLN                      = 1002;
		static const Type BlankNodeLabel               = 1003;
		static const Type LangTag                      = 1004;
		static const Type Integer                      = 1005;
		static const Type Decimal                      = 1006;
		static const Type Double                       = 1007;
		static const Type StringLiteralQuote           = 1008;
		static const Type StringLiteralSingleQuote     = 1009;
		static const Type StringLiteralLongSingleQuote = 1010;
		static const Type StringLiteralLongQuote       = 1011;
		static const Type False                        = 1012;
		static const Type True                         = 1013;
		static const Type Prefix                       = 1014;
		static const Type Base                         = 1015;
		static const Type SparqlPrefix                 = 1016;
		static const Type SparqlBase                   = 1017;
		static const Type CaretCaret                   = 1018;
		static const Type ReverseImplies               = 1019;
		static const Type Implies                      = 1020;
		static const Type Var                          = 1021;
		
	};
	
}

#endif /* N3_TOKEN_HH */
