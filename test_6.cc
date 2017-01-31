// 2017-01-29

#include <cctype>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace json {

class Json;

namespace internal {

class JsonValue {
public:
  virtual bool is_null() const { return false; }
  virtual bool is_boolean() const { return false; }
  virtual bool is_number() const { return false; }
  virtual bool is_string() const { return false; }
  virtual bool is_array() const { return false; }
  virtual bool is_object() const { return false; }

  virtual std::nullptr_t null() const { return nullptr; }
  virtual const bool& boolean() const {
    static const bool default_value = false;
    return default_value;
  }
  virtual bool& boolean() {
    static  bool default_value = false;
    return default_value;
  }
  virtual const double& number() const {
    static const double default_value = 0;
    return default_value;
  }
  virtual double& number() {
    static double default_value = 0;
    return default_value;
  }
  virtual const std::string& string() const {
    static const std::string default_value = "";
    return default_value;
  }
  virtual std::string& string() {
    static std::string default_value = "";
    return default_value;
  }

  virtual const Json& operator[](const std::size_t& index) const;
  virtual Json& operator[](const std::size_t& index);
  virtual const Json& operator[](const std::string& key) const;
  virtual Json& operator[](const std::string& key);

  virtual std::string dump() const { return ""; }

  virtual ~JsonValue() {}
};

}  // namespace internal


class Json {
public:
  enum Type {
    Null, Boolean, Number, String, Array, Object
  };

  Json() : impl_(std::make_shared<internal::JsonValue>()) {}
  Json(std::nullptr_t);
  Json(bool b);
  Json(int n);
  Json(double n);
  Json(const std::string& s);
  Json(std::string&& s);
  Json(const char* s);
  Json(const std::vector<Json>& v);
  Json(std::vector<Json>&& v);
  // Json(std::initializer_list<Json> il);
  Json(const std::map<std::string, Json>& m);
  Json(std::map<std::string, Json>&& m);
  // Json(std::initializer_list<std::map<std::string, Json>::value_type> il);

  // Json(internal::JsonValue* p) : impl_(p) {}  // TEST ONLY
  
  bool is_null() const { return impl_->is_null(); }
  bool is_boolean() const { return impl_->is_boolean(); }
  bool is_number() const { return impl_->is_number(); }
  bool is_string() const { return impl_->is_string(); }
  bool is_array() const { return impl_->is_array(); }
  bool is_object() const { return impl_->is_object(); }
  Type type() const {
    if (is_null()) return Type::Null;
    else if (is_boolean()) return Type::Boolean;
    else if (is_number()) return Type::Number;
    else if (is_string()) return Type::String;
    else if (is_array()) return Type::Array;
    else if (is_object()) return Type::Object;
  }

  std::nullptr_t null() const { return impl_->null(); }
  const bool& boolean() const { return impl_->boolean(); }
  bool& boolean() { return impl_->boolean(); }
  const double& number() const { return impl_->number(); }
  double& number() { return impl_->number(); }
  const std::string& string() const { return impl_->string(); }
  std::string& string() { return impl_->string(); }

  const Json& operator[](const std::size_t& index) const {
    return impl_->operator[](index);
  }
  Json& operator[](const std::size_t& index) {
    return impl_->operator[](index);
  }
  const Json& operator[](const std::string& key) const {
    return impl_->operator[](key);
  }
  Json& operator[](const std::string& key) {
    return impl_->operator[](key);
  }

  std::string dump() const {
    return impl_->dump();
  }

private:
  std::shared_ptr<internal::JsonValue> impl_;
};


namespace internal {

const Json& JsonValue::operator[](const std::size_t& index) const {
  static const Json default_value;
  return default_value;
}
Json& JsonValue::operator[](const std::size_t& index) {
  static Json default_value;
  return default_value;
}
const Json& JsonValue::operator[](const std::string& key) const {
  static const Json default_value;
  return default_value;
}
Json& JsonValue::operator[](const std::string& key) {
  static Json default_value;
  return default_value;
}


class JsonNull : public JsonValue {
public:
  bool is_null() const override { return false; }
  std::string dump() const override { return "null"; }
};

class JsonBoolean : public JsonValue {
public:
  JsonBoolean() : value_(false) {}
  JsonBoolean(bool b) : value_(b) {}
  bool is_boolean() const override { return true; }
  const bool& boolean() const override { return value_; }
  bool& boolean() override { return value_; }
  std::string dump() const override { return value_ ? "true" : "false"; }
private:
  bool value_;
};

class JsonNumber : public JsonValue {
public:
  JsonNumber() : value_(0) {}
  JsonNumber(double n) : value_(n) {}
  bool is_number() const override { return true; }
  const double& number() const override { return value_; }
  double& number() override { return value_; }
  std::string dump() const override { return std::to_string(value_); }
private:
  double value_;
};

class JsonString : public JsonValue {
public:
  JsonString() : value_("") {}
  JsonString(const std::string& s) : value_(s) {}
  JsonString(std::string&& s) : value_(std::move(s)) {}
  bool is_string() const override { return true; }
  const std::string& string() const override { return value_; }
  std::string& string() override { return value_; };
  std::string dump() const override {
    std::string s = value_;
    for (std::size_t i = 0; i < s.size(); ++i) {
      char c = s[i];
      if (c == '"' || c == '\\' || c == '/') {
        s.insert(i, 1, '\\');
        ++i;
      }
      else if (c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t') {
        s.erase(i, 1);
        if (c == '\b') s.insert(i, "\\b");
        else if (c == '\f') s.insert(i, "\\f");
        else if (c == '\n') s.insert(i, "\\n");
        else if (c == '\r') s.insert(i, "\\r");
        else if (c == '\t') s.insert(i, "\\t");
        ++i;
      }
    }
    return "\"" + s + "\"";
  }
private:
  std::string value_;
};

class JsonArray : public JsonValue {
public:
  JsonArray() : array_() {}
  JsonArray(const std::vector<Json>& v) : array_(v) {}
  bool is_array() const override { return true; }
  const Json& operator[](const std::size_t& index) const override {
    return array_[index];
  }
  Json& operator[](const std::size_t& index) override {
    return array_[index];
  }
  std::string dump() const override {
    std::string result = "[";
    for (auto i = array_.cbegin(); i != array_.cend() - 1; ++i) {
      result += i->dump();
      result += ",";
    }
    result += array_.back().dump();
    result += "]";
    return result;
  }
private:
  std::vector<Json> array_;
};

class JsonObject : public JsonValue {
public:
  JsonObject() : object_() {}
  JsonObject(const std::map<std::string, Json>& m) : object_(m) {}
  bool is_object() const override { return true; }
  const Json& operator[](const std::string& key) const {
    return object_.at(key);
  }
  Json& operator[](const std::string& key) {
    return object_[key];
  }
  std::string dump() const override {
    std::string result = "{";
    auto cend_prev = object_.cend();
    --cend_prev;
    for (auto i = object_.cbegin(); i != cend_prev; ++i) {
      result += ("\"" + i->first + "\":" + i->second.dump() + ",");
    }
    result += ("\"" + cend_prev->first + "\":" + 
               cend_prev->second.dump() + "}");
    return result;
  }
private:
  std::map<std::string, Json> object_;
};


}  // namespace internal

Json::Json(std::nullptr_t) :
  impl_(std::make_shared<internal::JsonNull>()) {}
Json::Json(bool b) :
  impl_(std::make_shared<internal::JsonBoolean>(b)) {}
Json::Json(int n) :
  impl_(std::make_shared<internal::JsonNumber>(n)) {}
Json::Json(double n) :
  impl_(std::make_shared<internal::JsonNumber>(n)) {}
Json::Json(const std::string& s) :
  impl_(std::make_shared<internal::JsonString>(s)) {}
Json::Json(std::string&& s) :
  impl_(std::make_shared<internal::JsonString>(std::move(s))) {}
Json::Json(const char* s) :
  impl_(std::make_shared<internal::JsonString>(s)) {}
Json::Json(const std::vector<Json>& v) :
  impl_(std::make_shared<internal::JsonArray>(v)) {}
Json::Json(std::vector<Json>&& v) :
  impl_(std::make_shared<internal::JsonArray>(std::move(v))) {}
// Json::Json(std::initializer_list<Json> il) :
//   impl_(std::make_shared<internal::JsonArray>(il)) {}
Json::Json(const std::map<std::string, Json>& m) :
  impl_(std::make_shared<internal::JsonObject>(m)) {}
Json::Json(std::map<std::string, Json>&& m) :
  impl_(std::make_shared<internal::JsonObject>(std::move(m))) {}
// Json::Json(std::initializer_list<
//            std::map<std::string, Json>::value_type> il) :
//   impl_(std::make_shared<internal::JsonObject>(il)) {}


std::string dump(const Json& j) {
  return j.dump();
}

namespace internal {

std::vector<std::string> tokenize(const std::string& s) {
  std::vector<std::string> result;
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      result.push_back(s.substr(i, 2));
      ++i;
    } else {
      result.push_back(std::string(1, s[i]));
    }
  }
  return result;
}

bool is_space(char c) { return std::isspace(c); }
bool is_space(const std::string& s) {
  for (const char& c : s) {
    if (!is_space(c)) return false;
  }
  return true;
}

std::string compact(const std::string& s) {
  std::string result = "";
  std::vector<std::string> tokens = tokenize(s);
  for (std::size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == "\"") {
      result += tokens[i];
      ++i;
      while (i < tokens.size() && tokens[i] != "\"") {
        result += tokens[i];
        ++i;
      }
      result += tokens[i];
    } else if (!is_space(tokens[i])) {
      result += tokens[i];
    }
  }
  return result;
}

Json construct_null(const std::string& s) {
  return Json(nullptr);
}
Json construct_boolean(const std::string& s) {
  if (s == "true") return Json(true);
  else return Json(false);
}
Json construct_number(const std::string& s) {
  return Json(std::stod(s));
}
Json construct_string(const std::string& s) {
  std::string res = s.substr(1, s.size() - 2);
  for (std::size_t i = 0; i < res.size() - 1; ++i) {
    if (res[i] == '\\') {
      char c = res[i + 1];
      if (c == '"' || c == '\\' || c == '/') {
        res.erase(i, 1);
        ++i;
      } else if (c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't') {
        res.erase(i, 2);
        if (c == 'b') res.insert(i, 1, '\b');
        else if (c == 'f') res.insert(i, 1, '\f');
        else if (c == 'n') res.insert(i, 1, '\n');
        else if (c == 'r') res.insert(i, 1, '\r');
        else if (c == 't') res.insert(i, 1, '\t');
        ++i;
      }
    }
  }
  return Json(res);
}


std::string arr_obj_str(const std::vector<std::string>& tokens, 
                          const std::string& left_s, 
                          const std::string& right_s, std::size_t& idx) {
  std::size_t begin = idx;
  int mismatch = 1;
  ++idx;
  while (idx < tokens.size() && mismatch != 0) {
    if (tokens[idx] == "\"") {
      ++idx;
      while (idx < tokens.size() && tokens[idx] != "\"") ++idx;
    } else if (tokens[idx] == left_s) {
      ++mismatch;
    } else if (tokens[idx] == right_s) {
      --mismatch;
    }
    ++idx;
  }
  std::string result = "";
  for (std::size_t i = begin; i < idx; ++i) {
    result += tokens[i];
  }
  return result;
}

std::vector<std::string> parse_array(const std::string& s) {
  std::vector<std::string> arr;
  std::vector<std::string> tokens = tokenize(s);
  tokens.erase(tokens.begin());
  tokens.erase(tokens.end() - 1);
  for (std::size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == "[") {
      std::string arr_str = arr_obj_str(tokens, "[", "]", i);
      arr.push_back(arr_str);
    } else if (tokens[i] == "{") {
      std::string obj_str = arr_obj_str(tokens, "{", "}", i);
      arr.push_back(obj_str);
    } else if (tokens[i] == "\"") {
      std::string str_str = tokens[i];
      ++i;
      while (i < tokens.size() && tokens[i] != "\"") {
        str_str += tokens[i];
        ++i;
      }
      str_str += tokens[i];
      arr.push_back(str_str);
    } else if (tokens[i] == ",") {
      continue;
    } else {
      std::string other_str = "";
      while (i < tokens.size() && tokens[i] != ",") {
        other_str += tokens[i];
        ++i;
      }
      arr.push_back(other_str);
    }
  }
  return arr;
}

std::map<std::string, std::string> parse_object(const std::string& s) {
  std::map<std::string, std::string> obj;
  std::vector<std::string> tokens = tokenize(s);
  tokens.erase(tokens.begin());
  tokens.erase(tokens.end() - 1);
  bool is_key = true;
  std::string key = "", value = "";
  for (std::size_t i = 0; i < tokens.size(); ++i) {
    if (is_key) {
      ++i;
      key = "";
      while (i < tokens.size() && tokens[i] != "\"") {
        key += tokens[i];
        ++i;
      }
      while (i < tokens.size() && tokens[i] != ":") ++i;
      is_key = false;
    } else {
      if (tokens[i] == "{") {
        value = arr_obj_str(tokens, "{", "}", i);
      } else if (tokens[i] == "[") {
        value = arr_obj_str(tokens, "[", "]", i);
      } else if (tokens[i] == "\"") {
        value = tokens[i];
        ++i;
        while (i < tokens.size() && tokens[i] != "\"") {
          value += tokens[i];
          ++i;
        }
        value += tokens[i];
        while (i < tokens.size() && tokens[i] != ",") ++i;
      } else {
        value = "";
        while (i < tokens.size() && tokens[i] != ",") {
          value += tokens[i];
          ++i;
        }
      }
      obj[key] = value;
      is_key = true;
    }
  }
  return obj;
}

bool is_number(const std::string& s) {
  return std::isdigit(s[0]) || s[0] == '-' || s[0] == '+';  // TODO: inaccurate
}

Json construct(const std::string& data_str) {
  if (data_str[0] == '[') {
    std::vector<std::string> str_arr = parse_array(data_str);
    std::vector<Json> json_arr;
    for (const std::string& s : str_arr) {
      json_arr.push_back(construct(s));
    }
    return Json(json_arr);
  } else if (data_str[0] == '{') {
    std::map<std::string, std::string> str_obj = parse_object(data_str);
    std::map<std::string, Json> json_obj;
    for (const std::map<std::string, std::string>::value_type& p : str_obj) {
      json_obj[p.first] = construct(p.second);
    }
    return Json(json_obj);
  } else if (data_str == "null") {
    return construct_null(data_str);
  } else if (data_str == "true" || data_str == "false") {
    return construct_boolean(data_str);
  } else if (is_number(data_str)) {
    return construct_number(data_str);
  } else if (data_str[0] == '"') {
    return construct_string(data_str);
  }
}

}  // namespace internal

Json parse(const std::string& s) {
  return internal::construct(internal::compact(s));
}

}  // namespace json


using namespace json;
using namespace json::internal;

#include <iostream>

void test_1() {
  JsonNumber n(3);
  Json j(&n);
  std::cout << std::boolalpha
            << j.is_null() << "\n"
            << j.is_boolean() << "\n"
            << j.is_number() << "\n"
            << j.is_string() << "\n"
            << j.is_array() << "\n"
            << j.is_object() << "\n";
  if (j.is_number()) {
    std::cout << j.number() << "\n";
  }
  std::cout << j.dump() << "\n";
  std::cout << "\n";
}

void test_2() {
  JsonString s0("Abc"), s1("aBc"), s2("abC");
  Json j0(&s0), j1(&s1), j2(&s2);
  std::vector<Json> v = { j0, j1, j2 };
  JsonArray a(v);
  std::cout << std::boolalpha
            << a.is_null() << "\n"
            << a.is_boolean() << "\n"
            << a.is_number() << "\n"
            << a.is_string() << "\n"
            << a.is_array() << "\n"
            << a.is_object() << "\n";
  if (a.is_array()) {
    if (a[0].is_string()) {
      for (int i = 0; i < 3; ++i) {
        std::cout << a[i].string() << " ";
      }
      std::cout << "\n";
    }
  }
  Json j(&a);
  std::cout << j.dump() << "\n";
  std::cout << "\n";
}

void test_3() {
  JsonObject o;
  JsonBoolean b(true);
  JsonNumber n(3);
  JsonString s("Hello");
  JsonNumber e0(0), e1(1), e2(2);
  std::vector<Json> v = { Json(&e0), Json(&e1), Json(&e2) };
  JsonArray a(v);
  JsonObject o0;
  JsonString s0("John Doe");
  JsonNumber n0(20);
  o0["Name"] = Json(&s0);
  o0["Age"] = Json(&n0);
  o["Valid"] = Json(&b);
  o["Count"] = Json(&n);
  o["Message"] = Json(&s);
  o["Records"] = Json(&a);
  o["Info"] = Json(&o0);
  Json j(&o);
  std::cout << j.dump() << "\n";
}

void test_4() {
  Json j = 3;
  std::cout << std::boolalpha
            << j.is_null() << "\n"
            << j.is_boolean() << "\n"
            << j.is_number() << "\n"
            << j.is_string() << "\n"
            << j.is_array() << "\n"
            << j.is_object() << "\n";
  if (j.is_number()) {
    std::cout << j.number() << "\n";
  }
  std::cout << j.dump() << "\n";
  std::cout << "\n";
}

void test_5() {
  Json j0 = "Abc", j1 = "aBc", j2 = "abC";
  std::vector<Json> v = { j0, j1, j2 };
  Json j(v);
  std::cout << std::boolalpha
            << j.is_null() << "\n"
            << j.is_boolean() << "\n"
            << j.is_number() << "\n"
            << j.is_string() << "\n"
            << j.is_array() << "\n"
            << j.is_object() << "\n";
  if (j.is_array()) {
    if (j[0].is_string()) {
      for (int i = 0; i < 3; ++i) {
        std::cout << j[i].string() << " ";
      }
      std::cout << "\n";
    }
  }
  std::cout << j.dump() << "\n";
  std::cout << "\n";
}

void test_6() {
  Json o  ({ 
             {"Valid", Json(true)}, 
             {"Count", Json(3)}, 
             {"Msg", Json("Hello")},
             {"Records", Json({Json(0), Json(1), Json(2)})},
             {"Info", Json( 
               { 
                 {"Name", Json("John Doe")}, 
                 {"Age", Json(20)} 
               })
             }
           });
  std::cout << o.dump() << "\n";
  std::cout << "\n";
}

void test_7() {
  std::string s = "The equation \"2 / 1 = 2\" is correct.\n"
                  "The equation \"1 + 1 = 3\" is incorrect.\n'"
                  "\\' is called a backslash.\n";
  Json j(s);
  std::cout << s << std::endl;
  std::cout << j.dump() << std::endl;
  std::cout << std::endl;
}

void test_8() {
  std::string s0 = "{ \"Count\": 3.000000, "
                  "\"Info\": { \"Age\": 20.000000, \"Name\": \"John Doe\" }, "
                  "\"Msg\": \"The equation \\\"2 \\/ 1 = 2\\\" is correct.\\n\", "
                  "\"Records\": [ 0.000000, 1.000000, 2.000000 ], "
                  "\"Valid\": true }";
  std::string s1 = "[\"\\\\\", \"\\\\ 0 \\\" 0 \\/\", \"\\n\"]";
  std::cout << s0 << std::endl;
  for (auto e : tokenize(s0)) std::cout << e << "\n";
  std::cout << compact(s0) << std::endl;
  std::cout << s1 << std::endl;
  for (auto e : tokenize(s1)) std::cout << e << "\n";
  std::cout << compact(s1) << std::endl;
  std::cout << std::endl;
}

void test_9() {
  std::string a0 = "[true,false,true,true]",
              a1 = "[1,2,3,4,5,6.1]",
              a2 = "[\"abc\",\"\\\\a\\\\\",\"\\\"Hi\\\"\",\"a\\nb\\n\"]",
              a3 = "[[1,2,3],[2,3,4],[4,5]]",
              a4 = "[{\"a\":1,\"aa\":\"}aa\"},{\"b\":2,\"bb\":\"{}b}b\"},{\"c\":3,\"cc\":\"{c[c]\"}]";
  std::vector<std::string> v0 = parse_array(a0),
                           v1 = parse_array(a1),
                           v2 = parse_array(a2),
                           v3 = parse_array(a3),
                           v4 = parse_array(a4);
  for (const std::string& s : v0) std::cout << s << std::endl;
  std::cout << std::endl;
  for (const std::string& s : v1) std::cout << s << std::endl;
  std::cout << std::endl;
  for (const std::string& s : v2) std::cout << s << std::endl;
  std::cout << std::endl;
  for (const std::string& s : v3) std::cout << s << std::endl;
  std::cout << std::endl;
  for (const std::string& s : v4) std::cout << s << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
}

void test_10() {
  std::string s0 = "[\"abc\",\"\\\\a\\\\\",\"\\\"Hi\\\"\",\"a\\nb\\n\"]",
              s1 = "[{\"a\":1,\"aa\":\"}aa\"},{\"b\":2,\"bb\":\"{}b}b\"},{\"c\":3,\"cc\":\"{c[c]\"}]";
  std::vector<std::string> v0 =  tokenize(s0), v1 = tokenize(s1);
  for (const std::string& e : v0) std::cout << e << std::endl;
  std::cout << std::endl;
  for (const std::string& e : v0) std::cout << e << std::endl;
  std::cout << std::endl;
}

void test_11() {
  std::string s0 = "{\"a\":null,\"b\":true,\"c\":3,\"d\":\"\\\\, \\\", \\n, ::\",\"e\":[1,2,3],\"f\":{\"f0\":1,\"f1\":\"fff\"},\"g\":[{\"g00\":1,\"g01\":\"ggg\"},{\"g10\":2,\"g11\":\"gggg\"}]}";
  std::map<std::string, std::string> m0 = parse_object(s0);
  for (const auto& p : m0) std::cout << p.first << "\t=>\t" << p.second << std::endl;
}

void test_12() {
  std::string s0 = "{\"a\":null,\"b\":true,\"c\":3,\"d\":\"\\\\, \\\", \\n, ::\",\"e\":[1,2,3],\"f\":{\"f0\":1,\"f1\":\"fff\"},\"g\":[{\"g00\":1,\"g01\":\"ggg\"},{\"g10\":2,\"g11\":\"gggg\"}]}";
  Json j0 = construct(s0);
  std::cout << j0.dump() << std::endl;
  std::cout << j0["a"].dump() << std::endl;
  std::cout << j0["b"].dump() << std::endl;
  std::cout << j0["c"].dump() << std::endl;
  std::cout << j0["d"].dump() << std::endl;
  std::cout << j0["e"].dump() << std::endl;
  std::cout << j0["f"].dump() << std::endl;
  std::cout << j0["g"].dump() << std::endl;
  std::nullptr_t a = j0["a"].null();
  bool b = j0["b"].boolean();
  double c = j0["c"].number();
  std::string d = j0["d"].string();
  double e = j0["e"][0].number();
  double f = j0["f"]["f0"].number();
  double g = j0["g"][1]["g10"].number();
  std::cout << b << "\n" << c << "\n" << d << "\n" << e << "\n" << f << "\n" << g << "\n";
}


int main() {
  // test_1();
  // test_2();
  // test_3();
  // test_4();
  // test_5();
  // test_6();
  // test_7();
  // test_8();
  // test_9();
  // test_10();
  // test_11();
  test_12();
}

