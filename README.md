# Carl: Another Rule Language

Carl is a tool for parsing [N3](http://www.w3.org/TeamSubmission/n3/) files and outputting the resulting triples in "N3P" format.
"N3P" is a RDF/rules serialization format used by the [Eye reasoner](http://eulersharp.sourceforge.net/).

## Usage

`carl [-b=base-uri] [-o=output-file] [input-files]`

* `-b=baseUri` the base URI to use when resolving relative URIs.
* `-o=output-file` where the results are written, write to stdout when omitted.
* `input-files` the Turtle input files to process, read from stdin when omitted.

## Limitations

* '@' keywords are not supported, with the exception of '@prefix' and '@base'.
* The rules for matching terminal symbols do not follow the team submission, but are adapted to match the Turtle and SPARQL grammar terminals.
* Variables are not allowed outside of graphs.
* Literals, lists and graphs are not allowed as property.
