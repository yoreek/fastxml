require 'mkmf'

extension_name = 'fastxml'
dir_config(extension_name)

puts ">>>>> Creating Makefile for #{RUBY_DESCRIPTION} <<<<<"

OPTS = Hash[
  ARGV.map { |a| a =~ /^--with-(.+)(?:=(.*))?$/ ? [$1.to_sym, $2 || true] : nil }.compact
].freeze

if OPTS[:debug]
  $CPPFLAGS += ' -g -Wall -Werror -Wextra -pedantic -std=c99 -DWITH_DEBUG -O0'
  $CPPFLAGS += ' -DWITH_TRACE' if OPTS[:trace]
end

create_makefile(extension_name)
