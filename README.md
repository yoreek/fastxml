# FastXML

Fast Ruby Hash to XML and XML to Ruby Hash converter written in pure C.

[![Build Status](https://secure.travis-ci.org/yoreek/fastxml.png?branch=master)](http://travis-ci.org/yoreek/fastxml)

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'fast-xml'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install fast-xml

## Release Notes

See [CHANGELOG.md](CHANGELOG.md)

## Usage

```ruby
require 'fastxml'

# convert hash to XML
FastXML.hash2xml({ tag1: { tag2: 'content' } }, indent: 2)
# =>
# <?xml version="1.0" encoding="utf-8"?>
# <root>
#   <tag1>
#     <tag2>content</tag2>
#   </tag1>
# </root>

# use enumerator
FastXML.hash2xml({ enumerator: 3.times }, indent:  2)
# =>
# <?xml version="1.0" encoding="utf-8"?>
# <root>
#   <enumerator>0</enumerator>
#   <enumerator>1</enumerator>
#   <enumerator>2</enumerator>
# </root>

# output to file handle
# fh = StringIO.new
FastXML.hash2xml({ enumerator: 3.times }, indent:  2, output: fh)
fh.string
# =>
# <?xml version="1.0" encoding="utf-8"?>
# <root>
#   <enumerator>0</enumerator>
#   <enumerator>1</enumerator>
#   <enumerator>2</enumerator>
# </root>

```

## Options

The following options are available to pass to FastXML.hash2xml(hash, options = {}).

* **:root** => 'root'
  * Root node name.

* **:version** => '1.0'
  * XML document version

* **:encoding** => 'utf-8'
  * XML input/output encoding

* **:indent** => 0
  * if indent great than "0", XML output should be indented according to
    its hierarchic structure. This value determines the number of
    spaces.

  * if indent is "0", XML output will all be on one line.

* **:output** => nil
  * XML output method

  * if output is nil, XML document dumped into string.

  * if output is filehandle, XML document writes directly to a filehandle or a
    stream.

* **:canonical** => false
  * if canonical is "true", converter will be write hashes sorted by key.

  * if canonical is "false", order of the element will be pseudo-randomly.

* **:use_attr** => false
  * if use_attr is "true", converter will be use the attributes.

  * if use_attr is "fale", converter will be use tags only.

* **:content** => nil
  * if defined that the key name for the text content(used only if
    use_attr = true).

* **:force_array** => nil
  * This option is similar to "ForceArray" from [XMl::Simple module]:
    (https://metacpan.org/pod/XML::Simple#ForceArray-1-in---important).

* **:force_content** => nil
  * This option is similar to "ForceContent" from [XMl::Simple module]:
    (https://metacpan.org/pod/XML::Simple#ForceContent-1-in---seldom-used).

* **:merge_text** => false
  * Setting this option to "true" will cause merge adjacent text nodes.

* **:xml_decl** => true
  * if xml_decl is "true", output will start with the XML declaration
    '<?xml version="1.0" encoding="utf-8"?>'.

  * if xml_decl is "false", XML declaration will not be output.

* **:trim** => false
  * Trim leading and trailing whitespace from text nodes.

* **:utf8** => true
  * Turn on utf8 flag for strings if enabled.

* **:max_depth** => 1024
  * Maximum recursion depth.

* **:buf_size** => 4096
  * Buffer size for reading end encoding data.

* **:keep_root** => false
  * Keep root element.

## Configuration
```
  FastXML.configure do |config|
    config.trim = true
  end
```

## Benchmarks

Performance benchmark in comparison with some popular gems:

```
Converting Hash to XML:
                           user     system      total        real
activesupport(rexml)    11.020000   0.000000  11.020000 ( 11.058084)
activesupport(libxml)   10.690000   0.000000  10.690000 ( 10.731521)
activesupport(nokogiri) 10.730000   0.010000  10.740000 ( 10.769866)
xmlsimple                1.470000   0.000000   1.470000 (  1.477457)
fastxml                  0.010000   0.000000   0.010000 (  0.018434)
```

## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).

