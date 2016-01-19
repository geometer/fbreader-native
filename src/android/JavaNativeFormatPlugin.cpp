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

#include <AndroidUtil.h>
#include <JniEnvelope.h>
#include <ZLFileImage.h>
#include <JSONWriter.h>
#include <JSONUtil.h>
#include <FileEncryptionInfo.h>

#include "fbreader/src/bookmodel/BookModel.h"
#include "fbreader/src/formats/FormatPlugin.h"
#include "fbreader/src/library/Author.h"
#include "fbreader/src/library/Book.h"
#include "fbreader/src/library/Tag.h"
#include "fbreader/src/library/UID.h"

static shared_ptr<FormatPlugin> findCppPlugin(jobject base) {
	const std::string fileType = AndroidUtil::Method_NativeFormatPlugin_supportedFileType->callForCppString(base);
	return PluginCollection::Instance().pluginByType(fileType);
}

static void fillUids(JNIEnv* env, jobject javaBook, Book &book) {
	const UIDList &uids = book.uids();
	for (UIDList::const_iterator it = uids.begin(); it != uids.end(); ++it) {
		JString type(env, (*it)->Type);
		JString id(env, (*it)->Id);
		AndroidUtil::Method_Book_addUid->call(javaBook, type.j(), id.j());
	}
}

static void fillMetaInfo(JNIEnv* env, jobject javaBook, Book &book) {
	JString title(env, book.title());
	AndroidUtil::Method_Book_setTitle->call(javaBook, title.j());

	JString language(env, book.language());
	if (language.j() != 0) {
		AndroidUtil::Method_Book_setLanguage->call(javaBook, language.j());
	}

	JString encoding(env, book.encoding());
	if (encoding.j() != 0) {
		AndroidUtil::Method_Book_setEncoding->call(javaBook, encoding.j());
	}

	JString seriesTitle(env, book.seriesTitle());
	if (seriesTitle.j() != 0) {
		JString indexString(env, book.indexInSeries());
		AndroidUtil::Method_Book_setSeriesInfo->call(javaBook, seriesTitle.j(), indexString.j());
	}

	const AuthorList &authors = book.authors();
	for (std::size_t i = 0; i < authors.size(); ++i) {
		const Author &author = *authors[i];
		JString name(env, author.name(), false);
		JString key(env, author.sortKey(), false);
		AndroidUtil::Method_Book_addAuthor->call(javaBook, name.j(), key.j());
	}

	const TagList &tags = book.tags();
	for (std::size_t i = 0; i < tags.size(); ++i) {
		JString jTag(env, tags[i]->fullName(), false);
		AndroidUtil::Method_Book_addTag->call(javaBook, jTag.j());
	}

	fillUids(env, javaBook, book);
}

static void fillLanguageAndEncoding(JNIEnv* env, jobject javaBook, Book &book) {
	JString language(env, book.language());
	if (language.j() != 0) {
		AndroidUtil::Method_Book_setLanguage->call(javaBook, language.j());
	}

	JString encoding(env, book.encoding());
	if (encoding.j() != 0) {
		AndroidUtil::Method_Book_setEncoding->call(javaBook, encoding.j());
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readMetainfoNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 1;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);

	if (!plugin->readMetainfo(*book)) {
		return 2;
	}

	fillMetaInfo(env, javaBook, *book);
	return 0;
}

extern "C"
JNIEXPORT jobject JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readEncryptionInfosNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 0;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	std::vector<shared_ptr<FileEncryptionInfo> > infos = plugin->readEncryptionInfos(*book);
	if (infos.empty()) {
		return 0;
	}

	jobjectArray jList = env->NewObjectArray(
		infos.size(), AndroidUtil::Class_FileEncryptionInfo.j(), 0
	);
	for (std::size_t i = 0; i < infos.size(); ++i) {
		jobject jInfo = AndroidUtil::createJavaEncryptionInfo(env, infos[i]);
		env->SetObjectArrayElement(jList, i, jInfo);
		env->DeleteLocalRef(jInfo);
	}
	return jList;
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readUidsNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);

	plugin->readUids(*book);
	fillUids(env, javaBook, *book);
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_detectLanguageAndEncodingNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	if (!plugin->readLanguageAndEncoding(*book)) {
		return;
	}

	fillLanguageAndEncoding(env, javaBook, *book);
}

static void writeInternalHyperlinks(BookModel &model, const std::string &cacheDir, shared_ptr<JSONMapWriter> writer) {
	ZLCachedMemoryAllocator allocator(131072, cacheDir, "nlinks");

	ZLUnicodeUtil::Ucs2String ucs2id;
	ZLUnicodeUtil::Ucs2String ucs2modelId;

	const std::map<std::string,BookModel::Label> &links = model.internalHyperlinks();
	std::map<std::string,BookModel::Label>::const_iterator it = links.begin();
	for (; it != links.end(); ++it) {
		const std::string &id = it->first;
		const BookModel::Label &label = it->second;
		if (label.Model.isNull()) {
			continue;
		}
		ZLUnicodeUtil::utf8ToUcs2(ucs2id, id);
		ZLUnicodeUtil::utf8ToUcs2(ucs2modelId, label.Model->id());
		const std::size_t idLen = ucs2id.size() * 2;
		const std::size_t modelIdLen = ucs2modelId.size() * 2;

		char *ptr = allocator.allocate(idLen + modelIdLen + 8);
		ZLCachedMemoryAllocator::writeUInt16(ptr, ucs2id.size());
		ptr += 2;
		std::memcpy(ptr, &ucs2id.front(), idLen);
		ptr += idLen;
		ZLCachedMemoryAllocator::writeUInt16(ptr, ucs2modelId.size());
		ptr += 2;
		std::memcpy(ptr, &ucs2modelId.front(), modelIdLen);
		ptr += modelIdLen;
		ZLCachedMemoryAllocator::writeUInt32(ptr, label.ParagraphNumber);
	}
	allocator.flush();

	writer->addElement("ext", allocator.fileExtension());
	writer->addElement("blks", allocator.blocksNumber());
}

static bool ct_compare(const shared_ptr<ContentsTree> &first, const shared_ptr<ContentsTree> &second) {
	return first->reference() < second->reference();
}

static void writeModel(const ZLTextModel &model, shared_ptr<JSONMapWriter> writer) {
	writer->addElementIfNotEmpty("id", model.id());
	writer->addElementIfNotEmpty("lang", model.language());
	writer->addElement("size", model.paragraphsNumber());
	const ZLCachedMemoryAllocator &allocator = model.allocator();
	writer->addElement("ext", allocator.fileExtension());
	writer->addElement("blks", allocator.blocksNumber());
	JSONUtil::serializeIntArrayAsCounts(model.startEntryIndices(), writer->addArray("ei"));
	JSONUtil::serializeIntArrayAsDiffs(model.startEntryOffsets(), writer->addArray("eo"));
	JSONUtil::serializeIntArray(model.paragraphLengths(), writer->addArray("pl"));
	JSONUtil::serializeIntArrayAsDiffs(model.textSizes(), writer->addArray("ts"));
	JSONUtil::serializeByteArray(model.paragraphKinds(), writer->addArray("pk"));
}

static void writeTOC(const ContentsTree &tree, shared_ptr<JSONMapWriter> writer) {
	const std::string &text = tree.text();
	writer->addElementIfNotEmpty("t", text);
	const int ref = tree.reference();
	if (ref >= 0) {
		writer->addElement("r", ref);
	}
	std::vector<shared_ptr<ContentsTree> > children = tree.children();
	if (children.size() > 0) {
		shared_ptr<JSONArrayWriter> childrenWriter = writer->addArray("c");
		std::sort(children.begin(), children.end(), ct_compare);
		for (std::vector<shared_ptr<ContentsTree> >::const_iterator it = children.begin(); it != children.end(); ++it) {
			writeTOC(**it, childrenWriter->addMap());
		}
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readModelNative(JNIEnv* env, jobject thiz, jobject javaBook, jstring javaCacheDir) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 1;
	}

	const std::string cacheDir = AndroidUtil::fromJavaString(env, javaCacheDir);

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	shared_ptr<BookModel> model = new BookModel(book, cacheDir);
	if (!plugin->readModel(*model)) {
		return 2;
	}
	if (!model->flush()) {
		return 3;
	}

	shared_ptr<JSONMapWriter> everythingWriter = new JSONMapWriter(cacheDir + "/MODELS");

	shared_ptr<JSONArrayWriter> modelsWriter = everythingWriter->addArray("mdls");
	writeModel(*model->bookTextModel(), modelsWriter->addMap());
	const std::map<std::string,shared_ptr<ZLTextModel> > &footnotes = model->footnotes();
	std::map<std::string,shared_ptr<ZLTextModel> >::const_iterator it = footnotes.begin();
	for (; it != footnotes.end(); ++it) {
		writeModel(*it->second, modelsWriter->addMap());
	}

	writeInternalHyperlinks(*model, cacheDir, everythingWriter->addMap("hlks"));

	shared_ptr<JSONArrayWriter> familiesWriter = everythingWriter->addArray("fams");
	const std::vector<std::vector<std::string> > familyLists = model->fontManager().familyLists();
	for (std::vector<std::vector<std::string> >::const_iterator it = familyLists.begin(); it != familyLists.end(); ++it) {
		JSONUtil::serializeStringArray(*it, familiesWriter->addArray());
	}

	shared_ptr<JSONArrayWriter> fontsWriter = everythingWriter->addArray("fnts");
	const std::map<std::string,shared_ptr<FontEntry> > &entries = model->fontManager().entries();
	for (std::map<std::string,shared_ptr<FontEntry> >::const_iterator it = entries.begin(); it != entries.end(); ++it) {
		if (!it->second.isNull()) {
			JSONUtil::serializeFontEntry(it->first, *it->second, fontsWriter->addMap());
		}
	}

	shared_ptr<JSONArrayWriter> imagesWriter = everythingWriter->addArray("imgs");
	const std::map<std::string,shared_ptr<const ZLImage> > &images = model->images();
	for (std::map<std::string,shared_ptr<const ZLImage> >::const_iterator it = images.begin(); it != images.end(); ++it) {
		if (!it->second.isNull()) {
			JSONUtil::serializeImage(it->first, (const ZLFileImage&)*it->second, imagesWriter->addMap());
		}
	}

	writeTOC(*model->contentsTree(), new JSONMapWriter(cacheDir + "/TOC"));

	return 0;
}

extern "C"
JNIEXPORT jstring JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readAnnotationNative(JNIEnv* env, jobject thiz, jobject file) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 0;
	}

	const std::string path = AndroidUtil::Method_ZLFile_getPath->callForCppString(file);
	return AndroidUtil::createJavaString(env, plugin->readAnnotation(ZLFile(path)));
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readCoverNative(JNIEnv* env, jobject thiz, jobject file, jobjectArray box) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	const std::string path = AndroidUtil::Method_ZLFile_getPath->callForCppString(file);

	shared_ptr<const ZLImage> image = plugin->coverImage(ZLFile(path));
	if (!image.isNull()) {
		jobject javaImage = AndroidUtil::createJavaImage(env, (const ZLFileImage&)*image);
		env->SetObjectArrayElement(box, 0, javaImage);
		env->DeleteLocalRef(javaImage);
	}
}
