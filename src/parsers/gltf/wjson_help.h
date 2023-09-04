#pragma once

#define JSON_GET(what, on) WJSONValue *v_##what = wjson_get(on, #what)
#define JSON_NUMBER(on) double num_##on = wjson_number(on)
#define JSON_INDEX(i, on) WJSONValue *v_##i = wjson_index(on, i)
#define JSON_TRY_GET(what, on, if_succeed)                                     \
  {                                                                            \
    JSON_GET(what, on);                                                        \
    if (v_##what != NULL) {                                                    \
      if_succeed                                                               \
    }                                                                          \
  }
