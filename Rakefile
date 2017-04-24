require 'bundler/gem_tasks'
require 'rspec/core/rake_task'
require 'rake/extensiontask'

RSpec::Core::RakeTask.new(:spec)

Rake::ExtensionTask.new 'fastxml' do |ext|
  ext.lib_dir = 'lib/fastxml'
end

task default: %i[compile spec]
