require 'rubygems'
require 'active_support'
require 'benchmark'

require 'native_support'

N = 10000

puts "Using #{N} runs"

module Namespaced
  module Inner
    module Const
    end
  end
end

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

native_float = 1.23456
native_float.extend(NativeSupport::CoreExtensions::Float::Rounding)
test_and_benchmark(1.23456, native_float, :round_with_precision, [0, 1, 2, 3, 4, 5, 6])


test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :constantize,
                   %w(Namespaced Namespaced::Inner Namespaced::Inner::Const ::Namespaced)) # meta testing

test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :demodulize,
                   %w(Class Namespaced::Class ::TopNamespacedClass Bogus:::Colons One:Colon Trailing::Colons::))

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

test_and_benchmark(ActiveSupport::Inflector, NativeSupport::Inflector, :camelize,
                   [['hello_world', false], ['hello__world', false],
                    ['hello___world', false], ['HelloWorld', false],
                    ['helloWorld', false], ['hello-world', false],
                    ['hello\ world', false]])
