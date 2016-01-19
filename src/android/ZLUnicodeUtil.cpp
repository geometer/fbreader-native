/*
 * Copyright (C) 2004-2015 FBReader.ORG Limited <contact@fbreader.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <cctype>
#include <cstdlib>
#include <map>

#include <AndroidUtil.h>
#include <JniEnvelope.h>

#include <ZLibrary.h>
#include <ZLFile.h>
#include <ZLXMLReader.h>

#include "ZLUnicodeUtil.h"

std::string ZLUnicodeUtil::toLowerFull(const std::string &utf8String) {
	if (utf8String.empty()) {
		return utf8String;
	}

	bool isAscii = true;
	const int size = utf8String.size();
	for (int i = size - 1; i >= 0; --i) {
		if ((utf8String[i] & 0x80) != 0) {
			isAscii = false;
			break;
		}
	}
	if (isAscii) {
		std::string result(size, ' ');
		for (int i = size - 1; i >= 0; --i) {
			result[i] = std::tolower(utf8String[i]);
		}
		return result;
	}
	JNIEnv *env = AndroidUtil::getEnv();
	jstring javaString = AndroidUtil::createJavaString(env, utf8String);
	jstring lowerCased = AndroidUtil::Method_java_lang_String_toLowerCase->callForJavaString(javaString);
	if (javaString == lowerCased) {
		env->DeleteLocalRef(lowerCased);
		env->DeleteLocalRef(javaString);
		return utf8String;
	} else {
		const std::string result = AndroidUtil::fromJavaString(env, lowerCased);
		env->DeleteLocalRef(lowerCased);
		env->DeleteLocalRef(javaString);
		return result;
	}
}
