require 'ffi'

module LibcageModule
  extend FFI::Library

  begin
    ffi_lib [File.dirname(__FILE__)+'/libcage-api.so']
  rescue => LoadError
    puts "#{LoadError.backtrace.join("\n")}"
    raise 'Failed to load Libcage Library!'
  end

  attach_function :libcage_open,          [:int],                     :pointer
  attach_function :libcage_print_state,   [:pointer],                 :void
  attach_function :libcage_join,          [:pointer, :string, :int],  :bool
  attach_function :libcage_dispatch,      [],                         :void
end
