/*
 * Copyright (C) 2011-2015 FBReader.ORG Limited <contact@fbreader.org>
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

#ifndef __JSONUTIL_H__
#define __JSONUTIL_H__

#include <string>
#include <vector>

class JSONArrayWriter;
class JSONMapWriter;

class FileInfo;
class FileEncryptionInfo;
class FontEntry;
class ZLFileImage;

struct JSONUtil {
	static void serializeStringArray(const std::vector<std::string> &array, shared_ptr<JSONArrayWriter> writer);
	static void serializeIntArray(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer);
	static void serializeIntArrayAsDiffs(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer);
	static void serializeIntArrayAsCounts(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer);
	static void serializeByteArray(const std::vector<unsigned char> &array, shared_ptr<JSONArrayWriter> writer);

	static void serializeFileEncryptionInfo(const FileEncryptionInfo& info, shared_ptr<JSONMapWriter> writer);
	static void serializeFileInfo(const FileInfo& info, shared_ptr<JSONMapWriter> writer);
	static void serializeFontEntry(const std::string &family, const FontEntry &entry, shared_ptr<JSONMapWriter> writer);
	static void serializeImage(const std::string &id, const ZLFileImage &image, shared_ptr<JSONMapWriter> writer);
};

#endif /* __JSONUTIL_H__ */
