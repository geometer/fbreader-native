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

#ifndef __MODELWRITER_H__
#define __MODELWRITER_H__

#include <string>

#include <shared_ptr.h>

class ZLTextModel;
class BookModel;
class ContentsTree;
class JSONMapWriter;

class ModelWriter {

public:
	ModelWriter(const std::string &dir);

	void writeModelInfo(const BookModel &model);

private:
	void writeModel(const ZLTextModel &model, shared_ptr<JSONMapWriter> writer);
	void writeInternalHyperlinks(const BookModel &model, shared_ptr<JSONMapWriter> writer);
	void writeTOC(const ContentsTree &tree, shared_ptr<JSONMapWriter> writer);

private:
	const std::string myDir;
};

#endif /* __MODELWRITER_H__ */
