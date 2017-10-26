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

#include <ostream>
#include <iostream>
#include <string>
#include <cstddef>
//#include <unordered_set>
#include <stdexcept>
#include <vector>


#include "Parser.hh"
#include "Utf8.hh"
#include "Utf16.hh"

#ifdef _WIN32
#	define CARL_CRLF
#	ifndef CARL_N3P_UTF8
#		define CARL_N3P_CESU8
#	endif
#endif

namespace n3 {
	
	class CN3Writer;
	
	class N3PFormatter : public N3NodeVisitor {
		
		CN3Writer &m_writer;
		std::streambuf *m_outbuf;
		
		bool m_rdivDecimal; // output decimals as rdivs
		
		std::vector<std::string> m_graphs;
		
		bool m_rule;
		
	public:
		static const std::string SKOLEM_PREFIX;
		static const char HEX_CHAR[];
		
		N3PFormatter(CN3Writer &writer, std::ostream &out, bool rdivDecimal) : N3NodeVisitor(), m_writer(writer), m_outbuf(out.rdbuf()), m_rdivDecimal(rdivDecimal), m_graphs(), m_rule()
		{
			// nop
		}
		
		void visit(const URIResource &resource) override;
		void visit(const BlankNode &blankNode) override;
		void visit(const Literal &literal) override;
		void visit(const BooleanLiteral &literal) override;
		void visit(const IntegerLiteral &literal) override;
		void visit(const DoubleLiteral &literal) override;
		void visit(const DecimalLiteral &literal) override;
		void visit(const StringLiteral &literal) override;
		void visit(const RDFList &list) override;
		void visit(const GraphTemplate &graph) override;
		void visit(const Var &var) override;
		
		void rule(bool rule) { m_rule = rule; }
		bool rule() const { return m_rule; }
		
		void output(const GraphTemplate &graph, bool wrap);
		
		void output(const std::string &s)
		{
			for (auto i = s.cbegin(); i != s.cend(); ++i) {
				char c = *i;
				if (c >= 0 && c <= 0x1F) {
					switch (c) {
						case '\n' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('n');  break;
						case '\r' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('r');  break;
						case '\t' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('t');  break;
						case '\f' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('f');  break;
						case '\b' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('b');  break; // backspace, "\u0008"
						default   : writeHex(c);
					}
				} else {
					switch (c) {
						case '"'  : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('"');                         break;
						case '\'' : m_outbuf->sputc('\\'); m_outbuf->sputc('\'');                                               break;
						case '\\' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); break;
						default   :
#ifdef CARL_N3P_CESU8
							if ((c & 0xF8) == 0xF0) {
								i += ouputCesu8(i, s.cend()) - 1;
							} else {
								m_outbuf->sputc(c);
							}
#else /* !CARL_N3P_CESU8 */
							m_outbuf->sputc(c);
#endif /* CARL_N3P_CESU8 */
					}
				}
			}
		}
		
		void outputUri(const std::string &s)
		{
#ifdef CARL_N3P_CESU8
			for (auto i = s.cbegin(); i != s.cend(); ++i) {
				char c = *i;
				if (c == '\'') {
					m_outbuf->sputc('\\');
					m_outbuf->sputc('\'');
				} else {
					if ((c & 0xF8) == 0xF0) {
						i += ouputCesu8(i, s.cend()) - 1;
					} else {
						m_outbuf->sputc(c);
					}
				}
			}
#else /* !CARL_N3P_CESU8 */
			if (s.find('\'') == std::string::npos) {
				m_outbuf->sputn(s.c_str(), s.length());
			} else {
				for (auto i = s.cbegin(); i != s.cend(); ++i) {
					char c = *i;
					if (c == '\'') {
						m_outbuf->sputc('\\');
						m_outbuf->sputc('\'');
					} else {
						m_outbuf->sputc(c);
					}
				}
			}
#endif /* CARL_N3P_CESU8 */
		}

	private:
	
#ifdef CARL_N3P_CESU8

		template<typename Iterator> std::size_t ouputCesu8(const Iterator &i, const Iterator &end)
		{
			const char32_t REPLACEMENT_CHARACTER = U'\uFFFD';
			
			char32_t cp;
			
			utf8::State state;
			int r = utf8::decode(&cp, i, end, &state);
			
			if (r == -1) {
				cp = REPLACEMENT_CHARACTER;
				r  = 1;
			} else if (r == -2) {
				cp = REPLACEMENT_CHARACTER;
				r  = end - i;
			}
			
			utf16::encodeCESU8(cp, std::ostreambuf_iterator<std::streambuf::char_type>(m_outbuf));
			
			return r;
		}
		
#endif /* CARL_N3P_CESU8 */

		void writeHex(char c)
		{
			int hi = (c & 0xF0) >> 4;
			int lo = (c & 0x0F);
			
			m_outbuf->sputn("\\u00", 4);
			
			m_outbuf->sputc(HEX_CHAR[hi]);
			m_outbuf->sputc(HEX_CHAR[lo]);
		}
	};

	class CN3Writer : public DefaultTripleSink {
		
		std::ostream &m_out;
		N3PFormatter m_formatter;
		std::string m_source;
//		std::unordered_set<std::string> m_properties;

//		void outputProperty(const std::string &uri);
		
		static Optional<std::string> extractPredicate(const GraphTemplate &graph);
//		void handleProperties(const GraphTemplate &graph);
		
		void output(const N3Node &node)
		{
			node.visit(m_formatter);
		}
		
		void endl()
		{
#ifdef CARL_CRLF
			m_out.put('\r');
#endif
			m_out.put('\n');
		}

		void writePrologue();
		void writeEpilogue();
		
	public:
		
		CN3Writer(std::ostream &out) : DefaultTripleSink(), m_out(out), m_formatter(*this, out, false), m_source()/*, m_properties()*/
		{
			// nop
		}
		
		void document(const std::string &source) override
		{
			m_source = source;
			
			m_out.write("scope('<", 8);
			m_formatter.outputUri(source);
			m_out.write(">').", 4);
			
			endl();
		}
		
		void outputTriple(const N3Node &subject, const N3Node &property, const N3Node &object, const GraphTemplate *graph = nullptr);
		void outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object, const GraphTemplate *graph = nullptr);
		
		void start() override { writePrologue(); }
		void end() override { writeEpilogue(); }
		
		void prefix(const std::string &prefix, const std::string &ns) override
		{
			m_out << "pfx('";
			m_formatter.output(prefix);
			m_out << ":','<";
			m_formatter.outputUri(ns);
			m_out << ">').";
			endl();
		}
		
		void triple(const N3Node &subject, const N3Node &property, const N3Node &object) override
		{
//			if (subject.isGraphTemplate())
//				handleProperties(static_cast<const GraphTemplate &>(subject));
//			
//			if (object.isGraphTemplate())
//				handleProperties(static_cast<const GraphTemplate &>(object));
			
			if (property.isURIResource()) {
				const URIResource &resource = static_cast<const URIResource &>(property);
//				const std::string &uri = resource.uri();
//				
//				if (!(uri == LOG::implies.uri() || uri == LOG::reverseImplies.uri())) {
//					if (m_properties.insert(uri).second)
//						outputProperty(uri);
//				}
				outputTriple(subject, resource, object);
			} else {
				outputTriple(subject, property, object);
			}
			
			m_out.put('.');
			endl();
		}

	};

}
