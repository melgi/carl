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

#ifndef CARL_PARSER_H
#define CARL_PARSER_H

#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <FlexLexer.h>

#include "Uri.hh"
#include "Token.hh"
#include "Model.hh"
#include "BlankNodeIdGenerator.hh"

namespace n3 {
	
	class ParseException : public std::runtime_error {
		int m_line;
	public:
		explicit ParseException(const std::string &message = std::string(), int line = -1) : std::runtime_error(message), m_line(line) {}
		int line() const noexcept { return m_line; }
	};
	
	struct TripleSink {
		
		virtual void start() = 0;
		virtual void end() = 0;
		virtual void document(const std::string &source) = 0;
		virtual void prefix(const std::string &prefix, const std::string &ns) = 0;
		virtual void triple(const N3Node &subject, const N3Node &property, const N3Node &object) = 0;
		virtual unsigned count() const = 0;
		
		virtual ~TripleSink() {}
	};
	
	class DefaultTripleSink : public TripleSink {
	protected:
		unsigned m_count;
	public:
		DefaultTripleSink() : TripleSink(), m_count(0) {}
		virtual void start() override {}
		virtual void end() override {}
		virtual void document(const std::string &source) override {}
		virtual void prefix(const std::string &prefix, const std::string &ns) override {}
		virtual void triple(const N3Node &subject, const N3Node &property, const N3Node &object) override { ++m_count; }
		virtual unsigned count() const override { return m_count; }
	};

	
	class Parser {
		
		static const std::string LOCAL_NAME_ESCAPE_CHARS;
		static const std::string INVALID_ESCAPES;
		
		::yyFlexLexer m_lexer;
		
		Uri m_base;
		TripleSink *m_sink;
		std::map<std::string, std::string> m_prefixMap;
		
		BlankNodeIdGenerator m_blanks;
		unsigned m_graphs;
		
		Token::Type m_lookAhead;
		std::string m_lexeme;
		
		Token::Type nextToken() { return m_lexer.yylex(); }
		
		void match(Token::Type token)
		{
			m_lexeme.assign(m_lexer.YYText(), m_lexer.YYLeng());
			if (m_lookAhead == token)
				m_lookAhead = nextToken();
			else
				throw ParseException("expected different symbol");
		}

		void match()
		{
			m_lexeme.assign(m_lexer.YYText(), m_lexer.YYLeng());
			m_lookAhead = nextToken();
		}
		
		Uri resolve(const std::string &uri);
		Uri resolve(std::string &&uri);
		std::string toUri(const std::string &pname) const;
		
		void n3doc();
		void base();
		void prefixID();
		void sparqlBase();
		void sparqlPrefix();
		void triples();
		std::unique_ptr<N3Node> subject(GraphTemplate *graph = nullptr);
		void propertylist(const N3Node *subject);
		void property(const N3Node *subject);
		std::string iri();
		void objectlist(const N3Node *subject, const Resource *property);
		std::unique_ptr<N3Node> object(GraphTemplate *graph = nullptr);
		std::unique_ptr<Literal> dtlang(std::string &&lexicalValue);
		std::unique_ptr<RDFList> collection(GraphTemplate *graph);
		std::unique_ptr<BlankNode> blanknodepropertylist();
		std::unique_ptr<BlankNode> blanknodepropertylistvar(GraphTemplate *graph);
		void propertylistopt(const N3Node *subject);
		void propertylistoptvar(GraphTemplate *graph, const N3Node *subject);
		std::unique_ptr<GraphTemplate> graphTemplate();
		void propertylistvar(GraphTemplate *graph, const N3Node *subject);
		void propertyorvar(GraphTemplate *graph, const N3Node *subject);
		std::unique_ptr<N3Node> subjectorvar(GraphTemplate *graph);
		void objectlistvar(GraphTemplate *graph, const N3Node *subject, const Resource *property);
		void objectlistvar(GraphTemplate *graph, const N3Node *subject, const Var *property);
		void addTriple(GraphTemplate *graph, const N3Node *subject, const Resource *property);
		void addTriple(GraphTemplate *graph, const N3Node *subject, const Var *property);
		
		std::unique_ptr<N3Node> objectorvar(GraphTemplate *graph);
		
		std::unique_ptr<N3Node> path(N3Node *subject);
		std::unique_ptr<N3Node> path(N3Node *subject, GraphTemplate *graph);
		
		static std::string unescape(std::size_t start, const std::string &localName);
		static std::string extractUri(const std::string &uriLiteral);
		static std::string extractString(const std::string &stringLiteral);
		
	public:
		Parser(std::istream *in, const Uri &base, TripleSink *sink) : m_lexer(in), m_base(base), m_sink(sink), m_prefixMap(), m_blanks(), m_graphs(0), m_lookAhead(0), m_lexeme() {}
		
		void parse()
		{
			m_sink->document(static_cast<std::string>(m_base));
			m_lookAhead = nextToken();
			
			n3doc();
		}

		int line() const { return m_lexer.lineno(); }
	};

}

#endif /* CARL_PARSER_H */
