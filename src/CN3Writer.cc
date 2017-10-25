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

#include "CN3Writer.hh"


namespace n3 {
	
	
	const std::string N3PFormatter::SKOLEM_PREFIX("https://melgi.github.io/.well-known/genid/#");
	const char N3PFormatter::HEX_CHAR[] = "0123456789ABCDEF";
	
	
	void N3PFormatter::visit(const URIResource &resource)
	{
		m_outbuf->sputc('\'');
		m_outbuf->sputc('<');
		outputUri(resource.uri());
		m_outbuf->sputc('>');
		m_outbuf->sputc('\'');
	}
	
	void N3PFormatter::visit(const BlankNode &blankNode)
	{
		const std::string &id = blankNode.id();
		
		if (!rule()) {
			m_outbuf->sputc('\'');
			m_outbuf->sputc('<');
			m_outbuf->sputn(SKOLEM_PREFIX.c_str(), SKOLEM_PREFIX.length());
			m_outbuf->sputn(id.c_str(), id.length());
			if (!m_graphs.empty()) {
				const std::string &suffix = m_graphs.back();
				m_outbuf->sputc('_');
				m_outbuf->sputn(suffix.c_str(), suffix.length());
			}
			m_outbuf->sputc('>');
			m_outbuf->sputc('\'');
		} else {
			// output as universal
			m_outbuf->sputc('V');
			
			std::size_t p = id.find('-');
			if (p != std::string::npos) {
				++p;
				m_outbuf->sputn(id.c_str() + p, id.length() - p);
			} else {
				m_outbuf->sputn(id.c_str(), id.length());
			}
			
			const std::string &suffix = m_graphs.back();
			m_outbuf->sputc('_');
			m_outbuf->sputn(suffix.c_str(), suffix.length());
		}
	}
	
	void N3PFormatter::visit(const Literal &literal)
	{
		m_outbuf->sputn("literal('", 9);
		output(literal.lexical());
		m_outbuf->sputn("',type('<", 9);
		outputUri(literal.datatype());
		m_outbuf->sputn(">'))", 4);
	}
	
	void N3PFormatter::visit(const BooleanLiteral &literal)
	{
		const std::string &lexical = literal.value() ? BooleanLiteral::VALUE_TRUE.lexical() : BooleanLiteral::VALUE_FALSE.lexical();
		
		m_outbuf->sputn(lexical.c_str(), lexical.length());
	}
	
	void N3PFormatter::visit(const IntegerLiteral &literal)
	{
		const std::string &lexical = literal.lexical();
		
		m_outbuf->sputn(lexical.c_str(), lexical.length());
	}
	
	void N3PFormatter::visit(const DoubleLiteral &literal)
	{
		const std::string &value = literal.lexical();
		
		// values like .5 and -.5 are not allowed in prolog
		// values like 5. and 5.E0 are not allowed in prolog
		
		bool appendZero = false;
		
		std::string s;
		const std::string *sp = &value;
		
		std::size_t p = value.find('.');
		if (p != std::string::npos) {
			++p;
			if (p == value.length()) {
				appendZero = true;
			} else if (value[p] == 'E' || value[p] == 'e') {
				s.reserve(value.length() + 1);
				s = value;
				s.insert(p, 1, '0');
				sp = &s;
			}
		}
		
		if ((*sp)[0] == '.') {
			m_outbuf->sputc('0');
			m_outbuf->sputn(sp->c_str(), sp->length());
			if (appendZero)
				m_outbuf->sputc('0');
		} else if ((*sp)[0] == '-' && (*sp)[1] == '.') {
			m_outbuf->sputc('-');
			m_outbuf->sputc('0');
			m_outbuf->sputn(sp->c_str() + 1, sp->length() - 1);
			if (appendZero)
				m_outbuf->sputc('0');
		} else {
			m_outbuf->sputn(sp->c_str(), sp->length());
			if (appendZero)
				m_outbuf->sputc('0');
		}
	}
	
	void N3PFormatter::visit(const DecimalLiteral &literal)
	{
		const std::string &value = literal.lexical();
		
		if (m_rdivDecimal) {
			std::size_t p = value.find('.');
			if (p == std::string::npos) {
				m_outbuf->sputn(value.c_str(), value.length());
				m_outbuf->sputn(" rdiv 1", 7);
			} else {
				m_outbuf->sputn(value.c_str(), p++);
				std::size_t len = value.length() - p;
				m_outbuf->sputn(value.c_str() + p, len);
				m_outbuf->sputn(" rdiv 1", 7);
				for (std::size_t i = 0; i < len; i++)
					m_outbuf->sputc('0');
			}
		} else {
			// values like .5 and -.5 are not allowed in prolog
			// values like 5. are not allowed in prolog
			
			if (value[0] == '.') {
				m_outbuf->sputc('0');
				m_outbuf->sputn(value.c_str(), value.length());
			} else if (value[0] == '-' && value[1] == '.') {
				m_outbuf->sputc('-');
				m_outbuf->sputc('0');
				m_outbuf->sputn(value.c_str() + 1, value.length() - 1);
			} else {
				m_outbuf->sputn(value.c_str(), value.length());
			}
			
			std::size_t length = value.length();
			if (length > 0 && value[length - 1] == '.')
				m_outbuf->sputc('0');
		}
	}

	void N3PFormatter::visit(const StringLiteral &literal)
	{
		m_outbuf->sputn("literal('", 9);
		output(literal.lexical());
		m_outbuf->sputc('\'');
		const std::string &lang = literal.language();
		if (!lang.empty()) {
			m_outbuf->sputn(",lang('", 7);
			m_outbuf->sputn(lang.c_str(), lang.length());
			m_outbuf->sputc('\'');
			m_outbuf->sputc(')');
		} else {
			m_outbuf->sputn(",type('<", 8);
			m_outbuf->sputn(StringLiteral::TYPE.c_str(), StringLiteral::TYPE.length());
			m_outbuf->sputc('>');
			m_outbuf->sputc('\'');
			m_outbuf->sputc(')');
		}
		
		m_outbuf->sputc(')');
	}
	
	void N3PFormatter::visit(const RDFList &list)
	{
		m_outbuf->sputc('[');
		if (!list.empty()) {
			auto i = list.begin();
			(*i)->visit(*this);
			++i;
			//m_count += 2;
			while (i != list.end()) {
				m_outbuf->sputc(',');
				(*i)->visit(*this);
				++i;
				//m_count += 2;
			}
		}
		m_outbuf->sputc(']');
	}
	
	void N3PFormatter::visit(const GraphTemplate &graph)
	{
		output(graph, true);
	}
	
	void N3PFormatter::output(const GraphTemplate &graph, bool wrap)
	{
		m_graphs.push_back(graph.id());
		
		switch (graph.size()) {
			case 0: m_outbuf->sputn("true", 4); break;
			case 1: {
				const TriplePattern &t = *graph.begin();
				m_writer.outputTriple(t.subject(), t.property(), t.object(), &graph);
				break;
			}
			default: {
				if (wrap)
					m_outbuf->sputc('(');
				
				auto i = graph.begin();
				
				m_writer.outputTriple(i->subject(), i->property(), i->object(), &graph);
				++i;
				
				while (i != graph.end()) {
					m_outbuf->sputc(',');
					m_outbuf->sputc(' ');
					m_writer.outputTriple(i->subject(), i->property(), i->object(), &graph);
					++i;
				}
				
				if (wrap)
					m_outbuf->sputc(')');
			}
		}
		
		m_graphs.pop_back();
		if (m_graphs.empty())
			rule(false);
	}
	
	void N3PFormatter::visit(const Var &var)
	{
		const std::string &name = var.name();
		
		m_outbuf->sputc('_');
		m_outbuf->sputn(name.c_str(), name.length());
	}
	
	void CN3Writer::writePrologue()
	{
		m_out << ":- style_check(-discontiguous)."; endl();
		m_out << ":- style_check(-singleton)."; endl();
		m_out << ":- multifile(exopred/3)."; endl();
		m_out << ":- multifile(implies/3)."; endl();
		m_out << ":- multifile(pfx/2)."; endl();
		m_out << ":- multifile(pred/1)."; endl();
		m_out << ":- multifile(prfstep/8)."; endl();
		m_out << ":- multifile(scope/1)."; endl();
		m_out << ":- multifile(scount/1)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#mu>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#pi>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#sigma>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#biconditional>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#conditional>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#reflexive>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#relabel>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#tactic>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#transaction>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#first>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#rest>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#implies>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#outputString>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2002/07/owl#sameAs>'/2)."; endl();
		m_out << "flag('no-skolem', '" << N3PFormatter::SKOLEM_PREFIX << "')."; endl();
	}

	void CN3Writer::writeEpilogue()
	{
		m_out << "scount(" << m_count << ")."; endl();
		m_out << "end_of_file."; endl();
		
		m_out << std::flush;
	}
	
	void CN3Writer::outputProperty(const std::string &uri)
	{
#ifdef CARL_N3P_CESU8
		m_out.write(":- dynamic('<", 13);
		m_formatter.outputUri(uri);
		m_out.write(">'/2).", 6);
		endl();
		m_out.write(":- multifile('<", 15);
		m_formatter.outputUri(uri);
		m_out.write(">'/2).", 6);
		endl();
//		m_out.write("pred('<", 7);
//		m_formatter.outputUri(uri);
//		m_out.write(">').", 4);
//		endl();
#else /* !CARL_N3P_CESU8 */
		if (uri.find('\'') == std::string::npos) {
			m_out.write(":- dynamic('<", 13);
			m_out.write(uri.c_str(), uri.length());
			m_out.write(">'/2).", 6);
			endl();
			m_out.write(":- multifile('<", 15);
			m_out.write(uri.c_str(), uri.length());
			m_out.write(">'/2).", 6);
			endl();
//			m_out.write("pred('<", 7);
//			m_out.write(uri.c_str(), uri.length());
//			m_out.write(">').", 4);
//			endl();
		} else {
			m_out.write(":- dynamic('<", 13);
			m_formatter.outputUri(uri);
			m_out.write(">'/2).", 6);
			endl();
			m_out.write(":- multifile('<", 15);
			m_formatter.outputUri(uri);
			m_out.write(">'/2).", 6);
			endl();
//			m_out.write("pred('<", 7);
//			m_formatter.outputUri(uri);
//			m_out.write(">').", 4);
//			endl();
		}
#endif /* CARL_N3P_CESU8 */
	}
	
	void CN3Writer::outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object, const GraphTemplate *graph)
	{
		bool implies = false;
		bool backwardsImplies = false;
		
		if (property.uri() == LOG::implies.uri()) {
			m_formatter.rule(true);
			if (graph) {
				m_formatter.visit(LOG::implies);
			} else {
				m_out.write("implies", 7);
				implies = true;
			}
			m_out.put('(');
		} else if (property.uri() == LOG::reverseImplies.uri()) {
			m_formatter.rule(true);
			if (subject.isGraphTemplate()) {
				const GraphTemplate &g = static_cast<const GraphTemplate &>(subject);
				Optional<std::string> p = extractPredicate(g);
				if (p) {
					if (!graph) {
						m_out.write("cpred('<", 8);
						m_formatter.outputUri(*p);
						m_out.write(">').", 4);
						endl();
					} else {
						m_out.put('(');
					}
					m_formatter.output(g, true);
					m_out.put(' ');
				}
			} else if (subject.isVar()) {
				if (graph) {
					m_out.put('(');
				}
				subject.visit(m_formatter);
				m_out.put(' ');
			}
			m_out.write(":- ", 3);
			backwardsImplies = true;
		} else {
			m_formatter.visit(property);
			m_out.put('(');
		}
		
		if (!backwardsImplies) {
			subject.visit(m_formatter);
			m_out.put(',').put(' ');
			if (implies)
				m_formatter.rule(true);
			object.visit(m_formatter);
		} else {
			m_formatter.rule(true);
			if (!object.isGraphTemplate())
				object.visit(m_formatter);
			else
				m_formatter.output(static_cast<const GraphTemplate &>(object), false);
			
			if (graph) {
				m_out.put(')');
			}
		}

		if (implies) {
			m_out.write(", '<", 4);
			m_formatter.outputUri(m_source);
			m_out.write(">'", 2);
		}
		
		if (!backwardsImplies)
			m_out.put(')');
		
		++m_count;
	}
	
	void CN3Writer::outputTriple(const N3Node &subject, const N3Node &property, const N3Node &object, const GraphTemplate *graph)
	{
		if (property.isURIResource()) {
			const URIResource &resource = static_cast<const URIResource &>(property);
			outputTriple(subject, resource, object, graph);
		} else {
			if (property.isVar()) {
				m_out.write("exopred(", 8);
				property.visit(m_formatter);
				m_out.write(", ", 2);
			} else {
				// TODO throw error here?
				property.visit(m_formatter);
				m_out.put('(');
			}
			
			subject.visit(m_formatter);
			m_out.put(',').put(' ');
			object.visit(m_formatter);
			
			//if (property.isVar())
			m_out.put(')');

			++m_count;
		}
		
	}
	
	
	Optional<std::string> CN3Writer::extractPredicate(const GraphTemplate &graph)
	{
		if (graph.size() != 1)
			return Optional<std::string>::none;
		
		const N3Node &property = graph.begin()->property();
		if (!property.isURIResource())
			return Optional<std::string>::none;
		
		return Optional<std::string>(static_cast<const URIResource &>(property).uri());
	}
	
	void CN3Writer::handleProperties(const GraphTemplate &graph)
	{
		for (const TriplePattern &t : graph) {
			const N3Node &subject = t.subject();
			if (subject.isGraphTemplate())
				handleProperties(static_cast<const GraphTemplate &>(subject));
			
			const N3Node &property = t.property();
			if (property.isURIResource()) {
				const std::string &uri = static_cast<const URIResource &>(property).uri();
				if (!(uri == LOG::implies.uri() || uri == LOG::reverseImplies.uri())) {
					if (m_properties.insert(uri).second)
						outputProperty(uri);
				}
			}
			
			const N3Node &object = t.object();
			if (object.isGraphTemplate())
				handleProperties(static_cast<const GraphTemplate &>(object));
		}
	}
	
}
