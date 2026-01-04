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
#include <array>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace FVV {
using namespace std;

// 继承自 STL 的 pair 与 vector 实现的保留插入顺序的 map
// 虽然继承 STL 很不好，但是能用就行

template<typename _key_type, typename _value_type>
struct KVPair : public pair<_key_type, _value_type> {
	using pair<_key_type, _value_type>::pair;
	inline _key_type& key(void) noexcept { return this->first; }
	constexpr inline _key_type const& key(void) const noexcept { return this->first; }
	inline _key_type&& key_rv(void) noexcept { return std::move(this->first); }
	inline _value_type& value(void) noexcept { return this->second; }
	constexpr inline _value_type const& value(void) const noexcept { return this->second; }
	inline _value_type&& value_rv(void) noexcept { return std::move(this->second); }
};
// 让 pair 支持 key 与 value 调用，由于直接定义引用在构造上会有些问题，所以只能定义成函数了

template<typename _key_type, typename _value_type>
struct KVList : public vector<KVPair<_key_type, _value_type>> {
	using _kv_type		 = KVPair<_key_type, _value_type>;
	using iterator		 = typename vector<_kv_type>::iterator;
	using const_iterator = typename vector<_kv_type>::const_iterator;

	inline vector<_kv_type>&& data_rv(void) noexcept { return std::move(*this); }

	inline _value_type& operator[](_key_type const& key) {
		iterator iter = find_if(
				this->begin(), this->end(), [&key](_kv_type const& kv) { return kv.key() == key; });
		if (iter != this->end()) return iter->value();
		return this->emplace_back(key, _value_type()), this->back().value();
	}
	inline _key_type& operator()(_value_type const& value) {
		iterator iter = find_if(
				this->begin(), this->end(), [&value](_kv_type const& kv) { return kv.value() == value; });
		if (iter != this->end()) return iter->key();
		return this->emplace_back(_key_type(), value), this->back().key();
	}

	inline bool contains_key(_key_type const& key) const {
		return any_of(this->begin(), this->end(), [&key](_kv_type const& kv) { return kv.key() == key; });
	}
	inline bool contains_value(_value_type const& value) const {
		return any_of(
				this->begin(), this->end(), [&value](_kv_type const& kv) { return kv.value() == value; });
	}
	inline const_iterator find_key(_key_type const& key) const {
		return find_if(
				this->begin(), this->end(), [&key](_kv_type const& kv) { return kv.key() == key; });
	}
	inline const_iterator find_value(_value_type const& value) const {
		return find_if(
				this->begin(), this->end(), [&value](_kv_type const& kv) { return kv.value() == value; });
	}
	inline void erase_key(_key_type const& key) {
		this->erase(remove_if(this->begin(), this->end(),
							[&key](_kv_type const& kv) { return kv.key() == key; }),
				this->end());
	}
	inline vector<_key_type> keys(void) const {
		vector<_key_type> rets;
		rets.reserve(this->size());
		transform(this->begin(), this->end(), back_inserter(rets),
				[](_kv_type const& kv) { return kv.key(); });
		return rets;
	}
	inline void erase_value(_value_type const& value) {
		this->erase(remove_if(this->begin(), this->end(),
							[&value](_kv_type const& kv) { return kv.value() == value; }),
				this->end());
	}
	inline vector<_value_type> values(void) const {
		vector<_value_type> rets;
		rets.reserve(this->size());
		transform(this->begin(), this->end(), back_inserter(rets),
				[](_kv_type const& kv) { return kv.value(); });
		return rets;
	}
	inline void sort(function<bool(_kv_type const&, _kv_type const&)> func = nullptr) {
		static function<bool(_kv_type const&, _kv_type const&)> const dflt
				= [](_kv_type const& a, _kv_type const& b) { return a.key() < b.key(); };
		std::sort(this->begin(), this->end(), func ? func : dflt);
	}
};
// 基于 vector 实现的 map，支持一些常用函数

// 格式化选项
struct FormatOpt {
	enum : unsigned long {
		Common = 0,					  // 什么也没有，默认配置

		UseWrapper = 1 << 0,		  // 最外面外面包裹一层花括号

		Minify = 1 << 1,			  // 最小化，移除所有缩进、空格、换行

		UseCRLF = 1 << 2,			  // \r\n 换行，默认 \n
		UseCR	= 1 << 3,			  // \r 换行，默认 \n

		UseSpace2 = 1 << 4,			  // 使用 2 个空格作为缩进，默认是 Tab
		UseSpace4 = 1 << 5,			  // 使用 4 个空格作为缩进，默认是 Tab

		IntBinary = 1 << 6,			  // 输出整数为 二进制 格式，默认是十进制
		IntOctal  = 1 << 7,			  // 输出整数为 八进制 格式，默认是十进制
		IntHex	  = 1 << 8,			  // 输出整数为 十六进制 格式，默认是十进制

		DigitSep3 = 1 << 9,			  // 输出整数或浮点数的整数部分时 每三个数字 插入一个分隔符
		DigitSep4 = 1 << 10,		  // 输出整数或浮点数的整数部分时 每四个数字 插入一个分隔符

		UseColon  = 1 << 11,		  // 使用 冒号 而非默认的 等号 作为定义符
		FullWidth = 1 << 12,		  // 将部分符号改为 全角

		KeepListSingle	   = 1 << 13, // 强制 列表 保持为一行
		ForceUseSeparator  = 1 << 14, // 强制添加 分隔符，最小化时无效
		RawMultilineString = 1 << 15, // 当 字符串 有多行时改为 有缩进的原始字符串，最小化时无效

		NoDescs		 = 1 << 16,		  // 移除所有描述，在 FWW 风格 下对 FVV 值列表 与 组 无效
		NoLinks		 = 1 << 17,		  // 移除所有链接
		FlattenPaths = 1 << 18,		  // 递归展平只有 一个值 的 组
		FWWStyle	 = 1 << 19,		  // FWW 风格，前置 组 的 描述
	};
};

class FVVV {
   private:
	// 值的类型
	enum class types : unsigned char {
		none,		 // 空值
		boolean,	 // 布尔值
		integer,	 // 整数
		float_point, // 浮点数
		text,		 // 文本
		list,		 // 列表
		fwv			 // 组
	};

	// 判断类型是否为 vector
	template<typename ValueType>
	struct _is_list : false_type {};
	template<typename ValueType, typename Allocator>
	struct _is_list<vector<ValueType, Allocator>> : true_type {};

	// 为从 C++ 17 的 variant 降级至 C++ 11 设计的类任意类
	class ValueData {
		// 设置友元以使用私有函数（但是 ValueData 已经是 FVVV 的私有类了？？？）
		friend class FVVV;

	   public:
		types type = types::none;

	   private:
		// 基本值直接存，类用指针存
		union _data_type {
			bool			   _boolean;
			long long		   _integer;
			double			   _float_point;
			string*			   _text;
			vector<ValueData>* _list;
			FVVV*			   _fwv;
		} _data;

	   public:
		inline ValueData(void) noexcept: type(types::none), _data() {}
		inline explicit ValueData(bool target) noexcept: type(types::boolean), _data() {
			_data._boolean = target;
		}
		template<typename ValueType,
				typename enable_if<is_integral<ValueType>::value && !is_same<ValueType, bool>::value,
						int>::type
				= 0>
		inline explicit ValueData(ValueType target) noexcept: type(types::integer), _data() {
			_data._integer = static_cast<long long>(target);
		}
		template<typename ValueType,
				typename enable_if<is_floating_point<ValueType>::value, int>::type = 0>
		inline explicit ValueData(ValueType target) noexcept: type(types::float_point), _data() {
			_data._float_point = static_cast<double>(target);
		}
		inline explicit ValueData(string const& target): type(types::text), _data() {
			_data._text = new string(target);
		}
		inline explicit ValueData(string&& target): type(types::text), _data() {
			_data._text = new string(std::move(target));
		}
		inline explicit ValueData(vector<ValueData> const& target): type(types::list), _data() {
			_data._list = new vector<ValueData>(target);
		}
		inline explicit ValueData(vector<ValueData>&& target): type(types::list), _data() {
			_data._list = new vector<ValueData>(std::move(target));
		}
		inline explicit ValueData(FVVV const& target): type(types::fwv), _data() {
			_data._fwv = new FVVV(target);
		}
		inline explicit ValueData(FVVV&& target): type(types::fwv), _data() {
			_data._fwv = new FVVV(std::move(target));
		}
		template<typename ValueType,
				typename enable_if<!is_same<typename decay<ValueType>::type, string>::value
										   && !is_same<typename decay<ValueType>::type, ValueData>::value
										   && is_constructible<string, ValueType>::value
										   && !is_same<typename decay<ValueType>::type, nullptr_t>::value,
						int>::type
				= 0>
		inline explicit ValueData(ValueType&& target): type(types::text), _data() {
			_data._text = new string(std::forward<ValueType>(target));
		}
		inline ValueData(char const* target, size_t len): type(types::text), _data() {
			_data._text = new string(target, len);
		}
		template<typename ValueType,
				typename enable_if<!is_same<ValueType, ValueData>::value, int>::type = 0>
		inline explicit ValueData(vector<ValueType> const& target): type(types::list), _data() {
			_data._list = new vector<ValueData>();
			_data._list->reserve(target.size());
			transform(target.begin(), target.end(), back_inserter(*_data._list),
					[](ValueType const& item) { return ValueData(item); });
		}
		template<typename ValueType,
				typename enable_if<!is_same<ValueType, ValueData>::value, int>::type = 0>
		inline explicit ValueData(vector<ValueType>&& target): type(types::list), _data() {
			_data._list = new vector<ValueData>();
			_data._list->reserve(target.size());
			transform(make_move_iterator(target.begin()), make_move_iterator(target.end()),
					back_inserter(*_data._list),
					[](ValueType&& item) { return ValueData(std::move(item)); });
		}

		inline ValueData(ValueData const& target): type(target.type), _data() {
			switch (this->type) {
				case types::text: this->_data._text = new string(*target._data._text); break;
				case types::list: this->_data._list = new vector<ValueData>(*target._data._list); break;
				case types::fwv : this->_data._fwv = new FVVV(*target._data._fwv); break;
				default			: this->_data = target._data;
			}
		}
		inline ValueData(ValueData&& target) noexcept: type(types::none), _data() { this->_swap(target); }

		inline ValueData& operator=(ValueData const& target) {
			if (this != &target) {
				ValueData tmp(target);
				this->_swap(tmp);
			}
			return *this;
		}
		// cppcheck-suppress operatorEqRetRefThis
		inline ValueData& operator=(ValueData&& target) noexcept { return this->_swap(target), *this; }
		template<typename ValueType,
				typename enable_if<!is_same<typename decay<ValueType>::type, ValueData>::value, int>::type
				= 0>
		// cppcheck-suppress operatorEqRetRefThis
		inline ValueData& operator=(ValueType&& target) {
			ValueData tmp(std::forward<ValueType>(target));
			return this->_swap(tmp), *this;
		}
		inline bool operator==(ValueData const& other) const noexcept {
			// 地址判断
			if (this == &other) return true;
			// 类型判断
			if (this->type != other.type) return false;
			switch (this->type) {
				// 基本值直接判断
				case types::boolean	   : return this->_data._boolean == other._data._boolean;
				case types::integer	   : return this->_data._integer == other._data._integer;
				case types::float_point: return this->_data._float_point == other._data._float_point;
				// 指针解引判断
				case types::text: return *(this->_data._text) == *(other._data._text);
				case types::list: return *(this->_data._list) == *(other._data._list);
				case types::fwv : return *(this->_data._fwv) == *(other._data._fwv);
				default			: return true;
			}
		}
		inline bool operator!=(ValueData const& target) const noexcept {
			// != 调用 == 判断
			return this != &target && !(*this == target);
		}

		inline ~ValueData(void) noexcept { clear(); }
		inline void clear(void) noexcept {
			// 销毁时清理非基本值
			switch (this->type) {
				case types::text: delete this->_data._text; break;
				case types::list: delete this->_data._list; break;
				case types::fwv : delete this->_data._fwv; break;
				default			: break;
			}
			this->type = types::none;
		}

	   private:
		template<typename ValueType>
		friend struct _getter;
		template<typename ValueType>
		struct _getter {
			// cppcheck-suppress unusedStructMember
			constexpr static types const type = types::none;
			static inline ValueType get(union _data_type const&) { return ValueType{}; }
		};

	   public:
		template<typename ValueType, typename enable_if<!_is_list<ValueType>::value, int>::type = 0>
		inline bool is(void) const noexcept {
			return this->type == _getter<ValueType>::type;
		}
		template<typename ListType, typename ValueType = typename ListType::value_type,
				typename enable_if<_is_list<ListType>::value, int>::type = 0>
		inline bool is(void) const noexcept {
			// 以第一个值的类型作为判断依据，因为不支持混合类型与空列表
			return this->type == types::list && _data._list->size()
				&& _data._list->front().type == _getter<ValueType>::type;
		}
		template<typename ValueType, typename enable_if<!_is_list<ValueType>::value, int>::type = 0>
		inline ValueType get(void) const {
			return this->is<ValueType>() ? _getter<ValueType>::get(_data) : ValueType{};
		}
		template<typename ListType, typename ValueType = typename ListType::value_type,
				typename enable_if<_is_list<ListType>::value, int>::type = 0>
		inline ListType get(void) const {
			if (this->type != types::list) return ListType{};
			ListType rets;
			rets.reserve(_data._list->size());
			transform(_data._list->begin(), _data._list->end(), back_inserter(rets),
					[&](ValueData const& index_value) -> ValueType {
				return index_value.get<ValueType>();
			});
			return rets;
		}
		inline bool empty(void) const noexcept {
			switch (this->type) {
				case types::none:
					return true; // 空值为空

				// 文本与列表则按其自身的判空
				case types::text: return this->_data._text->empty();
				case types::list: return this->_data._list->empty();

				default: return false; // 布尔值、整数、浮点数不为空，不存在组判空
			}
		}

	   private:
		inline void _swap(ValueData& target) noexcept {
			std::swap(this->type, target.type);
			std::swap(this->_data, target._data);
		}

		inline string _to_string(void) const {
			switch (this->type) {
				case types::text	   : return this->get<string>();
				case types::boolean	   : return this->get<bool>() ? "true" : "false";
				case types::integer	   : return std::to_string(this->get<long long>());
				case types::float_point: {
					double val = this->get<double>();

					constexpr static char const* const fmt = "%.15g";
#ifdef _MSC_VER
					int len = _scprintf(fmt, val);
#else
					int len = snprintf(nullptr, 0, fmt, val);
#endif
#if false
					if (len <= 0) return "";
#endif
					vector<char> buf(len + 1);
#ifdef _MSC_VER
					_snprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, val);
#else
					snprintf(buf.data(), buf.size(), fmt, val);
#endif
					return string(buf.data(), len);
				}
				default: return "";
			}
		}
		inline vector<ValueData>& _get_list(void) const noexcept { return *_data._list; }
	};

   private:
	ValueData _value;

   public:
	/// @brief 子节点
	KVList<string, FVVV> nodes;
	/// @brief 描述
	string desc;
	/// @brief 链接名称
	string link;

	inline FVVV(void) noexcept = default;
	inline FVVV(FVVV const& tgt): _value(tgt._value), nodes(tgt.nodes), desc(tgt.desc), link(tgt.link) {}
	inline FVVV(FVVV&& tgt) noexcept { this->_swap(tgt); }
	template<typename ValueType,
			typename enable_if<!is_same<typename decay<ValueType>::type, FVVV>::value, int>::type = 0>
	inline explicit FVVV(ValueType&& tgt): _value(std::forward<ValueType>(tgt)) {}
	inline FVVV(char const* tgt, size_t len): _value(tgt, len) {}

	inline FVVV& operator=(FVVV const& tgt) {
		if (this != &tgt) {
			FVVV tmp(tgt);
			this->_swap(tmp);
		}
		return *this;
	}
	// cppcheck-suppress operatorEqRetRefThis
	inline FVVV& operator=(FVVV&& tgt) noexcept { return this->_swap(tgt), *this; }
	template<typename ValueType,
			typename enable_if<!is_same<typename decay<ValueType>::type, FVVV>::value, int>::type = 0>
	// cppcheck-suppress operatorEqRetRefThis
	inline FVVV& operator=(ValueType&& tgt) {
		return _value = std::forward<ValueType>(tgt), *this;
	}

	inline bool operator==(FVVV const& other) const noexcept {
		// 注释与赋值不考虑进判断范围内
		return this == &other || (_value == other._value && nodes == other.nodes);
	}
	inline bool operator!=(FVVV const& other) const noexcept {
		// != 调用 == 判断
		return this != &other && !(*this == other);
	}
	inline FVVV& operator[](string const& key) {
		vector<string> const paths = _split_name(key);
		// 支持路径分隔地递归
		return *accumulate(paths.begin(), paths.end(), this,
				[](FVVV* tgt, string const& path) { return &tgt->nodes[path]; });
	}

	inline bool empty(void) const noexcept { return _value.empty(); }
	template<typename ValueType>
	inline bool is(void) const noexcept {
		return _value.is<ValueType>();
	}
	template<typename ValueType>
	inline ValueType value(ValueType const& defaultValue = ValueType{}) const {
		return _value.is<ValueType>() ? _value.get<ValueType>() : defaultValue;
	}
	template<typename ValueType>
	inline vector<ValueType> list(vector<ValueType> const& defaultValue = {}) const {
		return _value.is<vector<ValueType>>() ? _value.get<vector<ValueType>>() : defaultValue;
	}

	inline void unlink(void) noexcept {
		link.clear(), link.shrink_to_fit();
		for_each(nodes.begin(), nodes.end(),
				[](decltype(nodes)::value_type& item) { item.value().unlink(); });
	}

	inline string parse(string const& text) {
		if (_trim(text).empty()) return "";

		TextCtx		  ctx(text);
		vector<FVVV*> scope_stack;
		scope_stack.reserve(1);

		ctx.skip_blanks();
		bool   has_wrapper = ctx.match_any('{', "｛");
		string err		   = _parse_main(ctx, scope_stack);
		if (!err.empty()) return err;

		if (has_wrapper && (ctx.skip_blanks(), !ctx.match_any('}', "｝")))
			return ctx.err.NotFound("wrapper");
		if (ctx.skip_blanks(), !ctx.is_eof()) return ctx.err.WhyNotEOF();

		return "";
	}

	template<typename... Args>
	inline string to_string(Args... args) const {
		string	  ret;
		FormatCtx ctx(_merge_flags(args...));

		if (ctx.use_wrapper) {
			ret += ctx.fwv_begin;
			if (!ctx.minify) ret += ctx.newline;
		}
		this->_to_string_root(ctx, ret, ctx.use_wrapper);
		if (ctx.use_wrapper) {
			if (!ctx.minify) ret += ctx.newline;
			ret += ctx.fwv_end;
		}

		return ret.shrink_to_fit(), ret;
	}

	template<typename TgtType>
	inline string parse(string const& text, TgtType& target) {
		string err = parse(text);
		if (!err.empty()) return err;
		to(target);
		return "";
	}

	template<typename TgtType>
	inline void to(TgtType& target) const {
		_to(target);
	}

	template<typename TgtType>
	inline void from(TgtType const& target) {
		this->unlink(); // 从外部赋值时移除链接
		_from(target);
	}

   private:
	struct TextCtx {
		string const& input;
		size_t		  index;

		stack<size_t> lines_start;

		inline explicit TextCtx(string const& str): input(str), index(0), err(*this) {
			lines_start.push(0);
			constexpr static char const	  bom[]	  = "\xEF\xBB\xBF";
			constexpr static size_t const bom_len = sizeof(bom) - 1;
			if (input.length() >= bom_len)
				if (!input.compare(0, bom_len, bom)) index += bom_len;
		}

		inline char preview(void) const noexcept { return is_eof() ? '\0' : input[index]; }
		inline bool prematch(char tgt) const noexcept { return preview() == tgt; }
		template<size_t raw_len>
		inline bool prematch(char const (&tgt)[raw_len]) const noexcept {
			constexpr size_t const len = raw_len - 1;
			return index + len <= input.length() && !input.compare(index, len, tgt);
		}
		template<typename TgtType>
		inline bool prematch(TgtType const& tgt) const noexcept {
			return prematch(tgt);
		}
		template<typename TgtType, typename... Args>
		inline bool prematch(TgtType const& tgt, Args&&... tgts) const noexcept {
			return prematch(tgt) || prematch(std::forward<Args>(tgts)...);
		}

		inline char next(void) {
			if (is_eof()) return '\0';
			char ch = input[index++];
			if (ch == '\r') lines_start.push(index);
			else if (ch == '\n') {
				if (index >= 2 && input[index - 2] == '\r') lines_start.top() = index; // \r\n 时后移
				else lines_start.push(index);
			}
			return ch;
		}

		inline bool match(char tgt, bool skip_blanks = true, bool same_line = false) {
			if (skip_blanks) this->skip_blanks(same_line);
			return prematch(tgt) && next();
		}
		template<size_t raw_len>
		inline bool match(char const (&tgt)[raw_len], bool skip_blanks = true, bool same_line = false) {
			constexpr size_t const len = raw_len - 1;
			if (skip_blanks) this->skip_blanks(same_line);
			return prematch(tgt) && (index += len);
		}
		template<typename TgtType>
		inline bool match_any(TgtType const& tgt) {
			return match(tgt);
		}
		template<typename TgtType, typename... Args>
		inline bool match_any(TgtType const& tgt, Args&&... tgts) {
			return match(tgt) || match_any(std::forward<Args>(tgts)...);
		}

		inline void skip_blanks(bool same_line = false) {
			while (!is_eof() && isspace(static_cast<unsigned char>(preview())))
				if (same_line && prematch('\n', '\r')) break;
				else next();
		}

		inline bool is_eof(void) const noexcept { return index >= input.length(); }
		inline bool is_same_line(void) {
			return lines_start.size() == (skip_blanks(), lines_start.size());
		}

		struct ErrHandler {
		   private:
			TextCtx& _ctx;
			static inline void _build_string(stringstream&) noexcept {}
			template<typename TgtType, typename... Args>
			static inline void _build_string(stringstream& ss, TgtType const& arg, Args&&... args) {
				ss << arg, _build_string(ss, std::forward<Args>(args)...);
			}
			template<typename... Args>
			inline string _make_error(Args&&... args) const {
				stringstream ss;
				ss << _ctx.lines_start.size() << ":" << _ctx.index - _ctx.lines_start.top() + 1 << ": ";
				_build_string(ss, std::forward<Args>(args)...);
				return ss.str();
			}

		   public:
			inline explicit ErrHandler(TextCtx& context): _ctx(context) {}
			inline string Unknown(void) const { return _make_error("Why??? IDK!!!"); }
			inline string WhyEOF(void) const { return _make_error("Why EOF???"); }
			inline string WhyNotEOF(void) const { return _make_error("Why not EOF???"); }
			inline string NotFound(char tgt) const { return _make_error("Where is the '", tgt, "'?"); }
			inline string NotFound(char const* tgt) const {
				return _make_error("Where is the ", tgt, "?");
			}
			inline string NoValue(string const& tgt) const {
				return _make_error("Cannot find the value of '", tgt, "'");
			}
			inline string PlusList(void) const { return _make_error("Why plus with list?"); }
			inline string ValuePlusFVVV(void) const { return _make_error("Why value plus with FVVV?"); }
		};

		ErrHandler err;
	};
	struct FormatCtx {
		string newline	   = "\n";
		string indent_unit = "\t";
		string assign_op   = " = ";
		string list_begin = "[", list_end = "]";
		string fwv_begin = "{", fwv_end = "}";
		string item_sep = ",", stmt_sep = ";";

		unsigned char int_base = 10;

		size_t digit_sep_step = 0;
		string digit_sep_char;

		bool use_wrapper = false;

		bool minify = false;

		bool full_width = false;

		bool list_single = false;
		bool force_sep	 = false;
		bool raw_str	 = false;

		bool no_descs = false, no_links = false;
		bool flatten_paths = false;
		bool fww_style	   = false;

		inline explicit FormatCtx(unsigned long flags) {
			if (flags & FormatOpt::UseWrapper) use_wrapper = true;

			if (flags & FormatOpt::UseCRLF) newline = "\r\n";
			else if (flags & FormatOpt::UseCR) newline = "\r";

			if (flags & FormatOpt::UseSpace2) indent_unit = "  ";
			else if (flags & FormatOpt::UseSpace4) indent_unit = "    ";

			if (flags & FormatOpt::IntHex) int_base = 16;
			else if (flags & FormatOpt::IntOctal) int_base = 8;
			else if (flags & FormatOpt::IntBinary) int_base = 2;

			if (flags & FormatOpt::DigitSep3) digit_sep_step = 3;
			else if (flags & FormatOpt::DigitSep4) digit_sep_step = 4;

			if ((full_width = flags & FormatOpt::FullWidth)) {
				if (flags & FormatOpt::UseColon) assign_op = "：";
				list_begin = "［", list_end = "］";
				fwv_begin = "｛", fwv_end = "｝";
				item_sep = "，", stmt_sep = "；";
				if (digit_sep_step) digit_sep_char = "’";
			} else {
				if (flags & FormatOpt::UseColon) assign_op = ": ";
				if (digit_sep_step) digit_sep_char = "'";
			}

			list_single = flags & FormatOpt::KeepListSingle;
			force_sep	= flags & FormatOpt::ForceUseSeparator;
			raw_str		= flags & FormatOpt::RawMultilineString;

			no_descs = flags & FormatOpt::NoDescs, no_links = flags & FormatOpt::NoLinks;
			flatten_paths = flags & FormatOpt::FlattenPaths;
			fww_style	  = flags & FormatOpt::FWWStyle;

			if ((minify = flags & FormatOpt::Minify)) {
				newline.clear(), indent_unit.clear();

				assign_op = _trim(assign_op); // 清理左右空格
			}
		}
	};

	template<typename...>
	using _void = void;
	template<typename StructType, typename = void>
	struct _is_fvv_struct : false_type {};
	template<typename StructType>
	struct _is_fvv_struct<StructType,
			_void<decltype(declval<StructType>().fvv_register(declval<struct AnyBinder&>()))>>
		: true_type {};

	struct ReadBinder {
		FVVV const& node;
		inline explicit ReadBinder(FVVV const& node): node(node) {}
		template<typename ValueType>
		inline void operator()(string const& key, ValueType& value) {
			// 不想写类型名，也不想写 auto
			decltype(node.nodes)::const_iterator iter = node.nodes.find_key(key);
			if (iter != node.nodes.end()) iter->value().to(value);
		}
	};
	struct WriteBinder {
		FVVV& node;
		inline explicit WriteBinder(FVVV& node): node(node) {}
		template<typename ValueType>
		inline void operator()(string const& key, ValueType const& value) {
			node[key].from(value);
		}
	};

   private:
	inline string _parse_main(TextCtx& ctx, vector<FVVV*>& scope_stack) {
		scope_stack.push_back(this);
		string err;

		for (;;) {
			string idx_desc;
			if (!(err = _parse_desc(ctx, idx_desc, scope_stack, false)).empty())
				return scope_stack.pop_back(), err;	   // 不跳过最后一个注释后的空白
			if (!ctx.is_same_line()) idx_desc.clear(); // 清理与值不在同一行的注释

			if (ctx.is_eof() || ctx.prematch('}', "｝"))
				break; // 块结尾可能是最外层包装或组，预匹配以交由外部处理

			string name = _parse_name(ctx);
			if (name.empty()) return scope_stack.pop_back(), ctx.err.NotFound("name");
			if (!(err = _parse_desc(ctx, idx_desc, scope_stack)).empty())
				return scope_stack.pop_back(), err;
			if (!ctx.match_any('=', ':', "：")) return scope_stack.pop_back(), ctx.err.NotFound('=');
			if (!(err = _parse_desc(ctx, idx_desc, scope_stack)).empty())
				return scope_stack.pop_back(), err;

			FVVV* tgt_key = &((*this)[name]);
			if (ctx.match_any('[', "［")) {
				vector<ValueData> tgt_list;
				types			  list_type = types::none;
				for (;;) {
					string value_desc;
					if (!(err = _parse_desc(ctx, value_desc, scope_stack, false)).empty())
						return scope_stack.pop_back(), err;
					if (!ctx.is_same_line()) value_desc.clear();

					if (ctx.is_eof()) return scope_stack.pop_back(), ctx.err.WhyEOF();
					if (ctx.match_any('{', "｛")) {
						list_type = types::fwv;
						FVVV tmp_value;
						if (!(err = tmp_value._parse_main(ctx, scope_stack)).empty())
							return scope_stack.pop_back(), err;
						if (!ctx.match_any('}', "｝"))
							return scope_stack.pop_back(), ctx.err.NotFound('}');
						if (!(err = _parse_desc(ctx, value_desc, scope_stack, false, true)).empty())
							return scope_stack.pop_back(), err; // 只解析同行注释以避免串行
						tmp_value.desc = value_desc;
						tgt_list.emplace_back(std::move(tmp_value));
						if (ctx.is_same_line() && !ctx.match_any(',', "，") && !ctx.prematch(']', "］"))
							return scope_stack.pop_back(), ctx.err.NotFound("EOL");
						// 预匹配列表结尾以交由循环末尾处理
					} else {
						FVVV tgt_fwv;
						if (!(err = _parse_value(ctx, scope_stack, tgt_fwv, idx_desc, true)).empty())
							return scope_stack.pop_back(), err; // 解析值时会预匹配列表结尾

						if (tgt_fwv._value.type == types::list) {
							vector<ValueData> tmp_list = tgt_fwv._value._get_list();
							tgt_list.reserve(tgt_list.size() + tmp_list.size());
							tgt_list.insert(tgt_list.end(), make_move_iterator(tmp_list.begin()),
									make_move_iterator(tmp_list.end()));
						} else if (tgt_fwv._value.type != types::none)
							tgt_list.emplace_back(std::move(tgt_fwv._value));
						else tgt_list.emplace_back(std::move(tgt_fwv));

						if (list_type == types::none) list_type = tgt_list.back().type;
						else if (list_type != tgt_list.back().type) {
							if (list_type == types::fwv || tgt_list.back().type == types::fwv)
								return scope_stack.pop_back(), ctx.err.ValuePlusFVVV(); // 不允许混合类型
							switch (tgt_list.back().type) { // 字符串 > 浮点数 > 整数 > 布尔值
								case types::text: list_type = types::text; break;
								case types::float_point:
									if (list_type != types::text) list_type = types::float_point;
									break;
								case types::integer:
									if (list_type != types::text && list_type != types::float_point)
										list_type = types::integer;
									break;
								default: break;
							}
						}
					}
					if (ctx.match_any(']', "］")) break; // 统一匹配
				}
				if (list_type != types::fwv) {
					for_each(tgt_list.begin(), tgt_list.end(), [list_type](ValueData& item) {
						if (item.type == list_type) return;
						switch (list_type) { // 直接提升至列表中的最高类型
							case types::text: item = item._to_string(); break;
							case types::float_point:
								if (item.type == types::integer)
									item = static_cast<double>(item.get<long long>());
								else if (item.type == types::boolean)
									item = static_cast<double>(item.get<bool>());
								break;
							case types::integer:
								if (item.type == types::boolean)
									item = static_cast<long long>(item.get<bool>());
								break;
							default: break;
						}
					});
				}
				tgt_key->_value = tgt_list;
			} else if (ctx.match_any('{', "｛")) {
				if (!(err = tgt_key->_parse_main(ctx, scope_stack)).empty())
					return scope_stack.pop_back(), err;
				if (!ctx.match_any('}', "｝")) return scope_stack.pop_back(), ctx.err.NotFound('}');
			} else {
				if (!(err = _parse_value(ctx, scope_stack, *tgt_key, idx_desc)).empty())
					return scope_stack.pop_back(), err;
				goto set_desc;
			}

			if (!(err = _parse_desc(ctx, idx_desc, scope_stack, false, true)).empty())
				return scope_stack.pop_back(), err; // 只解析同行注释以避免串行
			if (ctx.is_same_line() && !ctx.is_eof() && !ctx.match_any(';', "；")
					&& !ctx.prematch('}', "｝"))
				return scope_stack.pop_back(), ctx.err.NotFound("EOL");
		set_desc:
			tgt_key->desc = idx_desc;
		}

		return scope_stack.pop_back(), "";
	}
	static inline string _parse_name(TextCtx& ctx) {
		ctx.skip_blanks();
		string name;
		while (!ctx.is_eof() && !ctx.prematch('=', ':', "：", '<')) // 注释会中断解析名称
			name += ctx.next();
		return name.empty() ? "" : (_trim_right(name), name);
	}
	static inline string _parse_value(TextCtx& ctx, vector<FVVV*> const& scope_stack, FVVV& tgt_fwv,
			string& idx_desc, bool in_list = false) {
		for (;;) {
			string err;

			if (!(err = _parse_desc(ctx, idx_desc, scope_stack, in_list, !in_list)).empty()) return err;
			if (ctx.is_eof()
					|| (in_list ? ctx.match_any(',', "，") || ctx.prematch(']', "］")
								: !ctx.is_same_line() || ctx.match_any(';', "；")
											|| ctx.prematch('}', "｝")))
				return ctx.err.NotFound("value");

			string tmp_str;
			if (ctx.prematch('"', "“", '`')) {
				if (!(err = _parse_text(ctx, tmp_str)).empty()) return err;
				tgt_fwv._value = tgt_fwv._value.type == types::none
									   ? tmp_str
									   : (tgt_fwv.link.clear(), tgt_fwv._value._to_string() + tmp_str);
				// 拼接时移除链接
			} else {
				while (!ctx.is_eof() && !ctx.prematch('<', '+') && !ctx.prematch('\r', '\n'))
					if (in_list ? ctx.prematch(',', "，", ']', "］") : ctx.prematch(';', "；", '}', "｝"))
						break;
					else tmp_str += ctx.next();
				_trim_right(tmp_str);
				if (tmp_str.empty()) return ctx.err.NotFound("value");

				bool is_true = _iequals(tmp_str, "true");
				if (is_true || _iequals(tmp_str, "false"))
					tgt_fwv._value = tgt_fwv._value.type == types::none
										   ? ValueData(is_true)
										   : (tgt_fwv.link.clear(),
													 ValueData(tgt_fwv._value._to_string() + tmp_str));
				// 拼接时移除链接
				else {
					ValueData tmp_value;
					if (_try_parse_number(tmp_value, tmp_str))
						tgt_fwv._value
								= tgt_fwv._value.type == types::none
										? tmp_value
										: (tgt_fwv.link.clear(),
												  ValueData(tgt_fwv._value._to_string() + tmp_str));
					// 拼接时移除链接
					else {
						FVVV const* target = _find_key(tmp_str, scope_stack);
						if (target) {
							if (tgt_fwv._value.type != types::none && target->_value.type == types::list)
								return ctx.err.PlusList(); // 常规赋值仅允许基本类型与组
							if (tgt_fwv._value.type == types::none) {
								tgt_fwv.link   = tmp_str;
								tgt_fwv._value = target->_value;
							} else {
								tgt_fwv.link.clear(); // 拼接时移除链接
								tgt_fwv._value = ValueData(
										tgt_fwv._value._to_string() + target->_value._to_string());
							}
							tgt_fwv.nodes = target->nodes;
						} else return ctx.err.NoValue(tmp_str);
					}
				}
			}
			if (!(err = _parse_desc(ctx, idx_desc, scope_stack, false, true)).empty())
				return err; // 只解析同行注释以避免串行
			if (ctx.is_eof() || !ctx.is_same_line()
					|| (in_list ? ctx.match_any(',', "，") || ctx.prematch(']', "］")
								: ctx.match_any(';', "；") || ctx.prematch('}', "｝")))
				return "";

			if (ctx.match('+')) continue;
			else return ctx.err.NotFound('+');
		}
	}
	static inline string _parse_desc(TextCtx& ctx, string& desc, vector<FVVV*> const& scope_stack,
			bool skip_blanks = true, bool same_line = false) {
		for (;;) {
			size_t orig_idx = ctx.index, orig_line = ctx.lines_start.size();
			if (!ctx.match('<', true, same_line)) {
				if (!skip_blanks) {
					ctx.index = orig_idx;
					while (ctx.lines_start.size() > orig_line) ctx.lines_start.pop();
				}
				break;
			}

			desc.clear();
			for (;;) {
				if (ctx.is_eof()) return ctx.err.WhyEOF();
				if (ctx.match('>', false)) {
					FVVV* target = _find_key(desc, scope_stack);
					if (target && target->is<string>())
						desc = target->_value.get<string>(); // 只赋值字符串类型，避免过于宽泛
					break;
				}
				if (ctx.match('\\', false)) {
					if (ctx.is_eof()) return ctx.err.WhyEOF();
					if (ctx.match('>', false)) desc += '>';
					else {
						char ch = ctx.next(), tgt = _escape_table[static_cast<unsigned char>(ch)];
						if (tgt) desc += tgt;
						else desc += '\\', desc += ch;
					}
				} else desc += ctx.next();
			}
		}
		return "";
	}
	static inline string _parse_text(TextCtx& ctx, string& text) {
		if (ctx.match('`')) {
			for (;;) {
				if (ctx.is_eof()) return ctx.err.WhyEOF();
				if (ctx.match('`', false)) break;
				text += ctx.next();
			}
			return text = _trim(_trim_indent(text)), "";
		}

		bool is_full_width = ctx.match("“"); // 只在字符串上区分全角与半角引号，避免字符串使用困难
		if (!is_full_width && !ctx.match('"')) return ctx.err.Unknown();
		for (;;) {
			if (ctx.is_eof()) return ctx.err.WhyEOF();
			if (is_full_width ? ctx.match("”", false) : ctx.match('"', false)) return "";
			if (ctx.match('\\', false)) {
				if (ctx.is_eof()) return ctx.err.WhyEOF();
				if (is_full_width && ctx.match("”", false)) text += "”";
				else if (!is_full_width && ctx.match('"', false)) text += '"';
				else {
					char ch = ctx.next(), tgt = _escape_table[static_cast<unsigned char>(ch)];
					if (tgt) text += tgt;
					else text += '\\', text += ch;
				}
			} else text += ctx.next();
		}
	}
	static inline bool _try_parse_number(ValueData& tgt_val, string const& tgt_str) {
		if (tgt_str.empty()) return false;

		char first = tgt_str[0]; // 加号的匹配永远都是失败的
		if (!isdigit(static_cast<unsigned char>(first)) && first != '+' && first != '-' && first != '.')
			return false;
		if ((first == '+' || first == '-' || first == '.') && tgt_str.size() == 1) return false;

		string final_str;
		final_str.reserve(tgt_str.size());
		size_t idx = 0;
		if (tgt_str[idx] == '+' || tgt_str[idx] == '-') final_str += tgt_str[idx++];

		bool is_hex = false, is_oct = false, is_bin = false;
		if (idx + 1 < tgt_str.size() && tgt_str[idx] == '0') {
			switch (tgt_str[idx + 1]) {
				case 'x':
				case 'X': is_hex = true, final_str += "0x", idx += 2; break;
				case 'o':
				case 'O': is_oct = true, idx += 2; break;
				case 'b':
				case 'B': is_bin = true, idx += 2; break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					is_oct	   = true;
					final_str += '0';
					++idx;
			}
		}

		bool has_dot = false;
		bool has_exp = false;
		for (; idx < tgt_str.size(); ++idx) {
			char ch = tgt_str[idx];
			if (ch == '\'') continue;
			constexpr static char const	  right_quote[]	  = "’";
			constexpr static size_t const right_quote_len = sizeof(right_quote) - 1;
			if (idx + right_quote_len <= tgt_str.size()
					&& !tgt_str.compare(idx, right_quote_len, right_quote)) {
				idx += right_quote_len - 1;
				continue;
			}

			if (is_hex)
				if (isxdigit(static_cast<unsigned char>(ch))) final_str += ch;
				else return false;
			else if (is_oct)
				if (isdigit(static_cast<unsigned char>(ch)) && ch != '8' && ch != '9') final_str += ch;
				else return false;
			else if (is_bin)
				if (ch == '0' || ch == '1') final_str += ch;
				else return false;
			else if (isdigit(static_cast<unsigned char>(ch))) final_str += ch;
			else if (ch == '.') {
				if (has_dot) return false;
				has_dot = true, final_str += ch;
			} else if (ch == 'e' || ch == 'E') {
				if (has_exp) return false;
				has_exp = true, final_str += ch;

				if (idx + 1 < tgt_str.size() && (tgt_str[idx + 1] == '+' || tgt_str[idx + 1] == '-'))
					final_str += tgt_str[++idx]; // 加号的匹配永远都是失败的
			} else return false;
		}

		if (final_str.empty() || final_str == "+" || final_str == "-") return false;
		final_str.shrink_to_fit();

		if (!is_hex && !is_oct && !is_bin && (has_dot || has_exp)) {
			char*  endptr;
			double final_val = strtod(final_str.c_str(), &endptr);
			if (*endptr) return false;

			tgt_val = final_val;
			return true;
		}

		int base = 0;
		if (is_bin) base = 2;
		else if (is_oct) base = 8;

		char*	  endptr;
		long long final_val = strtoll(final_str.c_str(), &endptr, base);
		if (*endptr) return false;

		tgt_val = final_val;
		return true;
	}

	static inline FVVV* _find_key(string const& path, vector<FVVV*> const& scope_stack) {
		vector<string> paths = _split_name(path);
		if (paths.empty()) return nullptr;
		FVVV* target = nullptr;
		for (size_t idx = scope_stack.size(); idx-- > 0;) {
			target = scope_stack[idx];
			for (string const& idx_path : paths)
				if (target->nodes.contains_key(idx_path)) target = &(*target)[idx_path];
				else {
					target = nullptr;
					break;
				}
			if (target) return target;
		}
		return nullptr;
	}

   private:
	constexpr static inline unsigned long _merge_flags() noexcept { return 0; }
	template<typename... Args>
	constexpr static inline unsigned long _merge_flags(unsigned long arg, Args... args) noexcept {
		return arg | _merge_flags(args...);
	}

	inline void _to_string_root(FormatCtx const& ctx, string& ret, size_t level) const {
		if (nodes.empty()) return;

		for_each(nodes.begin(), nodes.end(), [&](decltype(nodes)::value_type const& item) {
			item.value()._to_string_main(ctx, item.key(), ret, level, &item == &nodes.back());
		});
	}
	inline void _to_string_main(
			FormatCtx const& ctx, string name, string& ret, size_t level, bool is_back) const {
		if (name.empty() || (this->_value.type != types::text && this->empty() && this->nodes.empty()))
			return; // 名称为空 或 (值为无值且子值为空)（值为字符串说明不为无值，但是判空会判断空字符串，列表则是不允许为空）
		ret.reserve(ret.length() + this->nodes.size() * 6);

		FVVV const* tgt_node = this;
		if (ctx.flatten_paths) {
			name.reserve(name.length() + 6);

			// 仅有一个子值 且 无描述模式或无描述 且 无链接模式或无链接
			while (tgt_node->nodes.size() == 1 && (ctx.no_descs || tgt_node->desc.empty())
					&& (ctx.no_links || tgt_node->link.empty())) {
				// 不想写类型名，也不想写 auto
				decltype(nodes)::value_type const& node_pair = tgt_node->nodes.front();

				name += '.', name += node_pair.key();
				tgt_node = &node_pair.value();
			}
		}

		string indent;
		if (!ctx.minify && level) {
			indent.reserve(level * ctx.indent_unit.length());
			for (size_t idx = 0; idx < level; ++idx) indent += ctx.indent_unit;

			ret += indent;
		}
		ret += name;
		ret += ctx.assign_op;

		if (!ctx.no_links && !tgt_node->link.empty()) ret += tgt_node->link;
		else if (!tgt_node->nodes.empty()) {
			if (ctx.fww_style && !tgt_node->desc.empty()) {
				ret += _escape_string(tgt_node->desc, true);
				if (!ctx.minify) ret += ' ';
			}
			if (ctx.full_width && ret.back() == ' ') ret.pop_back();
			ret += ctx.fwv_begin;
			if (!ctx.minify) ret += ctx.newline;
			tgt_node->_to_string_root(ctx, ret, level + 1);
			if (!ctx.minify) {
				ret += ctx.newline;
				ret += indent;
			}
			ret += ctx.fwv_end;
		} else if (tgt_node->_value.type != types::list)
			_to_string_value(ctx, tgt_node->_value, ret, indent);
		else {
			vector<ValueData> const& raw_list = tgt_node->_value._get_list();

			bool multiline = false;
			if (!ctx.minify && !ctx.list_single) {
				if (!(multiline = (raw_list.front().type == types::fwv))) {
					unsigned char long_items = 0;
					multiline = any_of(raw_list.begin(), raw_list.end(), [&](ValueData const& item) {
						switch (item.type) {
							case types::text:
								if (item.get<string>().length() + 2 >= 16) ++long_items;
								break;
							case types::integer:
							case types::float_point:
								if (item._to_string().length() >= 16) ++long_items;
								break;
							default: break;
						}
						return long_items >= 6;
					});
					// 长度达到 16 达到 6
				}
			}

			string const value_indent = indent + ctx.indent_unit;
			size_t const value_level  = level + 1;

			if (ctx.full_width && ret.back() == ' ') ret.pop_back();
			ret += ctx.list_begin;
			if (multiline) ret += ctx.newline;

			for_each(raw_list.begin(), raw_list.end(), [&](ValueData const& item) {
				if (multiline) ret += value_indent;
				_to_string_value(ctx, item, ret, value_indent, value_level);
				if (multiline ? ctx.force_sep : &item != &raw_list.back()) {
					ret += ctx.item_sep;
					if (!multiline && !ctx.full_width && !ctx.minify) ret += ' ';
				}
				if (multiline) ret += ctx.newline;
			});

			if (multiline) ret += indent;
			ret += ctx.list_end;
		}

		// 非无描述模式 且 描述非空 且 (无子值且非 FVV 列表 或 链接非空 或 非 FWW 样式)
		if (!ctx.no_descs && !tgt_node->desc.empty()
				&& ((tgt_node->nodes.empty()
							&& (tgt_node->_value.type != types::list
									|| tgt_node->_value._get_list().front().type != types::fwv))
						|| !tgt_node->link.empty() || !ctx.fww_style)) {
			// 非最小化模式 且 (非全角模式 或 链接非空 或 (无子值且非列表且非字符串) 或 非多行原始字符串)
			if (!ctx.minify
					&& (!ctx.full_width || !tgt_node->link.empty()
							|| (tgt_node->nodes.empty() && tgt_node->_value.type != types::list
									&& tgt_node->_value.type != types::text)
							|| (tgt_node->_value.type == types::text && ret.back() == '`')))
				ret += ' ';
			ret += _escape_string(tgt_node->desc, true);
		}

		if (ctx.minify || ctx.force_sep) ret += ctx.stmt_sep;
		if (!ctx.minify && !is_back) ret += ctx.newline;
	}
	static inline void _to_string_value(FormatCtx const& ctx, ValueData const& tgt_val, string& ret,
			string const& indent, size_t level = 0) {
		switch (tgt_val.type) {
			case types::boolean: ret += tgt_val._to_string(); break;

			case types::integer: {
				// 仅非十进制走整数逻辑
				if (ctx.int_base != 10) {
					long long tgt_int = tgt_val.get<long long>();

					if (tgt_int == 0) {
						if (ctx.int_base == 16) ret += "0x0";
						else if (ctx.int_base == 8) ret += "0o0";
						else if (ctx.int_base == 2) ret += "0b0";
						break;
					}

					unsigned long long tgt_uint;
					if (tgt_int < 0) {
						ret		 += '-';
						tgt_uint  = 0ULL - static_cast<unsigned long long>(tgt_int);
					} else {
						tgt_uint = static_cast<unsigned long long>(tgt_int);
					}

					if (ctx.int_base == 2) {
						ret += "0b";
						string bin_str;
						bin_str.reserve(64);

						while (tgt_uint > 0) bin_str += (tgt_uint & 1) ? '1' : '0', tgt_uint >>= 1;

						reverse(bin_str.begin(), bin_str.end());
						ret += bin_str;
					} else {
						char const* fmt = ctx.int_base == 16 ? "0x%llx" : "0o%llo";
#ifdef _MSC_VER
						int len = _scprintf(fmt, tgt_uint);
#else
						int len = snprintf(nullptr, 0, fmt, tgt_uint);
#endif
#if false
						if (len <= 0) break;
#endif
						size_t ret_len = ret.length();
						ret.resize(ret_len + len + 1);
#ifdef _MSC_VER
						_snprintf_s(&ret[ret_len], len + 1, _TRUNCATE, fmt, tgt_uint);
#else
						snprintf(&ret[ret_len], len + 1, fmt, tgt_uint);
#endif
						ret.resize(ret_len + len);
					}
					break;
				}
			}
			// 十进制整数走浮点数逻辑
			case types::float_point: {
				string raw_num = tgt_val._to_string();
				if (!ctx.digit_sep_step) {
					ret += raw_num;
					break;
				}

				size_t start_idx = 0;
				if (raw_num[0] == '-' || raw_num[0] == '+') start_idx = 1;
				size_t dot_pos = raw_num.find('.');
				if (dot_pos == string::npos) dot_pos = raw_num.length();
				size_t int_len = dot_pos - start_idx;

				if (int_len <= ctx.digit_sep_step) {
					ret += raw_num;
					break;
				}

				ret.reserve(ret.length() + raw_num.length() + int_len / ctx.digit_sep_step + 1);
				if (start_idx > 0) ret += raw_num[0];
				for (size_t idx = 0; idx < int_len; ++idx) {
					if (idx > 0 && !((int_len - idx) % ctx.digit_sep_step)) ret += ctx.digit_sep_char;
					ret += raw_num[start_idx + idx];
				}

				if (dot_pos < raw_num.length()) ret += raw_num.substr(dot_pos);
				break;
			}

			case types::text: {
				string tgt_str = tgt_val.get<string>();
				// 非最小化模式且开启多行原始字符串模式时，如果长度达到 3 才进行判断
				if (!ctx.minify && ctx.raw_str && tgt_str.length() >= 3
						&& tgt_str.find('`') == string::npos
						&& _trim(tgt_str).find_first_of("\r\n") != string::npos) {
					// 字符串内无反引号且中间有换行就可以多行原始字符串
					string const str_indent = indent + ctx.indent_unit;
					ret.reserve(ret.length() + tgt_str.length());

					tgt_str = _trim(_trim_indent(tgt_val.get<string>()));

					ret += '`';
					ret += ctx.newline;
					for (size_t idx = 0; idx < tgt_str.length(); ++idx) {
						char ch = tgt_str[idx];
						if (ch == '\r' || ch == '\n') {
							if (ch == '\r' && idx + 1 < tgt_str.length() && tgt_str[idx + 1] == '\n')
								++idx;
							ret += ctx.newline;
						} else {
							if (ret.back() == '\r' || ret.back() == '\n') ret += str_indent;
							ret += ch;
						}
					}
					ret += ctx.newline;
					ret += indent;
					ret += '`';
					break;
				}
				if (!level && ctx.full_width && ret.back() == ' ') ret.pop_back();
				ret += _escape_string(tgt_str, false, ctx.full_width);
				break;
			}

			case types::fwv: {
				FVVV const tgt_fwv = tgt_val.get<FVVV>();
				if (ctx.fww_style && !tgt_fwv.desc.empty()) {
					ret += _escape_string(tgt_fwv.desc, true);
					if (!ctx.minify && !ctx.full_width) ret += ' ';
				}
				ret += ctx.fwv_begin;
				if (!ctx.minify) ret += ctx.newline;
				tgt_fwv._to_string_root(ctx, ret, level + 1);
				if (!ctx.minify) {
					ret += ctx.newline;
					ret += indent;
				}
				ret += ctx.fwv_end;
				if (!ctx.no_descs && !ctx.fww_style && !tgt_fwv.desc.empty()) {
					if (!ctx.minify && !ctx.full_width) ret += ' ';
					ret += _escape_string(tgt_fwv.desc, true);
				}
			}

			default: break;
		}
	}
	static inline string _escape_string(string const& str, bool is_desc, bool full_width = false) {
		string ret;
		ret.reserve(str.length());

		if (is_desc) ret += '<';
		else ret += full_width ? "“" : "\"";

		for (size_t idx = 0; idx < str.length(); ++idx) {
			char ch = str[idx];

			if (full_width && !is_desc) {
				constexpr static char const	  right_quote[]	  = "”";
				constexpr static size_t const right_quote_len = sizeof(right_quote) - 1;
				if (idx + right_quote_len <= str.length()
						&& !str.compare(idx, right_quote_len, right_quote)) {
					ret += "\\”";
					idx += right_quote_len - 1;
					continue;
				}
			}

			switch (ch) {
				case '\\': ret += "\\\\"; break;
				case '\b': ret += "\\b"; break;
				case '\f': ret += "\\f"; break;
				case '\n': ret += "\\n"; break;
				case '\r': ret += "\\r"; break;
				case '\t': ret += "\\t"; break;
				case '"':
					if (!full_width && !is_desc) ret += "\\\"";
					else ret += ch;
					break;
				case '>':
					if (is_desc) ret += "\\>";
					else ret += ch;
					break;
				default: ret += ch;
			}
		}

		if (is_desc) ret += '>';
		else ret += full_width ? "”" : "\"";
		return ret;
	}

   private:
	template<typename StructType>
	inline typename enable_if<_is_fvv_struct<StructType>::value>::type _to(StructType& target) const {
		ReadBinder binder(*this);
		target.fvv_register(binder);
	}
	template<typename ValueType>
	inline typename enable_if<is_same<typename decay<ValueType>::type, string>::value>::type _to(
			ValueType& target) const {
		if (this->is<ValueType>()) target = this->value<ValueType>();
	}
	template<typename ValueType>
	inline typename enable_if<is_arithmetic<ValueType>::value>::type _to(ValueType& target) const {
		if (this->is<ValueType>()) target = this->value<ValueType>();
		else if (is_integral<ValueType>::value && this->is<long long>())
			target = static_cast<ValueType>(this->value<long long>());
		else if (is_integral<ValueType>::value && this->is<double>())
			target = static_cast<ValueType>(this->value<double>());
		else if (is_floating_point<ValueType>::value && this->is<long long>())
			target = static_cast<ValueType>(this->value<long long>());
	}
	template<typename ValuesType>
	inline typename enable_if<_is_list<ValuesType>::value>::type _to(ValuesType& target) const {
		if (this->_value.type != types::list) return;

		vector<ValueData> const& values = this->_value._get_list();
		target.clear();
		target.reserve(values.size());

		using ItemType = typename ValuesType::value_type;
		for (ValueData const& item : values) {
			ItemType tmp_value{};
			if (item.type == types::fwv) item.get<FVVV>().to(tmp_value);
			else {
				FVVV tmp_fwv(item);
				tmp_fwv.to(tmp_value);
			}
			target.push_back(std::move(tmp_value));
		}
	}

	template<typename StructType>
	inline typename enable_if<_is_fvv_struct<StructType>::value>::type _from(StructType const& target) {
		this->_value.clear();
		WriteBinder binder(*this);
		const_cast<StructType&>(target).fvv_register(binder);
	}
	template<typename ValueType>
	inline typename enable_if<!_is_fvv_struct<ValueType>::value && !_is_list<ValueType>::value>::type
	_from(ValueType const& target) {
		*this = target;
	}
	template<typename ValuesType>
	inline typename enable_if<_is_list<ValuesType>::value>::type _from(ValuesType const& target) {
		vector<ValueData> values;
		values.reserve(target.size());

		using ItemType = typename ValuesType::value_type;
		for (ItemType const& item : target) {
			FVVV tmp_node;
			tmp_node.from(item);
			if (tmp_node.nodes.empty()) values.push_back(tmp_node._value);
			else values.emplace_back(std::move(tmp_node));
		}
		this->_value = std::move(values);
	}

   private:
	using _table_type = array<char, numeric_limits<unsigned char>::max() + 1>;

	static array<char, numeric_limits<unsigned char>::max() + 1> const _escape_table;

	static inline void _trim_right(string& str) {
		str.erase(find_if(str.rbegin(), str.rend(),
						  [](int ch) {
			return !isspace(static_cast<unsigned char>(ch));
		}).base(),
				str.end());
	}
	static inline string _trim(string const& str) {
		if (str.empty()) return "";
		size_t start = str.find_first_not_of(" \f\n\r\t\v");
		if (start == string::npos) return "";
		size_t end = str.find_last_not_of(" \f\n\r\t\v");
		return str.substr(start, end - start + 1);
	}
	static inline string _trim_indent(string const& str) {
		if (str.empty()) return "";
		vector<string> lines;
		stringstream   ss(str);
		string		   tmp_line;
		while (getline(ss, tmp_line)) lines.push_back(std::move(tmp_line));

		size_t min_indent = SIZE_MAX;
		for (string const& line : lines) {
			size_t first_char = line.find_first_not_of(" \t");
			if (first_char == string::npos) continue;
			if (first_char < min_indent) min_indent = first_char;
		}
		if (min_indent == SIZE_MAX) min_indent = 0;

		string ret;
		for (size_t idx = 0; idx < lines.size(); ++idx) {
			string const& line = lines[idx];
			if (line.length() >= min_indent && line.find_first_not_of(" \t") != string::npos)
				ret += line.substr(min_indent);
			if (idx < lines.size() - 1) ret += '\n';
		}
		return ret;
	}

	static inline vector<string> _split_name(string const& target) {
		if (target.empty()) return {};
		if (target.find('.') == string::npos) return { target };
		vector<string> rets;
		stringstream   ss(target);
		string		   item;
		while (getline(ss, item, '.'))
			if (!item.empty()) rets.push_back(std::move(item));
		return rets;
	}

	template<size_t raw_len>
	static bool _iequals(string const& a, char const (&b)[raw_len]) {
		constexpr size_t len = raw_len - 1;
		if (a.length() != len) return false;
		return equal(a.begin(), a.end(), b, [](char a_ch, char b_ch) {
			return tolower(static_cast<unsigned char>(a_ch)) == tolower(static_cast<unsigned char>(b_ch));
		});
	}

   private:
	inline void _swap(FVVV& target) noexcept {
		std::swap(this->_value, target._value);
		std::swap(this->nodes, target.nodes);
		std::swap(this->desc, target.desc);
		std::swap(this->link, target.link);
	}
};
template<>
struct FVVV::ValueData::_getter<bool> {
	constexpr static types const type = types::boolean;
	constexpr static inline bool get(union _data_type const& data) noexcept { return data._boolean; }
};
template<>
struct FVVV::ValueData::_getter<long long> {
	constexpr static types const type = types::integer;
	constexpr static inline long long get(union _data_type const& data) noexcept { return data._integer; }
};
template<>
struct FVVV::ValueData::_getter<double> {
	constexpr static types const type = types::float_point;
	constexpr static inline double get(union _data_type const& data) noexcept {
		return data._float_point;
	}
};
template<>
struct FVVV::ValueData::_getter<string> {
	constexpr static types const type = types::text;
	static inline string get(union _data_type const& data) { return *(data._text); }
};
template<>
struct FVVV::ValueData::_getter<FVVV> {
	constexpr static types const type = types::fwv;
	inline FVVV static get(union _data_type const& data) { return *(data._fwv); }
};
const FVVV::_table_type FVVV::_escape_table = []() -> FVVV::_table_type {
	FVVV::_table_type table = {};

	table['b']	= '\b';
	table['f']	= '\f';
	table['n']	= '\n';
	table['r']	= '\r';
	table['t']	= '\t';
	table['\\'] = '\\';

	return table;
}();
} // namespace FVV