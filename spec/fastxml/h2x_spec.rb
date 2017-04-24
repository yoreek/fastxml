require_relative '../spec_helper'

XML_DECL = %{<?xml version="1.0" encoding="utf-8"?>}

RSpec.describe FastXML do
  def conv_to_io(hash)
    fh = StringIO.new
    FastXML.hash2xml(hash, output: fh)
    fh.string
  end

  it 'has a version number' do
    expect(FastXML::VERSION).not_to be nil
  end

  describe '#configure' do
    %i[output method root version encoding utf8 indent canonical use_attr
       content xml_decl keep_root doc buf_size force_array force_content
       merge_text attr text trim cdata comm max_depth].each do |attribute|
      it { should respond_to attribute }
    end
  end

  describe '#hash2xml' do
    it 'with default options' do
      expect(FastXML.hash2xml({ node1: [ 'value1', { node2: 'value2' } ] }))
        .to eq(%{#{XML_DECL}\n<root><node1>value1</node1><node1><node2>value2</node2></node1></root>})
    end

    it 'when canonical = true' do
      expect(FastXML.hash2xml({ node3: 'value3', node1: 'value1', node2: 'value2' }, canonical: true))
        .to eq(%{#{XML_DECL}\n<root><node1>value1</node1><node2>value2</node2><node3>value3</node3></root>})
    end

    it 'when indent = 2' do
      expect(FastXML.hash2xml({ node1: [ 'value1', { node2: 'value2' } ] }, indent: 2))
        .to eq(<<-EOT)
#{XML_DECL}
<root>
  <node1>value1</node1>
  <node1>
    <node2>value2</node2>
  </node1>
</root>
EOT
    end

    it 'with string, float and integer' do
      expect(FastXML.hash2xml({ node1: [ 'string', 1.22, 1 ] }))
        .to eq(%{#{XML_DECL}\n<root><node1>string</node1><node1>1.22</node1><node1>1</node1></root>})
    end

    it 'when need to quote tag name' do
      expect(FastXML.hash2xml({ 1 => 'value1' }))
        .to eq(%{#{XML_DECL}\n<root><_1>value1</_1></root>})
    end

    it 'with lambda that returns a string' do
      expect(FastXML.hash2xml({ node1: -> { 'value1' } }))
        .to eq(%{#{XML_DECL}\n<root><node1>value1</node1></root>})
    end

    it 'with lambda that returns an array' do
      expect(FastXML.hash2xml({ node1: -> { [ 'value1' ] } }))
        .to eq(%{#{XML_DECL}\n<root><node1>value1</node1></root>})
    end

    it 'when need to escape' do
      expect(FastXML.hash2xml({ node1: "< > & \r"}))
        .to eq(%{#{XML_DECL}\n<root><node1>&lt; &gt; &amp; &#13;</node1></root>})
    end

    it 'when trim = false' do
      expect(FastXML.hash2xml({ node: " \t\ntest "  }, trim: false))
        .to eq(%{#{XML_DECL}\n<root><node> \t\ntest </node></root>})
    end

    it 'when trim = true' do
      expect(FastXML.hash2xml({ node: " \t\ntest "  }, trim: true))
        .to eq(%{#{XML_DECL}\n<root><node>test</node></root>})
    end

    it 'when use_attr = true' do
      expect(FastXML.hash2xml(
        {
          node1: 'value1"',
          node2: 'value2&',
          node3: { node31: 'value31' },
          node4: [ { node41: 'value41' }, { node42: 'value42' } ],
          node5: [ 51, 52, { node53: 'value53' } ],
          node6: {},
          node7: []
        },
        use_attr:  true,
        canonical: true,
        indent:    2
      )).to eq(<<-EOT)
#{XML_DECL}
<root node1="value1&quot;" node2="value2&amp;">
  <node3 node31="value31"/>
  <node4 node41="value41"/>
  <node4 node42="value42"/>
  <node5>51</node5>
  <node5>52</node5>
  <node5 node53="value53"/>
  <node6/>
</root>
EOT
    end

    it 'with content' do
      expect(FastXML.hash2xml(
        {
          content: 'content&1',
          node2: [ 21, {
              node22: "value22 < > & \" \t \n \r",
              content: "content < > & \r"
          } ],
        },
        use_attr:  true,
        canonical: true,
        indent:    2,
        content:   'content'
      )).to eq(<<-EOT)
#{XML_DECL}
<root>
  content&amp;1
  <node2>21</node2>
  <node2 node22="value22 &lt; &gt; &amp; &quot; &#9; &#10; &#13;">
    content &lt; &gt; &amp; &#13;
  </node2>
</root>
EOT
    end

    it 'with using enumerate' do
      expect(FastXML.hash2xml({ enum: 3.times }, indent: 2)).to eq(<<-EOT)
#{XML_DECL}
<root>
  <enum>0</enum>
  <enum>1</enum>
  <enum>2</enum>
</root>
EOT
    end

    it 'with cyrillic characters' do
      expect(FastXML.hash2xml({ ключ: 'значение' }))
        .to eq(%{#{XML_DECL}\n<root><ключ>значение</ключ></root>})
    end

    it 'with filehandle output' do
      expect(conv_to_io({ node1: 'value1' }))
        .to eq(%{#{XML_DECL}\n<root><node1>value1</node1></root>})
    end
  end
end
