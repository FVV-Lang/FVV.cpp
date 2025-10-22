//====================================================================================================
// Copyright (C) 2016-present ShIroRRen <http://shiror.ren>.                                         =
//                                                                                                   =
// Licensed under the F2DLPR License.                                                                =
//                                                                                                   =
// YOU MAY NOT USE THIS FILE EXCEPT IN COMPLIANCE WITH THE LICENSE.                                  =
// Provided "AS IS", WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,                                   =
// unless required by applicable law or agreed to in writing.                                        =
//                                                                                                   =
// For the F2DLPR License terms and conditions, visit: <http://license.fileto.download>.             =
//====================================================================================================

#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <cstdint>

#ifdef __GNUC__
#define FVV_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define FVV_INLINE __forceinline
#else
#define FVV_INLINE inline
#endif

namespace FVV {
	using namespace std;
	using str  = string;
	using strv = string_view;
	template<typename Tp>
	using vec = vector<Tp>;

	template<typename _keyTp, typename _valTp>
	struct KVPair : public pair<_keyTp, _valTp> {
		using pair<_keyTp, _valTp>::pair;
		FVV_INLINE _keyTp&		 key(void) { return this->first; }
		FVV_INLINE const _keyTp& key(void) const { return this->first; }
		FVV_INLINE _keyTp&&		 key_rv(void) { return std::move(this->first); }
		FVV_INLINE _valTp&		 value(void) { return this->second; }
		FVV_INLINE const _valTp& value(void) const { return this->second; }
		FVV_INLINE _valTp&&		 value_rv(void) { return std::move(this->second); }
	};
	template<typename _keyTp, typename _valTp>
	struct PairList : public vec<KVPair<_keyTp, _valTp>> {
		using iterator		 = typename vec<KVPair<_keyTp, _valTp>>::iterator;
		using const_iterator = typename vec<KVPair<_keyTp, _valTp>>::const_iterator;
		FVV_INLINE vec<KVPair<_keyTp, _valTp>>&& data_rv(void) { return std::move(*this); }
		FVV_INLINE _valTp&						 operator[](_keyTp const& key) {
			  if (auto it
				  = find_if(this->begin(), this->end(), [&key](auto const& p) { return p.key() == key; });
				  it != this->end())
				  return it->value();
			  return this->emplace_back(key, _valTp()), this->back().value();
		}
		FVV_INLINE _keyTp& operator()(_valTp const& value) {
			if (auto it = find_if(this->begin(),
								  this->end(),
								  [&value](auto const& p) { return p.value() == value; });
				it != this->end())
				return it->key();
			return this->emplace_back(_keyTp(), value), this->back().key();
		}
		FVV_INLINE bool hasKey(_keyTp const& key) const {
			return any_of(this->begin(), this->end(), [&key](auto const& p) { return p.key() == key; });
		}
		FVV_INLINE bool hasValue(_valTp const& value) const {
			return any_of(
					this->begin(), this->end(), [&value](auto const& p) { return p.value() == value; });
		}
		FVV_INLINE const_iterator findKey(_keyTp const& key) const {
			return find_if(this->begin(), this->end(), [&key](auto const& p) { return p.key() == key; });
		}
		FVV_INLINE const_iterator findValue(_valTp const& value) const {
			return find_if(
					this->begin(), this->end(), [&value](auto const& p) { return p.value() == value; });
		}
		FVV_INLINE void eraseKey(_keyTp const& key) {
			this->erase(
					remove_if(
							this->begin(), this->end(), [&key](auto const& p) { return p.key() == key; }),
					this->end());
		}
		FVV_INLINE void eraseValue(_valTp const& value) {
			this->erase(remove_if(this->begin(),
								  this->end(),
								  [&value](auto const& p) { return p.value() == value; }),
						this->end());
		}
		FVV_INLINE vec<_keyTp> keys(void) const {
			vec<_keyTp> keys;
			keys.reserve(this->size());
			transform(this->begin(), this->end(), back_inserter(keys), [](auto const& p) {
				return p.key();
			});
			return keys;
		}
		FVV_INLINE vec<_valTp> values(void) const {
			vec<_valTp> vals;
			vals.reserve(this->size());
			transform(this->begin(), this->end(), back_inserter(vals), [](auto const& p) {
				return p.second;
			});
			return vals;
		}
		FVV_INLINE void sort(
				function<bool(KVPair<_keyTp, _valTp> const&, KVPair<_keyTp, _valTp> const&)> func
				= nullptr) {
			std::sort(this->begin(), this->end(), func ? func : [](auto const& a, auto const& b) {
				return a.key() < b.key();
			});
		}
	};
	enum FormatOpt { Common, Min, BigList, NoDesc };
	class FVVV {
	   public:
		using FVVVT = variant<monostate,
							  bool,
							  int,
							  double,
							  str,
							  vec<bool>,
							  vec<int>,
							  vec<double>,
							  vec<str>,
							  vec<FVVV>>;
		/// @brief 值
		FVVVT value;
		/// @brief 子值
		PairList<str, FVVV> sub = {};
		/// @brief 描述
		str desc = "";
		/// @brief 链接名称
		str link = "";

		FVV_INLINE		 FVVV(void) = default;
		FVV_INLINE		 FVVV(const FVVVT& v): value(v) {}
		FVV_INLINE FVVV& operator[](strv key) { return sub[key.data()]; }
		FVV_INLINE FVVV& operator=(const FVVVT& val) { return value = val, *this; }
		FVV_INLINE FVVV& operator=(FVVVT&& val) { return value = val, *this; }
		FVV_INLINE bool	 operator==(const FVVV& other) const {
			 return addressof(other) == this || (value == other.value && sub == other.sub);
		}
		FVV_INLINE bool operator!=(const FVVV& other) const {
			return addressof(other) != this && (value != other.value || sub != other.sub);
		}
		/// @brief  以bool类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE bool asBool(bool defaultValue = _getDfltVal<bool>()) const {
			return as<bool>(defaultValue);
		}
		/// @brief  以int类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE int asInt(int defaultValue = _getDfltVal<int>()) const {
			return as<int>(defaultValue);
		}
		/// @brief  以double类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE double asDouble(double defaultValue = _getDfltVal<double>()) const {
			return as<double>(defaultValue);
		}
		/// @brief  以string类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const str& asString(str const& defaultValue = _getDfltVal<str>()) const {
			return as<str>(defaultValue);
		}
		/// @brief  以vector<bool>类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const vec<bool>& asBools(vec<bool> const& defaultValue
											= _getDfltVal<vec<bool>>()) const {
			return as<vec<bool>>(defaultValue);
		}
		/// @brief  以vector<int>类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const vec<int>& asInts(vec<int> const& defaultValue = _getDfltVal<vec<int>>()) const {
			return as<vec<int>>(defaultValue);
		}
		/// @brief  以vector<double>类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const vec<double>& asDoubles(vec<double> const& defaultValue
												= _getDfltVal<vec<double>>()) const {
			return as<vec<double>>(defaultValue);
		}
		/// @brief  以vector<string>类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const vec<str>& asStrings(vec<str> const& defaultValue
											 = _getDfltVal<vec<str>>()) const {
			return as<vec<str>>(defaultValue);
		}
		/// @brief  以vector<FVVV>类型返回值
		/// @param  默认值(可选)
		/// @return 值
		FVV_INLINE const vec<FVVV>& asFVVVs(vec<FVVV> const& defaultValue
											= _getDfltVal<vec<FVVV>>()) const {
			return as<vec<FVVV>>(defaultValue);
		}
		/// @brief              以指定类型返回值
		/// @param Tp           类型
		/// @param defaultValue 默认值(可选)
		/// @return             值为指定类型时返回值，否则为默认值
		template<typename Tp>
		FVV_INLINE Tp const& as(Tp const& defaultValue = _getDfltVal<Tp>()) const {
			if (auto ptr = get_if<Tp>(&value)) return *ptr;
			return defaultValue;
		}
		/// @brief  判断值是否为空
		/// @return 值为空时返回true，否则为false
		FVV_INLINE bool isEmpty(void) const { return holds_alternative<monostate>(value); }
		/// @brief  判断值是否为非空
		/// @return 值为非空时返回true，否则为false
		FVV_INLINE bool isNotEmpty(void) const { return !isEmpty(); }
		template<typename Tp>
		/// @brief  判断值是否为指定类型
		/// @param  类型
		/// @return 值为指定类型时返回true，否则为false
		FVV_INLINE bool isType(void) const {
			return isNotEmpty() && holds_alternative<Tp>(value);
		}
		/// @brief           输出FVV文本格式格式化后的值
		/// @param opt       格式化选项
		/// @param indent_lv 缩进等级
		/// @return          FVV文本格式格式化后的值
		FVV_INLINE str print(FormatOpt opt = FormatOpt::Common, size_t indent_lv = 0) const {
			stringstream result;

			function<void(strv, FVVV const*, size_t)> const print_func
					= [&](auto path, auto const* node, auto indent_lv) {
				if (path.empty() || (node->isEmpty() && node->sub.empty())) return;
				str indent(indent_lv * 2, ' ');
				if (!node->sub.empty() && node->link.empty()) {
					if (opt == FormatOpt::Min) result << path << "={";
					else result << indent << path << " = {\n";
				}
				if (!node->link.empty() || node->isNotEmpty()) {
					if (opt == FormatOpt::Min) result << path << '=';
					else result << indent << path << " = ";
					if (!node->link.empty()) result << node->link;
					else if (node->template isType<str>())
						result << '"' << _replace(node->template as<str>(), "\"", "\\\"") << '"';
					else if (node->template isType<bool>())
						result << (node->template as<bool>() ? "true" : "false");
					else if (node->template isType<int>()) result << to_string(node->template as<int>());
					else if (node->template isType<double>())
						result << to_string(node->template as<double>());
					else if (node->template isType<vec<str>>() || node->template isType<vec<bool>>()
							 || node->template isType<vec<int>>() || node->template isType<vec<double>>()
							 || node->template isType<vec<FVVV>>()) {
						if (!node->desc.empty() && node->template isType<vec<FVVV>>()) {
							result << '<' << _replace(node->desc, ">", "\\>") << '>';
							if (opt != FormatOpt::Min) result << ' ';
						}
						str list_indent((indent_lv + 1) * 2, ' ');
						result << '[';
						if (opt == FormatOpt::BigList
							|| (opt != FormatOpt::Min && node->template isType<vec<FVVV>>()))
							result << '\n';

						bool is_empty_list = true;
						if (node->template isType<vec<str>>())
							is_empty_list = writeList(result,
													  node->template as<vec<str>>(),
													  opt,
													  list_indent,
													  function<void(stringstream&, str const&)>(
															  [](auto& ss, auto const& v) {
								ss << '"' << _replace(v, "\"", "\\\"") << '"';
							}));
						else if (node->template isType<vec<bool>>())
							is_empty_list = writeList(result,
													  node->template as<vec<bool>>(),
													  opt,
													  list_indent,
													  function<void(stringstream&, bool const&)>(
															  [](auto& ss, auto const& v) {
								ss << str(v ? "true" : "false");
							}));
						else if (node->template isType<vec<int>>())
							is_empty_list = writeList(
									result,
									node->template as<vec<int>>(),
									opt,
									list_indent,
									function<void(stringstream&, int const&)>(
											[](auto& ss, auto const& v) { ss << to_string(v); }));
						else if (node->template isType<vec<double>>())
							is_empty_list = writeList(
									result,
									node->template as<vec<double>>(),
									opt,
									list_indent,
									function<void(stringstream&, double const&)>(
											[](auto& ss, auto const& v) { ss << to_string(v); }));
						else if (node->template isType<vec<FVVV>>()) {
							auto const tmp = node->template as<vec<FVVV>>();
							is_empty_list  = tmp.empty();
							for (auto const& value : tmp) {
								if (opt != FormatOpt::Min) result << list_indent;
								if (!value.desc.empty()) {
									result << '<' << _replace(value.desc, ">", "\\>") << '>';
									if (opt != FormatOpt::Min) result << ' ';
								}
								result << '{';
								if (opt != FormatOpt::Min) result << '\n';
								result << value.print(opt, indent_lv + 2);
								if (opt == FormatOpt::Min) result << ";}";
								else result << '\n' << list_indent << '}';
								if (opt == FormatOpt::Min) result << ',';
								else result << '\n';
							}
						}
						if (opt == FormatOpt::BigList
							|| (opt != FormatOpt::Min && node->template isType<vec<FVVV>>()))
							result << indent;
						else if (!is_empty_list) {
							_removeLastChar(result);
							if (opt != FormatOpt::Min) _removeLastChar(result);
						}
						result << ']';
					}
				} else
					for (auto const& [key, sub] : node->sub) print_func(key, &sub, indent_lv + 1);
				if (!node->sub.empty() && node->link.empty()) {
					if (opt != FormatOpt::Min) result << indent;
					result << '}';
				}
				if (!node->desc.empty() && !node->template isType<vec<FVVV>>() && opt != FormatOpt::Min
					&& opt != FormatOpt::NoDesc)
					result << " <" << _replace(node->desc, ">", "\\>") << '>';
				if (opt == FormatOpt::Min) result << ';';
				else result << '\n';
			};
			for (auto const& [k, v] : sub) print_func(k, &v, indent_lv);
			return _removeLastChar(result), result.str();
		}
		/// @brief     解析字符串到此FVVV
		/// @param txt FVV文本格式的字符串
		FVV_INLINE void parse(str txt) {
			if (txt.size() >= 3 && static_cast<unsigned char>(txt[0]) == _bom[0]
				&& static_cast<unsigned char>(txt[1]) == _bom[1]
				&& static_cast<unsigned char>(txt[2]) == _bom[2])
				txt = txt.substr(3);
			size_t start = txt.find_first_not_of(" \t\r\n");
			txt			 = start == str::npos ? "" : txt.substr(start);
			if (txt.empty()) return;
			if (txt.back() != '}' && txt.back() != '\n') txt += '\n';
			_replaceBase(txt, "\r\n", "\n"), _replaceBase(txt, "\r", "\n"), _shrink(&txt);

			struct FVVVDat {
				stringstream  value_name;
				str			  idx_desc;
				vec<str>	  group_names, value_names;
				vec<vec<str>> last_group_names;
				vec<FVVV>	  tmp_fvvs;
				bool		  in_value = false, in_list = false, is_list = false;
				size_t		  group_num = 0;
				FVVV		  root_key;
				FVVV*		  idx_key = &root_key;
			};
			auto static _getKey = [](vec<str> const& paths, FVVV* idx_key) -> FVVV* {
				return accumulate(paths.begin(), paths.end(), idx_key, [](FVVV* acc, str const& path) {
					return &(*acc)[path];
				});
			};
			auto static _findKey = [this](strv path, vec<FVVVDat>& stack_dat) -> FVVV* {
				vec<str> tmp_names = _split(path.data(), '.');
				FVVV*	 tmp_key   = nullptr;
				for (size_t idx = stack_dat.size(); idx-- > 0;) {
					FVVVDat& idx_dat = stack_dat[idx];
					tmp_key			 = idx_dat.idx_key;
					for (str const& tmp_name : tmp_names)
						if (tmp_key->sub.hasKey(tmp_name)) tmp_key = &(*tmp_key)[tmp_name];
						else {
							tmp_key = idx == 0 ? this : &idx_dat.root_key;
							for (str const& tmp_name : tmp_names)
								if (tmp_key->sub.hasKey(tmp_name)) tmp_key = &(*tmp_key)[tmp_name];
								else {
									tmp_key = nullptr;
									break;
								}
							break;
						}
					if (tmp_key) return tmp_key;
				}
				return nullptr;
			};
			stringstream tmp_desc, value;
			vec<str>	 values;
			bool		 old_fvv = false, end_group = false, is_real_char = false, in_desc = false,
				 in_str = false, is_str = false, is_all_str = false, is_empty_str = false;
			uint8_t			last_char_size = 0;
			vector<FVVVDat> fvv_stack(1);
			fvv_stack.front().idx_key = this;
			_utf8ForEach(
					txt, txt.size(), [&](auto const& idx, auto idx_char, auto const& char_size) -> bool {
				FVVVDat* idx_dat = &fvv_stack.back();
				idx_dat->idx_key = fvv_stack.size() > 1 ? &idx_dat->root_key : this;
				is_real_char
						= idx >= 1 ? (last_char_size == 1 ? (txt[idx - 1] != '\\' ? true : false) : true)
								   : true;
				last_char_size = char_size;
				if (in_desc)
					if (idx_char != ">" || !is_real_char) {
						if (idx_char == ">") _removeLastChar(tmp_desc);
						if (idx_dat->in_value || idx_dat->group_num > 0) tmp_desc << idx_char;
						return false;
					} else {
						idx_dat->idx_desc = tmp_desc.str();
						tmp_desc.str(""), tmp_desc.clear(), _shrink(&idx_dat->idx_desc);
						if (FVVV* key = _findKey(idx_dat->idx_desc, fvv_stack); key && key->isType<str>())
							idx_dat->idx_desc = key->asString();
						return in_desc = false, false;
					}
				else {
					if (!in_str && (idx_char == " " || idx_char == "\t")) return false;
					if (idx_char == "<") return in_desc = true, false;
				}
				if (idx_dat->in_value) {
					if (in_str)
						if (idx_char == "\"") {
							if (is_real_char) {
								is_empty_str  = value.str().empty();
								return in_str = false, false;
							} else {
								_removeLastChar(value);
								return value << idx_char, false;
							}
						} else return value << idx_char, false;
					else if (idx_char == "\"") return in_str = is_str = is_all_str = true, false;
					else if (idx_char == "[") return idx_dat->in_list = idx_dat->is_list = true, false;
					else if (idx_dat->in_list && idx_char == "{")
						return fvv_stack.push_back(FVVVDat()), false;
					else if (idx_dat->in_list && _eqOr(idx_char, strv(","), strv("]"), strv("\n"))) {
						str const value_str = value.str();
						if (idx_char == "]") {
							idx_dat->in_list	= false;
							size_t pos			= 1;
							bool   in_list_desc = false;
							for (;;) {
								if ([&txt, &idx, &pos, &in_list_desc]() -> bool {
									switch (txt[idx - pos]) {
										case '<':
											if (!in_list_desc) return true;
											if (idx - pos < 1 || txt[idx - pos - 1] != '\\')
												in_list_desc = false;
											return false;
										case '>' : return in_list_desc = true, false;
										case ' ' :
										case '\t': return false;
										case ',' :
										case '\n':
										default	 : return !in_list_desc;
									}
								}())
									break;
								++pos;
							}
							if (txt[idx - pos] == ',' || txt[idx - pos] == '\n') return false;
						} else if (idx_dat->tmp_fvvs.empty() && value_str.empty()
								   && (!is_all_str || !is_empty_str))
							return false;
						if ((is_all_str && is_str) || _eqOr(value_str, str("true"), str("false"))
							|| _isInt(value_str) || _isDouble(value_str))
							values.push_back(value_str);
						else if (idx_dat->idx_key = _getKey(
										 { value_str }, _getKey(idx_dat->group_names, idx_dat->idx_key));
								 idx_dat->idx_key->isNotEmpty()) {
							if (idx_dat->idx_key->isType<str>())
								values.push_back(idx_dat->idx_key->as<str>());
							else if (idx_dat->idx_key->isType<bool>())
								values.push_back(idx_dat->idx_key->as<bool>() ? "true" : "false");
							else if (idx_dat->idx_key->isType<int>())
								values.push_back(to_string(idx_dat->idx_key->as<int>()));
							else if (idx_dat->idx_key->isType<double>())
								values.push_back(to_string(idx_dat->idx_key->as<double>()));
							else if (idx_dat->idx_key->isType<vec<str>>()) {
								vec<str> const tmp = idx_dat->idx_key->as<vec<str>>();
								values.insert(values.end(), tmp.begin(), tmp.end());
							} else if (idx_dat->idx_key->isType<vec<bool>>()) {
								vec<bool> const tmp = idx_dat->idx_key->as<vec<bool>>();
								transform(tmp.begin(), tmp.end(), back_inserter(values), [](bool v) {
									return v ? "true" : "false";
								});
							} else if (idx_dat->idx_key->isType<vec<int>>()) {
								vec<int> const tmp = idx_dat->idx_key->as<vec<int>>();
								transform(tmp.begin(), tmp.end(), back_inserter(values), [](int v) {
									return to_string(v);
								});
							} else if (idx_dat->idx_key->isType<vec<double>>()) {
								vec<double> const tmp = idx_dat->idx_key->as<vec<double>>();
								transform(tmp.begin(), tmp.end(), back_inserter(values), [](double v) {
									return to_string(v);
								});
							} else if (idx_dat->idx_key->isType<vec<FVVV>>()) {
								vec<FVVV> const tmp = idx_dat->idx_key->as<vec<FVVV>>();
								idx_dat->tmp_fvvs.insert(idx_dat->tmp_fvvs.end(), tmp.begin(), tmp.end());
							}
						}
						if (idx_dat->tmp_fvvs.size() && !idx_dat->idx_desc.empty()) {
							idx_dat->tmp_fvvs.back().desc = idx_dat->idx_desc;
							_clearAndShrink(&idx_dat->idx_desc);
						}
						if (is_empty_str) is_empty_str = false;
						else value.str(""), value.clear();
						return is_str = false, false;
					} else if (idx_char == "{") {
						idx_dat->group_names.insert(idx_dat->group_names.end(),
													idx_dat->value_names.begin(),
													idx_dat->value_names.end());
						idx_dat->last_group_names.push_back(idx_dat->value_names),
								_clearAndShrink(&idx_dat->value_names);
						return ++idx_dat->group_num, idx_dat->in_value = false, false;
					} else if (!idx_dat->in_list && _eqOr(idx_char, strv(";"), strv("\n"))) {
						idx_dat->idx_key = _getKey(idx_dat->value_names,
												   _getKey(idx_dat->group_names, idx_dat->idx_key));
						if (idx_dat->is_list) {
							if (values.empty() && idx_dat->tmp_fvvs.empty()) *idx_dat->idx_key = FVVV();
							else if (!idx_dat->tmp_fvvs.empty()) *idx_dat->idx_key = idx_dat->tmp_fvvs;
							else if (is_all_str) *idx_dat->idx_key = values;
							else {
								str tmp_str = values.front();
								if (_eqOr(tmp_str, str("true"), str("false"))) {
									vec<bool> tmp;
									tmp.reserve(values.size());
									transform(
											values.begin(), values.end(), back_inserter(tmp), [](strv s) {
										return s == "true";
									});
									*idx_dat->idx_key = tmp;
								} else if (_isInt(tmp_str)) {
									vec<int> tmp;
									for (str const& str : values)
										if (_isInt(str)) tmp.push_back(stoi(str));
									*idx_dat->idx_key = tmp;
								} else if (_isDouble(tmp_str)) {
									vec<double> tmp;
									for (str const& str : values)
										if (_isDouble(str)) tmp.push_back(stod(str));
									*idx_dat->idx_key = tmp;
								}
							}
						} else {
							str const value_str = value.str();
							if (is_all_str) *idx_dat->idx_key = value_str;
							else if (_eqOr(value_str, str("true"), str("false")))
								*idx_dat->idx_key = value_str == "true";
							else if (_isInt(value_str)) *idx_dat->idx_key = stoi(value_str);
							else if (_isDouble(value_str)) *idx_dat->idx_key = stod(value_str);
							else if (FVVV* tmp_key = _findKey(value_str, fvv_stack); tmp_key) {
								if (tmp_key->sub.empty()) *idx_dat->idx_key = tmp_key->value;
								else idx_dat->idx_key->sub = tmp_key->sub;
								idx_dat->idx_key->link = value_str;
							}
						}
						idx_dat->idx_key->desc = idx_dat->idx_desc;
						value.str(""), value.clear();
						_clearAndShrink(
								&idx_dat->idx_desc, &values, &idx_dat->value_names, &idx_dat->tmp_fvvs);
						idx_dat->is_list = is_str = is_all_str = idx_dat->in_value = false;
						return false;
					} else return value << idx_char, false;
				} else if (!old_fvv && idx_char == "{" && idx_dat->value_name.str().empty())
					return old_fvv = true, false;
				else if (idx_char == "=") {
					idx_dat->value_names = _split(idx_dat->value_name.str(), '.');
					idx_dat->value_name.str(""), idx_dat->value_name.clear();
					return idx_dat->in_value = true, false;
				} else if (end_group && _eqOr(idx_char, strv(";"), strv("\n"))
						   && idx_dat->group_num > 0) {
					end_group = false;
					if (!idx_dat->idx_desc.empty()) {
						_getKey(idx_dat->group_names, idx_dat->idx_key)->desc = idx_dat->idx_desc;
						_clearAndShrink(&idx_dat->idx_desc);
					}
					for ([[maybe_unused]]
						 str const& _ : idx_dat->last_group_names.back())
						idx_dat->group_names.pop_back();
					idx_dat->last_group_names.pop_back(),
							_shrink(&idx_dat->group_names, &idx_dat->last_group_names);
					return --idx_dat->group_num, false;
				} else if (idx_char == "}") {
					if (!idx_dat->group_num)
						if (fvv_stack.size() > 1) {
							FVVV target_fvvv = fvv_stack.back().root_key;
							fvv_stack.pop_back();
							fvv_stack.back().tmp_fvvs.push_back(target_fvvv);
							return false;
						} else return true;
					else return end_group = true, false;
				} else return idx_dat->value_name << idx_char, false;
			});
		}

	   private:
		unsigned char static constexpr const _bom[] = { 0xEF, 0xBB, 0xBF };
		template<typename Tp>
		FVV_INLINE static const Tp& _getDfltVal() {
			if constexpr (is_same_v<Tp, bool>) {
				bool static constexpr const dfltBool = false;
				return dfltBool;
			} else if constexpr (is_same_v<Tp, int>) {
				int static constexpr const dfltInt = 0;
				return dfltInt;
			} else if constexpr (is_same_v<Tp, double>) {
				double static constexpr const dfltDouble = 0.0;
				return dfltDouble;
			} else if constexpr (is_same_v<Tp, str>) {
				str static const dfltStr = "";
				return dfltStr;
			} else if constexpr (is_same_v<Tp, vec<bool>>) {
				vec<bool> static const dfltBools = {};
				return dfltBools;
			} else if constexpr (is_same_v<Tp, vec<int>>) {
				vec<int> static const dfltInts = {};
				return dfltInts;
			} else if constexpr (is_same_v<Tp, vec<double>>) {
				vec<double> static const dfltDoubles = {};
				return dfltDoubles;
			} else if constexpr (is_same_v<Tp, vec<str>>) {
				vec<str> static const dfltStrs = {};
				return dfltStrs;
			} else if constexpr (is_same_v<Tp, vec<FVVV>>) {
				vec<FVVV> static const dfltFvvvs = {};
				return dfltFvvvs;
			}
			Tp static dfltTp {};
			return dfltTp;
		}
		template<typename T>
		FVV_INLINE static bool writeList(stringstream&							 result,
										 vec<T> const&							 list,
										 FormatOpt								 opt,
										 strv									 list_indent,
										 function<void(stringstream&, T const&)> printer) {
			for (auto const& value : list) {
				if (opt == FormatOpt::BigList) result << list_indent;
				printer(result, value);
				if (opt == FormatOpt::BigList) result << '\n';
				else {
					result << ',';
					if (opt != FormatOpt::Min) result << ' ';
				}
			}
			return list.empty();
		}
		template<typename Tp>
		FVV_INLINE static bool _eqOr(Tp a, Tp b) {
			return a == b;
		}
		template<typename Tp, typename... Args>
		FVV_INLINE static bool _eqOr(Tp a, Tp b, Args... args) {
			return a == b || _eqOr(a, args...);
		}
		FVV_INLINE static vec<str> _split(str const& path, char delimiter) {
			size_t start = path.find_first_not_of("\n");
			size_t end	 = path.find_last_not_of("\n");
			if (start == str::npos || end == str::npos) return {};
			str			 target = path.substr(start, end - start + 1);
			vec<str>	 result;
			stringstream ss(target);
			str			 item;
			while (getline(ss, item, delimiter)) result.push_back(item);
			return result;
		}
		FVV_INLINE static str  _replace(str s, strv f, strv t) { return _replaceBase(s, f, t), s; }
		FVV_INLINE static void _replaceBase(str& s, strv f, strv t) {
			size_t p = 0;
			while ((p = s.find(f, p)) != str::npos) s.replace(p, f.length(), t), p += t.length();
		}
		FVV_INLINE static bool _isInt(strv s) {
			if (s.empty()) return false;
			size_t start = 0;
			if (s[0] == '-' || s[0] == '+') {
				if (s.size() == 1) return false;
				start = 1;
			}
			return all_of(s.begin() + start, s.end(), ::isdigit);
		}
		FVV_INLINE static bool _isDouble(strv s) {
			if (s.empty()) return false;
			size_t start	= 0;
			bool   hasDigit = false, hasDot = false;
			if (s[0] == '-' || s[0] == '+') {
				if (s.size() == 1) return false;
				start = 1;
			}
			for (size_t i = start; i < s.size(); ++i) {
				char c = s[i];
				if (isdigit(c)) hasDigit = true;
				else if (c == '.') {
					if (hasDot) return false;
					hasDot = true;
				} else return false;
			}
			return hasDigit;
		}
		template<typename Tp>
		FVV_INLINE static void _shrink(Tp* container) {
			if (container) container->shrink_to_fit();
		}
		template<typename Tp, typename... Args>
		FVV_INLINE static void _shrink(Tp* container, Args... args) {
			_shrink(container), _shrink(args...);
		}
		template<typename Tp>
		FVV_INLINE static void _clearAndShrink(Tp* container) {
			if (container) container->clear(), container->shrink_to_fit();
		}
		template<typename Tp, typename... Args>
		FVV_INLINE static void _clearAndShrink(Tp* container, Args... args) {
			_clearAndShrink(container), _clearAndShrink(args...);
		}
		FVV_INLINE static void _removeLastChar(stringstream& ss) {
			if (str str = ss.str(); !str.empty()) str.pop_back(), ss.str(""), ss.clear(), ss << str;
		}
		FVV_INLINE static void _utf8ForEach(strv												target,
											size_t												size,
											function<bool(size_t const&, strv, uint8_t const&)> handler) {
			size_t i = 0;
			while (i < size) {
				unsigned char c			= target[i];
				uint8_t		  char_size = 0;
				if ((c & 0x80) == 0) char_size = 1;
				else if ((c & 0xE0) == 0xC0) char_size = 2;
				else if ((c & 0xF0) == 0xE0) char_size = 3;
				else if ((c & 0xF8) == 0xF0) char_size = 4;
				else char_size = 1;
				if (handler(i, target.substr(i, char_size), char_size)) break;
				i += char_size;
			}
		}
	};
} // namespace FVV