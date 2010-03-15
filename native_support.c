#include "ruby.h"

VALUE NativeSupport;
VALUE NativeSupport__Inflector;
VALUE NativeSupport__CoreExtensions;
VALUE NativeSupport__CoreExtensions__Float;
VALUE NativeSupport__CoreExtensions__Float__Rounding;

void Init_native_support();

VALUE method_camelize(int argc, VALUE * argv, VALUE self);
VALUE method_underscore(VALUE self, VALUE str);
VALUE method_dasherize(VALUE self, VALUE str);
VALUE method_demodulize(VALUE self, VALUE str);
VALUE method_constantize(VALUE self, VALUE str);
VALUE method_round_with_precision(int argc, VALUE * argv, VALUE self);

void Init_native_support() {
  NativeSupport = rb_define_module("NativeSupport");
  NativeSupport__Inflector = rb_define_module_under(NativeSupport, "Inflector");
	NativeSupport__CoreExtensions = rb_define_module_under(NativeSupport, "CoreExtensions");
	NativeSupport__CoreExtensions__Float = rb_define_module_under(NativeSupport__CoreExtensions, "Float");
	NativeSupport__CoreExtensions__Float__Rounding =
		rb_define_module_under(NativeSupport__CoreExtensions__Float, "Rounding");

  rb_define_singleton_method(NativeSupport__Inflector, "camelize", method_camelize, -1);
  rb_define_singleton_method(NativeSupport__Inflector, "underscore", method_underscore, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "dasherize", method_dasherize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "demodulize", method_demodulize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "constantize", method_constantize, 1);
	rb_define_method(NativeSupport__CoreExtensions__Float__Rounding,
									 "round_with_precision", method_round_with_precision, -1);
}

VALUE method_camelize(int argc, VALUE * argv, VALUE self) {
	VALUE str = argv[0];
  char * orig_str = RSTRING(str)->ptr;
  int orig_str_len = RSTRING(str)->len;
  VALUE camelized_ruby_str = rb_str_dup(str);
	rb_str_modify(camelized_ruby_str);
  char * camelized_str = RSTRING(camelized_ruby_str)->ptr;
  int o; // position in original string
  int c = 0; // position in camelized string
  int state = 1; // 0: Next is unchanged
             // 1: Next is capitalized
             // 2+: Next is lower case

	if (argc == 2 && !argv[1]) {
		state = 2;
	}

  for(o = 0; o < orig_str_len; o++) {
    if (orig_str[o] >= 'a' && orig_str[o] <= 'z') {
      if (state == 1) {
        camelized_str[c++] = orig_str[o] - 32;
      } else {
				if (c != o) {
					camelized_str[c++] = orig_str[o];
				} else {
					++c;
				}
      }
      state = 0;
		} else if (orig_str[o] >= 'A' && orig_str[o] <= 'Z' && state > 1) {
			camelized_str[c++] = orig_str[o] + 32;
    } else if (orig_str[o] == '_' && !state) {
      state = 1;
    } else {
			if (c != o) {
				camelized_str[c++] = orig_str[o];
			} else {
				++c;
			}
      state = 0;
    }
  }

	RSTRING(camelized_ruby_str)->len = c;
  return camelized_ruby_str;
}

VALUE method_underscore(VALUE self, VALUE str) {
  char * orig_str = RSTRING(str)->ptr;
  int orig_str_len = RSTRING(str)->len;
  VALUE underscore_ruby_str = rb_str_dup(str);
	rb_str_modify(underscore_ruby_str);
  char * underscore_str = RSTRING(underscore_ruby_str)->ptr;
  int o = 0, u = 0;
  short state = 0;

  for (o = 0; o < orig_str_len; o++) {
    if (orig_str[o] >= 'A' && orig_str[o] <= 'Z') {
      if (state) {
        underscore_str[u++] = '_';
      }
      underscore_str[u++] = orig_str[o] + 32;
      state = 1;
    } else {
			if (u != o) {
				underscore_str[u++] = orig_str[o];
			} else {
				++u;
			}
      state = (orig_str[o] >= 'a' && orig_str[o] <= 'z' || orig_str[o] >= '0' && orig_str[o] <= '9');
    }
  }

  RSTRING(underscore_ruby_str)->len = u;
  return underscore_ruby_str;
}

VALUE method_dasherize(VALUE self, VALUE str) {
	char * orig_str = RSTRING(str)->ptr;
	int orig_str_len = RSTRING(str)->len;
	VALUE dasherized_ruby_str = rb_str_dup(str);
	rb_str_modify(dasherized_ruby_str);
	char * dasherized_str = RSTRING(dasherized_ruby_str)->ptr;
	int i;

	for (i = 0; i < orig_str_len; i++) {
		if (orig_str[i] == '_') {
			dasherized_str[i] = '-';
		}
	}

	return dasherized_ruby_str;
}

VALUE method_demodulize(VALUE self, VALUE str) {
	char * orig_str = RSTRING(str)->ptr;
	int orig_str_len = RSTRING(str)->len;
	int demodulized_start = 0;
	VALUE demodulized_ruby_str;
	int o, c;

	for (o = 0; o < orig_str_len; o++) {
		if (orig_str[o] == ':') {
			for(c = 0; o+1 < orig_str_len && orig_str[o+1] == ':'; o++, c++);
			if (c) {
				demodulized_start = o + 1;
			}
		}
	}

	demodulized_ruby_str = rb_str_new(orig_str + demodulized_start, orig_str_len - demodulized_start);
	return demodulized_ruby_str;
}

VALUE method_constantize(VALUE self, VALUE str) {
	char * full_const_name = RSTRING(str)->ptr;
	int full_const_name_length = RSTRING(str)->len;
	char * current_const_name = ALLOC_N(char, full_const_name_length + 1);
	VALUE namespace = rb_cObject;
	int f = 0, c = 0;

	do {
		if (f == full_const_name_length || full_const_name[f] == ':' && full_const_name[f+1] == ':') {
			if (c) {
				current_const_name[c] = '\0';
				namespace = rb_const_get(namespace, rb_intern(current_const_name));
			}
			c = 0; f++;
		} else {
			current_const_name[c++] = full_const_name[f];
		}
	} while (f++ <= full_const_name_length);

	free(current_const_name);
	return namespace;
}

VALUE method_round_with_precision(int argc, VALUE * argv, VALUE self) {
	VALUE precision;
	int cprecision;
	double value;
	if (rb_scan_args(argc, argv, "01", &precision) == 0) {
		return rb_funcall(self, rb_intern("round_without_precision"), 0);
	} else {
		value = NUM2DBL(self);
		cprecision = NUM2INT(precision);
		return rb_float_new((float) round(value * (pow(10.0, cprecision))) / pow(10.0, cprecision));
	}
}
