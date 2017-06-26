require_relative '../spec_helper'

XML_DECL = %{<?xml version="1.0" encoding="utf-8"?>}

RSpec.describe FastXML do
  describe '.xml2hash' do
    context 'when "keep_root" option is' do
      context 'true' do
        it 'keeps the root node' do
          expect(FastXML.xml2hash('<root>value</root>', keep_root: true))
            .to eq('root' => 'value')
        end
      end

      context 'false' do
        it 'don`t keeps the root node' do
          expect(FastXML.xml2hash('<root>value</root>'))
            .to eq('value')
        end
      end
    end

    context 'when "force_content" option is' do
      context 'false' do
        it 'don`t creates a content element' do
          expect(FastXML.xml2hash('<root>value</root>'))
            .to eq('value')
        end
      end

      context 'true' do
        it 'creates a content element' do
          expect(FastXML.xml2hash('<root>value</root>', force_content: true))
            .to eq('content' => 'value')
        end
      end
    end

    context 'when "force_array" option is' do
      context 'nil' do
        it 'don`t puts the node value to array' do
          expect(FastXML.xml2hash('<root><a>value</a><b>value2</b></root>'))
            .to eq('a' => 'value', 'b' => 'value2')
        end
      end

      context 'true' do
        it 'puts the node value to array' do
          expect(FastXML.xml2hash('<root><a>value</a><b>value2</b></root>', force_array: true))
            .to eq('a' => ['value'], 'b' => ['value2'])
        end
      end

      context 'array of names' do
        it 'puts the node value to array of matched nodes' do
          expect(FastXML.xml2hash('<root><a>value</a><b>value2</b></root>', force_array: ['a']))
            .to eq('a' => ['value'], 'b' => 'value2')
        end
      end

      context 'array of patterns' do
        it 'puts the node value to array of matched nodes' do
          expect(FastXML.xml2hash('<root><a>value</a><b>value2</b></root>', force_array: [/a|b/]))
            .to eq('a' => ['value'], 'b' => ['value2'])
        end
      end
    end

    context 'when "merge_text" option is' do
      context 'false' do
        it 'don`t merges text nodes' do
          expect(FastXML.xml2hash('<root>value1<!-- comment -->value2</root>'))
            .to eq(['value1', 'value2'])
        end
      end

      context 'true' do
        it 'merges text nodes' do
          expect(FastXML.xml2hash('<root>value1<!-- comment -->value2</root>', merge_text: true))
            .to eq('value1value2')
        end
      end
    end

    context 'when "trim" option is' do
      context 'false' do
        it 'don`t trims text' do
          expect(FastXML.xml2hash('<root attr=" attr_value "> value1 </root>'))
            .to eq('attr' => ' attr_value ', 'content' => ' value1 ')
        end
      end

      context 'true' do
        it 'trims text' do
          expect(FastXML.xml2hash('<root attr=" attr_value "> value1 </root>', trim: true))
            .to eq('attr' => ' attr_value ', 'content' => 'value1')
        end
      end
    end

    context 'when "filter" option is' do
      context 'string' do
        it 'returns an array of filtered nodes' do
          expect(FastXML.xml2hash('<root><a>111</a><a>222</a></root>', filter: '/root/a'))
            .to eq([{ 'a' => '111' }, { 'a' => '222' }])
        end
      end

      context 'regexp' do
        it 'returns an array of filtered nodes' do
          expect(FastXML.xml2hash('<root><a>111</a><a>222</a></root>', filter: /a/))
            .to eq([{ 'a' => '111' }, { 'a' => '222' }])
        end
      end

      context 'array of patterns' do
        it 'returns an array of filtered nodes' do
          expect(FastXML.xml2hash('<root><a>111</a><b>222</b></root>', filter: ['/root/a', '/root/b']))
            .to eq([{ 'a' => '111' }, { 'b' => '222' }])
        end
      end

      context 'string and block is given' do
        let(:block) { lambda { |node| p node } }
        it 'pass nodes to block' do
          nodes = []
          block = -> (node){ nodes << node }
          expect(FastXML.xml2hash('<root><a>111</a><a>222</a></root>', filter: '/root/a', &block))
            .to eq(nil)
          expect(nodes).to eq([{ 'a' => '111' }, { 'a' => '222' }])
        end
      end
    end

    context 'when mixed content' do
      context 'when "content" option is' do
        context 'nil' do
          it 'creates a content element with default name' do
            expect(FastXML.xml2hash('<root attr="attr_value"><node>node value</node>content</root>'))
              .to eq('attr' => 'attr_value', 'node' => 'node value', 'content' => 'content')
          end
        end

        context 'specified' do
          it 'creates a content element with specified name' do
            expect(FastXML.xml2hash('<root attr="attr_value"><node>node value</node>content</root>', content: 'text'))
              .to eq('attr' => 'attr_value', 'node' => 'node value', 'text' => 'content')
          end
        end
      end
    end

    context 'when node with attributes' do
      it 'create an element with an empty string' do
        expect(FastXML.xml2hash('<root attr1="1" attr2="2"/>'))
          .to eq('attr1' => '1', 'attr2' => '2')
      end
    end

    context 'when node is empty' do
      it 'create an element with an empty string' do
        expect(FastXML.xml2hash('<root><empty_node/></root>'))
          .to eq('empty_node' => '')
      end
    end

    context 'when several nodes with the same name' do
      it 'puts node to array' do
        expect(FastXML.xml2hash(<<-XML))
          <root>
            <item>1</item>
            <item>2</item>
            <item>3</item>
          </root>
          XML
          .to eq('item' => ['1', '2', '3'])
      end
    end

    context 'when CDATA section contains special symbols' do
      it 'correct parsing' do
        expect(FastXML.xml2hash('<root><![CDATA[abcde!@#$%^&*<>]]></root>'))
          .to eq('abcde!@#$%^&*<>')
      end
    end

    context 'when parsing ending delimiter in CDATA section' do
      it 'correct parsing' do
        expect(FastXML.xml2hash('<root><![CDATA[ [ abc ] ]> ]]]]]]></root>'))
          .to eq(' [ abc ] ]> ]]]]')
      end
    end

    context 'when text contains line feeds' do
      it 'normalizes line feeds' do
        expect(FastXML.xml2hash("<a>\x0D\x0Aasd\x0D\x0Aasd\x0D\x0D\x0Aasd\x0D\x0A</a>", trim: true))
          .to eq("asd\x0Aasd\x0A\x0Aasd")
      end
    end

    context 'when attribute contains escaped characters' do
      it 'unescapes characters' do
        expect(FastXML.xml2hash(<<-XML, trim: true))
          <root text="amp:&amp;,apos:&apos;,quot:&quot;,lt:&lt;,gt:&gt;,chr(48):&#48;,chr(0x31):&#x31;,cr:&#x0d;,lf:&#x0a;"/>
          XML
          .to eq('text' => "amp:&,apos:',quot:\",lt:<,gt:>,chr(48):0,chr(0x31):1,cr:\r,lf:\n")
      end
    end

    context 'when many pieces of content in the one node' do
      it 'don`t merge pieces' do
        expect(FastXML.xml2hash('<root>bbb<!-- ccc -->ddd<eee>fff</eee>ggg</root>', trim: true))
          .to eq('content' => ['bbb', 'ddd', 'ggg'], 'eee' => 'fff')
      end
    end

    context 'when more then one root node' do
      it 'raises an error' do
        expect { FastXML.xml2hash('<root></root><root2></root2>') }.to raise_error(FastXML::ParseError, 'Invalid XML')
      end
    end

    context 'when the tag is not closed' do
      it 'raises an error' do
        expect { FastXML.xml2hash('<root></root><root2>') }.to raise_error(FastXML::ParseError, 'Invalid XML')
      end
    end

    context 'when there is no opening tag' do
      it 'raises an error' do
        expect { FastXML.xml2hash('</root>') }.to raise_error(FastXML::ParseError, 'Invalid XML')
      end
    end

    context 'when there is content after a closed root node' do
      it 'raises an error' do
        expect { FastXML.xml2hash('<root></root>text') }.to raise_error(FastXML::ParseError, 'Invalid XML')
      end
    end

    context 'when max depth exceeded' do
      it 'raises an error' do
        expect { FastXML.xml2hash('<root><x/></root>', max_depth: 2) }
          .to raise_error(FastXML::ParseError, 'Maximum depth exceeded')
      end
    end
  end
end
