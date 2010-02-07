#include "ruby.h"

VALUE NativeSupport;
VALUE NativeSupport__Inflector;

void Init_native_support();

VALUE method_camelize(VALUE self, VALUE str);
VALUE method_underscore(VALUE self, VALUE str);
VALUE method_dasherize(VALUE self, VALUE str);
VALUE method_ordinalize(VALUE self, VALUE str);

void Init_native_support() {
  NativeSupport = rb_define_module("NativeSupport");
  NativeSupport__Inflector = rb_define_module_under(NativeSupport, "Inflector");
  rb_define_singleton_method(NativeSupport__Inflector, "camelize", method_camelize, 1);
  rb_define_singleton_method(NativeSupport__Inflector, "underscore", method_underscore, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "dasherize", method_dasherize, 1);
	rb_define_singleton_method(NativeSupport__Inflector, "ordinalize", method_ordinalize, 1);
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
