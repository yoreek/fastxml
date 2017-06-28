#!/usr/bin/env ruby

$LOAD_PATH.push File.expand_path('../../lib', __FILE__)

require 'benchmark'
require 'fastxml'
begin
  require 'xmlhasher'
rescue
  puts "xmlhasher gem in not installed, run 'gem install xmlhasher'"
end
begin
  require 'nori'
rescue
  puts "nori gem in not installed, run 'gem install nori'"
end
begin
  require 'active_support/core_ext/hash/conversions'
rescue
  puts "active_support gem in not installed, run 'gem install activesupport'"
end
begin
  require 'xmlsimple'
rescue
  puts "xmlsimple gem in not installed, run 'gem install xml-simple'"
end
begin
  require 'nokogiri'
rescue
  puts "nokogiri gem in not installed, run 'gem install nokogiri'"
end
begin
  require 'libxml'
rescue
  puts "libxml gem in not installed, run 'gem install libxml-ruby'"
end

xml = File.read(File.expand_path('../../spec/fixtures/gistfile.xml', __FILE__))
hash = XmlSimple.xml_in(xml)

puts 'Converting Hash to XML:'
runs = 100
Benchmark.bm 20 do |x|
  ActiveSupport::XmlMini.backend = ActiveSupport::XmlMini_REXML
  x.report 'activesupport(rexml)   ' do
    runs.times { hash.to_xml() }
  end

  ActiveSupport::XmlMini.backend = 'LibXML'
  x.report 'activesupport(libxml)  ' do
    runs.times { hash.to_xml() }
  end

  ActiveSupport::XmlMini.backend = 'Nokogiri'
  x.report 'activesupport(nokogiri)' do
    runs.times { hash.to_xml() }
  end

  x.report 'xmlsimple              ' do
    runs.times { XmlSimple.xml_out(hash) }
  end

  x.report 'fastxml                ' do
    runs.times { FastXML.hash2xml(hash) }
  end
end

puts 'Converting XML to Hash:'
runs = 100
Benchmark.bm 10 do |x|
  ActiveSupport::XmlMini.backend = ActiveSupport::XmlMini_REXML
  x.report 'activesupport(rexml)   ' do
    runs.times { Hash.from_xml(xml) }
  end

  ActiveSupport::XmlMini.backend = 'LibXML'
  x.report 'activesupport(libxml)  ' do
    runs.times { Hash.from_xml(xml) }
  end

  ActiveSupport::XmlMini.backend = 'Nokogiri'
  x.report 'activesupport(nokogiri)' do
    runs.times { Hash.from_xml(xml) }
  end

  x.report 'xmlsimple              ' do
    runs.times { XmlSimple.xml_in(xml) }
  end

  x.report 'nori                   ' do
    runs.times { Nori.new(:advanced_typecasting => false).parse(xml) }
  end

  x.report 'xmlhasher              ' do
    runs.times { XmlHasher.parse(xml) }
  end

  x.report 'fastxml                ' do
    runs.times { FastXML.xml2hash(xml) }
  end
end
