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

#include <utility>

#include "Parser.hh"
#include "Utf8.hh"
#include "Utf16.hh"
#include "Model.hh"

namespace n3 {

	const std::string Parser::LOCAL_NAME_ESCAPE_CHARS("_~.-!$&\'()*+,;=/?#@%");
	
	// We do not check if uris are valid, this is used when translating \uxxxx escapes to chars
	const std::string Parser::INVALID_ESCAPES("<>\"{}|^`\\");

	inline Uri Parser::resolve(const std::string &uri)
	{
		Uri u(uri);
		if (u.absolute())
			return u;
		
		return m_base.resolve(u);
	}

	inline Uri Parser::resolve(std::string &&uri)
	{
		Uri u(std::move(uri));
		if (u.absolute())
			return u;
		
		return m_base.resolve(u);
	}

	std::string Parser::toUri(const std::string &pname) const
	{
		std::size_t p = pname.find(':');
		if (p == std::string::npos)
			throw ParseException();
		
		std::string prefix = pname.substr(0, p);
		
		auto i = m_prefixMap.find(prefix);
		if (i == m_prefixMap.end())
			throw ParseException("unknown prefix: " + prefix, line());
		
		return i->second + unescape(p + 1, pname);
		// checking for valid uris is redundant here, i->second is a valid uri, concatenating a fragment or path cannot give a invalid uri.
		//return static_cast<std::string>(Uri(i->second + unescape(p + 1, pname)));
	}
	
	
	/*
	  1 	n3doc ? statementlist 	PREFIX, SPARQLPREFIX, BASE, SPARQLBASE, BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN, LBRACKET
2 	statementlist ? e 	$
3 	statementlist ? statement statementlist 	PREFIX, SPARQLPREFIX, BASE, SPARQLBASE, BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN, LBRACKET
4 	statement ? directive 	PREFIX, SPARQLPREFIX, BASE, SPARQLBASE
5 	statement ? triples POINT 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN, LBRACKET
6 	directive ? prefixid 	PREFIX
7 	directive ? base 	BASE
8 	directive ? sparqlprefix 	SPARQLPREFIX
9 	directive ? sparqlbase 	SPARQLBASE
10 	prefixid ? PREFIX PNAME_NS IRIREF POINT 	PREFIX
11 	base ? BASE IRIREF POINT 	BASE
12 	sparqlbase ? SPARQLBASE IRIREF 	SPARQLBASE
13 	sparqlprefix ? SPARQLPREFIX PNAME_NS IRIREF 	SPARQLPREFIX
14 	triples ? subject propertylist 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN
15 	triples ? blanknodepropertylist propertylistopt 	LBRACKET
16 	propertylistopt ? e 	RBRACKET, POINT
17 	propertylistopt ? propertylist 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
18 	propertylist ? property propertyrest 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
19 	propertyrest ? e 	RBRACKET, POINT
20 	propertyrest ? SEMICOLON propertylist2 	SEMICOLON
21 	propertylist2 ? propertyopt 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
22 	propertylist2 ? propertyrest 	SEMICOLON
23 	propertyopt ? e 	RBRACKET, POINT
24 	propertyopt ? property 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
25 	property ? verb objectlist 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
26 	objectlist ? object objectrest 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACKET, LBRACE, LPAREN, STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE
27 	objectrest ? e 	SEMICOLON, RBRACKET, POINT
28 	objectrest ? COMMA objectlist 	COMMA
29 	verb ? iri 	IRIREF, PNAME_LN, PNAME_NS
30 	verb ? A 	A
31 	verb ? SAMEAS 	SAMEAS
32 	verb ? IMPLIES 	IMPLIES
33 	verb ? CONSEQUENCE 	CONSEQUENCE
34 	subject ? BLANK_NODE_LABEL 	BLANK_NODE_LABEL
35 	subject ? iri 	IRIREF, PNAME_LN, PNAME_NS
36 	subject ? collection 	LPAREN
37 	subject ? graph 	LBRACE
38 	object ? BLANK_NODE_LABEL 	BLANK_NODE_LABEL
39 	object ? iri 	IRIREF, PNAME_LN, PNAME_NS
40 	object ? collection 	LPAREN
41 	object ? blanknodepropertylist 	LBRACKET
42 	object ? literal 	STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE
43 	object ? graph 	LBRACE
44 	literal ? rdfliteral 	STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE
45 	literal ? numericliteral 	INTEGER, DECIMAL, DOUBLE
46 	literal ? booleanliteral 	TRUE, FALSE
47 	blanknodepropertylist ? LBRACKET propertylistopt RBRACKET 	LBRACKET
48 	collection ? LPAREN objectopt RPAREN 	LPAREN
49 	objectopt ? e 	RPAREN
50 	objectopt ? object objectopt 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACKET, LBRACE, LPAREN, STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE
51 	numericliteral ? INTEGER 	INTEGER
52 	numericliteral ? DECIMAL 	DECIMAL
53 	numericliteral ? DOUBLE 	DOUBLE
54 	rdfliteral ? string dtlang 	STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE
55 	dtlang ? e 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACKET, LBRACE, LPAREN, STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE, COMMA, RPAREN, SEMICOLON, RBRACKET, POINT, RBRACE
56 	dtlang ? LANGTAG 	LANGTAG
57 	dtlang ? CARETCARET iri 	CARETCARET
58 	booleanliteral ? TRUE 	TRUE
59 	booleanliteral ? FALSE 	FALSE
60 	string ? STRING_LITERAL_QUOTE 	STRING_LITERAL_QUOTE
61 	string ? STRING_LITERAL_SINGLE_QUOTE 	STRING_LITERAL_SINGLE_QUOTE
62 	string ? STRING_LITERAL_LONG_SINGLE_QUOTE 	STRING_LITERAL_LONG_SINGLE_QUOTE
63 	string ? STRING_LITERAL_LONG_QUOTE 	STRING_LITERAL_LONG_QUOTE
64 	iri ? IRIREF 	IRIREF
65 	iri ? prefixedname 	PNAME_LN, PNAME_NS
66 	prefixedname ? PNAME_LN 	PNAME_LN
67 	prefixedname ? PNAME_NS 	PNAME_NS
68 	graph ? LBRACE triplepatternlist RBRACE 	LBRACE
69 	triplepatternlist ? e 	RBRACE
70 	triplepatternlist ? triplepattern triplepatternlist2 	VAR, BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN, LBRACKET
71 	triplepatternlist2 ? e 	RBRACE
72 	triplepatternlist2 ? POINT triplepatternlist 	POINT
73 	triplepattern ? subjectorvar propertylistvar 	VAR, BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN
74 	triplepattern ? blanknodepropertylistvar propertylistoptvar 	LBRACKET
75 	subjectorvar ? subject 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACE, LPAREN
76 	subjectorvar ? VAR 	VAR
77 	propertylistvar ? propertyorvar propertyrestvar 	VAR, A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
78 	propertyorvar ? verborvar objectlistvar 	VAR, A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
79 	verborvar ? verb 	A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
80 	verborvar ? VAR 	VAR
81 	objectlistvar ? objectorvar objectrestvar 	VAR, BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACKET, LBRACE, LPAREN, STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE
82 	objectorvar ? object 	BLANK_NODE_LABEL, IRIREF, PNAME_LN, PNAME_NS, LBRACKET, LBRACE, LPAREN, STRING_LITERAL_QUOTE, STRING_LITERAL_SINGLE_QUOTE, STRING_LITERAL_LONG_SINGLE_QUOTE, STRING_LITERAL_LONG_QUOTE, TRUE, FALSE, INTEGER, DECIMAL, DOUBLE
83 	objectorvar ? VAR 	VAR
84 	objectrestvar ? e 	SEMICOLON, RBRACKET, POINT, RBRACE
85 	objectrestvar ? COMMA objectlistvar 	COMMA
86 	propertyrestvar ? e 	RBRACKET, POINT, RBRACE
87 	propertyrestvar ? SEMICOLON propertylist2var 	SEMICOLON
88 	propertylist2var ? propertyoptvar 	VAR, A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
89 	propertylist2var ? propertyrestvar 	SEMICOLON
90 	propertyoptvar ? e 	RBRACKET, POINT, RBRACE
91 	propertyoptvar ? propertyorvar 	VAR, A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS
92 	blanknodepropertylistvar ? LBRACKET propertylistoptvar RBRACKET 	LBRACKET
93 	propertylistoptvar ? e 	RBRACKET, POINT, RBRACE
94 	propertylistoptvar ? propertylistvar 	VAR, A, SAMEAS, IMPLIES, CONSEQUENCE, IRIREF, PNAME_LN, PNAME_NS

*/
	void Parser::n3doc()
	{
		try {
			while (m_lookAhead != Token::Eof) {
				if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' ||
				    m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote ||
				    m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
					triples();
					match('.');
				} else if (m_lookAhead == Token::Prefix) {
					prefixID();
				} else if (m_lookAhead == Token::Base) {
					base();
				} else if (m_lookAhead == Token::SparqlPrefix) {
					sparqlPrefix();
				} else if (m_lookAhead == Token::SparqlBase) {
					sparqlBase();
				} else
					throw ParseException("expected base, prefix or triple", line());
			}
		} catch (UriSyntaxException &e) {
			throw ParseException(e.what(), line());
		}
	}
	
	void Parser::base()
	{
		match(Token::Base);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		match('.');
		
		m_base = resolve(std::move(u));
	}
	
	void Parser::prefixID()
	{
		match(Token::Prefix);
		match(Token::PNameNS);
		std::string prefix = m_lexeme.substr(0, m_lexeme.length() - 1);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		match('.');
		
		std::string ns = static_cast<std::string>(resolve(std::move(u)));
		m_sink->prefix(prefix, ns);
		m_prefixMap[prefix] = ns;
	}
	
	void Parser::sparqlBase()
	{
		match(Token::SparqlBase);
		match(Token::IriRef);
		
		std::string u = extractUri(m_lexeme);
		
		m_base = resolve(std::move(u));
	}
	
	void Parser::sparqlPrefix()
	{
		match(Token::SparqlPrefix);
		match(Token::PNameNS);
		std::string prefix = m_lexeme.substr(0, m_lexeme.length() - 1);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		
		std::string ns = static_cast<std::string>(resolve(std::move(u)));
		m_sink->prefix(prefix, ns);
		m_prefixMap[prefix] = ns;
	}
	
	std::unique_ptr<N3Node> Parser::path(N3Node *subject)
	{
		std::unique_ptr<N3Node> s(subject);
		
		while (m_lookAhead == '^' || m_lookAhead == '!') {
			bool forward = m_lookAhead == '!';
			
			match();
			
			if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
				const URIResource property(iri());
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
				
				if (forward) {
					m_sink->triple(*s, property, *b);
				} else {
					m_sink->triple(*b, property, *s);
				}
				
				s = std::move(b);
			} else if (m_lookAhead == Token::BlankNodeLabel) {
				match();
				const BlankNode property(m_blanks.generate(m_lexeme.substr(2)));
				
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
				
				if (forward) {
					m_sink->triple(*s, property, *b);
				} else {
					m_sink->triple(*b, property, *s);
				}
				
				s = std::move(b);
			} else if (m_lookAhead == '[') {
				std::unique_ptr<BlankNode> property = blanknodepropertylist();
				
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
				
				if (forward) {
					m_sink->triple(*s, *property, *b);
				} else {
					m_sink->triple(*b, *property, *s);
				}
				
				s = std::move(b);
			} else
				throw ParseException("expected IRI ref, prefixed name or blanknode as path", line());
		}
		
		return s;
	}
	
	std::unique_ptr<N3Node> Parser::path(N3Node *subject, GraphTemplate *graph)
	{
		std::unique_ptr<N3Node> s(subject);
		
		while (m_lookAhead == '^' || m_lookAhead == '!') {
			bool forward = m_lookAhead == '!';
			
			match();
			
			if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
				const URIResource property(iri());
				
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
				
				if (forward) {
					graph->triple(*s, property, *b);
				} else {
					graph->triple(*b, property, *s);
				}
				
				s = std::move(b);
			} else if (m_lookAhead == Token::BlankNodeLabel) {
				match();
				const BlankNode property(m_blanks.generate(m_lexeme.substr(2)));
				
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
				
				if (forward) {
					graph->triple(*s, property, *b);
				} else {
					graph->triple(*b, property, *s);
				}
				
				s = std::move(b);
			} else if (m_lookAhead == '[') {
				const std::unique_ptr<BlankNode> property = blanknodepropertylist();
				
				std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
			
				if (forward) {
					graph->triple(*s, *property, *b);
				} else {
					graph->triple(*b, *property, *s);
				}
				
				s = std::move(b);
			} else
				throw ParseException("expected IRI ref or prefixed name as path", line());
		}
		
		return s;
	}
	
	void Parser::triples()
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '(' ||
		    m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote ||
		    m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
			std::unique_ptr<N3Node> s = subject();
			
			s = std::move(path(s.release()));
			
			propertylist(s.get());
		} else if (m_lookAhead == '[') {
			std::unique_ptr<N3Node> s = blanknodepropertylist();
			
			s = std::move(path(s.release()));
			
			propertylistopt(s.get());
		} else
			throw ParseException("expected blank node, uri or list as subject", line());
	}
	
	std::unique_ptr<N3Node> Parser::subject(GraphTemplate *graph)
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			return std::unique_ptr<Resource>(new URIResource(iri()));
		} else if (m_lookAhead == Token::BlankNodeLabel) {
			match();
			return std::unique_ptr<Resource>(new BlankNode(m_blanks.generate(m_lexeme.substr(2))));
		} else if (m_lookAhead == '{') {
			return graphTemplate();
		} else if (m_lookAhead == '(') {
			return collection(graph);
		} else if (m_lookAhead == Token::StringLiteralQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::StringLiteralLongQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::Integer) {
			match();
			return std::unique_ptr<Literal>(new IntegerLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Decimal) {
			match();
			return std::unique_ptr<Literal>(new DecimalLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Double) {
			match();
			return std::unique_ptr<Literal>(new DoubleLiteral(m_lexeme));
		} else if (m_lookAhead == Token::True) {
			match();
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == Token::False) {
			match();
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == Token::StringLiteralSingleQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::StringLiteralLongSingleQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else
			throw ParseException("expected blank node, uri or list as subject", line());
	}
	
	void Parser::propertylist(const N3Node *subject)
	{
		if (m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '[' || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies) {
			property(subject);
			while (m_lookAhead == ';') {
				match();
				if (m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '[' || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies) {
					property(subject);
				}
			}
		} else
			throw ParseException("expected 'a' or uri as property", line());
	}
	
	void Parser::property(const N3Node *subject)
	{
		if (m_lookAhead == 'a') {
			match();
			objectlist(subject, &RDF::type);
		} else if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			URIResource property(iri());
			objectlist(subject, &property);
		} else if (m_lookAhead == Token::BlankNodeLabel) {
			match();
			BlankNode property(m_blanks.generate(m_lexeme.substr(2)));
			objectlist(subject, &property);
		} else if (m_lookAhead == '[') {
			std::unique_ptr<BlankNode> property = blanknodepropertylist();
			objectlist(subject, property.get());
		} else if (m_lookAhead == Token::Implies) {
			match();
			objectlist(subject, &LOG::implies);
		} else if (m_lookAhead == Token::ReverseImplies) {
			match();
			objectlist(subject, &LOG::reverseImplies);
		} else if (m_lookAhead == '=') {
			match();
			objectlist(subject, &OWL::sameAs);
		} else
			throw ParseException("expected 'a' or uri as property", line());
	}
	
	std::string Parser::iri()
	{
		if (m_lookAhead == Token::IriRef) {
			match();
			std::string uri = extractUri(m_lexeme);
			if (Uri::absolute(uri))
				return std::move(uri);
			return static_cast<std::string>(resolve(std::move(uri)));
		} else if (m_lookAhead == Token::PNameLN) {
			match();
			return toUri(m_lexeme);
		} else if (m_lookAhead == Token::PNameNS) {
			match();
			return toUri(m_lexeme);
		} else
			throw ParseException("expected IRI ref or prefixed name", line());
	}
	
	void Parser::objectlist(const N3Node *subject, const Resource *property)
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' ||
		    m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote ||
		    m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
			std::unique_ptr<N3Node> obj = object();
			
			obj = std::move(path(obj.release()));
			
			m_sink->triple(*subject, *property, *obj);
			while (m_lookAhead == ',') {
				match();
				if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' ||
				    m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote ||
				    m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
					std::unique_ptr<N3Node> obj = object();
					
					obj = std::move(path(obj.release()));
					
					m_sink->triple(*subject, *property, *obj);
				} else
					throw ParseException("expected object after ','", line());
			}
		} else
			throw ParseException("expected object", line());
	}
	
	std::unique_ptr<N3Node> Parser::object(GraphTemplate *graph)
	{
		if (m_lookAhead == Token::BlankNodeLabel) {
			match();
			return std::unique_ptr<N3Node>(new BlankNode(m_blanks.generate(m_lexeme.substr(2))));
		} else if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			return std::unique_ptr<N3Node>(new URIResource(iri()));
		} else if (m_lookAhead == Token::StringLiteralQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::StringLiteralLongQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::Integer) {
			match();
			return std::unique_ptr<Literal>(new IntegerLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Decimal) {
			match();
			return std::unique_ptr<Literal>(new DecimalLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Double) {
			match();
			return std::unique_ptr<Literal>(new DoubleLiteral(m_lexeme));
		} else if (m_lookAhead == Token::True) {
			match();
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == Token::False) {
			match();
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == '{') {
			return graphTemplate();
		} else if (m_lookAhead == '[') {
			return graph ? blanknodepropertylistvar(graph) : blanknodepropertylist();
		} else if (m_lookAhead == '(') {
			return collection(graph);
		} else if (m_lookAhead == Token::StringLiteralSingleQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else if (m_lookAhead == Token::StringLiteralLongSingleQuote) {
			match();
			return dtlang(extractString(m_lexeme));
		} else {
			throw ParseException("expected blank node, iri, literal or list", line());
		}
	}
	
	std::unique_ptr<Literal> Parser::dtlang(std::string &&lexicalValue)
	{
		if (m_lookAhead == Token::LangTag) {
			match();
			return std::unique_ptr<Literal>(new StringLiteral(std::move(lexicalValue), m_lexeme.substr(1)));
		} else if (m_lookAhead == Token::CaretCaret) {
			match();
			std::string type = iri();
			if (type == IntegerLiteral::TYPE)
				return std::unique_ptr<Literal>(new IntegerLiteral(lexicalValue)); //TODO valid check
			if (type == DecimalLiteral::TYPE)
				return std::unique_ptr<Literal>(new DecimalLiteral(lexicalValue));
			if (type == BooleanLiteral::TYPE)
				return std::unique_ptr<Literal>(new BooleanLiteral(lexicalValue));
			if (type == DoubleLiteral::TYPE)
				return std::unique_ptr<Literal>(new DoubleLiteral(lexicalValue));
			if (type == StringLiteral::TYPE)
				return std::unique_ptr<Literal>(new StringLiteral(std::move(lexicalValue)));
			
			return std::unique_ptr<Literal>(new OtherLiteral(std::move(lexicalValue), std::move(type)));
		}
		
		return std::unique_ptr<Literal>(new StringLiteral(std::move(lexicalValue)));
	}
	
	std::unique_ptr<RDFList> Parser::collection(GraphTemplate *graph)
	{
		std::unique_ptr<RDFList> list(new RDFList());
		match('(');
		while (m_lookAhead != ')') {
			if (graph) {
				std::unique_ptr<N3Node> obj = objectorvar(graph);
				
				obj = std::move(path(obj.release(), graph));
				
				list->add(obj.release());
			} else {
				std::unique_ptr<N3Node> obj = object();
				
				obj = std::move(path(obj.release()));
				
				list->add(obj.release());
			}
		}
		match(')');
		
		return list;
	}
	
	std::unique_ptr<BlankNode> Parser::blanknodepropertylist()
	{
		std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
		match('['); propertylistopt(b.get()); match(']');
		return b;
	}
	
	std::unique_ptr<BlankNode> Parser::blanknodepropertylistvar(GraphTemplate *graph)
	{
		std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
		match('['); propertylistoptvar(graph, b.get()); match(']');
		return b;
	}
	
	void Parser::propertylistopt(const N3Node *subject)
	{
		if (m_lookAhead == 'a' || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameLN || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies)
			propertylist(subject);
	}
	
	void Parser::propertylistoptvar(GraphTemplate *graph, const N3Node *subject)
	{
		if (m_lookAhead == Token::Var || m_lookAhead == 'a' || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameLN || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies)
			propertylistvar(graph, subject);
	}
	
	std::unique_ptr<GraphTemplate> Parser::graphTemplate()
	{
		std::unique_ptr<GraphTemplate> graph(new GraphTemplate(std::to_string(++m_graphs)));
		
		match('{');
		
		while (m_lookAhead != '}') {
			if (m_lookAhead == Token::Var || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '(' || 
			    m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote ||
			    m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
				std::unique_ptr<N3Node> s = subjectorvar(graph.get());
				
				s = std::move(path(s.release(), graph.get()));
				
				propertylistvar(graph.get(), s.get());
				
				if (m_lookAhead == '.')
					match();
			} else if (m_lookAhead == '[') {
				std::unique_ptr<N3Node> s = blanknodepropertylistvar(graph.get());
				
				s = std::move(path(s.release(), graph.get()));
				
				propertylistoptvar(graph.get(), s.get());
				
				if (m_lookAhead == '.')
					match();
			} else
				throw ParseException("expected triple or '}'", line());
		}
		
		match('}');
		
		return graph;
	}
	
	void Parser::propertylistvar(GraphTemplate *graph, const N3Node *subject)
	{
		if (m_lookAhead == Token::Var || m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '[' || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies) {
			propertyorvar(graph, subject);
			while (m_lookAhead == ';') {
				match();
				if (m_lookAhead == Token::Var || m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == '[' || m_lookAhead == '=' || m_lookAhead == Token::Implies || m_lookAhead == Token::ReverseImplies) {
					propertyorvar(graph, subject);
				}
			}
		} else
			throw ParseException("expected var or uri as property", line());
	}
	
	void Parser::propertyorvar(GraphTemplate *graph, const N3Node *subject)
	{
		if (m_lookAhead == Token::Var) {
			match();
			Var var(m_lexeme.substr(1));
			objectlistvar(graph, subject, &var);
		} else if (m_lookAhead == 'a') {
			match();
			objectlistvar(graph, subject, &RDF::type);
		} else if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			URIResource property(iri());
			objectlistvar(graph, subject, &property);
		} else if (m_lookAhead == Token::BlankNodeLabel) {
			match();
			BlankNode property(m_blanks.generate(m_lexeme.substr(2)));
			objectlist(subject, &property);
		} else if (m_lookAhead == '[') {
			std::unique_ptr<BlankNode> property = blanknodepropertylist();
			objectlist(subject, property.get());
		} else if (m_lookAhead == Token::Implies) {
			match();
			objectlistvar(graph, subject, &LOG::implies);
		} else if (m_lookAhead == Token::ReverseImplies) {
			match();
			objectlistvar(graph, subject, &LOG::reverseImplies);
		} else if (m_lookAhead == '=') {
			match();
			objectlistvar(graph, subject, &OWL::sameAs);
		} else
			throw ParseException("expected var or uri as property", line());
	}
	
	std::unique_ptr<N3Node> Parser::subjectorvar(GraphTemplate *graph)
	{
		if (m_lookAhead == Token::Var) {
			match();
			
			return std::unique_ptr<Var>(new Var(m_lexeme.substr(1)));
		}
		
		return subject(graph);
	}
	
	void Parser::addTriple(GraphTemplate *graph, const N3Node *subject, const Resource *property)
	{
		if (m_lookAhead == '[') { // this hack rearanges the order of some triples for performance reasons
			graph->triple(*subject, *property, BlankNode(""));
			
			std::size_t p = graph->size() - 1;
			
			std::unique_ptr<N3Node> obj = objectorvar(graph);
		
			obj = std::move(path(obj.release(), graph));
			
			(*graph)[p].object(*obj);
		} else {
			std::unique_ptr<N3Node> obj = objectorvar(graph);
		
			obj = std::move(path(obj.release(), graph));
		
			graph->triple(*subject, *property, *obj);
		}
	}
	
	void Parser::objectlistvar(GraphTemplate *graph, const N3Node *subject, const Resource *property)
	{
		if (m_lookAhead == Token::Var || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
			
			addTriple(graph, subject, property);
			
			while (m_lookAhead == ',') {
				match();
				if (m_lookAhead == Token::Var || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
					addTriple(graph, subject, property);
				} else
					throw ParseException("expected object after ','", line());
			}
		} else
			throw ParseException("expected object", line());
	}
	
	void Parser::addTriple(GraphTemplate *graph, const N3Node *subject, const Var *property)
	{
		if (m_lookAhead == '[') { // this hack rearanges the order of some triples for performance reasons
			graph->triple(*subject, *property, BlankNode(""));
			
			std::size_t p = graph->size() - 1;
			
			std::unique_ptr<N3Node> obj = objectorvar(graph);
			
			obj = std::move(path(obj.release(), graph));
			
			(*graph)[p].object(*obj);
		} else {
			std::unique_ptr<N3Node> obj = objectorvar(graph);
			
			obj = std::move(path(obj.release(), graph));
			
			graph->triple(*subject, *property, *obj);
		}
	}
	
	void Parser::objectlistvar(GraphTemplate *graph, const N3Node *subject, const Var *property)
	{
		if (m_lookAhead == Token::Var || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
			
			addTriple(graph, subject, property);
			
			while (m_lookAhead == ',') {
				match();
				if (m_lookAhead == Token::Var || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '{' || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
					addTriple(graph, subject, property);
				} else
					throw ParseException("expected object after ','", line());
			}
		} else
			throw ParseException("expected object", line());
	}
	
	std::unique_ptr<N3Node> Parser::objectorvar(GraphTemplate *graph)
	{
		if (m_lookAhead == Token::Var) {
			match();
			
			return std::unique_ptr<Var>(new Var(m_lexeme.substr(1)));
		}
		
		return object(graph);
	}
	
	std::string Parser::unescape(std::size_t start, const std::string &localName)
	{
		std::size_t end = localName.length();
		
		if (localName.find('\\', start) == std::string::npos)
			return localName.substr(start, end - start);
		
		std::string buf;
		
		buf.reserve(end - start);
		
		for (std::size_t i = start; i < end; i++) {
			char c = localName[i];
			
			if (c == '\\') {
				c = localName[++i];
				if (LOCAL_NAME_ESCAPE_CHARS.find(c) != std::string::npos)
					buf.push_back(c);
				else
					throw ParseException("\"" + localName + "\" contains illegal escape \"\\" + c + "\"");
			} else {
				buf.push_back(c);
			}
		}
		
		return buf;
	}
	
	std::string Parser::extractUri(const std::string &uriLiteral)
	{
		if (uriLiteral.find('\\', 1) == std::string::npos)
			return uriLiteral.substr(1, uriLiteral.length() - 2);
		
		std::string buf;
		buf.reserve(uriLiteral.length());
		auto inserter = std::back_inserter(buf);
		
		std::uint16_t highSurrogate = 0;
		
		for (auto i = uriLiteral.begin() + 1; i < uriLiteral.end() - 1;) {
			char c = *i;
			
			if (c == '\\') {
				++i;
				c = *i;
				switch (c) {
					case 'u' : {
						auto begin = ++i; i += 4; 
						std::string value = std::string(begin, i);
						int v = std::stoi(value, nullptr, 16);
							
						if (utf16::isHighSurrogate(v)) {
							if (highSurrogate)
								throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
							
							highSurrogate = v;
						} else {
							
							if (utf16::isLowSurrogate(v)) {
								if (!highSurrogate)
									throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
								
								v = utf16::toChar(highSurrogate, v);
								utf8::encode(v, inserter);
								highSurrogate = 0;
							} else {
								if (v <= 0x20 || (v < 128 && INVALID_ESCAPES.find(std::string::traits_type::to_char_type(v)) != std::string::npos))
									throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\u" + value + "\"");
								
								utf8::encode(v, inserter);
							}
						}
						
						break;
					}
					case 'U' : {
						if (highSurrogate)
							throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
						
						auto begin = ++i; i += 8;
						std::string value = std::string(begin, i);
						int v = std::stoi(value, nullptr, 16);
						if (v <= 0x20 || (v < 128 && INVALID_ESCAPES.find(std::string::traits_type::to_char_type(v)) != std::string::npos))
							throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\U" + value + "\"");
						utf8::encode(v, inserter);
						break;
					}
					default  :
						throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\" + c + "\"");
				}
			} else {
				if (highSurrogate)
					throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
				inserter = c; // this will actually append c to buf;
				++i;
			}
		}
		
		return buf;
	}
	
	std::string Parser::extractString(const std::string &stringLiteral)
	{
		// Because of the lexer produced stringLiteral, we can assume that the its value is "well formed":
		// enclosed in matched quotes, escapes are valid, indexes will never go outside the string bounds...
		std::size_t start;
		std::size_t end;
		
		if (stringLiteral.find("\"\"\"") == 0 || stringLiteral.find("'''") == 0) {
			start = 3;
			end   = stringLiteral.length() - 3;
		} else {
			start = 1;
			end   = stringLiteral.length() - 1;
		}
		
		if (stringLiteral.find('\\', start) == std::string::npos)
			return stringLiteral.substr(start, end - start);
		
		std::string buf;
		buf.reserve(end - start);
		
		std::uint16_t highSurrogate = 0;
		
		for (std::size_t i = start; i < end; i++) {
			char c = stringLiteral[i];
			
			if (c == '\\') {
				c = stringLiteral[++i];
				
				if (highSurrogate && c != 'u')
					throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
				
				switch (c) {
					case 'n' : buf.push_back('\n'); break;
					case 'r' : buf.push_back('\r'); break;
					case 't' : buf.push_back('\t'); break;
					case 'f' : buf.push_back('\f'); break;
					case 'b' : buf.push_back('\b'); break; // backspace, "\u0008"
					case '"' : buf.push_back('"');  break;
					case '\'': buf.push_back('\''); break;
					case '\\': buf.push_back('\\'); break;
					case 'u' : {
						std::size_t begin = ++i; i += 3; 
						std::string value = stringLiteral.substr(begin, 4);
						int v = std::stoi(value, nullptr, 16);
						
						if (utf16::isHighSurrogate(v)) {
							if (highSurrogate)
								throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
								
							highSurrogate = v;
						} else {
							if (utf16::isLowSurrogate(v)) {
								if (!highSurrogate)
									throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
								
								v = utf16::toChar(highSurrogate, v);
								utf8::encode(v, std::back_inserter(buf));
								highSurrogate = 0;
							} else {
								utf8::encode(v, std::back_inserter(buf));
							}
						}
						break;
					}
					case 'U' : {
						std::size_t begin = ++i; i += 7;
						std::string value = stringLiteral.substr(begin, 8);
						int v = std::stoi(value, nullptr, 16);
						utf8::encode(v, std::back_inserter(buf));
						break;
					}
					
					default  :
						throw ParseException(stringLiteral + " contains \"\\" + c + "\"");
				}
			} else {
				if (highSurrogate)
					throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
				
				buf.push_back(c);
			}
		}
		
		return buf;
	}
	
}
