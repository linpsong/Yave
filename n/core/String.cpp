#include "String.h"
#include "Array.h"
#include <string>
#include <iomanip>
#include <cstring>

namespace n {
namespace core {

char *allocStr(uint s) {
	if(!s) {
		return 0;
	}
	char *c = (char *)malloc((s + 1) * sizeof(char));
	*c = '\0';
	//cppcheck-suppress memleak
	return c + 1;
}

char *reallocStr(char *p, uint s) {
	if(!s) {
		return 0;
	}
	if(!p) {
		return allocStr(s);
	}
	while(*p) p--;
	p = (char *)realloc(p, s + 1);
	return p + 1;
}

void freeStr(char *p) {
	if(!p) {
		return;
	}
	while(*p) p--;
	free(p);
}

String::String() : length(0), count(0), data(0) {
}

String::String(const char *cst) : String(cst, cst ? strlen(cst) : 0) {
}

String::String(const char *cst, uint l) : length(l), count(0), data(0) {
	if(l) {
		data = allocStr(length);//(char *)malloc(length * sizeof(char));
		if(cst) {
			memcpy(data, cst, length * sizeof(char));
		} else {
			*data = '\0';
		}
	}
}

String::String(const String &str) : length(str.length), count(str.count), data(str.data) {
	if(count) {
		(*count)++;
	} else if(data) {
		count = str.count = new uint(2);
	}
}

String::String(String &&str) : String() {
	swap(str);
}

String::~String() {
	if(data) {
		if(isUnique()) {
			delete count;
			freeStr(data);//free(data);
		} else {
			(*count)--;
		}
	}
}

void String::replace(const String &oldS, const String &newS) {
	operator=(replaced(oldS, newS));
}

void String::replace(uint beg, uint len, const String &newS) {
	operator=(replaced(beg, len, newS));
}

String String::replaced(const String &oldS, const String &newS) const {
	Array<String> concat;
	uint index = find(oldS);
	if(index != (uint)-1) {
		uint from = 0;
		uint offset = oldS.size();
		do {
			concat += subString(from, index - from);
			concat += newS;
			from = index + offset;
			index = find(oldS, from);
		} while(index != (uint)-1);
		concat += subString(from);
	} else {
		return *this;
	}
	return concat;
}

String String::replaced(uint beg, uint len, const String &newS) const {
	return subString(0, beg) + newS + subString(beg + len);
}

void String::clear() {
	detach(0);
}

uint String::size() const {
	return length;
}

bool String::isEmpty() const {
	return !length;
}

bool String::isNull() const {
	return !data;
}

char const *String::toChar() const {
	if(!data) {
		return (char *)&null;
	}
	detach(length + 1);
	length--;
	data[length] = '\0';
	return data;
}

uint String::find(char c, uint from) const {
	if(!(from < length)) {
		return -1;
	}
	for(uint i = from; i != length; i++) {
		if(data[i] == c) {
			return i;
		}
	}
	return -1;
}

uint String::find(const String &str, uint from) const {
	if(from + str.size() > length) {
		return -1;
	}
	for(uint i = from, l = length - str.length; i <= l; i++) {
		bool matched = true;
		for(uint j = 0; j != str.length; j++) {
			if(data[i + j] != str.data[j]) {
				matched = false;
				break;
			}
		}
		if(matched) {
			return i;
		}
	}
	return -1;
}

bool String::contains(char c) const {
	return find(c) < length;
}

bool String::contains(const String &str) const {
	return find(str) < length;
}

String::String(const String &str, uint beg, uint len) : length(len), count(str.count), data(str.data + beg) {
	if(count) {
		(*count)++;
	} else if(data) {
		count = str.count = new uint(2);
	}
}

String String::subString(uint beg, uint len) const {
	return String(*this, beg, len);
}

String String::subString(uint beg) const {
	return subString(beg, length - beg);
}

bool String::beginWith(const String &s) const {
	if(length < s.length) {
		return false;
	}
	if(isNull()) {
		return s.isEmpty();
	}
	if(s.isEmpty()) {
		return true;
	}
	for(uint i = 0; i != s.length; i++) {
		if(data[i] != s.data[i]) {
			return false;
		}
	}
	return true;
}

bool String::endWith(const String &s) const {
	if(length < s.length) {
		return false;
	}
	if(isNull()) {
		return s.isEmpty();
	}
	if(s.isEmpty()) {
		return true;
	}
	uint offset = length - s.length;
	for(uint i = s.length - 1; i; i--) {
		if(data[offset + i] != s.data[i]) {
			return false;
		}
	}
	return true;
}

void String::detach() {
	detach(length);
}

void String::swap(String &str) {
	uint l = str.length;
	char *ch = str.data;
	uint *c = str.count;
	str.length = length;
	str.data = data;
	str.count = count;
	count = c;
	data = ch;
	length = l;
}

Array<String> String::split(const String &str) const {
	Array<String> arr;
	uint p = -1;
	uint from = 0;
	while((p = find(str, from)) != (uint)-1) {
		arr.append(subString(from, p - from));
		from = p + str.size();
	}
	arr.append(subString(from));
	return arr;
}

float String::toFloat() const {
	return toDouble();
}

double String::toDouble() const {
	return atof(data);
}

int String::toInt() const {
	return atoi(data);
}

String String::toLower() const {
	return mapped([](char c) -> char { return tolower(c); });
}

String String::toUpper() const {
	return mapped([](char c) -> char { return toupper(c); });
}

std::string String::toStdString() const {
	return std::string(data, length);
}

String &String::operator+=(const String &s) {
	uint tl = length;
	uint ol = s.length;
	detach(tl + ol);
	memcpy(data + tl, s.data, ol * sizeof(char));
	return *this;
}

String String::operator+(const String &s) const {
	if(isEmpty()) {
		return s;
	}
	if(s.isEmpty()) {
		return *this;
	}
	String str(0, length + s.length);
	memcpy(str.data, data, length * sizeof(char));
	memcpy(str.data + length, s.data, s.length * sizeof(char));
	return str;
}

bool String::operator==(const String &str) const {
	if(str.length != length) {
		return false;
	}
	if(count && count == str.count) {
		return true;
	}
	for(uint i = 0; i != length; i++) {
		if(data[i] != str.data[i]) {
			return false;
		}
	}
	return true;
}

bool String::operator==(const char *str) const {
	for(uint i = 0; i != length; i++) {
		if(data[i] != str[i]) {
			return false;
		}
	}
	return true;
}

bool String::operator!=(const String &str) const {
	return !operator==(str);
}

bool String::operator!=(const char *str) const {
	return !operator==(str);
}

String &String::operator=(const String &s) {
	detach(0);
	data = s.data;
	count = s.count;
	length = s.length;
	if(count) {
		(*count)++;
	} else if(data) {
		count = s.count = new uint(2);
	}
	return *this;
}


String &String::operator=(String &&s) {
	swap(s);
	return *this;
}

bool String::operator<(const String &s) const {
	if(data && s.data) {
		for(uint i = 0, min = std::min(length, s.length); i != min; i++) {
			if(data[i] < s.data[i]) {
				return true;
			} else if(data[i] > s.data[i]) {
				return false;
			}
		}
	}
	return length < s.length;
}

String::operator const char *() const {
	return toChar();
}

String::const_iterator String::begin() const {
	return data;
}

String::const_iterator String::end() const {
	return data + length;
}

uint String::getHash() const {
	return length && data ? hash(data, length) : 0;
}

void String::detach(uint s) const {
	if(s) {
		if(isUnique()) {
			data = reallocStr(data, s);//(char *)realloc(data, s * sizeof(char));
		} else {
			(*count)--;
			count = 0;
			char *d = allocStr(s);//(char *)malloc(s * sizeof(char));
			memcpy(d, data, std::min(length, s) * sizeof(char));
			data = d;
		}
	} else {
		if(isUnique()) {
			delete count;
			freeStr(data);//free(data);
		}
		count = 0;
		data = 0;
	}
	length = s;
}

bool String::isUnique() const {
	return (!count || *count == 1);
}

bool String::isShared() const {
	return count && *count > 1;
}

}
}

std::istream &operator>>(std::istream &s, n::core::String &str) {
	std::string st;
	s>>st;
	str = n::core::String(st.c_str());
	return s;
}

std::ostream &operator<<(std::ostream &s, const n::core::String &str) {
	s<<str.toStdString();
	return s;
}




