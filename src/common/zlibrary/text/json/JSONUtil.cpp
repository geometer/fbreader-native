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

#include <JSONWriter.h>
#include <FileEncryptionInfo.h>
#include <FontMap.h>
#include <ZLFileImage.h>

#include "JSONUtil.h"

void JSONUtil::serializeStringArray(const std::vector<std::string> &array, shared_ptr<JSONArrayWriter> writer) {
	for (std::vector<std::string>::const_iterator it = array.begin(); it != array.end(); ++it) {
		writer->addElement(*it);
	}
}

void JSONUtil::serializeIntArray(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer) {
	for (std::vector<int>::const_iterator it = array.begin(); it != array.end(); ++it) {
		writer->addElement(*it);
	}
}

void JSONUtil::serializeIntArrayAsDiffs(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer) {
	int prev = 0;
	for (std::vector<int>::const_iterator it = array.begin(); it != array.end(); ++it) {
		writer->addElement(*it - prev);
		prev = *it;
	}
}

void JSONUtil::serializeIntArrayAsCounts(const std::vector<int> &array, shared_ptr<JSONArrayWriter> writer) {
	int value = 0;
	int count = 0;
	for (std::vector<int>::const_iterator it = array.begin(); it != array.end(); ++it) {
		while (value < *it) {
			writer->addElement(count);
			count = 0;
			++value;
		}
		++count;
	}
	writer->addElement(count);
}

void JSONUtil::serializeByteArray(const std::vector<unsigned char> &array, shared_ptr<JSONArrayWriter> writer) {
	for (std::vector<unsigned char>::const_iterator it = array.begin(); it != array.end(); ++it) {
		writer->addElement((int)*it);
	}
}

void JSONUtil::serializeFileEncryptionInfo(const FileEncryptionInfo& info, shared_ptr<JSONMapWriter> writer) {
	writer->addElementIfNotEmpty("u", info.Uri);
	writer->addElementIfNotEmpty("m", info.Method);
	writer->addElementIfNotEmpty("a", info.Algorithm);
	writer->addElementIfNotEmpty("c", info.ContentId);
}

void JSONUtil::serializeFileInfo(const FileInfo& info, shared_ptr<JSONMapWriter> writer) {
	writer->addElement("p", info.Path);
	if (!info.EncryptionInfo.isNull()) {
		serializeFileEncryptionInfo(*info.EncryptionInfo, writer->addMap("e"));
	}
}

void JSONUtil::serializeFontEntry(const std::string &family, const FontEntry &entry, shared_ptr<JSONMapWriter> writer) {
		writer->addElement("f", family);
		if (!entry.Normal.isNull()) {
			serializeFileInfo(*entry.Normal, writer->addMap("r"));
		}
		if (!entry.Bold.isNull()) {
			serializeFileInfo(*entry.Bold, writer->addMap("b"));
		}
		if (!entry.Italic.isNull()) {
			serializeFileInfo(*entry.Italic, writer->addMap("i"));
		}
		if (!entry.BoldItalic.isNull()) {
			serializeFileInfo(*entry.BoldItalic, writer->addMap("bi"));
		}
}

void JSONUtil::serializeImage(const std::string &id, const ZLFileImage &image, shared_ptr<JSONMapWriter> writer) {
	writer->addElement("id", id);
	writer->addElement("enco", image.encoding());
	writer->addElement("path", image.file().path());

	const ZLFileImage::Blocks &blocks = image.blocks();
	shared_ptr<JSONArrayWriter> offsets = writer->addArray("off");
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		offsets->addElement(blocks.at(i).offset);
	}
	shared_ptr<JSONArrayWriter> sizes = writer->addArray("szs");
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		sizes->addElement(blocks.at(i).size);
	}

	shared_ptr<FileEncryptionInfo> info = image.encryptionInfo();
	if (!info.isNull()) {
		serializeFileEncryptionInfo(*info, writer->addMap("encry"));
	}
}
