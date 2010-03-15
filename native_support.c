#include "ruby.h"

VALUE NativeSupport;
VALUE NativeSupport__Inflector;
VALUE NativeSupport__CoreExtensions;
VALUE NativeSupport__CoreExtensions__Float;
VALUE NativeSupport__CoreExtensions__Float__Rounding;

void Init_native_support();

VALUE method_camelize(VALUE self, VALUE str);
VALUE method_underscore(VALUE self, VALUE str);
VALUE method_dasherize(VALUE self, VALUE str);
VALUE method_ordinalize(VALUE self, VALUE str);
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

  rb_define_singleton_method(NativeSupport__Inflector, "camelize", method_camelize, 1);
  rb_define_singleton_method(NativeSupport__Inflector, "underscore", method_underscore, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "dasherize", method_dasherize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "ordinalize", method_ordinalize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "demodulize", method_demodulize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "constantize", method_constantize, 1);
	rb_define_method(NativeSupport__CoreExtensions__Float__Rounding,
									 "round_with_precision", method_round_with_precision, -1);
}

VALUE method_camelize(VALUE self, VALUE str) {
  char * orig_str = StringValuePtr(str);
  int orig_str_len = strlen(orig_str);
  char * camelized_str = ALLOC_N(char, orig_str_len);
  VALUE camelized_ruby_str;
  int o; // position in original string
  int c = 0; // position in camelized string
  int state = 1; // 0: Next is unchanged
                 // 1: Next is capitalized
                 // 2+: Next is lower case

  for(o = 0; o < orig_str_len; o++) {
    if (orig_str[o] >= 'a' && orig_str[o] <= 'z') {
      if (state == 1) {
        camelized_str[c++] = orig_str[o] - 32;
      } else {
        camelized_str[c++] = orig_str[o];
      }
      state = 0;
    } else if (orig_str[o] == '_' && !state) {
      state = 1;
    } else {
      camelized_str[c++] = orig_str[o];
      state = 0;
    }
  }

  camelized_ruby_str = rb_str_new(camelized_str, c);
  free(camelized_str);
  return camelized_ruby_str;
}

VALUE method_underscore(VALUE self, VALUE str) {
  char * orig_str = StringValuePtr(str);
  int orig_str_len = strlen(orig_str);
  char * underscore_str = ALLOC_N(char, orig_str_len * 2 - 1);
  VALUE underscore_ruby_str;
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
      underscore_str[u++] = orig_str[o];
      state = (orig_str[o] >= 'a' && orig_str[o] <= 'z' || orig_str[o] >= '0' && orig_str[o] <= '9');
    }
  }

  underscore_ruby_str = rb_str_new(underscore_str, u); 
  free(underscore_str);
  return underscore_ruby_str;
}

VALUE method_dasherize(VALUE self, VALUE str) {
	char * orig_str = StringValuePtr(str);
	int orig_str_len = strlen(orig_str);
	char * dasherized_str = ALLOC_N(char, orig_str_len);
	VALUE dasherized_ruby_str;
	int i;

	for (i = 0; i < orig_str_len; i++) {
		if (orig_str[i] == '_') {
			dasherized_str[i] = '-';
		} else {
			dasherized_str[i] = orig_str[i];
		}
	}

	dasherized_ruby_str = rb_str_new(dasherized_str, i);
	return dasherized_ruby_str;
}

VALUE method_ordinalize(VALUE self, VALUE num) {
	int cnum = NUM2INT(num);
	char * ordinalized_str = ALLOC_N(char, log(cnum) + 2);
	char * suffix;
	VALUE ordinalized_ruby_str;

	switch (cnum) {
		case 11:
		case 12:
		case 13:
			suffix = "th";
			break;
		default:
			switch (cnum % 10) {
				case 1:
					suffix = "st";
					break;
				case 2:
					suffix = "nd";
					break;
				case 3:
					suffix = "rd";
					break;
				default:
					suffix = "th";
					break;
			}
			break;
	}

	sprintf(ordinalized_str, "%d%s", cnum, suffix);
	ordinalized_ruby_str = rb_str_new2(ordinalized_str);
	free(ordinalized_str);
	return ordinalized_ruby_str;
}

VALUE method_demodulize(VALUE self, VALUE str) {
	char * orig_str = StringValuePtr(str);
	int orig_str_len = strlen(orig_str);
	char * demodulized_str = ALLOC_N(char, orig_str_len);
	VALUE demodulized_ruby_str;
	int o = 0, d = 0;
	short state = 0;

	for (o = 0; o < orig_str_len; o++) {
		demodulized_str[d++] = orig_str[o];
		if (orig_str[o] == ':') {
			if (++state > 1) {
				d = 0;
			}
		} else {
			state = 0;
		}
	}

	demodulized_ruby_str = rb_str_new(demodulized_str, d);
	free(demodulized_str);
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
