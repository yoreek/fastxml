require "bundler/setup"
require "fastxml"

XML_DECL = %{<?xml version="1.0" encoding="utf-8"?>}.freeze

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = ".rspec_status"

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end
end
