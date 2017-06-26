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

### Convert hash toXML

```ruby
require 'fastxml'

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

### Convert XML to hash

```ruby
FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b attr="bbb">bbb2</b></root>')
# => {"a"=>["aaa", "aaa2"], "b"=>{"attr"=>"bbb", "content"=>"bbb2"}}

# use filter
FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b>bbb</b></root>', filter: '/root/a') do |node|
  p node
end
# =>
# {"a"=>"aaa"}
# {"a"=>"aaa2"}
```


## Options

The following options are available to pass to FastXML.hash2xml(hash, options = {}).

* **:root => 'root' # hash2xml**
  * Root node name.

* **:version => '1.0' # hash2xml**
  * XML document version

* **:encoding => 'utf-8' # hash2xml + xml2hash**
  * XML input/output encoding

* **:indent => 0 # hash2xml**
  * if indent great than **0**, XML output should be indented according to
    its hierarchic structure. This value determines the number of
    spaces.

  * if indent is **0**, XML output will all be on one line.

* **:output => nil # hash2xml**
  * XML output method

  * if output is **nil**, XML document dumped into string.

  * if output is filehandle, XML document writes directly to a filehandle or a
    stream.

* **:canonical => false # hash2xml**
  * if canonical is **true**, the converter will be write hashes sorted by key.

  * if canonical is **false**, the order of the element will be pseudo-randomly.

* **:use_attr => false # hash2xml**
  * if use_attr is **true**, the converter will be use the attributes.

  * if use_attr is **fale**, the converter will be use tags only.

* **:content => 'content' # hash2xml + xml2hash**
  * The key name for the text content.

* **:force_array => nil # xml2hash**
  * When this option is **true**, the converter will be to force nested elements
    to be represented as arrays even when there is only one.

    ```FastXML.xml2hash('<root><a>aaa</a></root>', force_array: true)```

    will be converted to:

    ```{ "a" => ["aaa"] }```

    instead of:

    ```{ "a" => "aaa" }```

  * When this option is an array, this allows to specify a list of element
    names which should always be forced into an array representation,
    rather than the 'all or nothing' approach above.

    ```FastXML.xml2hash('<root><a>aaa</a><b>bbb</b></root>', force_array: ['a'])```

    will be converted to:

    ```{ "a" => ["aaa"], "b" => "bbb" }```

    ```FastXML.xml2hash('<root><a>aaa</a><a2>aaa</a2><b>bbb</b></root>', force_array: [/^a/])```

    will be converted to:

    ```{ "a" => ["aaa"], "a2" => ["aaa"], "b" => "bbb" }```

* **:force_content => false # xml2hash**
  * When this options is **true**, this allows you to force text content to always
    convert to a hash value.

    ```FastXML.xml2hash('<root><a>value</a></root>', force_content: true)```

    will be converted to:

    ```{ "a" => { "content" => "value" } }```

    instead of:

    ```{ "a" => "value" }```

* **:merge_text => false # xml2hash**
  * Setting this option to **true** will cause merge adjacent text nodes.

    ```FastXML.xml2hash('<root>value1<!-- comment -->value2</root>', merge_text: true)```

    will be converted to:

    ```"value1value2"```

    instead of:

    ```["value1", "value2"]```

* **:xml_decl => true # hash2xml**
  * if xml_decl is **true**, output will start with the XML declaration
    ```<?xml version="1.0" encoding="utf-8"?>```

  * if xml_decl is **false**, XML declaration will not be output.

* **:trim => false # hash2xml + xml2hash**
  * Trim leading and trailing whitespace from text nodes.

* **:utf8 => true # hash2xml + xml2hash**
  * Turn on utf8 flag for strings if is **true**.

* **:max_depth => 1024 # xml2hash**
  * Maximum recursion depth.

* **:buf_size => 4096 # hash2xml + xml2hash**
  * Buffer size for reading end encoding data.

* **:keep_root => false # xml2hash**
  * Keep root element.

    ```FastXML.xml2hash('<root>value1</root>', keep_root: true)```

    will be converted to:

    ```{ "root" => "value1" }```

    instead of:

    ```"value1"```

* **:filter => nil # xml2hash**
  * Filter nodes matched by pattern and return an array of nodes.

    ```FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b>bbb</b></root>', filter: '/root/a')```

    will be converted to:

    ```[{ "a" => "aaa" }, { "a" => "aaa2" }]```

    ```FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b>bbb</b></root>', filter: /a|b/)```

    will be converted to:

    ```[{ "a" => "aaa" }, { "a" => "aaa2" }, { "b" => "bbb" }]```

    ```FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b>bbb</b></root>', filter: ['/root/a', '/root/b'])```

    will be converted to:

    ```[{ "a" => "aaa" }, { "a" => "aaa2" }, { "b" => "bbb" }]```

  * You can pass a block as parameter.

    ```
    FastXML.xml2hash('<root><a>aaa</a><a>aaa2</a><b>bbb</b></root>', filter: '/root/a') do |node|
      p node
    end
    ```

    will be printed:

    ```
    {"a"=>"aaa"}
    {"a"=>"aaa2"}
    ```

    It may be used to parse large XML because does not require a lot of
    memory.

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
