require 'fastxml/version'
require 'fastxml/error'
require 'fastxml/fastxml'

module FastXML
  class << self
    attr_accessor :output,
                  :method,
                  :root,
                  :version,
                  :encoding,
                  :utf8,
                  :indent,
                  :canonical,
                  :use_attr,
                  :content,
                  :xml_decl,
                  :keep_root,
                  :doc,
                  :force_array,
                  :force_content,
                  :merge_text,
                  :attr,
                  :text,
                  :trim,
                  :cdata,
                  :comm,
                  :max_depth,
                  :buf_size
  end

  def self.configure
    yield(self)
  end

  @output        = nil
  @method        = 'NATIVE'
  @root          = 'root'
  @version       = '1.0'
  @encoding      = ''
  @utf8          = true
  @indent        = 0
  @canonical     = false
  @use_attr      = false
  @content       = ''
  @xml_decl      = true
  @keep_root     = false
  @doc           = false
  @force_array   = nil
  @force_content = false
  @merge_text    = false
  @attr          = '-'
  @text          = '#text'
  @trim          = false
  @cdata         = ''
  @comm          = ''
  @max_depth     = 1024
  @buf_size      = 4096
end
