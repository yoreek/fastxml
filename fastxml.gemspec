# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'fastxml/version'

Gem::Specification.new do |spec|
  spec.name          = 'fastxml'
  spec.version       = FastXML::VERSION
  spec.authors       = ['Yuriy Ustushenko']
  spec.email         = ['yoreek@yahoo.com']

  spec.summary       = 'Fast Ruby Hash to XML and XML to Ruby Hash converter'
  spec.description   = 'Fast Ruby Hash to XML and XML to Ruby Hash converter written in pure C'
  spec.homepage      = 'https://github.com/yoreek/fastxml'
  spec.license       = 'MIT'

  if spec.respond_to?(:metadata)
    spec.metadata['allowed_push_host'] = 'https://rubygems.org'
  else
    raise 'RubyGems 2.0 or newer is required to protect against ' \
      'public gem pushes.'
  end

  spec.files         = Dir['{lib,ext}/**/*.{rb,h,c}'] + %w(LICENSE README.md)

  spec.extensions    = %w(ext/fastxml/extconf.rb)

  spec.require_paths = %w(lib ext)

  spec.required_ruby_version = ">= 2.2.3"

  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'bundler', '~> 1.14'
  spec.add_development_dependency 'rake',    '~> 10.0'
  spec.add_development_dependency 'rspec',   '~> 3.0'
end
