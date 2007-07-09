require "rubygems"
require "test/unit"
require "open-uri"
require "json"

#TEST_URL = "app1.prvt.nytimes.com"
#TEST_PORT = 9090

if __FILE__ == $0
  raise "Requires dbslayer daemon and port" unless ARGV.size == 2
  $slayer_server = ARGV[0]
  $slayer_port = ARGV[1]
end

# {"RESULT" : {"TYPES" : ["MYSQL_TYPE_LONG" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_TINY" , "MYSQL_TYPE_SHORT" , "MYSQL_TYPE_INT24" , "MYSQL_TYPE_LONG" , "MYSQL_TYPE_LONGLONG" , "MYSQL_TYPE_NEWDECIMAL" , "MYSQL_TYPE_DOUBLE" , "MYSQL_TYPE_FLOAT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_VAR_STRING" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_TEXT" , "MYSQL_TYPE_BLOB" , "MYSQL_TYPE_DATE" , "MYSQL_TYPE_TIME" , "MYSQL_TYPE_DATETIME" , "MYSQL_TYPE_TIMESTAMP" , "MYSQL_TYPE_YEAR" , "MYSQL_TYPE_STRING" , "MYSQL_TYPE_STRING"] , "HEADER" : ["id" , "row" , "tinyint" , "smallint" , "mediumint" , "integer" , "bigint" , "decimal" , "double" , "float" , "tinytext" , "blob" , "text" , "charstr" , "varcharstr" , "mediumblob" , "longblob" , "longtext" , "date" , "time" , "datetime" , "timestamp" , "year" , "enum" , "set"] , "ROWS" : [[1 , "data" , 2 , 9 , 257 , 32768 , 2.14748e+09 , 400.85 , 500.99 , 3.14159 , "x" , null , "text" , "char" , "varchar" , null , null , "longtext" , "2001-10-11" , "12:30:00" , "2001-10-11 12:30:00" , "2001-10-11 12:30:00" , "2001" , "apple" , "teacher"] , [2 , "null" , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null , null]]}}
class TestDBSlayerTypes < Test::Unit::TestCase
  def query_url(sql)
    query_hash = { "SQL" => sql }
    url_args = URI.encode(query_hash.to_json)
    "http://#{$slayer_server}:#{$slayer_port}/db?#{url_args}"
  end
  
  def exec_query(sql)
    url = query_url(sql)
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
      h = JSON.parse(f.read)
      
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

  def test_bit
    assert_column_select('bit', :data, String, 'MYSQL_TYPE_BIT')
  end
  
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
    assert_column_select('blob', :data, String, 'MYSQL_TYPE_BLOB')
  end
  
  def test_blob_null
    assert_column_select('blob', :null, nil, 'MYSQL_TYPE_BLOB')
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
end

