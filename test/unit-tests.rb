require "rubygems"
require "test/unit"
require "open-uri"
require "json"
require "memcache"
#TEST_URL = "app1.prvt.nytimes.com"
#TEST_PORT = 9090

if __FILE__ == $0
  $slayer_server = ARGV[0]
  $slayer_port = ARGV[1]
  $memcache_server = ARGV[2]
  $memcache_port = ARGV[3]
end
$slayer_server ||= "localhost"
$slayer_port ||= 9090
$memcache_server = "127.0.0.1"
$memcache_port ||=11211
# {"RESULT" : {"TYPES" : ["MYSQL_TYPE_LONG" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_TINY" , "MYSQL_TYPE_SHORT" , "MYSQL_TYPE_INT24" , "MYSQL_TYPE_LONG" , "MYSQL_TYPE_LONGLONG" , "MYSQL_TYPE_NEWDECIMAL" , "MYSQL_TYPE_DOUBLE" , "MYSQL_TYPE_FLOAT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_VAR_STRING" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_DATE" , "MYSQL_TYPE_TIME" , "MYSQL_TYPE_DATETIME" , "MYSQL_TYPE_TIMESTAMP" , "MYSQL_TYPE_YEAR" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_STRING"] , "HEADER" : ["id" , "row" , "tinyint" , "smallint" , "mediumint" , "integer" , "bigint" , "decimal" , "double" , "float" , "tinytext" , "blob" , "text" , "charstr" , "varcharstr" , "mediumblob" , "longblob" , "longtext" , "date" , "time" , "datetime" , "timestamp" , "year" , "enum" , "set"] , "ROWS" : [[1 , "data" , 2 , 9 , 257 , 32768 , 2.14748e+09 , 400.85 , 500.99 , 3.14159 , "x" , null , "text" , "char" , "varchar" , null , null , "longtext" , "2001-10-11" , "12:30:00" , "2001-10-11 12:30:00" , "2001-10-11 12:30:00" , "2001" , "apple" , "teacher"] , [2 , "null" , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null]]}}
class TestDBSlayerTypes < Test::Unit::TestCase
  def start_slayer(opts={ })
    ifargs = ofargs = preload_args = ""
    path = File.expand_path(File.dirname(__FILE__))
    slayer = path + "/../server/dbslayer"
    conf = path + "/mysql.cnf"
    if opts[:lua_preload]
      preload_args = "-l " + opts[:lua_preload]
    end
    if opts[:input_filter]
      ifargs = "-f " + opts[:input_filter]
    end

    if opts[:mapper]
      margs = "-m " + opts[:mapper]
    end

    if opts[:output_filter]
      ofargs = "-o " +  opts[:output_filter]
    end

    `killall dbslayer 2>/dev/null`
    sleep(1);
    command = "#{slayer} -H #{$memcache_server} -P #{$memcache_port} -d 1 -s #{$slayer_server} -u slayer -p #{$slayer_port} -c #{conf} #{ifargs} #{ofargs} #{preload_args} #{margs}"
    #puts command
    @@slayer_pid = fork do
      exec(command)
    end
    sleep(1);
  end

  def start_memcached
    command = "memcached  -p #{$memcache_port} -l #{$memcache_server} -v"
    #puts command
    @@memcached_pid = fork do
        exec(command)
    end
    sleep(1);
  end

  def stop_memcached
    Process.kill("TERM", @@memcached_pid)
    Process.wait(@@memcached_pid)
    sleep(5)
  end

  def stop_slayer
    Process.kill("TERM", @@slayer_pid)
        Process.wait(@@slayer_pid)
        sleep(5)
  end

  def setup
    path = File.expand_path(File.dirname(__FILE__))
    @file_opts = { :input_filter => path + "/" + "test_input_filter.lua",
                   :output_filter => path + "/" + "test_output_filter.lua",
                   :mapper => path + "/" + "map.lua" }
    [:input_filter, :output_filter].each  do |key|
      File.open(@file_opts[key], "w") do |file|
        file.write("return json")
      end
    end
    @@slayer_pid ||= start_slayer(@file_opts)
  end


  def query_url(sql, options = {})
    query_hash = { "SQL" => sql }.merge(options)
    puts query_hash.to_json if query_hash["DEBUG"]
    url_args = URI.encode(query_hash.to_json)
    url =  "http://#{$slayer_server}:#{$slayer_port}/db?#{url_args}"
    puts url if query_hash["DEBUG"]
    url
  end

  def exec_query(sql, options = {})
    url = query_url(sql, options)
    open(url) do |f|
      yield f
    end
  end

  def assert_column_select(column, row, json_type, mysql_type)
    if row == :data
      sql = "select `#{column}` from TypeTest where row='data'"
    else
      sql = "select `#{column}` from TypeTest where row='null'"
    end
    exec_query(sql) do |f|
      # return the item, the metadata type
      res = f.read
      puts res if ENV['DEBUG']
      h = JSON.parse(res)
      assert_equal h["RESULT"]["HEADER"][0], column

      if json_type == nil
        assert_nil h["RESULT"]["ROWS"][0][0]
      else
        assert_kind_of json_type, h["RESULT"]["ROWS"][0][0]
      end

      # assert mysql type
      assert_equal mysql_type, h["RESULT"]["TYPES"][0]
    end
  end



  def test_connected
  end

#  def test_bit
#    assert_column_select('bit', :data, String, 'MYSQL_TYPE_BIT')
#  end

  def test_bit_null
    assert_column_select('bit', :null, nil, 'MYSQL_TYPE_BIT')
  end

  def test_tinyint
    assert_column_select('tinyint', :data, Integer, 'MYSQL_TYPE_TINY')
  end

  def test_tinyint_null
    assert_column_select('tinyint', :null, nil, 'MYSQL_TYPE_TINY')
  end

  def test_smallint
    assert_column_select('smallint', :data, Integer, 'MYSQL_TYPE_SHORT')
  end

  def test_smallint_null
    assert_column_select('smallint', :null, nil, 'MYSQL_TYPE_SHORT')
  end

  def test_mediumint
    assert_column_select('mediumint', :data, Integer, 'MYSQL_TYPE_INT24')
  end

  def test_mediumint_null
    assert_column_select('mediumint', :null, nil, 'MYSQL_TYPE_INT24')
  end

  def test_integer
    assert_column_select('integer', :data, Integer, 'MYSQL_TYPE_LONG')
  end

  def test_integer_null
    assert_column_select('integer', :null, nil, 'MYSQL_TYPE_LONG')
  end

  def test_bigint
    assert_column_select('bigint', :data, Float, 'MYSQL_TYPE_LONGLONG')
  end

  def test_bigint_null
    assert_column_select('bigint', :null, nil, 'MYSQL_TYPE_LONGLONG')
  end

  def test_decimal
    assert_column_select('decimal', :data, Float, 'MYSQL_TYPE_NEWDECIMAL')
  end

  def test_decimal_null
    assert_column_select('decimal', :null, nil, 'MYSQL_TYPE_NEWDECIMAL')
  end

  def test_float
    assert_column_select('float', :data, Float, 'MYSQL_TYPE_FLOAT')
  end

  def test_float_null
    assert_column_select('float', :null, nil, 'MYSQL_TYPE_FLOAT')
  end

  def test_double
    assert_column_select('double', :data, Float, 'MYSQL_TYPE_DOUBLE')
  end

  def test_double_null
    assert_column_select('double', :null, nil, 'MYSQL_TYPE_DOUBLE')
  end

  def test_tinytext
    assert_column_select('tinytext', :data, String, 'MYSQL_TYPE_BLOB')
  end

  def test_tinytext_null
    assert_column_select('tinytext', :null, nil, 'MYSQL_TYPE_BLOB')
  end

  def test_blob
    assert_column_select('blob', :data, nil, 'MYSQL_TYPE_TEXT')
  end

  def test_blob_null
    assert_column_select('blob', :null, nil, 'MYSQL_TYPE_TEXT')
  end

  def test_char
    assert_column_select('char', :data, String, 'MYSQL_TYPE_STRING')
  end

  def test_char_null
    assert_column_select('char', :null, nil, 'MYSQL_TYPE_STRING')
  end

  def test_varchar
    assert_column_select('varchar', :data, String, 'MYSQL_TYPE_VAR_STRING')
  end

  def test_varchar_null
    assert_column_select('varchar', :null, nil, 'MYSQL_TYPE_VAR_STRING')
  end

  def test_date
    assert_column_select('date', :data, String, 'MYSQL_TYPE_DATE')
  end

  def test_date_null
    assert_column_select('date', :null, nil, 'MYSQL_TYPE_DATE')
  end

  def test_time
    assert_column_select('time', :data, String, 'MYSQL_TYPE_TIME')
  end

  def test_time_null
    assert_column_select('time', :null, nil, 'MYSQL_TYPE_TIME')
  end

  def test_datetime
    assert_column_select('datetime', :data, String, 'MYSQL_TYPE_DATETIME')
  end

  def test_datetime_null
    assert_column_select('datetime', :null, nil, 'MYSQL_TYPE_DATETIME')
  end

  def test_timestamp
    assert_column_select('timestamp', :data, String, 'MYSQL_TYPE_TIMESTAMP')
  end

  def test_timestamp_null
    assert_column_select('timestamp', :null, nil, 'MYSQL_TYPE_TIMESTAMP')
  end

  def test_year
    assert_column_select('year', :data, String, 'MYSQL_TYPE_YEAR')
  end

  def test_year_null
    assert_column_select('year', :null, nil, 'MYSQL_TYPE_YEAR')
  end

 def test_schema
    open "http://#{$slayer_server}:#{$slayer_port}/schema" do  |f|
      h=JSON.parse(f.read)
      assert_equal "MYSQL_TYPE_TEXT", h["SCHEMA"]["TypeTest"]["mediumblob"]
      assert_equal "MYSQL_TYPE_STRING", h["SCHEMA"]["City"]["CountryCode"]
    end
  end

  def test_lua_schema
    File.open(@file_opts[:output_filter], "w") do |file|
      file << "t = Json.Decode( json )\n"
      file << "t.SCHEMA = Json.Decode( schema_json ).SCHEMA\n"
      file << "return Json.Encode(t)\n"
    end
    exec_query("select * from City where CountryCode IS NULL") do |f|
      json = f.read
      h=JSON.parse(json)
      assert_equal "MYSQL_TYPE_TEXT", h["SCHEMA"]["TypeTest"]["mediumblob"]
      assert_equal "MYSQL_TYPE_STRING", h["SCHEMA"]["City"]["CountryCode"]
    end
  end

  def test_map
    require 'fileutils'
    open("http://#{$slayer_server}:#{$slayer_port}/map") do |f|
      output = f.read
      h = JSON.parse(output)
      assert_equal "STRING", h["RESULT"]["MAP"]["Country"]["members"]["Name"]
      assert_equal "many_to_many", h["RESULT"]["MAP"]["CountryLanguage"]["members"]["tenants"]["relation"]
      assert_equal "many_to_one", h["RESULT"]["MAP"]["Apartment"]["members"]["tenants"]["relation"]
      #puts JSON.pretty_generate(h)
    end
  end



  def test_lua
    File.open(@file_opts[:input_filter], "w") do |file|
      file << "t = Json.Decode( json )\n"
      file << "t.SQL = 'ALL YOUR SQL BELONG TO US'\n"
      file << "return Json.Encode(t)\n"
    end
    File.open(@file_opts[:output_filter], "w") do |file|
      file << "t = Json.Decode( json )\n"
      file << "t.OUTPUT_FILTER_MESSAGE = 'HAHA'"
      file << "return Json.Encode(t)\n"
    end
    exec_query("select * from City") do |f|
      foo = f.read
      h = JSON.parse(foo)
      assert_not_nil h["MYSQL_ERROR"]
      assert_not_nil h["MYSQL_ERROR"].match(/ALL YOUR SQL BELONG TO US/)
      assert_equal "HAHA", h["OUTPUT_FILTER_MESSAGE"]
    end
  end

  def test_lua_concurrancy
    failure = nil
    threads = []
    File.open(@file_opts[:output_filter], "w") do |file|
      file << "t = Json.Decode( json )\n"
      file << "t.SCHEMA = Json.Decode( schema_json ).SCHEMA\n"
      file << "return Json.Encode(t)\n"
    end
    10.times do |i|
      threads << Thread.new(i) do |which|
        10.times do |n|
          exec_query("select * from City where CountryCode IS NULL") do |f|
            foo = f.read
            h = JSON.parse(foo)
            unless h
              failure = true
            end
            if h["MYSQL_ERROR"]
              failure = true
            end
            unless "MYSQL_TYPE_TEXT" == h["SCHEMA"]["TypeTest"]["mediumblob"]
              failure = true
            end
            unless  "MYSQL_TYPE_STRING" ==  h["SCHEMA"]["City"]["CountryCode"]
              failure = true
            end
          end
        end
      end
    end
    threads.each{|thr| thr.join }
    assert_nil failure
  end

  def test_cache
     start_memcached
     sql = "select * from City where ID=1"
     cache = MemCache::new "#{$memcache_server}:#{$memcache_port}"
     cache.flush_all
     exec_query( sql, "CACHE" => 1, "CACHE_TTL" => 3000000 ) do |f|
            output = f.read
            r=JSON.parse(output)
            assert r["CACHE_WRITE"]
     end
     exec_query( sql, "CACHE" => 1, "CACHE_TTL" => 3000000 ) do |f|
       output = f.read
       r=JSON.parse(output)
       assert r["CACHED"]
     end
     exec_query( sql ) do |f|
       output = f.read
       r=JSON.parse(output)
       assert_nil r["CACHED"]
     end
     exec_query( sql, "CACHE" => 1) do |f|
       output = f.read
       r=JSON.parse(output)
       assert r["CACHED"]
     end
     stop_memcached
   end
end
