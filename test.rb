require 'rubygems'
require 'active_support'
require 'benchmark'

require 'native_support'

N = 10000

puts "Using #{N} runs"

def test_and_benchmark(as, ns, method, values)
  values.each do |input|
    args = Array(input)
    expected, actual = as.__send__(method, *args), ns.__send__(method, *args)
    ns_desc = "#{ns.inspect}.#{method}(#{args.map { |arg| arg.inspect }.join(', ')})"
    as_desc = "#{as.inspect}.#{method}(#{args.map { |arg| arg.inspect }.join(', ')})"
    abort("Expected #{expected.inspect}, got #{actual.inspect} from #{ns_desc}") unless expected == actual

    Benchmark.bm(64) do |x|
      x.report(ns_desc) do
        N.times { ns.send(method, *args) }
      end
      x.report(as_desc) do
        N.times { as.send(method, *args) }
      end
    end
  end
end

test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :demodulize,
                   %w(Class Namespaced::Class ::TopNamespacedClass Bogus:::Colons One:Colon Trailing::Colons::))
test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :ordinalize,
                   [1, 2, 3, 4, 10, 11, 12, 13, 14, 21, 22, 23, 24])
test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :dasherize,
                   %w(hello_world hello_there_world hello-world HelloWorld hello\ world))
test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :underscore,
                   %w(HelloWorld HElloWorld hello_world Hello_world Helloworld
                      Hello\ World Hello2World Hello2world Hello2\ World
                      Hello\ 2World Hello2\ world Hello\ 2world Hello+World
                      Hello+world))
test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :camelize,
                   %w(hello_world hello__world hello___world HelloWorld helloWorld
                      hello-world hello\ world))
