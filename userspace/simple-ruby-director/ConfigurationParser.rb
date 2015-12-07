#!/usr/bin/ruby -w

# require 'logger' # for testing
require 'pathname'

################# CLASS CONFIGURATIONPARSER #################
#
# Class to parse Clondike config file.
# 
# Save the configuration directives to a hash table
#  and return a required value of the directive
#
# Author: Zdenek Novy (novyzde3@fit.cvut.cz)
#
################# CLASS CONFIGURATIONPARSER #################
#
class ConfigurationParser
################## Constants ###################
   REGEX_CHAR               = '[A-Za-z0-9_-]'
   REGEX_DIRECTIVE          = '\s*(' + REGEX_CHAR + '+)\s*'
   REGEX_COMMENT            = '\s*#.*$'
   REGEX_LINEOFWHITESPACES  = '^\s*$'
   REGEX_SPACEANDTAB        = '[ \t]'
   DEBUG                    = false
   #################################################

   ################ Class variables ###############
   # Config file where is stored the configuration
	@@configFile = nil

	# Hash key => value table
	#   with directives and their values
   @@ya_directives = nil
   #################################################


   ############ Methods ##############
   # Display error message and exit the program
   # @param y_msg string
   def error( y_msg, i_err=-1 )
       $log.error( "[ERROR]\t(ConfigurationParser.rb): #{y_msg}!" )
       exit( i_err )
   end
 
   # Create an object ConfigurationParser and load
   #  config file to hash table
   # @param string   name of config file to load
   def initialize( configFile )
      @@ya_directives = Hash.new

      # Check file existence
      config = Pathname.new(configFile)
      if not config.exist?
 	      error("config file #{configFile} does not exist", 1)
      end
      
      @@configFile=configFile
      if ( ! loadConfigFile() )
         error("Log file #{@@configFile} can not be loaded", 1)
      end
   end

   # Check config file and parse them into has table
   def loadConfigFile( )
      if @@configFile == nil
         error("config file #{@@configFile} is not inicialized")
      end

      ### Reading file in a cycle
      File.foreach( @@configFile ).with_index do |y_line, i_line|
         i_line+=1

         # Remove comments ('whitespaces # anything')
         y_line=y_line.sub(/#{REGEX_COMMENT}/, '')

         # Remove newline symbol from the line
         y_line=y_line.chomp

			# Skip line with whitespaces only
         if y_line =~ /#{REGEX_LINEOFWHITESPACES}/
            next
         end

         ### Check syntax if wrong - exit ###
         # value in quotation marks ( "value" )
         if y_line.include? '"'
            if y_line =~ /^#{REGEX_DIRECTIVE}=#{REGEX_SPACEANDTAB}*".+"#{REGEX_SPACEANDTAB}*$/
               ya_line=y_line.split('=')
               y_value=ya_line[1].gsub(/^#{REGEX_SPACEANDTAB}*"(.+)"#{REGEX_SPACEANDTAB}*$/, '\\1')
            else
               error("config file '" + @@configFile + "' has bad syntax on line "+i_line.to_s, 10)
            end

         # value in apostrophes ( 'value' )
		   elsif y_line.include? "'"
            if y_line =~ /^#{REGEX_DIRECTIVE}=#{REGEX_SPACEANDTAB}*'.+'#{REGEX_SPACEANDTAB}*$/
               ya_line=y_line.split('=')
               y_value=ya_line[1].gsub(/^#{REGEX_SPACEANDTAB}*'(.+)'#{REGEX_SPACEANDTAB}*$/, '\\1')
            else
               error("Config file '" + @@configFile + "' has bad syntax on line "+i_line.to_s, 11)
            end

         # value is without special characters ( value )
         else
            if y_line =~ /^#{REGEX_DIRECTIVE}=#{REGEX_SPACEANDTAB}*#{REGEX_CHAR}+#{REGEX_SPACEANDTAB}*$/
               ya_line=y_line.split('=')
               y_value=ya_line[1].gsub(/^#{REGEX_SPACEANDTAB}*(#{REGEX_CHAR}+)#{REGEX_SPACEANDTAB}*$/, '\\1')
            else
               error("Config file '" + @@configFile + "' has bad syntax on line "+i_line.to_s, 12)
            end
         end

         ### Parse the directive
         y_directive=ya_line[0].gsub(/^#{REGEX_DIRECTIVE}$/, '\\1')

         ### Set value to hash table
         @@ya_directives[y_directive] = y_value
      end
      return true
   end

   ### Get the value of wanted directive
   # @param    string   name of the wanted directive
   # @return   string   value of the directive or null if not exists
   def getValue(directive, defaultValue = nil)
      if @@ya_directives.empty?
         error("Error: ya_directives si null", 21)
      end
      if @@ya_directives[directive].nil?
          $log.warn( "[WARNING]\tReturning directive '#{directive}' with default value '#{defaultValue}'" )
      else
          $log.debug( "[OK]\tReturning directive '#{directive}' with value '" + @@ya_directives[directive] + "'" )
      end
      return @@ya_directives[directive] if not @@ya_directives[directive].nil?
      return defaultValue
   end

   # Method to print actual Hash table
   #  with actually loaded directives and their value
   def printDirectives()
      y_directives=""
      @@ya_directives.each do |key, value|
          y_directives += key.to_s + ":  '" + value + "'\n"
      end
      $log.info "Directives of #{@@configFile}:\n#{y_directives}"
   end

end
################# END OF CLASS CONFIGURATION #################


### TESTING THE CLASS ###
# require 'logger'
# $log = Logger.new(STDOUT)
# $log.level = Logger::DEBUG;
# $log.datetime_format = "%Y-%m-%d %H:%M:%S"
# 
# configuration = ConfigurationParser.new("clondike.conf")
# configuration.printDirectives()
# 
# # Testing bootstrapList
# bootstrapList = configuration.getValue("bootstrap", ['192.168.22.136', '192.168.22.137'])
# puts "Prvni slozka: "+bootstrapList[0]
# puts "Druha slozka: "+bootstrapList[1]
# puts "Cely seznam: #{bootstrapList}"

